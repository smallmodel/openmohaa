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

// ui_quarks.c -- basic functionality

#include "ui_local.h"

uiStatic_t		uis;

vec4_t menu_text_color	    = {1.0f, 1.0f, 1.0f, 1.0f};
vec4_t menu_dim_color       = {0.0f, 0.0f, 0.0f, 0.75f};
vec4_t color_black	    = {0.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_white	    = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_yellow	    = {1.00f, 1.00f, 0.00f, 1.00f};
vec4_t color_blue	    = {0.00f, 0.00f, 1.00f, 1.00f};
vec4_t color_lightOrange    = {1.00f, 0.68f, 0.00f, 1.00f };
vec4_t color_orange	    = {1.00f, 0.43f, 0.00f, 1.00f};
vec4_t color_red	    = {1.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_dim	    = {0.00f, 0.00f, 0.00f, 0.25f};
vec4_t color_gray	    = {0.00f, 0.00f, 0.00f, 0.6f};
vec4_t color_green	    = {0.10f, 0.70f, 0.10f, 1.00f};
vec4_t color_dkgreen	= {0.40f, 0.40f, 0.40f, 1.00f};

// current color scheme
vec4_t pulse_color          = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t text_color_disabled  = {0.50f, 0.50f, 0.50f, 1.00f};	// light gray
//vec4_t text_color_normal    = {1.00f, 0.43f, 0.00f, 1.00f};	// light orange
vec4_t text_color_normal    = {1.00f, 1.00f, 1.00f, 1.00f};		//white
vec4_t text_color_highlight = {1.00f, 1.00f, 0.00f, 1.00f};	// bright yellow
vec4_t listbar_color        = {1.00f, 0.43f, 0.00f, 0.30f};	// transluscent orange
vec4_t text_color_status    = {1.00f, 1.00f, 1.00f, 1.00f};	// bright white

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	trap_Error( va("%s", text) );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

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
vmCvar_t	ui_voodoo;
vmCvar_t	ui_signshader;
vmCvar_t	ui_dmmap;
vmCvar_t	ui_inactivespectate;
vmCvar_t	ui_inactivekick;
vmCvar_t	ui_hostname;
vmCvar_t	ui_maplist_team;
vmCvar_t	ui_dedicated;
vmCvar_t	ui_gamespy;
vmCvar_t	ui_maxclients;
vmCvar_t	ui_timelimit;
vmCvar_t	ui_fraglimit;
vmCvar_t	ui_teamdamage;
vmCvar_t	cg_drawviewmodel;
vmCvar_t	ter_error;
vmCvar_t	r_lodscale;
vmCvar_t	ter_maxlod;
vmCvar_t	cg_effectdetail;
vmCvar_t	r_subdivisions;
vmCvar_t	cg_shadows;


static cvarTable_t		cvarTable[] = {
	{ &ui_mohui, "ui_mohui", "1", CVAR_INIT },
	{ &ui_voodoo, "ui_voodoo", "0", CVAR_ARCHIVE },
	{ &ui_signshader, "ui_signshader", "", CVAR_TEMP },
	{ &ui_dmmap, "ui_dmmap", "dm/mohdm1", CVAR_ARCHIVE },
	{ &ui_inactivespectate, "ui_inactivespectate", "60", CVAR_ARCHIVE },
	{ &ui_inactivekick, "ui_inactivekick", "900", CVAR_ARCHIVE },
	{ &ui_hostname, "ui_hostname", "openmohaa battle", CVAR_ARCHIVE },
	{ &ui_maplist_team, "ui_maplist_team", "dm/mohdm1 dm/mohdm2 dm/mohdm3 dm/mohdm4 dm/mohdm5 dm/mohdm6 dm/mohdm7", CVAR_ARCHIVE },
	{ &ui_dedicated, "ui_dedicated", "0", CVAR_ARCHIVE },
	{ &ui_gamespy, "ui_gamespy", "1", CVAR_ARCHIVE },
	{ &ui_maxclients, "ui_maxclients", "20", CVAR_ARCHIVE },
	{ &ui_timelimit, "ui_timelimit", "0", CVAR_ARCHIVE },
	{ &ui_fraglimit, "ui_fraglimit", "0", CVAR_ARCHIVE },
	{ &ui_teamdamage, "ui_teamdamage", "1", CVAR_ARCHIVE },
	{ &cg_drawviewmodel, "cg_drawviewmodel", "2", CVAR_ARCHIVE },
	{ &ter_error, "ter_error", "4", CVAR_ARCHIVE },
	{ &r_lodscale, "r_lodscale", "1.1", CVAR_ARCHIVE },
	{ &ter_maxlod, "ter_maxlod", "6", CVAR_ARCHIVE },
	{ &cg_effectdetail, "cg_effectdetail", "1.0", CVAR_ARCHIVE },
	{ &r_subdivisions, "r_subdivisions", "3", CVAR_ARCHIVE },
	{ &cg_shadows, "cg_shadows", "2", CVAR_ARCHIVE }
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


void UI_RegisterMedia( void ) {
	trap_R_RegisterFont( "facfont-20", 0, &uis.menuFont );

	uis.whiteShader = trap_R_RegisterShaderNoMip( "*white" );
	uis.blackShader = trap_R_RegisterShaderNoMip( "textures/mohmenu/black.tga" );
	uis.cursor = trap_R_RegisterShaderNoMip( "gfx/2d/mouse_cursor.tga" );

	UI_LoadURC( "connecting", &uis.connecting );
}

/*
=================
UI_Init
=================
*/
void UI_Init( void ) {

	UI_InitMemory();
	
	UI_RegisterCvars();
Com_Printf( "hello mama\n" );
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
	UI_RegisterMedia();

	uis.MSP = -1;
	UI_PushMenu( "main" );
}

/*
=================
UI_KeyEvent
=================
*/
void UI_KeyEvent( int key, int down ) {
	int				i;
	uiMenu_t		*menu;
	uiResource_t	*res;
	char			buffer[64];

	if (!uis.activemenu) {
		return;
	}
//	Com_Printf( "key %i, down %i\n", key, down );

	if (!down) {
		menu = &uis.menuStack[uis.MSP];

		for (i=0;i<=menu->resPtr;i++) {
			res = &menu->resources[i];
			if (res->pressed == qtrue) {
				res->pressed = qfalse;
				if ( uis.cursorx >= res->rect[0]+res->rect[2] && uis.cursorx <= res->rect[0]+res->rect[2]+128 )
					if ( uis.cursory >= res->rect[1] && uis.cursory <= res->rect[1]+ 16*res->selentries && res->linkstring2[(uis.cursory-res->rect[1])/16] )
						UI_CmdExecute( res->linkstring2[(uis.cursory-res->rect[1])/16] );
			}
		}
		return;
	}

	// menu system keys
	switch ( key )
	{
		case K_MOUSE2:
		case K_ESCAPE:
			UI_PopMenu();
			break;
		case K_MOUSE1:
		case K_ENTER:
			menu = &uis.menuStack[uis.MSP];
			
			for (i=0;i<=menu->resPtr;i++) {
				res = &menu->resources[i];
				if (res->active ==qfalse)
					continue;
				switch (res->type) {
					case UI_RES_BUTTON:
						if ( res->stuffcommand )
							UI_CmdExecute( res->stuffcommand );
						break;
					case UI_RES_CHECKBOX:
						trap_Cvar_Update( &res->linkcvar );
						if (res->linkcvar.integer)
							trap_Cvar_SetValue( res->linkcvarname, 0 );
						else
							trap_Cvar_SetValue( res->linkcvarname, 1 );
						break;
					case UI_RES_PULLDOWN:
						res->pressed = qtrue;
				}
			}
			break;
		case K_BACKSPACE:
			menu = &uis.menuStack[uis.MSP];
			
			for (i=0;i<=menu->resPtr;i++) {
				res = &menu->resources[i];
				if (res->active ==qfalse)
					continue;
				if (res->type == UI_RES_FIELD) {
					trap_Cvar_VariableStringBuffer(res->linkcvarname, buffer,64);
					buffer[strnlen(buffer,64)-1] = 0;
					trap_Cvar_Set( res->linkcvarname, buffer );
				}
			}
			break;
		default:
			if ( key&K_CHAR_FLAG )
				break;
			menu = &uis.menuStack[uis.MSP];

			for (i=0;i<=menu->resPtr;i++) {
				res = &menu->resources[i];
				if (res->active ==qfalse)
					continue;
				if (res->type == UI_RES_FIELD) {
					trap_Cvar_VariableStringBuffer(res->linkcvarname, buffer,64);
					if ( (key >= 'a'&&key <= 'z') || (key >= '0'&&key <= '9')) {
						buffer[strnlen(buffer,64)] = key;
						buffer[strnlen(buffer,64)+1] = 0;
					}
					trap_Cvar_Set( res->linkcvarname, buffer );
				}
			}
			break;
	}
}

/*
=================
UI_MouseEvent
=================
*/
void UI_MouseEvent( int dx, int dy )
{
	int				i;
	uiMenu_t		*menu;
	uiResource_t	*res;

	if (!uis.activemenu)
		return;

	// update mouse screen position
	uis.cursorx += dx;
	if (uis.cursorx < 0)
		uis.cursorx = 0;
	else if (uis.cursorx > SCREEN_WIDTH)
		uis.cursorx = SCREEN_WIDTH;

	uis.cursory += dy;
	if (uis.cursory < 0)
		uis.cursory = 0;
	else if (uis.cursory > SCREEN_HEIGHT)
		uis.cursory = SCREEN_HEIGHT;

	// region test the active menu items
	menu = &uis.menuStack[uis.MSP];

	for (i=0; i<=menu->resPtr;i++) {
		res = &menu->resources[i];
		if (res->type != UI_RES_BUTTON
			&& res->type != UI_RES_CHECKBOX
			&& res->type != UI_RES_FIELD
			&& res->type != UI_RES_PULLDOWN)
			continue;
		if (	uis.cursorx >= res->rect[0]
				&& uis.cursorx <= res->rect[2]+res->rect[0]
				&& uis.cursory >= res->rect[1]
				&& uis.cursory <= res->rect[3]+res->rect[1] ) {

					res->active = qtrue;
					if ( res->lastState == qfalse && res->hovercommand )
						UI_CmdExecute( res->hovercommand );
					res->lastState = qtrue;
				}
		else {
			res->active = qfalse;
			res->lastState = qfalse;
		}
	}
}

/*
========================
UI_DrawMenu

Logic to bring the menu resources to screen
========================
*/
void UI_DrawMenu( uiMenu_t *menu, qboolean foreground ) {
	int				j;
	int				k;
	;
	uiResource_t	*res;
	qhandle_t		cvarshader;

	for (j=0; j<=menu->resPtr; j++ ) {
		res = &menu->resources[j];
		if ( foreground == qtrue ) //foreground menu
			UI_SetColor( res->fgcolor );
		else UI_SetColor( res->bgcolor );
		switch ( res->type ) {

			case UI_RES_LABEL:
				if (res->enablewithcvar)
					if (!res->enabledcvar.integer)
						break;
				if ( res->title ) {
					if ( res->linkcvarname ) {
						trap_Cvar_Update( &res->linkcvar );
						if ( res->selentries > 0 ) {
							for (k=0;k < res->selentries;k++) {
								if (!Q_strncmp( res->linkstring1[k], res->linkcvar.string, UI_MAX_NAME ))
									trap_R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,res->linkstring2[k],1,0,qtrue,qtrue);
							}
						}
						else
							trap_R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,res->linkcvar.string,1,0,qtrue,qtrue);
					} else {
						UI_DrawBox( res->rect[0],res->rect[1], res->rect[2],res->rect[3],qfalse );
						trap_R_Text_Paint( res->font,res->rect[0]+ (res->rect[2] - trap_R_Text_Width( res->font, res->title, 0,qtrue ))/2,res->rect[1]+ (res->rect[3] -trap_R_Text_Height( res->font, res->title, 0,qtrue ))/2,1,0,res->title,1,0,qtrue,qtrue);
					}
				}
				else if ( res->linkcvartoshader ) {
					trap_Cvar_Update( &res->linkcvar );
					cvarshader=trap_R_RegisterShaderNoMip(res->linkcvar.string);
					if ( cvarshader )
						UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], cvarshader );
				}
				else
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->shader );
				break;

			case UI_RES_BUTTON:
				if (res->hoverDraw && res->active)
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->hoverShader );
				else if ( res->shader )
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->shader );
				break;
			case UI_RES_FIELD:
				trap_Cvar_Update( &res->linkcvar );
				if ( res->active )
					UI_SetColor(color_yellow);
				trap_R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,res->linkcvar.string,1,0,qtrue,qtrue);
				UI_SetColor( NULL );
				break;
			case UI_RES_CHECKBOX:
				trap_Cvar_Update( &res->linkcvar );
				if (res->linkcvar.integer)
					UI_DrawHandlePic( res->rect[0], res->rect[1], UI_CHECKBOX_SIZE, UI_CHECKBOX_SIZE, res->checked_shader );
				else UI_DrawHandlePic( res->rect[0], res->rect[1], UI_CHECKBOX_SIZE, UI_CHECKBOX_SIZE, res->unchecked_shader );
				break;
			case UI_RES_PULLDOWN:
				if (res->pressed == qtrue) {
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->selmenushader );
					UI_DrawBox( res->rect[0]+res->rect[2], res->rect[1], 128, 16 * res->selentries +3, qfalse );
					for (k=0;k<res->selentries;k++)
						trap_R_Text_Paint( res->font, res->rect[0]+res->rect[2]+3,3+res->rect[1]+(k*16),1,1,res->linkstring1[k],0,0,qtrue,qtrue );
				}
				else {
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->menushader );
				}
				break;
		}
	}
}

