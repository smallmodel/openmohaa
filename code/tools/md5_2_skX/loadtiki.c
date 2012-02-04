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

// loadtiki.c - loading MoHAA .tik, .skc and .skd files.

#include "md5_2_skX.h"

int getBoneIndex(tModel_t *m, const char *s) {
	int i;

	if(!Q_stricmp(s,"worldbone")) {
		return -1;
	}
	for(i = 0; i < m->numBones; i++) {
		if(!Q_stricmp(m->bones[i].name,s)) {
			return i;
		}
	}
	T_Printf("Warning: cant find bone %s in %s\n",s,m->fname);
	return -1;
}
tModel_t *readSKD(const char *fname, float scale) {
	int len;
	skdHeader_t *h;
	skdSurface_t *sf;
	skdBone_t *b;
	tBone_t *ob;
	tModel_t *out;
	tSurf_t *os;
	int i, j, k;

	T_Printf("Loading MoHAA skd model file %s...\n",fname);

	len = F_LoadBuf(fname,(byte**)&h);

	if(len == -1) {
		T_Printf("readSKD: Cannot open %s\n",fname);
		return 0;
	}

	out = T_Malloc(sizeof(tModel_t));

	strcpy(out->fname,fname);

	out->numSurfaces = h->numSurfaces;
	out->numBones = h->numBones;
	out->surfs = T_Malloc(sizeof(tSurf_t)*out->numSurfaces);
	out->bones = T_Malloc(sizeof(tBone_t)*out->numBones);

	// load bones

	b = (skdBone_t *) ( (byte *)h + h->ofsBones );
	ob = out->bones;
	for ( i = 0; i < h->numBones; i++, ob++) {
		strcpy(ob->name,b->name);
		if(b->jointType != 1) {
			T_Error("readSKD: joints types other than 1 are not supported yet (skd file %s)\n",fname);
		}
		b = (skdBone_t *)( (byte *)b + b->ofsEnd );
	}		

	b = (skdBone_t *) ( (byte *)h + h->ofsBones );
	ob = out->bones;
	for ( i = 0; i < h->numBones; i++, ob++) {
		ob->parent = getBoneIndex(out,b->parent);
		b = (skdBone_t *)( (byte *)b + b->ofsEnd );
	}		

	// load surfaces
	sf = (skdSurface_t *) ( (byte *)h + h->ofsSurfaces );
	os = out->surfs;
	for ( i = 0; i < h->numSurfaces; i++,os++) {
		skdVertex_t *v;
		skdTriangle_t *t;
		tVert_t *ov;

		strcpy(os->name,sf->name);
		os->numTris = sf->numTriangles;
		os->numVerts = sf->numVerts;
		os->tris = T_Malloc(sizeof(tTri_t)*sf->numTriangles);
		os->verts = T_Malloc(sizeof(tVert_t)*sf->numVerts);

		// copy triangles
		t = (skdTriangle_t *) ( (byte *)sf + sf->ofsTriangles );
		assert(sizeof(skdTriangle_t) == sizeof(tTri_t));
		memcpy(os->tris,t,sizeof(skdTriangle_t)*sf->numTriangles);

		// copy and scale vertices data
		v = (skdVertex_t *) ( (byte *)sf + sf->ofsVerts );
		ov = os->verts;
		for ( j = 0; j < sf->numVerts; j++,ov++) {
			skdWeight_t *w;
			tWeight_t *ow;
	
			ov->numWeights = v->numWeights;
			VectorCopy(v->normal,ov->normal);
			ov->texCoords[0] = v->texCoords[0];
			ov->texCoords[1] = v->texCoords[1];
			ov->weights = T_Malloc(sizeof(tWeight_t)*ov->numWeights);

			w = (skdWeight_t *) ( (byte *)v + sizeof(skdVertex_t)+(sizeof(skdMorph_t)*v->numMorphs));
			ow = ov->weights;
			for ( k = 0; k < v->numWeights; k++,ow++) {
				VectorScale(w->offset,scale,ow->offset);
				if(w->boneWeight<0 || w->boneWeight>1) {
					T_Error("invalid bone weight %f in %s \n",w->boneWeight,out->fname);
				}
				ow->boneWeight = w->boneWeight;
				ow->boneNum = w->boneIndex;
				w = (skdWeight_t *) ( (byte *)w + sizeof(skdWeight_t));
			}
			v = (skdVertex_t *) ( (byte *)v + sizeof(skdVertex_t) + v->numWeights*sizeof(skdWeight_t) + v->numMorphs*sizeof(skdMorph_t) );
		}
		sf = (skdSurface_t *)( (byte *)sf + sf->ofsEnd );
	}

	F_FreeBuf((byte*)h);

	T_Printf("Succesfully loaded MoHAA model %s\n",fname);

	return out;
}
int getChannelIndexInternal(skcHeader_t *h, const char *channelName) {
	const char *c;
	int i;

	c = ( (byte *)h + h->ofsChannels );
	for(i = 0; i < h->numChannels; i++) {
		if(!Q_stricmp(c,channelName)) 
			return i;
		c += SKC_MAX_CHANNEL_CHARS;
	}

	return -1;
}
float *getChannelValue(skcHeader_t *h, const char *name, int frameNum) {
	int channelIndex;
	skcFrame_t *f;
	float *values;

	channelIndex = getChannelIndexInternal(h,name);

	if(channelIndex == -1)
		return 0;

	f = (skcFrame_t *)( (byte *)h + sizeof(*h) + sizeof(*f) * frameNum );

	values = (float*)((byte *)h+f->ofsValues);

	return &values[4 * channelIndex];
}
float *findPosChannel(skcHeader_t *h, const char *name, int frameNum) {
	char channelName[32];
	strcpy(channelName,name);
	strcat(channelName," pos");
	return getChannelValue(h,channelName,frameNum);
}
float *findRotChannel(skcHeader_t *h, const char *name, int frameNum) {
#if 0
	char channelName[32];
	strcpy(channelName,name);
	strcat(channelName," rot");
	return getChannelValue(h,channelName,frameNum);
#else
static int i = 0;
	static quat_t qs[256];
	float *q;
	char channelName[32];
	float *f;
	
	i++;
	i %= 256;
	q = qs[i];

	strcpy(channelName,name);
	strcat(channelName," rot");
	f = getChannelValue(h,channelName,frameNum);
	QuatCopy(f,q);
	QuatInverse(q); // inverse quaterion for Doom 3
#if 1
	// make sure that we will get the same 
	// quaternion if we recalculate W component...
	{
		quat_t copy;
		QuatCopy(q,copy);
		QuatCalcW(q);
		if(abs(q[3] - copy[3]) > 0.1) {
			__asm int 3
		}
	}
#endif
	return q;
#endif
}
tAnim_t *appendSKC(tModel_t *m, const char *fname, float scale) {
	int len;
	skcHeader_t *h;
	skcFrame_t *f, *firstFrame;
	tAnim_t *out;
	tFrame_t *of;
	const char *c;
	int i, j;
	int cFlags[512];
	bone_t baseFrame[512];
	int numAnimatedComponents;

	T_Printf("Loading MoHAA skc animation file %s...\n",fname);

	len = F_LoadBuf(fname,(byte**)&h);

	if(len == -1) {
		T_Printf("Cannot open %s\n",fname);
		return 0;
	}

	memset(cFlags,0,sizeof(cFlags));

	out = T_Malloc(sizeof(tAnim_t));
	out->frameRate = 1.f / h->frameTime;
	out->numBones = m->numBones;
	out->numFrames = h->numFrames;
	out->frames = T_Malloc(sizeof(tFrame_t)*h->numFrames);
	out->boneData = T_Malloc(sizeof(tAnimBone_t)*m->numBones);

	// copy frame bounding boxes
	f = (skcFrame_t *)( (byte *)h + sizeof(*h) );
	of = out->frames;
	for(i = 0; i < h->numFrames; i++,of++,f++) {
		//anim->frames[i].radius = f->radius;
		VectorCopy(f->bounds[1],of->maxs);
		VectorCopy(f->bounds[0],of->mins);
	}

	// detect which components changes
	for(j = 0; j < m->numBones; j++) {
		float *baseRot, *testRot;
		float *basePos, *testPos;

		basePos = findPosChannel(h,m->bones[j].name,0);
		if(basePos == 0) {
			VectorSet(baseFrame[j].p,0,0,0);
		} else {
			VectorCopy(basePos,baseFrame[j].p);
			for(i = 1; i < h->numFrames; i++) {
				testPos = findPosChannel(h,m->bones[j].name,i);
				// detect X change
				if(testPos[0] != basePos[0]) {
					cFlags[j] |= COMPONENT_BIT_TX;
				}
				// detect Y change
				if(testPos[1] != basePos[1]) {
					cFlags[j] |= COMPONENT_BIT_TY;
				}
				// detect Z change
				if(testPos[2] != basePos[2]) {
					cFlags[j] |= COMPONENT_BIT_TZ;
				}
			}	
		}

		baseRot = findRotChannel(h,m->bones[j].name,0);
		if(baseRot == 0) {
			QuatSet(baseFrame[j].q,0,0,0,1);
		} else {
			QuatCopy(baseRot,baseFrame[j].q);
			for(i = 1; i < h->numFrames; i++) {
				testRot = findRotChannel(h,m->bones[j].name,i);
				// detect X change
				if(testRot[0] != baseRot[0]) {
					cFlags[j] |= COMPONENT_BIT_QX;
				}
				// detect Y change
				if(testRot[1] != baseRot[1]) {
					cFlags[j] |= COMPONENT_BIT_QY;
				}
				// detect Z change
				if(testRot[2] != baseRot[2]) {
					cFlags[j] |= COMPONENT_BIT_QZ;
				}
				// NOTE: quaternion W component is not stored at all in md5 files
			}	
		}
	}

	// count the number of animated components and copy some bone data
	numAnimatedComponents = 0;
	for(j = 0; j < m->numBones; j++) {
		//int c;

		out->boneData[j].firstComponent = numAnimatedComponents;
		//c = 0;

		for(i = 0; i < 6; i++) {
			if(cFlags[j] & (1 << i)) {
				numAnimatedComponents++;
			//	c++;
			}
		}

		//out->boneData[j].numAnimatedComponents = c;
		out->boneData[j].componentBits = cFlags[j];
		strcpy(out->boneData[j].name,m->bones[j].name);
		out->boneData[j].parent = m->bones[j].parent;
	}

	// copy results out
	out->baseFrame = T_Malloc(sizeof(bone_t)*m->numBones);
	memcpy(out->baseFrame,baseFrame,sizeof(bone_t)*m->numBones);
	out->numAnimatedComponents = numAnimatedComponents;
	of = out->frames;
	for(i = 0; i < h->numFrames; i++,of++) {
		int c;
		float *cp;

		cp = of->components = T_Malloc(numAnimatedComponents*sizeof(float));

		//c = 0;
		for(j = 0; j < m->numBones; j++) {
			float *pos, *rot;

			pos = findPosChannel(h,m->bones[j].name,i);
			// write X change
			if(cFlags[j] & COMPONENT_BIT_TX) {
				*cp = pos[0];
				cp++;
			}
			// write Y change
			if(cFlags[j] & COMPONENT_BIT_TY) {
				*cp = pos[1];
				cp++;
			}
			// write Z change
			if(cFlags[j] & COMPONENT_BIT_TZ) {
				*cp = pos[2];
				cp++;
			}

			rot = findRotChannel(h,m->bones[j].name,i);
			// write X change
			if(cFlags[j] & COMPONENT_BIT_QX) {
				*cp = rot[0];
				cp++;
			}
			// write Y change
			if(cFlags[j] & COMPONENT_BIT_QY) {
				*cp = rot[1];
				cp++;
			}
			// write Z change
			if(cFlags[j] & COMPONENT_BIT_QZ) {
				*cp = rot[2];
				cp++;
			}				
		}

		c = cp - of->components;
		assert(c == numAnimatedComponents);
	}

	F_FreeBuf((byte*)h);

	T_Printf("Succesfully loaded MoHAA animation %s\n",fname);

	return out;
}

// TIKI loading.
// For the purpose of this exporter,
// we need to load setup and animation section.
// Include keywords are currently ignored.

void loadTIKI(const char *fname) {


}
























