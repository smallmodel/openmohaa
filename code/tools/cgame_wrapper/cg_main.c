/*
===========================================================================
Copyright (C) 2012 Michael Rieder

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

#include "qcommon.h"
#include <Windows.h>

// original cgame dll
HMODULE cg_dll;
// interface function
typedef clientGameExport_t* (*cgapi)();

// cgame definitions
clientGameExport_t cge;
clientGameExport_t cge_out;
clientGameImport_t cgi;
clientGameImport_t cgi_out;

snapshot_t	emptySnap;
snapshot_t	*snapshot = &emptySnap;

void CG_Init( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum ) {
	memcpy( &cgi, imported, sizeof(cgi) );
	memcpy( &cgi_out, &cgi, sizeof(cgi) );

	facfont = cgi.R_LoadFont( "facfont-20" );

	// Reroute import functions
	cgi_out.R_RegisterModel			= R_RegisterModel;
	cgi_out.TIKI_FindTiki			= TIKI_FindTiki;
	cgi_out.R_Model_GetHandle		= R_Model_GetHandle;
	cgi_out.R_AddRefEntityToScene	= R_AddRefEntityToScene;
	cgi_out.GetSnapshot				= GetSnapshot;

	// Call original function
	cge.CG_Init(&cgi_out, serverMessageNum, serverCommandSequence, clientNum);

	// su44: init cvars from cg_skeletor.c
	CG_InitSkeletorCvarsAndCmds();

	cgi.Printf( "\n" );
	cgi.Printf( "=============================================\n" );
	cgi.Printf( "MOHAA cgame wrapper loaded successfully.\n" );
	cgi.Printf( "=============================================\n" );
	cgi.Printf( "\n" );
}

void CG_Shutdown () {
	cge.CG_Shutdown();
	FreeLibrary( cg_dll );
}

__declspec(dllexport) clientGameExport_t *GetCGameAPI() {
	cgapi		cg_dll_proc;
	DWORD		err;

	// Load original DLL
	cg_dll = LoadLibrary( "main\\cgamex86mohaa.dll" );
	if (cg_dll == NULL) {
		err = GetLastError();
		return NULL;
	}

	cg_dll_proc = (cgapi)GetProcAddress( cg_dll, "GetCGameAPI" );
	if (cg_dll_proc == NULL) {
		err = GetLastError();
		return NULL;
	}

	// Call original GetCGameAPI to get the pointers
	memcpy( &cge, cg_dll_proc(), sizeof(cge) );
	memcpy( &cge_out, &cge, sizeof(cge) );

	// reroute exported functions
	cge_out.CG_Init				= CG_Init;
	cge_out.CG_Draw2D			= CG_Draw2D;
	cge_out.CG_DrawActiveFrame	= CG_DrawActiveFrame;
	cge_out.CG_ConsoleCommand	= CG_ConsoleCommand;
	cge_out.CG_Shutdown			= CG_Shutdown;

	return &cge_out;
}