/*
=================
UI_Refresh
=================
*/
void UI_Refresh( int realtime )
{
	int				i;
	int				j;
	int				k;
	uiMenu_t		*menu;
	uiResource_t	*res;
	qhandle_t		cvarshader;

	uis.frametime = realtime - uis.realtime;
	uis.realtime  = realtime;

	if ( !( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	UI_UpdateCvars();

	for (i=0;i<uis.MSP;i++) {
		UI_DrawMenu( &uis.menuStack[i], qfalse );
	}
	// last one is foreground menu
	UI_DrawMenu( &uis.menuStack[uis.MSP], qtrue );

	// draw cursor
	UI_SetColor( NULL );
	UI_DrawHandlePic( uis.cursorx, uis.cursory, 32, 32, uis.cursor);
}

qboolean UI_IsFullscreen( void ) {
	uiMenu_t *menu;

	if ( uis.MSP!= -1 && ( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
		menu = &uis.menuStack[uis.MSP];
		
		return menu->fullscreen;
	}

	return qfalse;
}

void UI_SetActiveMenu( uiMenuCommand_t menu ) {
	uis.activemenu = menu;

	trap_Key_SetCatcher( KEYCATCH_UI );
}

char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}

int	UI_Argc( void ) {

	return trap_Argc( );
}

/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	char	*cmd;

	cmd = UI_Argv( 0 );


	if ( Q_stricmp (cmd, "ui_connect") == 0 ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "connect 63.210.148.39:12203\n" ); // STONER SERVER
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "ui_test") == 0 ) {
		Com_Printf( "openmohaa menu up and running!\n uis taking %i KB\n", sizeof(uiStatic_t)/1024 );
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "pushmenu") == 0 ) {
		if ( UI_Argc() == 2 )
			UI_PushMenu( UI_Argv(1) );
		else Com_Printf( "Usage: pushmenu <menuname>" );
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "popmenu") == 0 ) {
		UI_PopMenu();
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "pushmenu_teamselect") == 0 ) {
		UI_PushMenu( "dm_teamselect" );
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
	UI_DrawMenu( &uis.connecting, qtrue );
	// draw cursor
	UI_SetColor( NULL );
	UI_DrawHandlePic( uis.cursorx, uis.cursory, 32, 32, uis.cursor);
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
	*x = *x * uis.xscale + uis.bias;
	*y *= uis.yscale;
	*w *= uis.xscale;
	*h *= uis.yscale;
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

void UI_SetColor( const float *rgba ) {
	trap_R_SetColor( rgba );
}

void UI_CmdExecute( const char *text ){
	trap_Cmd_ExecuteText( EXEC_APPEND, text );
}

// Wombat
/*
=================
UI_DrawBox
ctrCoord == qtrue: draw box with center at x,y
ctrCoord == qfalse: box with upper left corner at x,y
=================
*/
#define LINE_THICKNESS	1
void	UI_DrawBox( int x, int y, int w, int h, qboolean ctrCoord ) {

	if ( ctrCoord ) {
		x -= w/2;
		y -= h/2;
	}

	trap_R_SetColor( color_gray );
	trap_R_DrawStretchPic( x*uis.xscale, y*uis.yscale, w*uis.xscale, h*uis.yscale, 0, 0, 16, 16, uis.blackShader );
	trap_R_SetColor( color_white );
	trap_R_DrawStretchPic( x*uis.xscale, y*uis.yscale, w*uis.xscale, LINE_THICKNESS*uis.yscale, 0, 0, 32, 32, uis.blackShader );
	trap_R_DrawStretchPic( x*uis.xscale, y*uis.yscale, LINE_THICKNESS*uis.xscale, h*uis.yscale, 0, 0, 32, 32, uis.blackShader );
	trap_R_DrawStretchPic( x*uis.xscale, (y+h)*uis.yscale, w*uis.xscale, LINE_THICKNESS*uis.yscale, 0, 0, 32, 32, uis.blackShader );
	trap_R_DrawStretchPic( (x+w)*uis.xscale, y*uis.yscale, LINE_THICKNESS*uis.xscale, (h+LINE_THICKNESS)*uis.yscale, 0, 0, 32, 32, uis.blackShader );
	trap_R_SetColor( NULL );
}

