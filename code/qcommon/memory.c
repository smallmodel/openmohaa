/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// memory.c: Memory manager

#include "q_shared.h"
#include "qcommon.h"

#ifndef DEDICATED
#  include "../client/client.h"
#endif

#define	ZONEID			0x7331
#define ZONEID_CONST	0xC057

void Z_CheckHeap(void);

typedef struct zonedebug_s {
	const char *label;
	const char *file;
	int line;
	int allocSize;
} zonedebug_t;

typedef struct memblock_s {
	size_t	size;			// including the header and possibly tiny fragments
	struct memblock_s		*next, *prev;
	int     id;				// should be ZONEID
#ifdef ZONE_DEBUG
	zonedebug_t d;
#endif
} memblock_t;

typedef struct memconstant_s {
	memblock_t b;
	unsigned char mem[ 2 ];
} memconstant_t;

memconstant_t emptystring = { sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST, 0, 0 };
memconstant_t numberstring[] =
{
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'0', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'1', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'2', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'3', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'4', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'5', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'6', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'7', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'8', 0} },
	{ {sizeof( memconstant_t ), NULL, NULL, ZONEID_CONST}, {'9', 0} },
};

static memblock_t mem_blocks[ TAG_NUM_TOTAL_TAGS ];

/*
========================
Z_EmptyStringPointer
========================
*/
const char *Z_EmptyStringPointer( void )
{
	return ( const char * )emptystring.mem;
}

/*
========================
Z_NumberStringPointer
========================
*/
const char *Z_NumberStringPointer( int iNum )
{
	return ( const char * )numberstring[ iNum - '0' ].mem;
}

/*
========================
Z_Free
========================
*/
void Z_Free( void *ptr )
{
	memblock_t *block = ( memblock_t * )( ( byte * )ptr - sizeof( memblock_t ) );

	// don't free constant memory
	if( block->id == ZONEID_CONST ) {
		return;
	}

	if( block->id != ZONEID ) {
		Com_Error( ERR_FATAL, "Z_Free: freed a pointer without ZONEID" );
	}

	// check the memory trash tester
#ifndef _DEBUG
	if( *( int * )( ( byte * )block + block->size - sizeof( int ) ) != ZONEID ) {
		Com_Error( ERR_FATAL, "Z_Free: memory block wrote past end" );
	}
#endif

	block->next->prev = block->prev;
	block->prev->next = block->next;
	block->prev = block;
	block->next = block;

	// free the block
	free( block );
}

/*
========================
Z_FreeTags
========================
*/
void Z_FreeTags( int tag )
{
	memblock_t *block;
	memblock_t *next;

	for( block = mem_blocks[ tag ].next; block != &mem_blocks[ tag ]; block = next )
	{
		next = block->next;
		Z_Free( ( ( byte * )block + sizeof( memblock_t ) ) );
	}

	mem_blocks[ tag ].prev = &mem_blocks[ tag ];
	mem_blocks[ tag ].next = &mem_blocks[ tag ];
}

