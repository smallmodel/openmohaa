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
//
#ifndef __UI_LOCAL_H__
#define __UI_LOCAL_H__

#include "../qcommon/q_shared.h"
#include "../renderer/tr_types.h"
#include "ui_public.h"
#include "../client/keycodes.h"
#include "../game/bg_public.h"


#define UI_MAX_URCSIZE		131072
#define UI_MAX_MENUS			16
#define UI_MAX_NAME			128
#define UI_MAX_RESOURCES	64
#define UI_MAX_SELECT			32

#define UI_CHECKBOX_SIZE	16

typedef void (*voidfunc_f)(void);

typedef enum uiBorderstyle_s {
	UI_BORDER_NONE,
	UI_BORDER_RAISED,
	UI_BORDER_INDENT,
	UI_BORDER_3D
} uiBorderstyle_t;

typedef enum uiMenuDirection_s {
	UI_NONE,
	UI_FROM_TOP,
	UI_FROM_BOTTOM,
	UI_FROM_LEFT,
	UI_FROM_RIGHT
} uiMenuDirection_t;

typedef enum uiAlign_s {
	UI_ALIGN_LEFT,
	UI_ALIGN_CENTER,
	UI_ALIGN_RIGHT
} uiAlign_t;

typedef enum uiResType_s {
	UI_RES_LABEL,
	UI_RES_BUTTON,
	UI_RES_BINDBUTTON,
	UI_RES_CHECKBOX,
	UI_RES_FIELD,
	UI_RES_LIST,
	UI_RES_LISTBOX,
	UI_RES_SLIDER,
	UI_RES_PULLDOWN
} uiResType_t;

typedef enum uiSlidertype_s {
	UI_ST_FLOAT
} uiSlidertype_t;

typedef enum {
	STATBAR_NONE,
	STATBAR_HORIZONTAL,
	STATBAR_VERTICAL,
	STATBAR_VERTICAL_REVERSE,
	STATBAR_VERTICAL_STAGGER_EVEN,
	STATBAR_VERTICAL_STAGGER_ODD,
	STATBAR_CIRCULAR,
	STATBAR_NEEDLE,
	STATBAR_ROTATOR,
	STATBAR_COMPASS,
	STATBAR_SPINNER,
	STATBAR_HEADING_SPINNER
} uiStatbar_t;

typedef struct uiResource_s {
	uiResType_t			type;
	char				name[UI_MAX_NAME];
	uiMenuDirection_t	align;
	int					rect[4];
	vec4_t				fgcolor;
	vec4_t				bgcolor;
	uiBorderstyle_t		borderstyle;
	qhandle_t			shader;
	qhandle_t			hoverShader;
	qboolean			hoverDraw;
	qhandle_t			tileShader;

	qhandle_t			checked_shader;
	qhandle_t			unchecked_shader;
	char				*clicksound;
	char				*stuffcommand;
	char				*hovercommand;
	vmCvar_t			enabledcvar;
	qboolean			enablewithcvar;
	vmCvar_t			linkcvar;
	char				*linkcvarname;
	qboolean			linkcvartoshader;
	char				*linkstring1[UI_MAX_SELECT];
	char				*linkstring2[UI_MAX_SELECT];

	char				*title;
	uiAlign_t			textalign;
	qhandle_t			menushader;
	qhandle_t			selmenushader;
	int					selentries;

	uiSlidertype_t		slidertype;
	float				flRange[2];
	float				flStepsize;

	qboolean			rendermodel;
	char				*modeloffset;
	char				*modelrotateoffset;
	char				*modelangles;
	float				modelscale;
	char				*modelanim;
// storing extra memory for each resource is too memory-expensive.
// letting this point to uis.menuFont for now
	fontInfo_t			*font;	
	int					ordernumber;

	qboolean			active;
	qboolean			lastState;
	qboolean			pressed;

	uiStatbar_t			statbar;
	statIndex_t			playerstat;
	statIndex_t			itemstat;
	qhandle_t			statbarshader;
	qhandle_t			statbartileshader;
	qhandle_t			statbarshaderflash;
	int					statbarRange[2];
	int					statbarendangles[2];
	float				rotatorsize[2];
	int					invmodelhand;
	float				fadein;
} uiResource_t;

typedef struct uiMenu_s {
	char			name[UI_MAX_NAME];
	int				size[2];
	uiAlign_t		align[2];
	uiMenuDirection_t	motion;
	int				time;
	vec4_t			fgcolor;
	vec4_t			bgcolor;
	uiBorderstyle_t	borderstyle;
	vec4_t			bgfill;
	qboolean		fullscreen;
	int				vidmode;
	float			fadein;
	char			include[UI_MAX_NAME];
	qboolean		virtualres;

	uiResource_t	resources[UI_MAX_RESOURCES];
	int				resPtr;

	qboolean		standard;
} uiMenu_t;

typedef enum {
	HUD_AMMO,
	HUD_COMPASS,
	HUD_FRAGLIMIT,
	HUD_HEALTH,
	HUD_ITEMS,
	HUD_SCORE,
	HUD_STOPWATCH,
	HUD_TIMELIMIT,
	HUD_WEAPONS,
	HUD_MAX
} uiHudMenus_t; 

