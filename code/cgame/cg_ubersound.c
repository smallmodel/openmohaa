/*
===========================================================================
Copyright (C) 2010-2011 Michael Rieder

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "cg_local.h"

#define	MAX_UBERSOUNDS	2400
ubersound_t		snd_indexes[MAX_UBERSOUNDS];
int				snd_numIndexes;

#define FILE_HASH_SIZE		256
static	ubersound_t*		hashTable[FILE_HASH_SIZE];

static long generateHashValue( const char *fname ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (FILE_HASH_SIZE-1);
	return hash;
}

static ubersound_t*	US_Find( const char *snd_name ) {
	ubersound_t*	snd;
	long			hash;

	hash = generateHashValue(snd_name);
	
	for (snd=hashTable[hash] ; snd ; snd=snd->hashNext) {
		if (!Q_stricmp(snd_name, snd->name)) {
			return snd;
		}
	}

	return NULL;
}

ubersound_t*	CG_GetUbersound( const char *name ) {
	ubersound_t *snd;
	snd = US_Find( name );

	if ( !snd ) {
		CG_Printf( "CG_GetUbersound: sound %s not found.\n", name );
		return NULL;
	}
	else if ( snd->hasLoaded == qtrue )
		return snd;
	else {
		snd->sfxHandle = trap_S_RegisterSound( snd->wavfile, qfalse );
	}
}

void CG_ParseUSline( char **ptr, ubersound_t *snd ) {
	char *chan;
	// name
	Q_strncpyz( snd->name, COM_Parse(ptr), MAX_QPATH );
	Q_strncpyz( snd->wavfile, COM_Parse(ptr), MAX_QPATH );
	// soundparms
	if ( !Q_strncmp( COM_Parse(ptr), "soundparms", MAX_QPATH ) ) {
		snd->basevol	= atof(COM_Parse(ptr));
		snd->volmod		= atof(COM_Parse(ptr));
		snd->basepitch	= atof(COM_Parse(ptr));
		snd->pitchmod	= atof(COM_Parse(ptr));
		snd->minDist	= atof(COM_Parse(ptr));
		snd->maxDist	= atof(COM_Parse(ptr));

		// channel
		chan = COM_Parse(ptr);
		if ( !Q_strncmp( chan, "auto", MAX_QPATH ) )
			snd->channel = CHAN_AUTO;
		else if ( !Q_strncmp( chan, "body", MAX_QPATH ) )
			snd->channel = CHAN_BODY;
		else if ( !Q_strncmp( chan, "item", MAX_QPATH ) )
			snd->channel = CHAN_ITEM;
		else if ( !Q_strncmp( chan, "weaponidle", MAX_QPATH ) )
			snd->channel = CHAN_WEAPONIDLE;
		else if ( !Q_strncmp( chan, "voice", MAX_QPATH ) )
			snd->channel = CHAN_VOICE;
		else if ( !Q_strncmp( chan, "local", MAX_QPATH ) )
			snd->channel = CHAN_LOCAL;
		else if ( !Q_strncmp( chan, "weapon", MAX_QPATH ) )
			snd->channel = CHAN_WEAPON;
		else if ( !Q_strncmp( chan, "dialog_secondary", MAX_QPATH ) )
			snd->channel = CHAN_DIALOG_SECONDARY;
		else if ( !Q_strncmp( chan, "dialog", MAX_QPATH ) )
			snd->channel = CHAN_DIALOG;
		else if ( !Q_strncmp( chan, "menu", MAX_QPATH ) )
			snd->channel = CHAN_MENU;
		else
			CG_Printf( "Ubersound unrecognized channel %s for %s\n", chan, snd->name );
		// loaded /Streamed
		chan = COM_Parse(ptr);
		if ( !Q_strncmp( chan, "loaded", MAX_QPATH ) )
			snd->loaded = qtrue;
		else if ( !Q_strncmp( chan, "streamed", MAX_QPATH ) )
			snd->loaded = qfalse;
		else
			CG_Printf( "Ubersound unrecognized loaded state %s for %s\n", chan, snd->name );
	}
	// subtitle - su44: we need this for ubersound/uberdialog.scr loading
	if ( !Q_strncmp( COM_Parse(ptr), "subtitle", MAX_QPATH ) ) 
		Q_strncpyz( snd->subtitle, COM_Parse(ptr), sizeof(snd->subtitle) );
	// maplist
	if ( !Q_strncmp( COM_Parse(ptr), "maps", MAX_QPATH ) ) 
		Q_strncpyz( snd->mapstring, COM_Parse(ptr), MAPSTRING_LENGTH );
}

qboolean US_CheckMapstring( const char *mapstring ) {
	char *ptr, *token;

	ptr = (char*)mapstring;
	token = COM_Parse(&ptr);
	while (*token) {
		if ( strstr( cgs.mapname, token ) )
			return qtrue;
		token = COM_Parse(&ptr);
	}

	return qfalse;
}

void CG_LoadUberSoundFile( const char *fname ) {
	fileHandle_t	f;
	int				len;

	char			buffer[UBERSOUND_SIZE];
	char			*token;
	char			*ptr;
	qboolean		end;
	ubersound_t*	snd;
	int				i;

	CG_Printf( "=== Loading %s ===\n", fname );

	len = trap_FS_FOpenFile( fname, &f, FS_READ);
	if (!f) {
		CG_Printf( "couldn't load %s. file not found.\n", fname );
		return;
	}
	if ( len >= UBERSOUND_SIZE ) {
		Com_Printf( ".scr file too large, %i KB. Max size is %i KB\n", len/1024, UBERSOUND_SIZE/1024 );
		return;
	}

	trap_FS_Read( buffer, len, f );
	buffer[len] = 0;
	end = qfalse;
	ptr = buffer;
	token = COM_Parse( &ptr );

	while (*token) {
		if ( !Q_strncmp(token,"aliascache",MAX_QPATH) || !Q_strncmp(token,"alias",MAX_QPATH) ) {
			CG_ParseUSline( &ptr, &snd_indexes[snd_numIndexes] );

			if ( US_CheckMapstring( snd_indexes[snd_numIndexes].mapstring ) == qtrue ) {
				if ( snd_indexes[snd_numIndexes].name[strnlen(snd_indexes[snd_numIndexes].name,MAX_QPATH)-1] == '1' )
					snd_indexes[snd_numIndexes].name[strnlen(snd_indexes[snd_numIndexes].name,MAX_QPATH)-1] = 0;
				i = generateHashValue(snd_indexes[snd_numIndexes].name);
				snd = hashTable[i];
				if (snd) {
					while (snd->hashNext)
						snd = snd->hashNext;
					snd->hashNext = &snd_indexes[snd_numIndexes];
				}
				else
					hashTable[i] = &snd_indexes[snd_numIndexes];
				if ( snd_indexes[snd_numIndexes].loaded == qtrue ) {
					 snd_indexes[snd_numIndexes].sfxHandle = trap_S_RegisterSound( snd_indexes[snd_numIndexes].wavfile, qfalse );
					 snd_indexes[snd_numIndexes].hasLoaded = qtrue;
				}
				snd_numIndexes++;
				if ( snd_numIndexes >= MAX_UBERSOUNDS ) {
					snd_numIndexes--;
					CG_Error( "CG_LoadUbersound: too many aliascaches in file.\n" );
				}
			}
		}
		else if ( !Q_strncmp( token, "end", MAX_QPATH ) ) {
			end = qtrue;
			break;
		}
		token = COM_Parse( &ptr );
	}
	if ( end == qfalse )
		Com_Printf( "CG_LoadUbersound hit end of file without end statement\n" );

	trap_FS_FCloseFile( f );
	CG_Printf( "=== Finished %s ===\n", fname );
}

void CG_LoadUbersound( void ) {
	Com_Memset(snd_indexes, 0, sizeof(snd_indexes));
	Com_Memset(hashTable, 0, sizeof(hashTable));
	snd_numIndexes = 0;

	CG_LoadUberSoundFile(UBERSOUND_FILE);
	// su44: used by MoHAA voicechats
	CG_LoadUberSoundFile("ubersound/uberdialog.scr");
}

char buffer[MUSIC_SIZE];
const char* CG_LoadMusic( const char *musicfile ) {

	fileHandle_t	f;
	int				len;

	char			*token;
	char			*ptr;
	int				i;
	char			path[MAX_QPATH];
	char			name[MAX_QPATH];

	CG_Printf( "=== Loading %s ===\n", musicfile );

	len = trap_FS_FOpenFile( musicfile, &f, FS_READ);
	if (!f) {
		CG_Printf( "couldn't load %s. file not found.\n", musicfile );
		return;
	}
	if ( len >= MUSIC_SIZE ) {
		Com_Printf( ".scr file too large, %i KB. Max size is %i KB\n", len/1024, MUSIC_SIZE/1024 );
		return;
	}

	trap_FS_Read( buffer, len, f );
	buffer[len] = 0;
	ptr = buffer;
	token = COM_Parse( &ptr );
	while (*token) {
		if ( !Q_strncmp(token,"path",MAX_QPATH) )
			Q_strncpyz( path, COM_Parse(&ptr), MAX_QPATH );
		else if ( !Q_strncmp(token,"normal",MAX_QPATH) )
			Q_strncpyz( name, COM_Parse(&ptr), MAX_QPATH );
		token = COM_Parse( &ptr );
	}
	Q_snprintf( buffer, MAX_QPATH, "%s/%s", path, name );
	return buffer;
}