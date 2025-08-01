/*
===========================================================================
Copyright (C) 2023-2024 the OpenMoHAA team

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

#pragma once

#include "../uilib/ui_extern.h"
#include "../uilib/uiwidget.h"

typedef struct {
    UIReggedMaterial *loading;
} static_media_t;

//
// cvars
//
extern cvar_t *ui_health_start;
extern cvar_t *ui_health_end;
extern cvar_t *sound_overlay;
extern cvar_t *cl_movieaudio;

//
// initialization
//
void CL_FillUIDef(void);
void CL_InitializeUI(void);
void CL_ShutdownUI(void);

//
// general ui stuff
//
extern inventory_t  client_inv;
extern bind_t       client_bind;
extern cvar_t      *cl_greenfps;
extern qboolean     server_loading;
extern const UColor UWhiteChatMessageColor;
extern const UColor URedChatMessageColor;
extern const UColor UGreenChatMessageColor;

const char *CvarGetForUI(const char *name, const char *defval);
void        UI_ClearState(void);
void        CL_BeginRegistration(void);
void        CL_EndRegistration(void);
float       UI_GetObjectivesTop(void);
void        UI_GetHighResolutionScale(vec2_t scale);

//
// menu
//
extern Menu *ui_pLoadingMenu;

void     UI_ShowMenu(const char *name, qboolean bForce);
void     UI_HideMenu(const char *name, qboolean bForce);
void     UI_ForceMenuOff(bool force);
void     UI_PushMenu(const char *name);
void     UI_ForceMenu(const char *name);
qboolean UI_BindActive(void);
void     UI_SetReturnMenuToCurrent(void);
void     UI_ActivateView3D(void);
void     UI_MenuEscape(const char *name);
void     UI_CreateScoreboard(void);
qboolean UI_MenuActive(void);
qboolean UI_MenuUp(void);
void     UI_FocusMenuIfExists(void);
void     UI_DrawConnect(void);
void     UI_CreateDialog(
        const char *title,
        char       *cvarname,
        const char *command,
        const char *cancelCommand,
        int         width,
        int         height,
        const char *shader,
        const char *okshader,
        const char *cancelshader
    );
void UI_ToggleDMConsole(int iMode);
void UI_ShowScoreboard_f(const char *pszMenuName);
void UI_HideScoreboard_f(void);
void UI_SetScoreBoardItem(
    int          iItemNumber,
    const char  *pszData1,
    const char  *pszData2,
    const char  *pszData3,
    const char  *pszData4,
    const char  *pszData5,
    const char  *pszData6,
    const char  *pszData7,
    const char  *pszData8,
    const vec4_t pTextColor,
    const vec4_t pBackColor,
    qboolean     bIsHeader
);
void UI_DeleteScoreBoardItems(int iMaxIndex);

//
// ressources
//
void     UI_BeginLoad(const char *pszMapName);
void     UI_EndLoad(void);
void     UI_AbortLoad(void);
int      CL_FontStringWidth(fontheader_t *pFont, const char *pszString, int iMaxLen);
void     UI_BeginLoadResource(void);
void     UI_EndLoadResource(void);
void     UI_EndLoadResource(const char *name);
qboolean UI_IsResourceLoaded(const char *name);
void     UI_RegisterLoadResource(const char *name);
void     UI_ClearResource(void);
void     UI_LoadResource(const char *name);

//
// windows
//
void UI_DeactiveFloatingWindows(void);

//
// console
//
void     UI_OpenConsole(void);
qboolean UI_ConsoleIsVisible(void);
qboolean UI_ConsoleIsOpen(void);
void     UI_CloseConsole(void);
void     UI_ToggleConsole(void);
void     UI_OpenDMConsole(int iMode);
void     UI_CloseDMConsole(void);
void     UI_ToggleDMConsole(int iMode);
void     UI_OpenDeveloperConsole(void);
void     UI_CloseDeveloperConsole(void);
void     UI_PrintConsole(const char *msg);

//
// callbacks
//
void UI_Update(void);
void UI_ServerLoaded(void);
void UI_ResolutionChange(void);
void UI_KeyEvent(int key, unsigned int time);
void UI_CharEvent(int ch);
