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

static float null = 0;
void ConvertAnimation(tAnim_t *a, const char *outFName) {
	int i,j;
	tFrame_t *f;
	skcHeader_t h;
	FILE *out;
	bone_t *b;
	tAnimBone_t *bd;
	char name[SKC_MAX_CHANNEL_CHARS];
	int ofsChannelNames;

	out = fopen(outFName,"wb");
	if(out == 0) {
		T_Error("ConvertAnimation: Cannot open %s\n",outFName);
	}

	memset(&h,0,sizeof(h));
	h.ident = SKC_IDENT;
	h.version = SKC_VERSION;
	h.numFrames = a->numFrames;
	h.frameTime = 1.f / a->frameRate;
	h.numChannels = a->numBones * 2;

	fwrite(&h,sizeof(h),1,out);

	for(i = 0, f = a->frames; i < a->numFrames; i++, f++) {
		skcFrame_t outFrame;
		VectorCopy(f->mins,outFrame.bounds[0]);
		VectorCopy(f->maxs,outFrame.bounds[1]);
		outFrame.radius = RadiusFromBounds(f->mins,f->maxs);
		VectorSet(outFrame.delta,0,0,0);
		outFrame.unknown = 0;
		outFrame.ofsValues = sizeof(h) + sizeof(skcFrame_t) * a->numFrames 
			+ (a->numBones * sizeof(float)*8) * i;

		fwrite(&outFrame,sizeof(outFrame),1,out);
	}

	// write channel values
	for(i = 0, f = a->frames; i < a->numFrames; i++, f++) {
		bone_t *bones;

		bones = setupMD5AnimBones(a,i);

		for(j = 0, b = bones; j < a->numBones; j++, b++) {
			// first quat
			fwrite(&b->q[0],sizeof(quat_t),1,out);
			// then pos
			fwrite(&b->p[0],sizeof(vec3_t),1,out);
			fwrite(&null,sizeof(int),1,out);
		}
	}
	ofsChannelNames = ftell(out);
	// write channel names
	for(i = 0, bd = a->boneData; i < a->numBones; i++, bd++) {
		Q_strncpyz(name,bd->name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(name," rot");
		fwrite(name,SKC_MAX_CHANNEL_CHARS,1,out);
		Q_strncpyz(name,bd->name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(name," pos");
		fwrite(name,SKC_MAX_CHANNEL_CHARS,1,out);

	}

	// reupdate the header
	h.ofsEnd = ftell(out);
	h.ofsChannels = ofsChannelNames;
	fseek(out,0,SEEK_SET);
	fwrite(&h,sizeof(h),1,out);
	if(ftell(out) != sizeof(h)) {
		T_Error("Fatal file write error\n");
	}
	fclose(out);
}
void SplitSurfaces(tModel_t *m);
int countVertBytes(tSurf_t *sf) {
	int i;
	int r = sf->numVerts * sizeof(skdVertex_t);
	for(i = 0; i < sf->numVerts; i++) {
		r += (sizeof(skdWeight_t) * sf->verts[i].numWeights);
	}
	return r;
}
void ConvertModel(tModel_t *m, const char *outFName) {
	skdHeader_t h;
	int i,j,k;
	tBone_t *b;
	tSurf_t *s;
	tVert_t *v;
	tWeight_t *w;
	FILE *out;
	int afterBones;

	out = fopen(outFName,"wb");
	if(out == 0) {
		T_Error("ConvertModel: Cannot open %s\n",outFName);
	}

	if(noLimits == qfalse) {
		// ensure that there are no surfaces with
		// numVerts >= 1000 or numTris >= 2000
		SplitSurfaces(m);
	}

	memset(&h,0,sizeof(h));
	h.ident = SKD_IDENT;
	h.version = SKD_VERSION;
	h.numSurfaces = m->numSurfaces;
	h.numBones = m->numBones;
	h.ofsBones = sizeof(h);

	fwrite(&h,sizeof(h),1,out);

	for(i = 0, b = m->bones; i < m->numBones; i++,b++) {
		skdBone_t sb;
		char chanRot[SKC_MAX_CHANNEL_CHARS];
		char chanPos[SKC_MAX_CHANNEL_CHARS];
		int channelStringLen;
		vec3_t values;

		memset(&sb,0,sizeof(sb));
		sb.jointType = JT_POSROT_SKC;
		// bone name
		strcpy(sb.name,b->name);
		// parent name
		if(b->parent == -1) {
			strcpy(sb.parent,"worldbone");
		} else {
			strcpy(sb.parent,m->bones[b->parent].name);
		}
		
		// construct channel names
		Q_strncpyz(chanPos,sb.name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(chanPos," pos");

		Q_strncpyz(chanRot,sb.name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(chanRot," rot");

		channelStringLen = strlen(chanPos) + 1 
			+ strlen(chanRot) + 1;

		// calc offsets
		sb.ofsValues = sizeof(skdBone_t);
		sb.ofsChannels = sizeof(skdBone_t) + sizeof(vec3_t);
		sb.ofsEnd = sizeof(skdBone_t) + sizeof(vec3_t) +
			channelStringLen;

		sb.ofsRefs = sb.ofsEnd; // no references for classic POSROT bone.

		// write main bone data
		fwrite(&sb,sizeof(sb),1,out);	
		// write bone values 
		VectorSet(values,1.f,1.f,1.f);
		fwrite(&values,sizeof(values),1,out);	
		// write channel names 
		// su44 NOTE: ROT MUST GO FIRST.
		// (it will NOT work in MoHAA other way!)
		fwrite(chanRot,strlen(chanRot)+1,1,out);
		// .. then pos channel.
		fwrite(chanPos,strlen(chanPos)+1,1,out);
	}

	afterBones = ftell(out);

	for(i = 0, s = m->surfs; i < m->numSurfaces; i++,s++) {
		skdSurface_t sf;
		int numTriBytes;
		int numVertBytes;

		numTriBytes = s->numTris * sizeof(skdTriangle_t);
		//numVertBytes = s->numWeights * sizeof(skdWeight_t) +
		//	s->numVerts * sizeof(skdVertex_t);
		numVertBytes = countVertBytes(s);

		memset(&sf,0,sizeof(sf));
		sf.ident = SKD_SURFACE_IDENT;
		Q_strncpyz(sf.name,s->name,sizeof(sf.name));
		sf.numTriangles = s->numTris;
		sf.numVerts = s->numVerts;
		sf.ofsTriangles = sizeof(skdSurface_t);
		sf.ofsVerts = sizeof(skdSurface_t) + numTriBytes;
		sf.ofsEnd = sizeof(skdSurface_t) + numTriBytes + numVertBytes;
		
		// write surface
		fwrite(&sf,sizeof(sf),1,out);

		// write triangles
		fwrite(s->tris,s->numTris*sizeof(skdTriangle_t),1,out);

		// write vertices
		for(j = 0, v = s->verts; j < s->numVerts; j++, v++) {
			skdVertex_t sv;
			VectorSet(sv.normal,1,0,0); // FIXME!
			sv.numWeights = v->numWeights;
			sv.numMorphs = 0;
			sv.texCoords[0] = v->texCoords[0];
			sv.texCoords[1] = v->texCoords[1];
			fwrite(&sv,sizeof(sv),1,out);
			for(k = 0, w = v->weights; k < v->numWeights; k++, w++) {
				skdWeight_t sw;
				VectorCopy(w->offset,sw.offset);
				sw.boneIndex = w->boneNum;
				sw.boneWeight = w->boneWeight;
				fwrite(&sw,sizeof(sw),1,out);
			}
		}
	}

	// reupdate the header
	h.ofsEnd = ftell(out);
	h.ofsBoxes = h.ofsEnd;
	h.ofsMorphTargets = h.ofsEnd;
	h.ofsSurfaces = afterBones;
	fseek(out,0,SEEK_SET);
	fwrite(&h,sizeof(h),1,out);
	if(ftell(out) != sizeof(h)) {
		T_Error("Fatal file write error\n");
	}
	fclose(out);
}

int verts[8192];
tTri_t tris[8192];
int numVerts;
int numTris;
int regVert(int v) {
	int i;
	for(i = 0; i < numVerts; i++) {
		if(verts[i] == v)
			return i;
	}
	verts[numVerts] = v;
	numVerts++;
	return i;
}
void addTri(tTri_t t) {
	tTri_t n;

	n.indexes[0] = regVert(t.indexes[0]);
	n.indexes[1] = regVert(t.indexes[1]);
	n.indexes[2] = regVert(t.indexes[2]);

	tris[numTris] = n;

	numTris++;
}
void SplitSurfaces(tModel_t *m) {
	int i,j,k;
	tSurf_t *sf, *nsf, *oldSurfs;
	int oldNumSurfs;
	int add;
	int realNewSurfacesCount;

	sf = m->surfs;
	add = 0;

	for(i = 0; i < m->numSurfaces; i++, sf++) {
		int vertSplits;
		int triSplits;

		vertSplits = 0;
		triSplits = 0;

		// MoHAA cant handle more than 1000 vertices
		if(sf->numVerts > 1000) {
			int tmp = sf->numVerts;
			while(tmp > 1000) {
				vertSplits++;
				tmp -= 1000;
			}
		}
		// or more than 2000 triangles
		if(sf->numTris > 2000) {
			int tmp = sf->numTris;
			while(tmp > 2000) {
				triSplits++;
				tmp -= 2000;
			}
		}
		add += (vertSplits + triSplits);
	}

	if(add == 0)
		return;
	oldNumSurfs = m->numSurfaces;
	oldSurfs = m->surfs;

	realNewSurfacesCount = 0;

	m->numSurfaces += add;
	m->surfs = malloc(m->numSurfaces * sizeof(tSurf_t));
	memset(m->surfs,0,sizeof(tSurf_t)*m->numSurfaces);
	sf = oldSurfs;
	nsf = m->surfs;
	for(i = 0; i < oldNumSurfs; i++, sf++,nsf++,realNewSurfacesCount++) {
		if(sf->numVerts > 1000 || sf->numTris > 2000) {
			numTris = 0;
			numVerts = 0;
			// iterate all the tris while performing the splits
			for(j = 0; j < sf->numTris; j++) {
				addTri(sf->tris[j]);
				if(numVerts + 3 >= 1000 || numTris + 1 >= 2000) {
					// copy out existing data and continue splitting
					strcpy(nsf->name,sf->name);
					nsf->numTris = numTris;
					nsf->tris = malloc(sizeof(tTri_t)*numTris);
					memcpy(nsf->tris,tris,sizeof(tTri_t)*numTris);
					nsf->numVerts = numVerts;
					nsf->verts = malloc(sizeof(tVert_t)*numVerts);
					for(k = 0; k < numVerts; k++) {
						nsf->verts[k] = sf->verts[verts[k]];
					}
					realNewSurfacesCount++;
					nsf++; // next new surface
					numVerts = 0;
					numTris = 0;
				}
			}
			// copy out remaining data
			assert(numTris);
			assert(numVerts);
			strcpy(nsf->name,sf->name);
			nsf->numTris = numTris;
			nsf->tris = malloc(sizeof(tTri_t)*numTris);
			memcpy(nsf->tris,tris,sizeof(tTri_t)*numTris);
			nsf->numVerts = numVerts;
			nsf->verts = malloc(sizeof(tVert_t)*numVerts);
			for(k = 0; k < numVerts; k++) {
				nsf->verts[k] = sf->verts[verts[k]];
			}
			continue;
		}
		*nsf = *sf;
	}
	
	m->numSurfaces = realNewSurfacesCount;
 }

void stripExt(char *s) {
	int l;
	char *p;

	l = strlen(s);

	p = s + l;
	while(p != s) {
		if(*p == '.') {
			*p = 0;
			return;
		}
		p--;
	}
}
void Convert() {
	char fname[MAX_TOOLPATH];
	int i;
	tModel_t *m;
	tAnim_t *a;
	
	// load bone defs and surfaces
	m = loadMD5Mesh(inMD5Mesh);
	if(m == 0) {
		T_Error("Couldn't load %s\n",inMD5Mesh);
	}

	strcpy(fname,inMD5Mesh);
	stripExt(fname);
	strcat(fname,".skd");
	ConvertModel(m,fname);

	// load animations
	for(i = 0; i < numAnims; i++) {
		a = loadMD5Anim(inMD5Anim[i]);
		if(a == 0) {
			T_Printf("Failed to load: %s\n",inMD5Anim[i]);
		}
		md5Anims[i] = a;
	}

	// converts animations
	for(i = 0; i < numAnims; i++) {
		a = md5Anims[i];
		if(a == 0) {
			continue;
		}
		strcpy(fname,inMD5Anim[i]);
		stripExt(fname);
		strcat(fname,".skc");
		ConvertAnimation(a,fname);
	}
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
		} else {
			T_Printf("Unknown argument \"%s\"\n",argv[i]);
			// TODO: print help
		}
	}

	Convert();

	printf("Done.\n");
	// let the user see the results
	system("pause");
}