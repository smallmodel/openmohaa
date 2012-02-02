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

#ifndef __MD5_2_SKX_H__
#define __MD5_2_SKX_H__

// this tool is dependant on OpenMoHAA engine headers,
// not on the ones used by q3map and rest of utilities
#include "../../qcommon/q_shared.h"
#include "../../qcommon/tiki_local.h"

#define MD5_2_SKX_VERSION "0.1"

// maximum size of file path used in this tool
#define MAX_TOOLPATH 256

// tool model data
typedef struct tBone_s {
	char name[32];
	int parent; // -1 == no parent
	//skdJointType_t type; // TODO ?
} tBone_t;

typedef struct {
	unsigned int indexes[3];
} tTri_t;

typedef struct {
	vec3_t offset;
	float boneWeight;
	int boneNum;
} tWeight_t;

typedef struct {
	vec2_t texCoords;
	//vec3_t normal; // TODO
	int firstWeight;
	int numWeights;
	tWeight_t *weights;
} tVert_t;

typedef struct {
	char name[MAX_TOOLPATH];
	int numVerts;
	int numWeights;
	int numTris;
	tVert_t *verts;
	tTri_t *tris;
	tWeight_t *weights;
} tSurf_t;

typedef struct tModel_s {
	char fname[MAX_TOOLPATH];
	int numSurfaces;
	int numBones;
	tSurf_t *surfs;
	tBone_t *bones;
	bone_t *baseFrame;
} tModel_t;

// md5 animation data

typedef struct {
	vec3_t mins;
	vec3_t maxs;
	float *components;
} tFrame_t;

typedef struct {
	char name[MAX_TOOLPATH];
	int parent;
	int firstComponent;
	int componentBits;
} tAnimBone_t;

typedef struct {
	char fname[MAX_TOOLPATH];
	float frameRate;
	int numFrames;
	int numBones;
	int numAnimatedComponents;
	bone_t *baseFrame;
	tAnimBone_t *boneData;
	tFrame_t *frames;
} tAnim_t;

// command line parameters
extern qboolean verbose;
extern qboolean noLimits;
extern qboolean createTIK;
extern char outTIKI[MAX_TOOLPATH];

// doom3md5model.c
tModel_t *loadMD5Mesh(const char *fname);
tAnim_t *loadMD5Anim(const char *fname);
bone_t *setupMD5AnimBones(tAnim_t *a, int frameNum);
bone_t *setupMD5MeshBones(tModel_t *mod);

// misc_utils.c
void T_Printf(const char *format, ...);
void T_VerbPrintf(const char *format, ...);
void T_Error(const char *format, ...);

int F_LoadBuf(const char *fname, byte **out);
void F_FreeBuf(byte *b);



#endif // __MD5_2_SKX_H__
