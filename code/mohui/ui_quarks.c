/*
===========================================================================
Copyright (C) 1999-2005 wombat, Inc.

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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// ui_quarks.c -- basic functionality

#include "ui_local.h"

uiStatic_t		uis;

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[2048];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	trap_Error( va("%s", text) );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[2048];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Print( va("%s", text) );
}

/*
=================
UI_ClampCvar
=================
*/
float UI_ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*
================
cvars
================
*/

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

vmCvar_t	ui_mohui;

static cvarTable_t		cvarTable[] = {
	{ &ui_mohui, "ui_mohui", "1", CVAR_INIT }
};

static int cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);


/*
=================
UI_RegisterCvars
=================
*/
void UI_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
	}
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Update( cv->vmCvar );
	}
}

/*
=================
UI_Shutdown
=================
*/
void UI_Shutdown( void ) {
}

/*
=================
UI_Init
=================
*/
void UI_Init( void ) {
	UI_RegisterCvars();

//	UI_InitGameinfo();

	// cache redundant calulations
	trap_GetGlconfig( &uis.glconfig );

	// for 640x480 virtualized screen
	uis.xscale = uis.glconfig.vidWidth * (1.0/640.0);
	uis.yscale = uis.glconfig.vidHeight * (1.0/480.0);
	if ( uis.glconfig.vidWidth * 480 > uis.glconfig.vidHeight * 640 ) {
		// wide screen
		uis.bias = 0.5 * ( uis.glconfig.vidWidth - ( uis.glconfig.vidHeight * (640.0/480.0) ) );
		uis.xscale = uis.yscale;
	}
	else {
		// no wide screen
		uis.bias = 0;
	}

	// initialize the menu system


	trap_R_RegisterFont( "facfont-20", 0, &uis.menuFont );
}

/*
=================
UI_KeyEvent
=================
*/
void UI_KeyEvent( int key, int down ) {

}

/*
=================
UI_MouseEvent
=================
*/
void UI_MouseEvent( int dx, int dy )
{

}

/*
=================
UI_Refresh
=================
*/
void UI_Refresh( int realtime )
{

}

qboolean UI_IsFullscreen( void ) {
//	if ( uis.activemenu && ( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
//		return uis.activemenu->fullscreen;
//	}

	return qfalse;
}

void UI_SetActiveMenu( uiMenuCommand_t menu ) {

}

char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}

/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	char	*cmd;

	cmd = UI_Argv( 0 );



	if ( Q_stricmp (cmd, "ui_mohtest") == 0 ) {
		Com_Printf( "openmohaa menu up and running!\n" );
		return qtrue;
	}

	return qfalse;
}

/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawConnectScreen( qboolean overlay ) {

}