/*
========================
Z_TagMalloc
========================
*/
#ifdef ZONE_DEBUG
void *Z_TagMallocDebug( int size, int tag, const char *label, const char *file, int line ) {
	int		allocSize;
#else
void *Z_TagMalloc( int size, int tag ) {
#endif
	memblock_t *block;

	if( size <= 0 )
	{
		//Z_Meminfo_f();
		Com_DPrintf( "Z_TagMalloc, Negative or zero size %d tag %i\n", size, tag );
		return NULL;
	}

	if( tag == TAG_FREE ) {
		Com_Error( ERR_FATAL, "Z_TagMalloc: tried to use a 0 tag" );
	}

#ifdef ZONE_DEBUG
	allocSize = size;
#endif

	size += sizeof( memblock_t );				// account for size of block header
#ifndef _DEBUG
	size += sizeof( int );						// space for memory trash tester
#endif
	size = PAD( size, sizeof( intptr_t ) );		// align to 32/64 bit boundary

	block = ( memblock_t * )malloc( size );
	block->id = ZONEID;
	block->size = size;
	block->next = &mem_blocks[ tag ];
	block->prev = mem_blocks[ tag ].prev;
	block->prev->next = block;
	mem_blocks[ tag ].prev = block;

#ifdef ZONE_DEBUG
	block->d.label = label;
	block->d.file = file;
	block->d.line = line;
	block->d.allocSize = allocSize;
#endif

#ifndef _DEBUG
	// marker for memory trash testing
	*( int * )( ( byte * )block + block->size - sizeof( int ) ) = ZONEID;
#endif

	return ( void * )( ( byte * )block + sizeof( memblock_t ) );
}

/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap( void )
{
#ifndef _DEBUG
	int k;
	memblock_t *block;

	for( k = 0; k < TAG_NUM_TOTAL_TAGS; k++ )
	{
		for( block = mem_blocks[ k ].next; block != &mem_blocks[ k ]; block = block->next )
		{
			if( *( int * )( ( byte * )block + block->size - 4 ) != ZONEID ) {
				Com_Error( ERR_FATAL, "Z_CheckHeap: memory block wrote past end" );
			}
		}
	}
#endif
}

/*
========================
Z_TouchMemory

Touch all known used data to make sure it is paged in
========================
*/
void Z_TouchMemory( void )
{
	size_t i, j;
	int k;
	int sum;
	int start, end;
	memblock_t *block;

	Z_CheckHeap();
	start = Sys_Milliseconds();

	sum = 0;

	for( k = 0; k < TAG_NUM_TOTAL_TAGS; k++ )
	{
		for( block = mem_blocks[ k ].next; block != &mem_blocks[ k ]; block = block->next )
		{
			j = block->size >> 2;
			for( i = 0; i < j; i += 64 ) {				// only need to touch each page
				sum += ( ( int * )block )[ i ];
			}
		}
	}

	end = Sys_Milliseconds();

	Com_Printf( "Z_TouchMemory: %i msec\n", end - start );
}

/*
========================
Z_Meminfo_f
========================
*/
void Z_Meminfo_f( void )
{
	int k;
	size_t totalBlocks;
	size_t totalBytes;
	size_t numBlocks;
	size_t numBytes;
	memblock_t *block;

	totalBlocks = 0;
	totalBytes = 0;

	Com_Printf( "-------------------------------------------\n" );

	for( k = TAG_CONST + 1; k < TAG_NUM_TOTAL_TAGS; k++ )
	{
		numBlocks = 0;
		numBytes = 0;

		for( block = mem_blocks[ k ].next; block != &mem_blocks[ k ]; block = block->next )
		{
			numBlocks++;
			numBytes += block->size;

			if( Cmd_Argc() != 1 ) {
				//Com_Printf( "block: %p size: %i\n", block, block->size );
			}
		}

		Com_Printf( "%zu bytes in %zu blocks in ", numBytes, numBlocks );

		switch( k )
		{
		case TAG_GENERAL:
			Com_Printf( "general memory pool.\n" );
			break;
		case TAG_BOTLIB:
			Com_Printf( "botlib memory pool.\n" );
			break;
		case TAG_RENDERER:
			Com_Printf( "renderer memory pool.\n" );
			break;
		case TAG_CGAME:
			Com_Printf( "cgame memory pool.\n" );
			break;
		case TAG_GAME:
			Com_Printf( "game memory pool.\n" );
			break;
		case TAG_CLIENT:
			Com_Printf( "client memory pool.\n" );
			break;
		case TAG_TIKI:
			Com_Printf( "tiki memory pool.\n" );
			break;
		case TAG_STRINGS_AND_COMMANDS:
			Com_Printf( "strings and commands memory pool.\n" );
			break;
		case TAG_SOUND:
			Com_Printf( "sound memory pool.\n" );
			break;
		case TAG_STATIC:
			Com_Printf( "static memory pool.\n" );
			break;
		case TAG_STATIC_RENDERER:
			Com_Printf( "static renderer memory pool.\n" );
			break;
		case TAG_SKEL:
			Com_Printf( "skeletor memory pool.\n" );
			break;
		case TAG_TEMP:
			Com_Printf( "temporary memory pool.\n" );
			break;
		case TAG_EXE:
			Com_Printf( "exe memory pool.\n" );
			break;
		default:
			Com_Error( ERR_FATAL, "Z_MemInfo: unknown memory pool\n" );
			break;
		}

		totalBlocks += numBlocks;
		totalBytes += numBytes;
	}

	Com_Printf( "\n%.2f Kbytes in %zu blocks in all memory pools\n", ( float )totalBytes / 1024.0f, totalBlocks );
	Com_Printf( "\n%.2f megabytes in 'new' system memory\n", 1.024f );

#ifndef DEDICATED
	if (re.CountTextureMemory) {
		Com_Printf( "\n%.2f megabytes in texture memory\n", ( float )re.CountTextureMemory() / 1024.0f );
		Com_Printf( "\n%.1f megabytes in total allocations\n", ( float )re.CountTextureMemory() + totalBytes - 1 / 1024.0f );
	}
#endif
}

/*
========================
Z_InitMemory
========================
*/
void Z_InitMemory( void ) {
	int k;

	memset( &mem_blocks, 0, sizeof( mem_blocks ) );

	for( k = 0; k < 16; k++ )
	{
		mem_blocks[ k ].prev = &mem_blocks[ k ];
		mem_blocks[ k ].next = &mem_blocks[ k ];
	}

	Cmd_AddCommand( "meminfo", Z_Meminfo_f );
}

/*
========================
Z_Shutdown
========================
*/
void Z_Shutdown( void ) {
	int k;

	Z_CheckHeap();

	for( k = 0; k < TAG_NUM_TOTAL_TAGS; k++ ) {
		Z_FreeTags( k );
	}
}

/*
=================
Hunk_Alloc

Allocate permanent (until the hunk is cleared) memory
=================
*/
#ifdef HUNK_DEBUG
void *Hunk_AllocDebug( int size, ha_pref preference, const char *label, const char *file, int line ) {
#else
void *Hunk_Alloc( int size, ha_pref preference ) {
#endif
	void *ptr;

	ptr = Z_TagMalloc( size, TAG_STATIC );
	memset( ptr, 0, size );

	return ptr;
}

/*
=================
Hunk_Clear

The server calls this before shutting down or loading a new map
=================
*/
void Hunk_Clear( void ) {
	Z_FreeTags( TAG_STATIC );
}

/*
=================
Hunk_AllocateTempMemory

This is used by the file loading system.
Multiple files can be loaded in temporary memory.
When the files-in-use count reaches zero, all temp memory will be deleted
=================
*/
void *Hunk_AllocateTempMemory(int size ) {
	return Z_TagMalloc( size, TAG_TEMP );
}

/*
========================
Hunk_FreeTempMemory
========================
*/
void Hunk_FreeTempMemory( void *ptr ) {
	Z_Free( ptr );
}

/*
=================
Hunk_ClearTempMemory

The temp space is no longer needed.  If we have left more
touched but unused memory on this side, have future
permanent allocs use this side.
=================
*/
void Hunk_ClearTempMemory( void ) {
	Z_FreeTags( TAG_TEMP );
}

/*
===================
Hunk_SetMark

The server calls this after the level and game VM have been loaded
===================
*/
void Hunk_SetMark( void ) {

}

/*
===============
Com_TouchMemory

Touch all known used data to make sure it is paged in
===============
*/
void Com_TouchMemory( void ) {
	Z_TouchMemory();
}

/*
=================
Com_InitHunkZoneMemory
=================
*/
void Com_InitHunkMemory( void ) {
}

/*
========================
Z_Malloc
========================
*/
#ifdef ZONE_DEBUG
void *Z_MallocDebug( int size, const char *label, const char *file, int line ) {
#else
void *Z_Malloc( int size ) {
#endif
	void	*buf;
	
  //Z_CheckHeap ();	// DEBUG

#ifdef ZONE_DEBUG
	buf = Z_TagMallocDebug( size, TAG_GENERAL, label, file, line );
#else
	buf = Z_TagMalloc( size, TAG_GENERAL );
#endif
	Com_Memset( buf, 0, size );

	return buf;
}
