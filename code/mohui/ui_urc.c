/*
===========================================================================
Copyright (C) 2010-2011 wombat

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
		else if ( !Q_strncmp(var,"INDENT_BORDER",13) )
			res->borderstyle = UI_BORDER_INDENT;
		else if ( !Q_strncmp(var,"3D_BORDER",9) )
			res->borderstyle = UI_BORDER_3D;
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
		res->stuffcommand = UI_Alloc( UI_MAX_NAME );
		Com_sprintf( res->stuffcommand, UI_MAX_NAME, "%s\n", PARSE_PTR );
		
	}
	else if ( !Q_strncmp( token, "hovershader", 11 ) ) {
		res->hoverDraw = qtrue;
		res->hoverShader = trap_R_RegisterShaderNoMip( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "clicksound", 10 ) ) {
		res->clicksound = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->clicksound, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "hovercommand", 12 ) ) {
		res->hovercommand = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->hovercommand, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "linkcvartoshader", 16 ) ) {
		res->linkcvartoshader = qtrue;
	}
	else if ( !Q_strncmp( token, "linkcvar", 8 ) ) {
		res->linkcvarname = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->linkcvarname, PARSE_PTR, UI_MAX_NAME );
		trap_Cvar_Register( &res->linkcvar, res->linkcvarname, "", 0 );
	}
	else if ( !Q_strncmp( token, "font", 4 ) ) {
//		trap_R_RegisterFont( PARSE_PTR, 0, &res->font );
		PARSE_PTR;
		res->font = &uis.menuFont;
	}
	else if ( !Q_strncmp( token, "title", 5 ) ) {
		res->title = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->title, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "textalign", 9 ) ) {
		var = PARSE_PTR;
		if ( !Q_strncmp(var,"left",4) )
			res->textalign = UI_ALIGN_LEFT;
		else if ( !Q_strncmp(var,"right",5) )
			res->textalign = UI_ALIGN_RIGHT;
	}
	else if ( !Q_strncmp( token, "checked_shader", 14 ) ) {
		res->checked_shader = trap_R_RegisterShaderNoMip( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "unchecked_shader", 16 ) ) {
		res->unchecked_shader = trap_R_RegisterShaderNoMip( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "menushader", 10 ) ) {
		PARSE_PTR;
		res->menushader = trap_R_RegisterShaderNoMip( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "selmenushader", 13 ) ) {
		PARSE_PTR;
		res->selmenushader = trap_R_RegisterShaderNoMip( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "linkstring", 10 ) ) {
		if ( res->selentries >= UI_MAX_SELECT ) {
			Com_Printf( "Too many select entries %i\n", res->selentries );
			PARSE_PTR;
			PARSE_PTR;
			return;
		}
		res->linkstring1[res->selentries] = UI_Alloc( UI_MAX_NAME );
		res->linkstring2[res->selentries] = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->linkstring1[res->selentries], PARSE_PTR, UI_MAX_NAME );
		Q_strncpyz( res->linkstring2[res->selentries], PARSE_PTR, UI_MAX_NAME );
		res->selentries++;
	}
	else if ( !Q_strncmp( token, "addpopup", 8 ) ) {
		PARSE_PTR;
		if ( res->selentries >= UI_MAX_SELECT ) {
			Com_Printf( "Too many select entries %i\n", res->selentries );
			PARSE_PTR;
			PARSE_PTR;
			PARSE_PTR;
			return;
		}
		res->linkstring1[res->selentries] = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->linkstring1[res->selentries], PARSE_PTR, UI_MAX_NAME );
		PARSE_PTR;
		res->linkstring2[res->selentries] = UI_Alloc( UI_MAX_NAME );
		Com_sprintf( res->linkstring2[res->selentries], UI_MAX_NAME, "%s\n", PARSE_PTR );
		res->selentries++;
	}
	else if ( !Q_strncmp( token, "ordernumber", 11 ) ) {
		res->ordernumber = atoi(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "slidertype", 10 ) ) {
		res->ordernumber = atoi(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "setrange", 8 ) ) {
		res->flRange[0] = atof(PARSE_PTR);
		res->flRange[1] = atof(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "stepsize", 8 ) ) {
		res->flStepsize = atof(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "rendermodel", 11 ) ) {
		res->rendermodel = (qboolean)atoi(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "modeloffset", 1 ) ) {
		res->modeloffset = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->modeloffset, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "modelrotateoffset", 17 ) ) {
		res->modelrotateoffset = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->modelrotateoffset, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "modelangles", 11 ) ) {
		res->modelangles = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->modelangles, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "modelscale", 10 ) ) {
		res->modelscale = atof(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "modelanim", 9 ) ) {
		res->modelanim = UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->modelanim, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "resource", 8 ) ) {
		Com_Printf( "UI_ParseMenuResource: new resource not expected: forgot }?\n" );
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
	else if ( !Q_strncmp( token, "include", 7 ) ) {
		Q_strncpyz( menu->include, PARSE_PTR, UI_MAX_NAME );
		UI_LoadINC( menu->include, menu );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "align", 5 ) ) {
		PARSE_PTR;
		PARSE_PTR;
		return qfalse;
	}
	else if ( !Q_strncmp( token, "vidmode", 7 ) ) {
		menu->vidmode = atoi( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "fadein", 7 ) ) {
		menu->fadein = atof( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "fgcolor", 7 ) ) {
		for ( i=0;i<4;i++ )
			menu->fgcolor[i] = atof( PARSE_PTR );
		return qfalse;
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
		res->font = &uis.menuFont;
//		trap_R_RegisterFont( "facfont-20", 0, &res->font ); //load standard font for resource

		var = PARSE_PTR;
		if ( !Q_strncmp( var, "Label", 5 ) )
			res->type = UI_RES_LABEL;
		else if ( !Q_strncmp( var, "Button", 6 ) )
			res->type = UI_RES_BUTTON;
		else if ( !Q_strncmp( var, "Field", 5 ) )
			res->type = UI_RES_FIELD;
		else if ( !Q_strncmp( var, "CheckBox", 8 ) )
			res->type = UI_RES_CHECKBOX;
		else if ( !Q_strncmp( var, "PulldownMenuContainer", 21 ) )
			res->type = UI_RES_PULLDOWN;
		else if ( !Q_strncmp( var, "Slider", 6 ) )
			res->type = UI_RES_SLIDER;
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
			// "startnew" and "changemap" buttons are equal. we skip changemap
			// sigh sigh sigh
			if (!Q_strncmp(res->name,"changemap",UI_MAX_NAME)) {
				Com_Memset( res, 0, sizeof(uiResource_t) );
				menu->resPtr--;
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

void	UI_LoadINC( const char *name, uiMenu_t *menu ) {
	fileHandle_t	f;
	int				len;
	char			filename[MAX_QPATH];
	char			buffer[UI_MAX_URCSIZE];
	char			*token;
	char			*ptr;
	uiResource_t	*res;
	char			*var;

	Q_strncpyz( filename, name, sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".inc" );

	len = trap_FS_FOpenFile( filename, &f, FS_READ);
	if (!f) {
		Com_Printf( "couldn't load INC file %s\n", name );
		return;
	}
	if ( len > UI_MAX_URCSIZE ) {
		Com_Printf( "INC file too large, %i KB. Max size is %i KB\n", len/1024, UI_MAX_URCSIZE/1024 );
		return;
	}

	trap_FS_Read( buffer, len, f );
	buffer[len] = 0;

	ptr = buffer;
	token = COM_Parse( &ptr );

	while (*token) {
		if ( !Q_strncmp( token, "resource", 8 ) ) {
			menu->resPtr++;
			if ( menu->resPtr >= UI_MAX_RESOURCES ) {
				Com_Printf( "UI_ParseMenuToken: more than %i resources!\n", UI_MAX_RESOURCES );
				menu->resPtr--;
				return;
			}
			res = &menu->resources[menu->resPtr];
			res->font = &uis.menuFont;
//			trap_R_RegisterFont( "facfont-20", 0, &res->font ); //load standard font for resource

			var = COM_Parse( &ptr );
			if ( !Q_strncmp( var, "Label", 5 ) )
				res->type = UI_RES_LABEL;
			else if ( !Q_strncmp( var, "Button", 6 ) )
				res->type = UI_RES_BUTTON;
			else if ( !Q_strncmp( var, "Field", 5 ) )
				res->type = UI_RES_FIELD;
			else if ( !Q_strncmp( var, "CheckBox", 8 ) )
				res->type = UI_RES_CHECKBOX;
			else if ( !Q_strncmp( var, "PulldownMenuContainer", 21 ) )
				res->type = UI_RES_PULLDOWN;
			else if ( !Q_strncmp( var, "Slider", 6 ) )
				res->type = UI_RES_SLIDER;
			else Com_Printf( "UI_ParseMenuToken: unknown menu resource type %s\n", var );

			var = COM_Parse( &ptr ) ;
			if ( *var == '{' ) {
				var = COM_Parse( &ptr );
				while (*var != '}') {
					UI_ParseMenuResource( var, &ptr, res );
					var = COM_Parse( &ptr );
					if (*var ==0) {
						Com_Memset( res, 0, sizeof(uiResource_t) );
						menu->resPtr--;
						return;
					}
				}
			}
		}

		token = COM_Parse( &ptr );
	}

	trap_FS_FCloseFile( f );
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
		Com_Printf( "couldn't load URC menu %s. file not found.\n", name );
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
	uiMenu_t	*menu;

	uis.MSP++;
	if ( uis.MSP >= UI_MAX_MENUS ) {
		Com_Printf( "UI_PushMenu: max number of menus (%i) exceeded.\n", UI_MAX_MENUS );
		uis.MSP--;
		return;
	}

	menu = &uis.menuStack[uis.MSP];
	Com_Memset( menu, 0, sizeof(uiMenu_t) );

	// remap mpoptions menus where menu name and file name disagree
	if ( !Q_strncmp( name, "mpoptions", 9 ) )
		UI_LoadURC( "multiplayeroptions", menu );
	else if ( !Q_strncmp( name, "video_options", 9 ) )
		UI_LoadURC( "video options", menu );
	else
		UI_LoadURC( name, menu );

}

void	UI_PopMenu( void ) {

	if ( uis.MSP > 0 ) {
		
		uis.MSP--;
		return;
	}
}
