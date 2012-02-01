/*
===========================================================================
Copyright (C) 2012 su44

This file is part of md5_2_skX source code.

md5_2_skX source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

md5_2_skX source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with md5_2_skX source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "md5_2_skX.h"

qboolean verbose = qfalse;
qboolean noLimits = qfalse;
qboolean createTIK = qfalse;
char inMD5Mesh[MAX_TOOLPATH];
int numAnims = 0;
char inMD5Anim[MAX_TOOLPATH][256];
tAnim_t *md5Anims[256];
char outTIKI[MAX_TOOLPATH];

void Convert() {
	// TODO
}
int main(int argc, const char **argv) {
	int i;

	printf("md5_2_skX - v" MD5_2_SKX_VERSION " (c) 2012 su44.\n");

	// parse arguments list
	for(i = 1; i < argc; i++) {
		if(!Q_stricmp(argv[i], "-v")) {
			verbose = qtrue;
		} else if(!Q_stricmp(argv[i], "-nolimit") || !Q_stricmp(argv[i], "-nolimits")) {
			noLimits = qtrue;
		} else if(!Q_stricmp(argv[i], "-tik")) {
			createTIK = qtrue;
		} else if(!Q_stricmp(argv[i], "-anim")) {
			// get anim filename
			i++;
			strcpy(inMD5Anim[numAnims],argv[i]);
			numAnims++;		
		} else if(!Q_stricmp(argv[i], "-mesh")) {
			// get mesh filename
			i++;
			strcpy(inMD5Mesh,argv[i]);
		} else if(strstr(argv[i], "md5mesh")) {
			// that's a mesh filename
			strcpy(inMD5Mesh,argv[i]);
		} else if(strstr(argv[i], "md5anim")) {
			// that's a anim filename
			strcpy(inMD5Anim[numAnims],argv[i]);
			numAnims++;
		} else if(strstr(argv[i], "help")) {
			// TODO - print help
		}
	}
loadMD5Mesh(inMD5Mesh);
	////loadMD5Anim(inMD5Anim);

	//Convert();

	printf("Done.\n");
	// let the user see the results
	system("pause");
}