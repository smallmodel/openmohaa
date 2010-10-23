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

#include "ui_local.h"

#define PARSE_PTR	COM_Parse(ptr)

void UI_ParseMenuResource( const char *token, char **ptr, uiResource_t *res ) {
	char	*var;
	int		i;

	if ( !Q_strncmp( token, "name", 4 ) ) {
		Q_strncpyz( res->name, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "rect", 4 ) ) {
		for (i=0;i<4;i++)
			res->rect[i] = atoi( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "fgcolor", 7 ) ) {
		for (i=0;i<4;i++)
			res->fgcolor[i] = atof( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "bgcolor", 7 ) ) {
		for (i=0;i<4;i++)
			res->bgcolor[i] = atof( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "borderstyle", 11 ) ) {
		var = PARSE_PTR;
		if ( !Q_strncmp(var,"RAISED",6) )
			res->borderstyle = UI_BORDER_RAISED;
		else res->borderstyle = UI_BORDER_NONE;
	}
	else if ( !Q_strncmp( token, "shader", 6 ) ) {
		res->shader = trap_R_RegisterShaderNoMip( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "enabledcvar", 11 ) ) {
		res->enablewithcvar = qtrue;
		trap_Cvar_Register( &res->enabledcvar, PARSE_PTR, "", 0 );
	}
	else if ( !Q_strncmp( token, "stuffcommand", 12 ) ) {
		Q_strncpyz( res->stuffcommand, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "hovershader", 11 ) ) {
		res->hoverDraw = qtrue;
		res->hoverShader = trap_R_RegisterShaderNoMip( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "clicksound", 10 ) ) {
		Q_strncpyz( res->clicksound, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "hovercommand", 12 ) ) {
		Q_strncpyz( res->hovercommand, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "linkcvartoshader", 16 ) ) {
		res->linkcvartoshader = qtrue;
	}
	else if ( !Q_strncmp( token, "linkcvar", 8 ) ) {
		trap_Cvar_Register( &res->linkcvar, PARSE_PTR, "", 0 );
	}
	else if ( !Q_strncmp( token, "resource", 8 ) ) {
		Com_Printf( "UI_ParseMenuResource: new resource not expected: forgot }?\n", *ptr );
	}
	else {
		Com_Printf( "UI_ParseMenuResource: unknown token %s\n", token );
	}
}

qboolean UI_ParseMenuToken( const char *token, char **ptr, uiMenu_t *menu ) {
	char			*var;
	int				i;
	uiResource_t	*res;

	if ( !Q_strncmp( token, "menu", 4 ) ) {
		Q_strncpyz( menu->name, PARSE_PTR, sizeof(menu->name));
		menu->size[0] = atoi( PARSE_PTR );
		menu->size[1] = atoi( PARSE_PTR );
		var = COM_Parse( ptr );
		if ( !Q_strncmp( var, "FROM_TOP", 8 ))
			menu->motion = UI_FROM_TOP;
		else if ( !Q_strncmp( var, "FROM_TOP", 8 ))
			menu->motion = UI_FROM_BOTTOM;
		else if ( !Q_strncmp( var, "FROM_BOTTOM", 8 ))
			menu->motion = UI_FROM_LEFT;
		else if ( !Q_strncmp( var, "FROM_LEFT", 8 ))
			menu->motion = UI_FROM_RIGHT;
		else if ( !Q_strncmp( var, "NONE", 8 ))
			menu->motion = UI_NONE;
		menu->time = atoi( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "end.", 4 ) ) {
		return qtrue;
	}
	else if ( !Q_strncmp( token, "bgcolor", 7 ) ) {
		for ( i=0;i<4;i++ )
			menu->bgcolor[i] = atof( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "borderstyle", 11 ) ) {
		var = PARSE_PTR;
		if ( !Q_strncmp(var,"RAISED",6) )
			menu->borderstyle = UI_BORDER_RAISED;
		else menu->borderstyle = UI_BORDER_NONE;
		return qfalse;
	}
	else if ( !Q_strncmp( token, "bgfill", 6 ) ) {
		for ( i=0;i<4;i++ )
			menu->bgfill[i] = atof( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "fullscreen", 10 ) ) {
		menu->fullscreen = (qboolean)atoi( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "virtualres", 10 ) ) {
		menu->virtualres = (qboolean)atoi( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "resource", 8 ) ) {
		menu->resPtr++;
		if ( menu->resPtr >= UI_MAX_RESOURCES ) {
			Com_Printf( "UI_ParseMenuToken: more than %i resources!\n", UI_MAX_RESOURCES );
			menu->resPtr--;
			return qfalse;
		}
		res = &menu->resources[menu->resPtr];

		var = PARSE_PTR;
		if ( !Q_strncmp( var, "Label", 5 ) )
			res->type = UI_RES_LABEL;
		else if ( !Q_strncmp( var, "Button", 5 ) )
			res->type = UI_RES_BUTTON;
		else Com_Printf( "UI_ParseMenuToken: unknown menu resource type %s\n", var );

		var = PARSE_PTR ;
		if ( *var == '{' ) {
			var = PARSE_PTR;
			while (*var != '}') {
				UI_ParseMenuResource( var, ptr, res );
				var = PARSE_PTR;
				if (*var ==0) {
					Com_Memset( res, 0, sizeof(uiResource_t) );
					menu->resPtr--;
					return qfalse;
				}
			}
			return qfalse;
		}
		Com_Printf( "UI_ParseMenuToken: error loading resource\n" );
		return qfalse;
	}
	else {
		Com_Printf( "UI_ParseMenuToken: unknown token %s\n", token );
		return qfalse;
	}
}

void	UI_LoadURC( const char *name, uiMenu_t *menu ) {
	fileHandle_t	f;
	int				len;
	char			filename[MAX_QPATH];
	char			buffer[UI_MAX_URCSIZE];
	char			*token;
	char			*ptr;
	qboolean		end;

	Q_strncpyz( filename, "ui/", sizeof(filename) );
	Q_strncpyz( filename + 3, name, sizeof( filename )-3 );
	COM_DefaultExtension( filename, sizeof( filename ), ".urc" );

	len = trap_FS_FOpenFile( filename, &f, FS_READ);
	if (!f) {
		Com_Printf( "couldn't load URC menu %s\n", name );
		return;
	}
	if ( len > UI_MAX_URCSIZE ) {
		Com_Printf( "URC file too large, %i KB. Max size is %i KB\n", len/1024, UI_MAX_URCSIZE/1024 );
		return;
	}

	trap_FS_Read( buffer, len, f );
	buffer[len] = 0;

	ptr = buffer;
	token = COM_Parse( &ptr );

	menu->resPtr = -1;
	while (*token) {
		end = UI_ParseMenuToken( token, &ptr, menu );
		token = COM_Parse( &ptr );
	}
	if (!end)
		Com_Printf( "UI_LoadURC hit end of file without end statement\n" );

	trap_FS_FCloseFile( f );
}

void	UI_PushMenu( const char *name ) {

	uis.msp++;
	if ( uis.msp >= UI_MAX_MENUS ) {
		Com_Printf( "UI_PushMenu: cannot load more than %i menus\n", UI_MAX_MENUS );
		return;
	}
	Com_Memset( &uis.menuStack[uis.msp], 0, sizeof(uiMenu_t) );
	UI_LoadURC( name, &uis.menuStack[uis.msp] );
}

void	UI_PopMenu( void ) {
	uis.msp--;
}
