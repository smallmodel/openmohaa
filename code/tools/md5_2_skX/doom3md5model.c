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

// doom3md5model.c - Doom3 md5mesh and md5anim loading and animation code

#include "md5_2_skX.h"

/*
====================================================================

Doom3 md5mesh/md5anim LOADING

====================================================================
*/

tModel_t *loadMD5Mesh(const char *fname) {
	int i,j,k;
	char *txt;
	int len;
	char *p;
	const char *token;
	tModel_t *out;
	tBone_t *boneDef;
	tVert_t *v;
	tWeight_t *weight;
	tSurf_t *surf;
	tTri_t *tri;
	bone_t *bone;
	int tmpNumSurfs;
	int tmpNumBones;

	len = F_LoadBuf(fname,(byte**)&txt);

	if(len == -1) {
		T_Error("Cannot open %s\n",fname);
		return 0;
	}

	// NOTE: this will not open the "fname" file!
	COM_BeginParseSession(fname);

	p = txt;

	token = COM_ParseExt(&p, qfalse);
	if(Q_stricmp(token,"MD5Version")) {
		T_Error("loadMD5Mesh: Expected \"MD5Version\", found %s in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qfalse);
	if(Q_stricmp(token,"10")) {
		T_Error("loadMD5Mesh: Expected \"10\", found %s in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// skip optional commandline
	token = COM_ParseExt(&p, qtrue);
	if(!Q_stricmp(token,"commandline")) {
		token = COM_ParseExt(&p, qfalse);
		token = COM_ParseExt(&p, qtrue);
	}

	// parse numJoints <number>
	if(Q_stricmp(token, "numJoints")) {
		T_Error("loadMD5Mesh: expected \"numJoints\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumBones = atoi(token);

	// parse numMeshes <number>
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "numMeshes")) {
		T_Error("loadMD5Mesh: expected \"numMeshes\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumSurfs = atoi(token);

	// parse all the surfaces
	if(tmpNumSurfs < 1) {
		T_Error("loadMD5Mesh: \"%s\" has no surfaces\n", fname);
		return 0;
	}

	// okay, let's assume that md5mesh file is valid, and alloc 
	// output model. Still, it will be fried if any error occur
	out = malloc(sizeof(tModel_t));
	
	out->numBones = tmpNumBones;
	out->numSurfaces = tmpNumSurfs;
	out->bones = malloc(sizeof(tBone_t)*out->numBones);
	out->baseFrame = malloc(sizeof(bone_t)*out->numBones);

	// parse "joints"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "joints")) {
		T_Error("loadMD5Mesh: expected \"joints\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "{"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "{")) {
		T_Error("loadMD5Mesh: expected \"{\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	boneDef = out->bones;
	bone = out->baseFrame;
	for(i = 0; i < out->numBones; i++,bone++,boneDef++) {
		// bone name (eg "Bip01 Spine1")
		token = COM_ParseExt(&p, qtrue);
		Q_strncpyz(boneDef->name, token, sizeof(boneDef->name));

		token = COM_ParseExt(&p, qfalse);
		boneDef->parent = atoi(token);

		T_VerbPrintf("loadMD5Mesh: bone %i of %i - %s, parent %i\n",
			i,out->numBones,boneDef->name,boneDef->parent);

		// skip (
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Mesh: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(j = 0; j < 3; j++) {
			token = COM_ParseExt(&p, qfalse);
			bone->p[j] = atof(token);
		}

		// skip )
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Mesh: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// skip (
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Mesh: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(j = 0; j < 3; j++) {
			token = COM_ParseExt(&p, qfalse);
			bone->q[j] = atof(token);
		}
		QuatCalcW(bone->q);

		// skip )
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Mesh: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
	}

	// parse closing } of joints block
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "}")) {
		T_Error("loadMD5Mesh: expected \"}\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	out->surfs = malloc(sizeof(*surf) * out->numSurfaces);
	for(i = 0, surf = out->surfs; i < out->numSurfaces; i++, surf++) {
		// parse mesh {
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "mesh")) {
			T_Error("loadMD5Mesh: expected \"mesh\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, "{")) {
			T_Error("loadMD5Mesh: expected \"{\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// parse shader <name>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "shader")) {
			T_Error("loadMD5Mesh: expected \"shader\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		Q_strncpyz(surf->name, token, sizeof(surf->name));

		// parse numVerts <number>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "numVerts")) {
			T_Error("loadMD5Mesh: expected \"numVerts\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		surf->numVerts = atoi(token);

		surf->verts = malloc(sizeof(*v) * surf->numVerts);
		for(j = 0, v = surf->verts; j < surf->numVerts; j++, v++) {
			// skip vert <number>
			token = COM_ParseExt(&p, qtrue);
			if(Q_stricmp(token, "vert")) {
				T_Error("loadMD5Mesh: expected \"vert\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}
			COM_ParseExt(&p, qfalse);

			// skip (
			token = COM_ParseExt(&p, qfalse);
			if(Q_stricmp(token, "(")) {
				T_Error("loadMD5Mesh: expected \"(\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}

			for(k = 0; k < 2; k++) {
				token = COM_ParseExt(&p, qfalse);
				v->texCoords[k] = atof(token);
			}

			// skip )
			token = COM_ParseExt(&p, qfalse);
			if(Q_stricmp(token, ")")) {
				T_Error("loadMD5Mesh: expected \")\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}

			token = COM_ParseExt(&p, qfalse);
			v->firstWeight = atoi(token);

			token = COM_ParseExt(&p, qfalse);
			v->numWeights = atoi(token);
		}

		// parse numTris <number>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "numTris")) 	{
			T_Error("loadMD5Mesh: expected \"numTris\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		surf->numTris = atoi(token);

		surf->tris = malloc(sizeof(*tri) * surf->numTris);
		for(j = 0, tri = surf->tris; j < surf->numTris; j++, tri++) {
			// skip tri <number>
			token = COM_ParseExt(&p, qtrue);
			if(Q_stricmp(token, "tri")) {
				T_Error("loadMD5Mesh: expected \"tri\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}
			COM_ParseExt(&p, qfalse);

			for(k = 0; k < 3; k++) {
				token = COM_ParseExt(&p, qfalse);
				tri->indexes[k] = atoi(token);
			}
		}

		// parse numWeights <number>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "numWeights")) {
			T_Error("loadMD5Mesh: expected \"numWeights\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		surf->numWeights = atoi(token);

		surf->weights = malloc(sizeof(*weight) * surf->numWeights);
		for(j = 0, weight = surf->weights; j < surf->numWeights; j++, weight++) {
			// skip weight <number>
			token = COM_ParseExt(&p, qtrue);
			if(Q_stricmp(token, "weight")) {
				T_Error("loadMD5Mesh: expected \"weight\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}
			COM_ParseExt(&p, qfalse);

			token = COM_ParseExt(&p, qfalse);
			weight->boneNum = atoi(token);

			token = COM_ParseExt(&p, qfalse);
			weight->boneWeight = atof(token);

			// skip (
			token = COM_ParseExt(&p, qfalse);
			if(Q_stricmp(token, "(")) {
				T_Error("loadMD5Mesh: expected \"(\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}

			for(k = 0; k < 3; k++) {
				token = COM_ParseExt(&p, qfalse);
				weight->offset[k] = atof(token);
			}

			// skip )
			token = COM_ParseExt(&p, qfalse);
			if(Q_stricmp(token, ")")) {
				T_Error("loadMD5Mesh: expected \")\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}
		}

		// parse }
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "}")) {
			T_Error("loadMD5Mesh: expected \"}\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// loop trough all vertices and set up the vertex weights
		for(j = 0, v = surf->verts; j < surf->numVerts; j++, v++) {
			v->weights = malloc(sizeof(*v->weights) * v->numWeights);

			for(k = 0; k < v->numWeights; k++) {
				v->weights[k] = surf->weights[v->firstWeight + k];
			}
		}
	}
	T_Printf("Succesfully loaded %s\n",fname);
	return out;
}

tAnim_t *loadMD5Anim(const char *fname) {
	int i,/*j,*/k;
	char *txt;
	int len;
	char *p;
	const char *token;
	tAnim_t *out;
	tFrame_t *frame;
	bone_t *bone;
	tAnimBone_t *boneDef;
	int tmpNumJoints, tmpNumFrames;
	int tmpNumAnimatedComponents;
	float tmpFrameRate;

	len = F_LoadBuf(fname,(byte**)&txt);

	if(len == -1) {
		T_Error("Cannot open %s\n",fname);
		return 0;
	}

	// NOTE: this will not open the "fname" file!
	COM_BeginParseSession(fname);

	p = txt;

	token = COM_ParseExt(&p, qfalse);
	if(Q_stricmp(token,"MD5Version")) {
		T_Error("loadMD5Anim: Expected \"MD5Version\", found %s in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qfalse);
	if(Q_stricmp(token,"10")) {
		T_Error("loadMD5Anim: Expected \"10\", found %s in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// skip optional commandline
	token = COM_ParseExt(&p, qtrue);
	if(!Q_stricmp(token,"commandline")) {
		token = COM_ParseExt(&p, qfalse);
		token = COM_ParseExt(&p, qtrue);
	}

	// parse numFrames <number>
	if(Q_stricmp(token, "numFrames")) {
		T_Error("loadMD5Anim: expected \"numFrames\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumFrames = atoi(token);

	if(tmpNumFrames < 1) {
		T_Error("loadMD5Anim: \"%s\" has no frames\n", fname);
		return 0;
	}

	// parse numJoints <number>
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "numJoints")) {
		T_Error("loadMD5Anim: expected \"numJoints\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumJoints = atoi(token);

	if(tmpNumJoints < 1) {
		T_Error("loadMD5Anim: \"%s\" has no joints\n", fname);
		return 0;
	}

	// parse frameRate <number>
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "frameRate")) {
		T_Error("loadMD5Anim: expected \"frameRate\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpFrameRate = atof(token);

	// parse numAnimatedComponents <number>
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "numAnimatedComponents")) {
		T_Error("loadMD5Anim: expected \"numAnimatedComponents\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumAnimatedComponents = atoi(token);

	// okay, let's assume that md5anim file is valid, and alloc 
	// output animation. Still, it will be fried if any error occur
	out = malloc(sizeof(tAnim_t));
	
	out->numAnimatedComponents = tmpNumAnimatedComponents;
	out->numBones = tmpNumJoints;
	out->frameRate = tmpFrameRate;
	out->numFrames = tmpNumFrames;
	out->boneData = malloc(sizeof(tAnimBone_t)*out->numBones);
	out->baseFrame = malloc(sizeof(bone_t)*out->numBones);
	out->frames = malloc(sizeof(tFrame_t)*out->numFrames);

	// parse "hierarchy"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "hierarchy")) {
		T_Error("loadMD5Anim: expected \"hierarchy\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "{"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "{")) {
		T_Error("loadMD5Anim: expected \"{\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	boneDef = out->boneData;
	for(i = 0; i < out->numBones; i++,boneDef++) {
		// bone name (eg "Bip01 Spine1")
		token = COM_ParseExt(&p, qtrue);
		Q_strncpyz(boneDef->name, token, sizeof(boneDef->name));

		token = COM_ParseExt(&p, qtrue);
		boneDef->parent = atoi(token);

		T_VerbPrintf("loadMD5Anim: bone %i of %i - %s, parent %i\n",
			i,out->numBones,boneDef->name,boneDef->parent);

		token = COM_ParseExt(&p,qtrue);
		boneDef->componentBits = atoi(token);

		token = COM_ParseExt(&p,qtrue);
		boneDef->firstComponent = atoi(token);
	}

	// parse closing } of joints block
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "}")) {
		T_Error("loadMD5Anim: expected \"}\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "bounds"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "bounds")) {
		T_Error("loadMD5Anim: expected \"bounds\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "{"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "{")) {
		T_Error("loadMD5Anim: expected \"{\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	for(i = 0, frame = out->frames; i < out->numFrames; i++, frame++) {
		// parse mins
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Anim: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < 3; k++) {
			token = COM_ParseExt(&p, qtrue);
			frame->mins[k] = atof(token);
		}
	
		// skip )
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Anim: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// parse maxs
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Anim: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < 3; k++) {
			token = COM_ParseExt(&p, qfalse);
			frame->maxs[k] = atof(token);
		}
	
		// skip )
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Anim: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
	}

	// parse "}"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "}")) {
		T_Error("loadMD5Anim: expected \"}\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "baseframe"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "baseframe")) {
		T_Error("loadMD5Anim: expected \"baseframe\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "{"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "{")) {
		T_Error("loadMD5Anim: expected \"{\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	for(i = 0, bone = out->baseFrame; i < out->numBones; i++, bone++) {
		// parse ofs
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Anim: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < 3; k++) {
			token = COM_ParseExt(&p, qtrue);
			bone->p[k] = atof(token);
		}
	
		// skip )
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Anim: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// parse quat
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Anim: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < 3; k++) {
			token = COM_ParseExt(&p, qfalse);
			bone->q[k] = atof(token);
		}
	
		// skip )
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Anim: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
	}

	// skip }
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "}")) {
		T_Error("loadMD5Anim: expected \"}\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse frames
	for(i = 0, frame = out->frames; i < out->numFrames; i++, frame++) {
		// parse frame <number>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "frame")) {
			T_Error("loadMD5Anim: expected \"frame\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qtrue);
		if(atoi(token) != i) {
			T_Error("loadMD5Anim: expected \"frame %i\" found \"frame %s\" in file %s at line %i\n",
				i,token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		frame->components = malloc(sizeof(float)*out->numAnimatedComponents);
		// parse components block
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "{")) {
			T_Error("loadMD5Anim: expected \"{\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < out->numAnimatedComponents; k++) {
			token = COM_ParseExt(&p, qtrue);
			frame->components[k] = atof(token);
		}
	
		// skip  }
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "}")) {
			T_Error("loadMD5Anim: expected \"}\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		
	}

	T_Printf("Succesfully loaded %s\n",fname);
	return out;
}


/*
====================================================================

Doom3 md5mesh/md5anim ANIMATION code

====================================================================
*/

enum
{
	COMPONENT_BIT_TX = 1 << 0,
	COMPONENT_BIT_TY = 1 << 1,
	COMPONENT_BIT_TZ = 1 << 2,
	COMPONENT_BIT_QX = 1 << 3,
	COMPONENT_BIT_QY = 1 << 4,
	COMPONENT_BIT_QZ = 1 << 5
};

bone_t *setupMD5AnimBones(tAnim_t *a, int frameNum) {
	static bone_t bones[256];
	tFrame_t *f;
	int i;
	bone_t *baseBone;
	bone_t *out;
	tAnimBone_t *boneDef;

	if(a->numFrames <= frameNum) {
		T_Error("Frame index out of range (frame %i, numFrames %i, anim file %s\n",
			frameNum,a->numFrames,a->fname);
	}

	f = a->frames + frameNum;

	for(i = 0, baseBone = a->baseFrame, boneDef = a->boneData, out = bones;
		i < a->numBones; i++, baseBone++,boneDef++, out++) {
		vec3_t p;
		quat_t q;
		int componentsApplied;

		// set baseframe values
		VectorCopy(baseBone->p, p);
		QuatCopy(baseBone->q, q);

		componentsApplied = 0;

		// update tranlation bits
		if(boneDef->componentBits & COMPONENT_BIT_TX) {
			p[0] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		if(boneDef->componentBits & COMPONENT_BIT_TY) {
			p[1] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		if(boneDef->componentBits & COMPONENT_BIT_TZ) {
			p[2] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		// update quaternion rotation bits
		if(boneDef->componentBits & COMPONENT_BIT_QX) {
			((vec_t *) q)[0] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		if(boneDef->componentBits & COMPONENT_BIT_QY) {
			((vec_t *) q)[1] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		if(boneDef->componentBits & COMPONENT_BIT_QZ) {
			((vec_t *) q)[2] = f->components[boneDef->firstComponent + componentsApplied];
		}

		// calculate quaternion W value, as it isnt stored in md5 anim
		QuatCalcW(q);
		QuatNormalize(q);
		
		// copy out results
		VectorCopy(p,out->p);
		QuatCopy(q,out->q);
	}

	return bones;
}


bone_t *setupMD5MeshBones(tModel_t *mod) {
	static bone_t bones[256];
	int i;
	bone_t *out;
	bone_t *baseBone;
	tBone_t *bone;

	for(i = 0, bone = mod->bones, baseBone = mod->baseFrame, out = bones;
		i < mod->numBones; i++, bone++, baseBone++, out++) {
		matrix_t m;
		matrix_t pM;
		bone_t *baseParent;

		if(bone->parent == -1) {
			continue; // nothing to do.
		}

		baseParent = &mod->baseFrame[bone->parent];

		//
		// calculate bone position in local space
		// (relative to parent)
		//

		// get transformation matrix of this bone
		//MatrixFromQuat(m,baseBone->q);
		MatrixSetupTransformFromQuat(m,baseBone->q,baseBone->p);

		// get the inverse transform matrix of parent bone
		MatrixSetupTransformFromQuat(pM,baseParent->q,baseParent->p);
		MatrixInverse(pM);

		// multiple them
		MatrixMultiply2(m,pM);

		// convert result matrix back to quaternion and vector
		QuatFromMatrix(out->q,m);
		VectorCopy(&m[12],out->p);
	}

	return bones;
}








