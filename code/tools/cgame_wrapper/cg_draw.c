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

fontheader_t *facfont;

int GetSnapshot ( int snapshotNumber, snapshot_t *snap ) {
	snapshot = snap;
	return cgi.GetSnapshot( snapshotNumber, snap );
}

qhandle_t R_RegisterModel ( char *name ) {
	qhandle_t ret;

	ret = cgi.R_RegisterModel( name );
	return ret;
}

// su44: we need to access skeletor_c class somehow
void R_AddRefEntityToScene(refEntity_t *ent) {
	skeletor_c *skel;
	
	if(ent->tiki) {
		vec3_t	end;
		// su44: to get entity skeletor pointer, we need to know
		// its TIKI pointer and entityNumber (the one send 
		// through entityState_t). Bad things happen if TIKI
		// pointer passed to TIKI_GetSkeletor is NULL.
		skel = cgi.TIKI_GetSkeletor(ent->tiki,ent->entityNumber);
		// got it
		VectorCopy( ent->origin, end );
		end[2] += 50;
		cgi.R_DebugLine( ent->origin, end, 1, 1, 1, 1 );
	} else {
		skel = 0;
	}


	cgi.R_AddRefEntityToScene(ent);

}

void CG_Draw2D() {
	cge.CG_Draw2D();

	// 2D drawing on top of cgame's 2D
	cgi.R_DrawString( facfont, "Hooked", 10, 140, 7, qtrue );
}

void CG_DrawActiveFrame ( int serverTime, int frametime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	// Issue 3D drawing here before letting cgame draw the frame

	cge.CG_DrawActiveFrame( serverTime, frametime, stereoView, demoPlayback );
}