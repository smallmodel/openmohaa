#include "qcommon.h"
#include <Windows.h>

typedef clientGameExport_t* (*cgapi)();
clientGameExport_t cge;
clientGameExport_t cge_out;
clientGameImport_t cgi;

void CG_Init( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum ) {
	memcpy( &cgi, imported, sizeof(cgi) );
	cge.CG_Init(imported, serverMessageNum, serverCommandSequence, clientNum);
	cgi.Printf( "MOHAA cgame wrapper loaded successfuly.\n" );
}

__declspec(dllexport) clientGameExport_t *GetCGameAPI() {
	HMODULE		cg_dll;
	cgapi		cg_dll_proc;
	DWORD		err;
	clientGameExport_t *ret;

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

	memcpy( &cge, cg_dll_proc(), sizeof(cge) );
	memcpy( &cge_out, &cge, sizeof(cge) );

	cge_out.CG_Init = CG_Init;

	return &cge_out;
}