// ui_quarks.c
typedef struct {
	int					frametime;
	int					realtime;
	int					cursorx;
	int					cursory;

	glconfig_t			glconfig;
	qboolean			debug;
	qhandle_t			whiteShader;
	qhandle_t			blackShader;
	qhandle_t			charset;

	qhandle_t			cursor;
	float				xscale;
	float				yscale;
	float				bias;
	qboolean			firstdraw;

	uiMenuCommand_t		activemenu;

	// wombat
	fontInfo_t			menuFont;

	uiMenu_t			*stack[UI_MAX_MENUS];
	int					MSP; // 'Menu Stack Pointer'

	// menus that are always used
	uiMenu_t			main; // main menu
	uiMenu_t			connecting; // connecting screen, used by UI_DrawConnectScreen
	uiMenu_t			loading;
	// for other windows, we use the cache.
	uiMenu_t			cache[UI_MAX_MENUS];
	int					CP; // 'Cache Pointer'
	
	// HUD
	uiMenu_t			hudMenus[HUD_MAX];
} uiStatic_t;


// ui_main.c
void *UI_Alloc( int size );
void UI_Free( void *p );
void UI_InitMemory( void );
qboolean UI_OutOfMemory( void );
// ui_quarks.c
//
extern uiStatic_t uis;

void UI_DrawHUD( int stats[] );
void UI_Init( void );
void UI_Shutdown( void );
void UI_KeyEvent( int key, int down );
void UI_MouseEvent( int dx, int dy );
void UI_Refresh( int realtime );
qboolean UI_IsFullscreen( void );
void UI_SetActiveMenu( uiMenuCommand_t menu );
qboolean UI_ConsoleCommand( int realTime );
void UI_DrawConnectScreen( qboolean overlay );

void UI_SetColor( const float *rgba );
void UI_UpdateScreen( void );
void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader );
void UI_CmdExecute( const char *text );
void UI_DrawBox( int x, int y, int w, int h, qboolean ctrCoord );

// ui_urc.c
void	UI_LoadINC( const char *name, uiMenu_t *menu );
void	UI_LoadURC( const char *name, uiMenu_t *menu );
void	UI_PushMenu( const char *name );
void	UI_PopMenu( void );

//
// ui_syscalls.c
//
void			trap_Print( const char *string );
void			trap_Error( const char *string );
int				trap_Milliseconds( void );
void			trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void			trap_Cvar_Update( vmCvar_t *vmCvar );
void			trap_Cvar_Set( const char *var_name, const char *value );
float			trap_Cvar_VariableValue( const char *var_name );
void			trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void			trap_Cvar_SetValue( const char *var_name, float value );
void			trap_Cvar_Reset( const char *name );
void			trap_Cvar_Create( const char *var_name, const char *var_value, int flags );
void			trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize );
int				trap_Argc( void );
void			trap_Argv( int n, char *buffer, int bufferLength );
void			trap_Cmd_ExecuteText( int exec_when, const char *text );	// don't use EXEC_NOW!
int				trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void			trap_FS_Read( void *buffer, int len, fileHandle_t f );
void			trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void			trap_FS_FCloseFile( fileHandle_t f );
int				trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
int				trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t
qhandle_t		trap_R_RegisterModel( const char *name );
qhandle_t		trap_R_RegisterSkin( const char *name );
qhandle_t		trap_R_RegisterShaderNoMip( const char *name );
void			trap_R_ClearScene( void );
void			trap_R_AddRefEntityToScene( const refEntity_t *re );
void			trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void			trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void			trap_R_RenderScene( const refdef_t *fd );
void			trap_R_SetColor( const float *rgba );
void			trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void			trap_UpdateScreen( void );
int				trap_CM_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName );
void			trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed );
void			trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void			trap_Key_GetBindingBuf( int keynum, char *buf, int buflen );
void			trap_Key_SetBinding( int keynum, const char *binding );
qboolean		trap_Key_IsDown( int keynum );
qboolean		trap_Key_GetOverstrikeMode( void );
void			trap_Key_SetOverstrikeMode( qboolean state );
void			trap_Key_ClearStates( void );
int				trap_Key_GetCatcher( void );
void			trap_Key_SetCatcher( int catcher );
void			trap_GetClipboardData( char *buf, int bufsize );
void			trap_GetClientState( uiClientState_t *state );
void			trap_GetGlconfig( glconfig_t *glconfig );
int				trap_GetConfigString( int index, char* buff, int buffsize );
int				trap_LAN_GetServerCount( int source );
void			trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void			trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int				trap_LAN_GetPingQueueCount( void );
int				trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
void			trap_LAN_ClearPing( int n );
void			trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime );
void			trap_LAN_GetPingInfo( int n, char *buf, int buflen );
int				trap_MemoryRemaining( void );
void			trap_GetCDKey( char *buf, int buflen );
void			trap_SetCDKey( char *buf );

qboolean               trap_VerifyCDKey( const char *key, const char *chksum);

void			trap_SetPbClStatus( int status );

void			trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
int				trap_R_Text_Width(fontInfo_t *font, const char *text, int limit, qboolean useColourCodes);
int				trap_R_Text_Height(fontInfo_t *font, const char *text, int limit, qboolean useColourCodes);
void			trap_R_Text_Paint(fontInfo_t *font, float x, float y, float scale, float alpha, const char *text, float adjust, int limit, qboolean useColourCodes, qboolean is640);
void			trap_R_Text_PaintChar(fontInfo_t *font, float x, float y, float scale, int c, qboolean is640);

#endif
