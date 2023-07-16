/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_main.c -- main control flow for each frame

#include "tr_local.h"
#include "tiki.h"

trGlobals_t		tr;

static float	s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


refimport_t	ri;

// entities that will have procedurally generated surfaces will just
// point at this for their sorting surface
surfaceType_t	entitySurface = SF_ENTITY;

/*
=================
R_CullLocalBox

Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
int R_CullLocalBoxOffset(const vec3_t offset, vec3_t bounds[2]) {
	int		i, j;
	vec3_t	transformed[8];
	float	dists[8];
	vec3_t	v;
	cplane_t	*frust;
	int			anyBack;
	int			front, back;

	if ( r_nocull->integer ) {
		return CULL_CLIP;
	}

	// transform into world space
	for (i = 0 ; i < 8 ; i++) {
		v[0] = bounds[i&1][0];
		v[1] = bounds[(i>>1)&1][1];
		v[2] = bounds[(i>>2)&1][2];

		VectorCopy( tr.ori.origin, transformed[i] );
		VectorAdd(transformed[i], offset, transformed[i]);
		VectorMA( transformed[i], v[0], tr.ori.axis[0], transformed[i] );
		VectorMA( transformed[i], v[1], tr.ori.axis[1], transformed[i] );
		VectorMA( transformed[i], v[2], tr.ori.axis[2], transformed[i] );
	}

	// check against frustum planes
	anyBack = 0;
	for (i = 0 ; i < tr.viewParms.fog.extrafrustums + 4 ; i++) {
		frust = &tr.viewParms.frustum[i];

		front = back = 0;
		for (j = 0 ; j < 8 ; j++) {
			dists[j] = DotProduct(transformed[j], frust->normal);
			if ( dists[j] > frust->dist ) {
				front = 1;
				if ( back ) {
					break;		// a point is in front
				}
			} else {
				back = 1;
			}
		}
		if ( !front ) {
			// all points were behind one of the planes
			return CULL_OUT;
		}
		anyBack |= back;
	}

	if ( !anyBack ) {
		return CULL_IN;		// completely inside frustum
	}

	return CULL_CLIP;		// partially clipped
}

int R_CullLocalBox(vec3_t bounds[2]) {
	return R_CullLocalBoxOffset(vec3_origin, bounds);
}

/*
** R_CullLocalPointAndRadius
*/
int R_CullLocalPointAndRadius( vec3_t pt, float radius )
{
	vec3_t transformed;

	R_LocalPointToWorld( pt, transformed );

	return R_CullPointAndRadius( transformed, radius );
}

/*
** R_CullPointAndRadius
*/
int R_CullPointAndRadius( vec3_t pt, float radius )
{
	int		i;
	float	dist;
	cplane_t	*frust;
	qboolean mightBeClipped = qfalse;

	if ( r_nocull->integer ) {
		return CULL_CLIP;
	}

	// check against frustum planes
	for (i = 0 ; i < tr.viewParms.fog.extrafrustums + 4; i++)
	{
		frust = &tr.viewParms.frustum[i];

		dist = DotProduct( pt, frust->normal) - frust->dist;
		if ( dist < -radius )
		{
			return CULL_OUT;
		}
		else if ( dist <= radius ) 
		{
			mightBeClipped = qtrue;
		}
	}

	if ( mightBeClipped )
	{
		return CULL_CLIP;
	}

	return CULL_IN;		// completely inside frustum
}

int R_DistanceCullLocalPointAndRadius(float fDist, const vec3_t pt, float radius) {
	vec3_t transformed;

	R_LocalPointToWorld(pt, transformed);

	return R_DistanceCullPointAndRadius(fDist, transformed, radius);
}

int R_DistanceCullPointAndRadius(float fDist, const vec3_t pt, float radius) {
	if (!r_nocull->integer)
	{
		vec3_t vDelta;
		float fLengthSquared;
		float fTotalDistSquared;

		VectorSubtract(pt, tr.viewParms.ori.origin, vDelta);
		fLengthSquared = VectorLengthSquared(vDelta);
		fTotalDistSquared = (fDist + radius) * (fDist + radius);

		if ((fDist + radius) * (fDist + radius) < vDelta[2] + fLengthSquared) {
			return CULL_OUT;
		} else if ((fDist - radius) * (fDist - radius) > vDelta[2] + fLengthSquared) {
			return CULL_IN;
		}
	}

	return CULL_CLIP;
	// FIXME: unimplemented
}

/*
=================
R_LocalNormalToWorld

=================
*/
void R_LocalNormalToWorld (const vec3_t local, vec3_t world) {
	world[0] = local[0] * tr.ori.axis[0][0] + local[1] * tr.ori.axis[1][0] + local[2] * tr.ori.axis[2][0];
	world[1] = local[0] * tr.ori.axis[0][1] + local[1] * tr.ori.axis[1][1] + local[2] * tr.ori.axis[2][1];
	world[2] = local[0] * tr.ori.axis[0][2] + local[1] * tr.ori.axis[1][2] + local[2] * tr.ori.axis[2][2];
}

/*
=================
R_LocalPointToWorld

=================
*/
void R_LocalPointToWorld (const vec3_t local, vec3_t world) {
	world[0] = local[0] * tr.ori.axis[0][0] + local[1] * tr.ori.axis[1][0] + local[2] * tr.ori.axis[2][0] + tr.ori.origin[0];
	world[1] = local[0] * tr.ori.axis[0][1] + local[1] * tr.ori.axis[1][1] + local[2] * tr.ori.axis[2][1] + tr.ori.origin[1];
	world[2] = local[0] * tr.ori.axis[0][2] + local[1] * tr.ori.axis[1][2] + local[2] * tr.ori.axis[2][2] + tr.ori.origin[2];
}

/*
=================
R_WorldToLocal

=================
*/
void R_WorldToLocal (vec3_t world, vec3_t local) {
	local[0] = DotProduct(world, tr.ori.axis[0]);
	local[1] = DotProduct(world, tr.ori.axis[1]);
	local[2] = DotProduct(world, tr.ori.axis[2]);
}

/*
==========================
R_TransformModelToClip

==========================
*/
void R_TransformModelToClip( const vec3_t src, const float *modelMatrix, const float *projectionMatrix,
							vec4_t eye, vec4_t dst ) {
	int i;

	for ( i = 0 ; i < 4 ; i++ ) {
		eye[i] = 
			src[0] * modelMatrix[ i + 0 * 4 ] +
			src[1] * modelMatrix[ i + 1 * 4 ] +
			src[2] * modelMatrix[ i + 2 * 4 ] +
			1 * modelMatrix[ i + 3 * 4 ];
	}

	for ( i = 0 ; i < 4 ; i++ ) {
		dst[i] = 
			eye[0] * projectionMatrix[ i + 0 * 4 ] +
			eye[1] * projectionMatrix[ i + 1 * 4 ] +
			eye[2] * projectionMatrix[ i + 2 * 4 ] +
			eye[3] * projectionMatrix[ i + 3 * 4 ];
	}
}

/*
==========================
R_TransformClipToWindow

==========================
*/
void R_TransformClipToWindow( const vec4_t clip, const viewParms_t *view, vec4_t normalized, vec4_t window ) {
	normalized[0] = clip[0] / clip[3];
	normalized[1] = clip[1] / clip[3];
	normalized[2] = ( clip[2] + clip[3] ) / ( 2 * clip[3] );

	window[0] = 0.5f * ( 1.0f + normalized[0] ) * view->viewportWidth;
	window[1] = 0.5f * ( 1.0f + normalized[1] ) * view->viewportHeight;
	window[2] = normalized[2];

	window[0] = (int) ( window[0] + 0.5 );
	window[1] = (int) ( window[1] + 0.5 );
}


/*
==========================
myGlMultMatrix

==========================
*/
void myGlMultMatrix( const float *a, const float *b, float *out ) {
	int		i, j;

	for ( i = 0 ; i < 4 ; i++ ) {
		for ( j = 0 ; j < 4 ; j++ ) {
			out[ i * 4 + j ] =
				a [ i * 4 + 0 ] * b [ 0 * 4 + j ]
				+ a [ i * 4 + 1 ] * b [ 1 * 4 + j ]
				+ a [ i * 4 + 2 ] * b [ 2 * 4 + j ]
				+ a [ i * 4 + 3 ] * b [ 3 * 4 + j ];
		}
	}
}

void R_AdjustVisBoundsForSprite(refSprite_t* ent, viewParms_t* viewParms, orientationr_t* or )
{
	if (tr.viewParms.visBounds[0][0] > ent->origin[0] - ent->scale) {
		tr.viewParms.visBounds[0][0] = ent->origin[0] - ent->scale;
    }
    if (tr.viewParms.visBounds[0][1] > ent->origin[1] - ent->scale) {
        tr.viewParms.visBounds[0][1] = ent->origin[1] - ent->scale;
    }
    if (tr.viewParms.visBounds[0][2] > ent->origin[2] - ent->scale) {
        tr.viewParms.visBounds[0][2] = ent->origin[2] - ent->scale;
    }

	if (tr.viewParms.visBounds[1][0] < ent->origin[0] + ent->scale) {
        tr.viewParms.visBounds[1][0] = ent->origin[0] + ent->scale;
    }
    if (tr.viewParms.visBounds[1][1] < ent->origin[1] + ent->scale) {
        tr.viewParms.visBounds[1][1] = ent->origin[1] + ent->scale;
    }
    if (tr.viewParms.visBounds[1][2] < ent->origin[2] + ent->scale) {
        tr.viewParms.visBounds[1][2] = ent->origin[2] + ent->scale;
    }
}

/*
=================
R_RotateForEntity

Generates an orientation for an entity and viewParms
Does NOT produce any GL calls
Called by both the front end and the back end
=================
*/
void R_RotateForEntity( const trRefEntity_t *ent, const viewParms_t *viewParms,
					   orientationr_t *ori ) {
	float	glMatrix[16];
	vec3_t	delta;
	float	axisLength;

	if ( ent->e.reType != RT_MODEL ) {
		*ori = viewParms->world;
		return;
	}

	VectorCopy( ent->e.origin, ori->origin );

	VectorCopy( ent->e.axis[0], ori->axis[0] );
	VectorCopy( ent->e.axis[1], ori->axis[1] );
	VectorCopy( ent->e.axis[2], ori->axis[2] );

	glMatrix[0] = ori->axis[0][0];
	glMatrix[4] = ori->axis[1][0];
	glMatrix[8] = ori->axis[2][0];
	glMatrix[12] = ori->origin[0];

	glMatrix[1] = ori->axis[0][1];
	glMatrix[5] = ori->axis[1][1];
	glMatrix[9] = ori->axis[2][1];
	glMatrix[13] = ori->origin[1];

	glMatrix[2] = ori->axis[0][2];
	glMatrix[6] = ori->axis[1][2];
	glMatrix[10] = ori->axis[2][2];
	glMatrix[14] = ori->origin[2];

	glMatrix[3] = 0;
	glMatrix[7] = 0;
	glMatrix[11] = 0;
	glMatrix[15] = 1;

	myGlMultMatrix( glMatrix, viewParms->world.modelMatrix, ori->modelMatrix );

	// calculate the viewer origin in the model's space
	// needed for fog, specular, and environment mapping
	VectorSubtract( viewParms->ori.origin, ori->origin, delta );

	// compensate for scale in the axes if necessary
	if ( ent->e.nonNormalizedAxes ) {
		axisLength = VectorLength( ent->e.axis[0] );
		if ( !axisLength ) {
			axisLength = 0;
		} else {
			axisLength = 1.0f / axisLength;
		}
	} else {
		axisLength = 1.0f;
	}

	ori->viewOrigin[0] = DotProduct( delta, ori->axis[0] ) * axisLength;
	ori->viewOrigin[1] = DotProduct( delta, ori->axis[1] ) * axisLength;
	ori->viewOrigin[2] = DotProduct( delta, ori->axis[2] ) * axisLength;
}

/*
=================
R_RotateForStaticModel

Generates an orientation for a static model and viewParms
=================
*/
void R_RotateForStaticModel( cStaticModelUnpacked_t *SM, const viewParms_t *viewParms,
					   orientationr_t *ori ) {
	float	glMatrix[16];
	vec3_t	delta;
	float	tiki_scale;

	tiki_scale = SM->tiki->load_scale * SM->scale;

	VectorCopy( SM->origin, ori->origin );

	VectorCopy( SM->axis[0], ori->axis[0] );
	VectorCopy( SM->axis[1], ori->axis[1] );
	VectorCopy( SM->axis[2], ori->axis[2] );

	glMatrix[0]  = ori->axis[0][0] * tiki_scale;
	glMatrix[4]  = ori->axis[1][0] * tiki_scale;
	glMatrix[8]  = ori->axis[2][0] * tiki_scale;
	glMatrix[12] = ori->origin[0];

	glMatrix[1]  = ori->axis[0][1] * tiki_scale;
	glMatrix[5]  = ori->axis[1][1] * tiki_scale;
	glMatrix[9]  = ori->axis[2][1] * tiki_scale;
	glMatrix[13] = ori->origin[1];

	glMatrix[2]  = ori->axis[0][2] * tiki_scale;
	glMatrix[6]  = ori->axis[1][2] * tiki_scale;
	glMatrix[10] = ori->axis[2][2] * tiki_scale;
	glMatrix[14] = ori->origin[2];

	glMatrix[3]  = 0;
	glMatrix[7]  = 0;
	glMatrix[11] = 0;
	glMatrix[15] = 1;

	myGlMultMatrix( glMatrix, viewParms->world.modelMatrix, ori->modelMatrix );

	// calculate the viewer origin in the model's space
	// needed for fog, specular, and environment mapping
	VectorSubtract( viewParms->ori.origin, ori->origin, delta );

	ori->viewOrigin[0] = DotProduct( delta, ori->axis[0] );
	ori->viewOrigin[1] = DotProduct( delta, ori->axis[1] );
	ori->viewOrigin[2] = DotProduct( delta, ori->axis[2] );
}

/*
=================
R_RotateForViewer

Sets up the modelview matrix for a given viewParm
=================
*/
void R_RotateForViewer (void) 
{
	float	viewerMatrix[16];
	vec3_t	origin;

	Com_Memset (&tr.ori, 0, sizeof(tr.ori));
	tr.ori.axis[0][0] = 1;
	tr.ori.axis[1][1] = 1;
	tr.ori.axis[2][2] = 1;
	VectorCopy (tr.viewParms.ori.origin, tr.ori.viewOrigin);

	// transform by the camera placement
	VectorCopy( tr.viewParms.ori.origin, origin );

	viewerMatrix[0] = tr.viewParms.ori.axis[0][0];
	viewerMatrix[4] = tr.viewParms.ori.axis[0][1];
	viewerMatrix[8] = tr.viewParms.ori.axis[0][2];
	viewerMatrix[12] = -origin[0] * viewerMatrix[0] + -origin[1] * viewerMatrix[4] + -origin[2] * viewerMatrix[8];

	viewerMatrix[1] = tr.viewParms.ori.axis[1][0];
	viewerMatrix[5] = tr.viewParms.ori.axis[1][1];
	viewerMatrix[9] = tr.viewParms.ori.axis[1][2];
	viewerMatrix[13] = -origin[0] * viewerMatrix[1] + -origin[1] * viewerMatrix[5] + -origin[2] * viewerMatrix[9];

	viewerMatrix[2] = tr.viewParms.ori.axis[2][0];
	viewerMatrix[6] = tr.viewParms.ori.axis[2][1];
	viewerMatrix[10] = tr.viewParms.ori.axis[2][2];
	viewerMatrix[14] = -origin[0] * viewerMatrix[2] + -origin[1] * viewerMatrix[6] + -origin[2] * viewerMatrix[10];

	viewerMatrix[3] = 0;
	viewerMatrix[7] = 0;
	viewerMatrix[11] = 0;
	viewerMatrix[15] = 1;

	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	myGlMultMatrix( viewerMatrix, s_flipMatrix, tr.ori.modelMatrix );

	tr.viewParms.world = tr.ori;

}

/*
** SetFarClip
*/
static void SetFarClip( void )
{
	float	farthestCornerDistance = 0;
	int		i;

	// if not rendering the world (icons, menus, etc)
	// set a 2k far clip plane
	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		tr.viewParms.zFar = 2048;
		return;
	}

	//
	// set far clipping planes dynamically
	//
	farthestCornerDistance = 0;
	for ( i = 0; i < 8; i++ )
	{
		vec3_t v;
		vec3_t vecTo;
		float distance;

		if ( i & 1 )
		{
			v[0] = tr.viewParms.visBounds[0][0];
		}
		else
		{
			v[0] = tr.viewParms.visBounds[1][0];
		}

		if ( i & 2 )
		{
			v[1] = tr.viewParms.visBounds[0][1];
		}
		else
		{
			v[1] = tr.viewParms.visBounds[1][1];
		}

		if ( i & 4 )
		{
			v[2] = tr.viewParms.visBounds[0][2];
		}
		else
		{
			v[2] = tr.viewParms.visBounds[1][2];
		}

		VectorSubtract( v, tr.viewParms.ori.origin, vecTo );

		distance = vecTo[0] * vecTo[0] + vecTo[1] * vecTo[1] + vecTo[2] * vecTo[2];

		if ( distance > farthestCornerDistance )
		{
			farthestCornerDistance = distance;
		}
	}
	tr.viewParms.zFar = sqrt( farthestCornerDistance );
}


/*
===============
R_SetupProjection
===============
*/
void R_SetupProjection( void ) {
	float	xmin, xmax, ymin, ymax;
	float	width, height, depth;
	float	zNear, zFar;

	// dynamically compute far clip plane distance
	SetFarClip();

	//
	// set up projection matrix
	//
	zNear	= r_znear->value;
	zFar	= tr.viewParms.zFar;

	ymax = zNear * tan( tr.refdef.fov_y * M_PI / 360.0f );
	ymin = -ymax;

	xmax = zNear * tan( tr.refdef.fov_x * M_PI / 360.0f );
	xmin = -xmax;

	width = xmax - xmin;
	height = ymax - ymin;
	depth = zFar - zNear;

	tr.viewParms.projectionMatrix[0] = 2 * zNear / width;
	tr.viewParms.projectionMatrix[4] = 0;
	tr.viewParms.projectionMatrix[8] = ( xmax + xmin ) / width;	// normally 0
	tr.viewParms.projectionMatrix[12] = 0;

	tr.viewParms.projectionMatrix[1] = 0;
	tr.viewParms.projectionMatrix[5] = 2 * zNear / height;
	tr.viewParms.projectionMatrix[9] = ( ymax + ymin ) / height;	// normally 0
	tr.viewParms.projectionMatrix[13] = 0;

	tr.viewParms.projectionMatrix[2] = 0;
	tr.viewParms.projectionMatrix[6] = 0;
	tr.viewParms.projectionMatrix[10] = -( zFar + zNear ) / depth;
	tr.viewParms.projectionMatrix[14] = -2 * zFar * zNear / depth;

	tr.viewParms.projectionMatrix[3] = 0;
	tr.viewParms.projectionMatrix[7] = 0;
	tr.viewParms.projectionMatrix[11] = -1;
	tr.viewParms.projectionMatrix[15] = 0;
}

/*
=================
R_SetupFrustum

Setup that culling frustum planes for the current view
=================
*/
void R_SetupFrustum (void) {
	int		i;
	float	xs, xc;
	float	ang;

	ang = tr.viewParms.fovX / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	VectorScale( tr.viewParms.ori.axis[0], xs, tr.viewParms.frustum[0].normal );
	VectorMA( tr.viewParms.frustum[0].normal, xc, tr.viewParms.ori.axis[1], tr.viewParms.frustum[0].normal );

	VectorScale( tr.viewParms.ori.axis[0], xs, tr.viewParms.frustum[1].normal );
	VectorMA( tr.viewParms.frustum[1].normal, -xc, tr.viewParms.ori.axis[1], tr.viewParms.frustum[1].normal );

	ang = tr.viewParms.fovY / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	VectorScale( tr.viewParms.ori.axis[0], xs, tr.viewParms.frustum[2].normal );
	VectorMA( tr.viewParms.frustum[2].normal, xc, tr.viewParms.ori.axis[2], tr.viewParms.frustum[2].normal );

	VectorScale( tr.viewParms.ori.axis[0], xs, tr.viewParms.frustum[3].normal );
	VectorMA( tr.viewParms.frustum[3].normal, -xc, tr.viewParms.ori.axis[2], tr.viewParms.frustum[3].normal );

	for (i=0 ; i<4 ; i++) {
		tr.viewParms.frustum[i].type = PLANE_NON_AXIAL;
		tr.viewParms.frustum[i].dist = DotProduct (tr.viewParms.ori.origin, tr.viewParms.frustum[i].normal);
		SetPlaneSignbits( &tr.viewParms.frustum[i] );
	}

	if (tr.portalsky.inUse) {
		tr.viewParms.farplane_distance = 0.0;
	}
	else if (r_farplane->integer)
	{
		tr.viewParms.farplane_distance = r_farplane->value;

		sscanf(
			r_farplane_color->string,
			"%f %f %f",
			&tr.viewParms.farplane_color[0],
			&tr.viewParms.farplane_color[1],
			&tr.viewParms.farplane_color[2]);

		tr.viewParms.farplane_cull = !r_farplane_nocull->integer;
	}

	if (tr.viewParms.farplane_distance)
	{
		vec3_t farPoint;
		float realPlaneLen;
		float tmp;
		vec4_t fogColor;

		tr.viewParms.fog.len = tr.viewParms.farplane_distance;
		tr.viewParms.fog.oolen = 1.0 / tr.viewParms.farplane_distance;

		farPoint[0] = tr.viewParms.ori.origin[0] + tr.viewParms.ori.axis[0][0] * tr.viewParms.farplane_distance;
		farPoint[1] = tr.viewParms.ori.origin[1] + tr.viewParms.ori.axis[0][1] * tr.viewParms.farplane_distance;
		farPoint[2] = tr.viewParms.ori.origin[2] + tr.viewParms.ori.axis[0][2] * tr.viewParms.farplane_distance;

		VectorNegate(tr.viewParms.ori.axis[0], tr.viewParms.frustum[4].normal);
		tr.viewParms.frustum[4].type = PLANE_NON_AXIAL;
		tr.viewParms.frustum[4].dist = DotProduct(farPoint, tr.viewParms.frustum[4].normal);
		SetPlaneSignbits(&tr.viewParms.frustum[4]);

		tr.viewParms.fog.enabled = r_farplane_nofog->integer == 0;
		tr.viewParms.fog.extrafrustums = tr.viewParms.farplane_cull ? 1 : 0;

		if (!r_farplane_nofog->integer)
		{
			qglFogf(GL_FOG_END, tr.viewParms.farplane_distance);

			fogColor[0] = tr.viewParms.farplane_color[0] * tr.identityLight;
			fogColor[1] = tr.viewParms.farplane_color[1] * tr.identityLight;
			fogColor[2] = tr.viewParms.farplane_color[2] * tr.identityLight;
			fogColor[3] = 1.0;

			GL_SetFogColor(fogColor);
			glState.externalSetState |= GLS_FOG;
		}
		else
		{
			glState.externalSetState &= ~GLS_FOG;
		}
	}
	else
	{
		tr.viewParms.fog.enabled = 0;
		tr.viewParms.fog.extrafrustums = 0;

		glState.externalSetState &= ~GLS_FOG;
	}
}


/*
=================
R_MirrorPoint
=================
*/
void R_MirrorPoint (vec3_t in, orientation_t *surface, orientation_t *camera, vec3_t out) {
	int		i;
	vec3_t	local;
	vec3_t	transformed;
	float	d;

	VectorSubtract( in, surface->origin, local );

	VectorClear( transformed );
	for ( i = 0 ; i < 3 ; i++ ) {
		d = DotProduct(local, surface->axis[i]);
		VectorMA( transformed, d, camera->axis[i], transformed );
	}

	VectorAdd( transformed, camera->origin, out );
}

void R_MirrorVector (vec3_t in, orientation_t *surface, orientation_t *camera, vec3_t out) {
	int		i;
	float	d;

	VectorClear( out );
	for ( i = 0 ; i < 3 ; i++ ) {
		d = DotProduct(in, surface->axis[i]);
		VectorMA( out, d, camera->axis[i], out );
	}
}


/*
=============
R_PlaneForSurface
=============
*/
void R_PlaneForSurface (surfaceType_t *surfType, cplane_t *plane) {
	srfTriangles_t	*tri;
	srfPoly_t		*poly;
	drawVert_t		*v1, *v2, *v3;
	vec4_t			plane4;

	if (!surfType) {
		Com_Memset (plane, 0, sizeof(*plane));
		plane->normal[0] = 1;
		return;
	}
	switch (*surfType) {
	case SF_FACE:
		*plane = ((srfSurfaceFace_t *)surfType)->plane;
		return;
	case SF_TRIANGLES:
		tri = (srfTriangles_t *)surfType;
		v1 = tri->verts + tri->indexes[0];
		v2 = tri->verts + tri->indexes[1];
		v3 = tri->verts + tri->indexes[2];
		PlaneFromPoints( plane4, v1->xyz, v2->xyz, v3->xyz );
		VectorCopy( plane4, plane->normal ); 
		plane->dist = plane4[3];
		return;
	case SF_POLY:
		poly = (srfPoly_t *)surfType;
		PlaneFromPoints( plane4, poly->verts[0].xyz, poly->verts[1].xyz, poly->verts[2].xyz );
		VectorCopy( plane4, plane->normal ); 
		plane->dist = plane4[3];
		return;
	default:
		Com_Memset (plane, 0, sizeof(*plane));
		plane->normal[0] = 1;		
		return;
	}
}

/*
=================
R_GetPortalOrientation

entityNum is the entity that the portal surface is a part of, which may
be moving and rotating.

Returns qtrue if it should be mirrored
=================
*/
qboolean R_GetPortalOrientations( drawSurf_t *drawSurf, int entityNum, 
							 orientation_t *surface, orientation_t *camera,
							 vec3_t pvsOrigin, qboolean *mirror ) {
	int			i;
	cplane_t	originalPlane, plane;
	trRefEntity_t	*e;
	float		d;
	vec3_t		transformed;

	// create plane axis for the portal we are seeing
	R_PlaneForSurface( drawSurf->surface, &originalPlane );

	// rotate the plane if necessary
	if ( entityNum != ENTITYNUM_WORLD ) {
		tr.currentEntityNum = entityNum;
		tr.currentEntity = &tr.refdef.entities[entityNum];

		// get the orientation of the entity
		R_RotateForEntity( tr.currentEntity, &tr.viewParms, &tr.ori );

		// rotate the plane, but keep the non-rotated version for matching
		// against the portalSurface entities
		R_LocalNormalToWorld( originalPlane.normal, plane.normal );
		plane.dist = originalPlane.dist + DotProduct( plane.normal, tr.ori.origin );

		// translate the original plane
		originalPlane.dist = originalPlane.dist + DotProduct( originalPlane.normal, tr.ori.origin );
	} else {
		plane = originalPlane;
	}

	VectorCopy( plane.normal, surface->axis[0] );
	PerpendicularVector( surface->axis[1], surface->axis[0] );
	CrossProduct( surface->axis[0], surface->axis[1], surface->axis[2] );

	// locate the portal entity closest to this plane.
	// origin will be the origin of the portal, origin2 will be
	// the origin of the camera
	for ( i = 0 ; i < tr.refdef.num_entities ; i++ ) {
		e = &tr.refdef.entities[i];
		if ( e->e.reType != RT_PORTALSURFACE ) {
			continue;
		}

		d = DotProduct( e->e.origin, originalPlane.normal ) - originalPlane.dist;
		if ( d > 64 || d < -64) {
			continue;
		}

		// get the pvsOrigin from the entity
		VectorCopy( e->e.oldorigin, pvsOrigin );

		// if the entity is just a mirror, don't use as a camera point
		if ( e->e.oldorigin[0] == e->e.origin[0] && 
			e->e.oldorigin[1] == e->e.origin[1] && 
			e->e.oldorigin[2] == e->e.origin[2] ) {
			VectorScale( plane.normal, plane.dist, surface->origin );
			VectorCopy( surface->origin, camera->origin );
			VectorSubtract( vec3_origin, surface->axis[0], camera->axis[0] );
			VectorCopy( surface->axis[1], camera->axis[1] );
			VectorCopy( surface->axis[2], camera->axis[2] );

			*mirror = qtrue;
			return qtrue;
		}

		// project the origin onto the surface plane to get
		// an origin point we can rotate around
		d = DotProduct( e->e.origin, plane.normal ) - plane.dist;
		VectorMA( e->e.origin, -d, surface->axis[0], surface->origin );
			
		// now get the camera origin and orientation
		VectorCopy( e->e.oldorigin, camera->origin );
		AxisCopy( e->e.axis, camera->axis );
		VectorSubtract( vec3_origin, camera->axis[0], camera->axis[0] );
		VectorSubtract( vec3_origin, camera->axis[1], camera->axis[1] );

		// optionally rotate
		if ( e->e.skinNum ) {
			d = e->e.skinNum;
			VectorCopy( camera->axis[1], transformed );
			RotatePointAroundVector( camera->axis[1], camera->axis[0], transformed, d );
			CrossProduct( camera->axis[0], camera->axis[1], camera->axis[2] );
		}
		*mirror = qfalse;
		return qtrue;
	}

	// if we didn't locate a portal entity, don't render anything.
	// We don't want to just treat it as a mirror, because without a
	// portal entity the server won't have communicated a proper entity set
	// in the snapshot

	// unfortunately, with local movement prediction it is easily possible
	// to see a surface before the server has communicated the matching
	// portal surface entity, so we don't want to print anything here...

	//ri.Printf( PRINT_ALL, "Portal surface without a portal entity\n" );

	return qfalse;
}

static qboolean IsMirror( const drawSurf_t *drawSurf, int entityNum )
{
	int			i;
	cplane_t	originalPlane, plane;
	trRefEntity_t	*e;
	float		d;

	// create plane axis for the portal we are seeing
	R_PlaneForSurface( drawSurf->surface, &originalPlane );

	// rotate the plane if necessary
	if ( entityNum != ENTITYNUM_WORLD ) 
	{
		tr.currentEntityNum = entityNum;
		tr.currentEntity = &tr.refdef.entities[entityNum];

		// get the orientation of the entity
		R_RotateForEntity( tr.currentEntity, &tr.viewParms, &tr.ori );

		// rotate the plane, but keep the non-rotated version for matching
		// against the portalSurface entities
		R_LocalNormalToWorld( originalPlane.normal, plane.normal );
		plane.dist = originalPlane.dist + DotProduct( plane.normal, tr.ori.origin );

		// translate the original plane
		originalPlane.dist = originalPlane.dist + DotProduct( originalPlane.normal, tr.ori.origin );
	} 
	else 
	{
		plane = originalPlane;
	}

	// locate the portal entity closest to this plane.
	// origin will be the origin of the portal, origin2 will be
	// the origin of the camera
	for ( i = 0 ; i < tr.refdef.num_entities ; i++ ) 
	{
		e = &tr.refdef.entities[i];
		if ( e->e.reType != RT_PORTALSURFACE ) {
			continue;
		}

		d = DotProduct( e->e.origin, originalPlane.normal ) - originalPlane.dist;
		if ( d > 64 || d < -64) {
			continue;
		}

		// if the entity is just a mirror, don't use as a camera point
		if ( e->e.oldorigin[0] == e->e.origin[0] && 
			e->e.oldorigin[1] == e->e.origin[1] && 
			e->e.oldorigin[2] == e->e.origin[2] ) 
		{
			return qtrue;
		}

		return qfalse;
	}
	return qfalse;
}

/*
** SurfIsOffscreen
**
** Determines if a surface is completely offscreen.
*/
qboolean SurfIsOffscreen(const srfSurfaceFace_t* surface, shader_t* shader, int entityNum) {
	float shortest = 100000000;
	int numTriangles;
	qboolean doRange;
	orientationr_t surfOr;
	unsigned int* indices;
	vec4_t clip, eye;
	int i;
	unsigned int pointOr = 0;
	unsigned int pointAnd = (unsigned int)~0;
	
	if ( glConfig.smpActive ) {		// FIXME!  we can't do RB_BeginSurface/RB_EndSurface stuff with smp!
		return qfalse;
	}

	if (entityNum == ENTITYNUM_WORLD) {
		surfOr = tr.viewParms.world;
	} else {
		R_RotateForEntity(&tr.refdef.entities[entityNum], &tr.viewParms, &surfOr);
	}

	assert( tess.numVertexes < 128 );

	for ( i = 0; i < tess.numVertexes; i++ )
	{
		int j;
		unsigned int pointFlags = 0;

		R_TransformModelToClip( tess.xyz[i], tr.ori.modelMatrix, tr.viewParms.projectionMatrix, eye, clip );

		for ( j = 0; j < 3; j++ )
		{
			if ( clip[j] >= clip[3] )
			{
				pointFlags |= (1 << (j*2));
			}
			else if ( clip[j] <= -clip[3] )
			{
				pointFlags |= ( 1 << (j*2+1));
			}
		}
		pointAnd &= pointFlags;
		pointOr |= pointFlags;
	}

	// trivially reject
	if ( pointAnd )
	{
		return qtrue;
	}

	// determine if this surface is backfaced and also determine the distance
	// to the nearest vertex so we can cull based on portal range.  Culling
	// based on vertex distance isn't 100% correct (we should be checking for
	// range to the surface), but it's good enough for the types of portals
	// we have in the game right now.
	numTriangles = tess.numIndexes / 3;

	for ( i = 0; i < tess.numIndexes; i += 3 )
	{
		vec3_t normal;
		float dot;
		float len;

		VectorSubtract( tess.xyz[tess.indexes[i]], tr.viewParms.ori.origin, normal );

		len = VectorLengthSquared( normal );			// lose the sqrt
		if ( len < shortest )
		{
			shortest = len;
		}

		if ( ( dot = DotProduct( normal, tess.normal[tess.indexes[i]] ) ) >= 0 )
		{
			numTriangles--;
		}
	}
	if ( !numTriangles )
	{
		return qtrue;
	}

	if (shader->fDistRange > 0.0)
	{
		if (shortest > (tess.shader->portalRange * tess.shader->portalRange))
		{
			return qtrue;
		}
	}

	return qfalse;
}

static qboolean DrawSurfIsOffscreen(drawSurf_t* drawSurf) {
	int entityNum;
	shader_t* shader;
	int dlighted;
	qboolean bStaticModel;

	R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &dlighted, &bStaticModel);
	return SurfIsOffscreen((srfSurfaceFace_t*)drawSurf->surface, shader, entityNum);
}

/*
========================
R_MirrorViewBySurface

Returns qtrue if another view has been rendered
========================
*/
qboolean R_MirrorViewBySurface (drawSurf_t *drawSurf, int entityNum) {
	viewParms_t		newParms;
	viewParms_t		oldParms;
	orientation_t	surface, camera;

	// don't recursively mirror
	if (tr.viewParms.isPortal) {
		ri.Printf( PRINT_DEVELOPER, "WARNING: recursive mirror/portal found\n" );
		return qfalse;
	}

	if ( r_noportals->integer || (r_fastsky->integer == 1) ) {
		return qfalse;
	}

	// trivially reject portal/mirror
	if (DrawSurfIsOffscreen( drawSurf ) ) {
		return qfalse;
	}

	// save old viewParms so we can return to it after the mirror view
	oldParms = tr.viewParms;

	newParms = tr.viewParms;
	newParms.isPortal = qtrue;
	if ( !R_GetPortalOrientations( drawSurf, entityNum, &surface, &camera, 
		newParms.pvsOrigin, &newParms.isMirror ) ) {
		return qfalse;		// bad portal, no portalentity
	}

	R_MirrorPoint (oldParms.ori.origin, &surface, &camera, newParms.ori.origin );

	VectorSubtract( vec3_origin, camera.axis[0], newParms.portalPlane.normal );
	newParms.portalPlane.dist = DotProduct( camera.origin, newParms.portalPlane.normal );
	
	R_MirrorVector (oldParms.ori.axis[0], &surface, &camera, newParms.ori.axis[0]);
	R_MirrorVector (oldParms.ori.axis[1], &surface, &camera, newParms.ori.axis[1]);
	R_MirrorVector (oldParms.ori.axis[2], &surface, &camera, newParms.ori.axis[2]);

	// OPTIMIZE: restrict the viewport on the mirrored view

	// render the mirror view
	R_RenderView (&newParms);

	tr.viewParms = oldParms;

	return qtrue;
}

/*
==========================================================================================

DRAWSURF SORTING

==========================================================================================
*/

static void SwapDrawSurf(drawSurf_t* a, drawSurf_t* b) {
	drawSurf_t temp = *a;
	*a = *b;
	*b = temp;
}

/*
=================
qsort replacement

=================
*/
//#define	SWAP_DRAW_SURF(a,b) temp=((int *)a)[0];((int *)a)[0]=((int *)b)[0];((int *)b)[0]=temp; temp=((int *)a)[1];((int *)a)[1]=((int *)b)[1];((int *)b)[1]=temp;

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8            /* testing shows that this is good value */

static void shortsort( drawSurf_t *lo, drawSurf_t *hi ) {
    drawSurf_t	*p, *max;
	int			temp;

    while (hi > lo) {
        max = lo;
        for (p = lo + 1; p <= hi; p++ ) {
            if ( p->sort > max->sort ) {
                max = p;
            }
        }
		SwapDrawSurf(max, hi);
        hi--;
    }
}


/* sort the array between lo and hi (inclusive)
FIXME: this was lifted and modified from the microsoft lib source...
 */

void qsortFast (
    void *base,
    unsigned num,
    unsigned width
    )
{
    char *lo, *hi;              /* ends of sub-array currently sorting */
    char *mid;                  /* points to middle of subarray */
    char *loguy, *higuy;        /* traveling pointers for partition step */
    unsigned size;              /* size of the sub-array */
    char *lostk[30], *histk[30];
    int stkptr;                 /* stack for saving sub-array to be processed */
	int	temp;

    /* Note: the number of stack entries required is no more than
       1 + log2(size), so 30 is sufficient for any array */

    if (num < 2 || width == 0)
        return;                 /* nothing to do */

    stkptr = 0;                 /* initialize stack */

    lo = base;
    hi = (char *)base + width * (num-1);        /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       prserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1;        /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF) {
         shortsort((drawSurf_t *)lo, (drawSurf_t *)hi);
    }
    else {
        /* First we pick a partititioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the
           median of the values, but also that we select one fast.  Using
           the first one produces bad performace if the array is already
           sorted, so we use the middle one, which would require a very
           wierdly arranged array for worst case performance.  Testing shows
           that a median-of-three algorithm does not, in general, increase
           performance. */

        mid = lo + (size / 2) * width;      /* find middle element */
		SwapDrawSurf(mid, lo);               /* swap it to beginning of array */

        /* We now wish to partition the array into three pieces, one
           consisiting of elements <= partition element, one of elements
           equal to the parition element, and one of element >= to it.  This
           is done below; comments indicate conditions established at every
           step. */

        loguy = lo;
        higuy = hi + width;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;) {
            /* lo <= loguy < hi, lo < higuy <= hi + 1,
               A[i] <= A[lo] for lo <= i <= loguy,
               A[i] >= A[lo] for higuy <= i <= hi */

            do  {
                loguy += width;
            } while (loguy <= hi &&  
				( ((drawSurf_t *)loguy)->sort <= ((drawSurf_t *)lo)->sort ) );

            /* lo < loguy <= hi+1, A[i] <= A[lo] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[lo] */

            do  {
                higuy -= width;
            } while (higuy > lo && 
				( ((drawSurf_t *)higuy)->sort >= ((drawSurf_t *)lo)->sort ) );

            /* lo-1 <= higuy <= hi, A[i] >= A[lo] for higuy < i <= hi,
               either higuy <= lo or A[higuy] < A[lo] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy <= lo, then we would have exited, so
               A[loguy] > A[lo], A[higuy] < A[lo],
               loguy < hi, highy > lo */

			SwapDrawSurf(loguy, higuy);

            /* A[loguy] < A[lo], A[higuy] > A[lo]; so condition at top
               of loop is re-established */
        }

        /*     A[i] >= A[lo] for higuy < i <= hi,
               A[i] <= A[lo] for lo <= i < loguy,
               higuy < loguy, lo <= higuy <= hi
           implying:
               A[i] >= A[lo] for loguy <= i <= hi,
               A[i] <= A[lo] for lo <= i <= higuy,
               A[i] = A[lo] for higuy < i < loguy */

		SwapDrawSurf(lo, higuy);     /* put partition element in place */

        /* OK, now we have the following:
              A[i] >= A[higuy] for loguy <= i <= hi,
              A[i] <= A[higuy] for lo <= i < higuy
              A[i] = A[lo] for higuy <= i < loguy    */

        /* We've finished the partition, now we want to sort the subarrays
           [lo, higuy-1] and [loguy, hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if ( higuy - 1 - lo >= hi - loguy ) {
            if (lo + width < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;
            }                           /* save big recursion for later */

            if (loguy < hi) {
                lo = loguy;
                goto recurse;           /* do small recursion */
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;               /* save big recursion for later */
            }

            if (lo + width < higuy) {
                hi = higuy - width;
                goto recurse;           /* do small recursion */
            }
        }
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;           /* pop subarray from stack */
    }
    else
        return;                 /* all subarrays done */
}


//==========================================================================================

/*
=================
R_AddDrawSurf
=================
*/
void R_AddDrawSurf(surfaceType_t* surface, shader_t* shader, int dlightMap) {
	int			index;

	// instead of checking for overflow, we just mask the index
	// so it wraps around
	index = tr.refdef.numDrawSurfs & DRAWSURF_MASK;
	// the sort data is packed into a single 32 bit value so it can be
	// compared quickly during the qsorting process
	tr.refdef.drawSurfs[index].sort = (shader->sortedIndex << QSORT_SHADERNUM_SHIFT) 
		| tr.shiftedEntityNum | tr.shiftedIsStatic | ((int)dlightMap & 15);
	tr.refdef.drawSurfs[index].surface = surface;
	tr.refdef.numDrawSurfs++;
}

/*
=================
R_AddSpriteSurf
=================
*/
void R_AddSpriteSurf(surfaceType_t* surface, shader_t* shader, float zDistance)
{
	int index;

	if (zDistance > MAX_SPRITE_DIST_SQUARED) {
		zDistance = MAX_SPRITE_DIST_SQUARED;
	}

	index = tr.refdef.numSpriteSurfs % MAX_SPRITES;
    tr.refdef.spriteSurfs[index].sort = (int)(MAX_SPRITE_DIST_SQUARED - zDistance) | (shader->sortedIndex << QSORT_SHADERNUM_SHIFT);
    tr.refdef.spriteSurfs[index].surface = surface;
    tr.refdef.numSpriteSurfs++;
}

/*
=================
R_DecomposeSort
=================
*/
void R_DecomposeSort(unsigned int sort, int* entityNum, shader_t** shader, int* dlightMap, qboolean* bStaticModel) {
	*shader = tr.sortedShaders[ ( sort >> QSORT_SHADERNUM_SHIFT ) & (MAX_SHADERS-1) ];
	*entityNum = ( sort >> QSORT_ENTITYNUM_SHIFT ) & 1023;
	*dlightMap = sort & 15;
	*bStaticModel = sort & (1 << QSORT_STATICMODEL_SHIFT);
}

/*
=================
R_SortDrawSurfs
=================
*/
void R_SortDrawSurfs( drawSurf_t *drawSurfs, int numDrawSurfs, drawSurf_t *spriteSurfs, int numSpriteSurfs ) {
	shader_t		*shader;
	int				entityNum;
	int				dlighted;
	int				i;
	qboolean		bStaticModel;

	// it is possible for some views to not have any surfaces
	if ( numDrawSurfs < 1 ) {
		// we still need to add it for hyperspace cases
		R_AddDrawSurfCmd( drawSurfs, numDrawSurfs );
		return;
	}

	// if we overflowed MAX_DRAWSURFS, the drawsurfs
	// wrapped around in the buffer and we will be missing
    // the first surfaces, not the last ones
    if (numDrawSurfs > MAX_DRAWSURFS) {
        numDrawSurfs = MAX_DRAWSURFS;
    }
    if (numSpriteSurfs > MAX_SPRITESURFS) {
		numSpriteSurfs = MAX_SPRITESURFS;
    }

	// sort the drawsurfs by sort type, then orientation, then shader
	qsortFast (drawSurfs, numDrawSurfs, sizeof(drawSurf_t) );
	qsortFast (spriteSurfs, numSpriteSurfs, sizeof(drawSurf_t) );

	R_Sky_Render();

	if (!tr.viewParms.isPortal)
	{
		// check for any pass through drawing, which
		// may cause another view to be rendered first
		for (i = 0; i < numDrawSurfs; i++) {
			R_DecomposeSort((drawSurfs + i)->sort, &entityNum, &shader, &dlighted, &bStaticModel);

			if (shader->sort > SS_PORTAL) {
				break;
			}

			// no shader should ever have this sort type
			if (shader->sort == SS_BAD) {
				ri.Error(ERR_DROP, "Shader '%s'with sort == SS_BAD", shader->name);
			}

			// if the mirror was completely clipped away, we may need to check another surface
			if (R_MirrorViewBySurface((drawSurfs + i), entityNum)) {
				// this is a debug option to see exactly what is being mirrored
				if (r_portalOnly->integer) {
					return;
				}
				break;		// only one mirror view at a time
			}
		}
	}

    R_AddDrawSurfCmd(drawSurfs, numDrawSurfs);
    R_AddSpriteSurfCmd(spriteSurfs, numSpriteSurfs);
}

/*
=============
R_AddEntitySurfaces
=============
*/
void R_AddEntitySurfaces (void) {
	trRefEntity_t	*ent;
	shader_t		*shader;

	tr.shiftedIsStatic = 0;

	for ( tr.currentEntityNum = 0; 
	      tr.currentEntityNum < tr.refdef.num_entities; 
		  tr.currentEntityNum++ ) {
		ent = tr.currentEntity = &tr.refdef.entities[tr.currentEntityNum];

		ent->needDlights = qfalse;

		// preshift the value we are going to OR into the drawsurf sort
		tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

		//
		// the weapon model must be handled special --
		// we don't want the hacked weapon position showing in 
		// mirrors, because the true body position will already be drawn
		//
		if ( (ent->e.renderfx & RF_FIRST_PERSON) && tr.viewParms.isPortal) {
			continue;
		}

		// simple generated models, like sprites and beams, are not culled
		switch ( ent->e.reType ) {
		case RT_PORTALSURFACE:
			break;		// don't draw anything
		case RT_SPRITE:
		case RT_BEAM:
			// self blood sprites, talk balloons, etc should not be drawn in the primary
			// view.  We can't just do this check for all entities, because md3
			// entities may still want to cast shadows from them
			if ( (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal) {
				continue;
			}
			shader = R_GetShaderByHandle( ent->e.customShader );
			R_AddDrawSurf( &entitySurface, shader, 0 );
			break;

		case RT_MODEL:
			// we must set up parts of tr.ori for model culling
			R_RotateForEntity( ent, &tr.viewParms, &tr.ori );

			tr.currentModel = R_GetModelByHandle( ent->e.hModel );
			if (!tr.currentModel) {
				R_AddDrawSurf( &entitySurface, tr.defaultShader, 0 );
			} else {
				switch ( tr.currentModel->type ) {
				case MOD_SPRITE:
					Com_Printf("sprite model '%s' being added to renderer!\n", tr.currentModel->name);
					break;
				case MOD_TIKI:
					R_AddSkelSurfaces( ent );
					break;
				case MOD_BRUSH:
					R_AddBrushModelSurfaces( ent );
					break;
				case MOD_BAD:		// null model axis
					if ( (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal) {
						break;
					}
					shader = R_GetShaderByHandle( ent->e.customShader );
					R_AddDrawSurf( &entitySurface, tr.defaultShader, 0 );
					break;
				default:
					ri.Error( ERR_DROP, "R_AddEntitySurfaces: Bad modeltype" );
					break;
				}
			}
			break;
		default:
			ri.Error( ERR_DROP, "R_AddEntitySurfaces: Bad reType" );
		}
	}

}

void R_AddSpriteSurfaces()
{
	refSprite_t* sprite;
	vec3_t delta;

	if (!r_drawsprites->integer) {
		return;
	}

	for (tr.currentSpriteNum = 0; tr.currentSpriteNum < tr.refdef.num_sprites; ++tr.currentSpriteNum)
	{
		sprite = &tr.refdef.sprites[tr.currentSpriteNum];

        if (tr.portalsky.inUse)
        {
			if ((sprite->renderfx & RF_SKYENTITY) == 0) {
				continue;
			}
        }
        else if ((sprite->renderfx & RF_SKYENTITY) != 0)
        {
            continue;
        }
        
		if (tr.viewParms.isPortal)
        {
			if (!(sprite->renderfx & (RF_SHADOW_PLANE | RF_WRAP_FRAMES))) {
				continue;
			}
        }
		else if (sprite->renderfx & RF_SHADOW_PLANE) {
			continue;
		}

		tr.currentEntityNum = ENTITYNUM_WORLD;
        tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;
		if (sprite->hModel && sprite->hModel < tr.numModels) {
			tr.currentModel = &tr.models[sprite->hModel];
		} else {
			tr.currentModel = &tr.models[0];
		}

		R_AdjustVisBoundsForSprite(&tr.refdef.sprites[tr.currentSpriteNum], &tr.viewParms, &tr.ori );
		sprite->shaderNum = tr.currentModel->d.sprite->shader->sortedIndex;

		VectorSubtract(sprite->origin, tr.refdef.vieworg, delta);
		R_AddSpriteSurf(&sprite->surftype, tr.currentModel->d.sprite->shader, VectorLengthSquared(delta));
	}
}

/*
====================
R_GenerateDrawSurfs
====================
*/
void R_GenerateDrawSurfs( void ) {
	R_AddWorldSurfaces ();
	if (!(tr.refdef.rdflags & RDF_NOWORLDMODEL)) {
		R_AddSwipeSurfaces();
	}

	R_AddPolygonSurfaces();

    R_AddTerrainMarkSurfaces();

    R_AddEntitySurfaces();

	R_AddSpriteSurfaces();

	// set the projection matrix with the minimum zfar
	// now that we have the world bounded
	// this needs to be done before entities are
	// added, because they use the projection
	// matrix for lod calculation
	R_SetupProjection ();
}

/*
================
R_DebugPolygon
================
*/
void R_DebugPolygon( int color, int numPoints, float *points ) {
	int		i;

	GL_State( GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

	// draw solid shade

	qglColor3f( color&1, (color>>1)&1, (color>>2)&1 );
	qglBegin( GL_POLYGON );
	for ( i = 0 ; i < numPoints ; i++ ) {
		qglVertex3fv( points + i * 3 );
	}
	qglEnd();

	// draw wireframe outline
	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
	qglDepthRange( 0, 0 );
	qglColor3f( 1, 1, 1 );
	qglBegin( GL_POLYGON );
	for ( i = 0 ; i < numPoints ; i++ ) {
		qglVertex3fv( points + i * 3 );
	}
	qglEnd();
	qglDepthRange( 0, 1 );
}

/*
====================
R_DebugGraphics

Visualization aid for movement clipping debugging
====================
*/
void R_DebugGraphics( void ) {
	if ( !r_debugSurface->integer ) {
		return;
	}

	// the render thread can't make callbacks to the main thread
	R_SyncRenderThread();

	GL_Bind( tr.whiteImage);
	GL_Cull( CT_FRONT_SIDED );
	ri.CM_DrawDebugSurface( R_DebugPolygon );
}

#define CIRCLE_LENGTH		25

/*
================
R_DebugCircle
================
*/
void R_DebugCircle(const vec3_t org, float radius, float r, float g, float b, float alpha, qboolean horizontal) {
    int				i;
    float			ang;
	debugline_t* line;
	vec3_t			forward, right;
	vec3_t			pos, lastpos;

	if (!ri.DebugLines || !ri.numDebugLines) {
		return;
	}

	if (horizontal)
	{
		VectorSet(forward, 1, 0, 0);
		VectorSet(right, 0, 1, 0);
	}
	else
	{
		VectorCopy(tr.refdef.viewaxis[1], right);
		VectorCopy(tr.refdef.viewaxis[2], forward);
	}

	VectorClear(pos);
	VectorClear(lastpos);

	for (i = 0; i < CIRCLE_LENGTH; i++) {
		VectorCopy(pos, lastpos);

		ang = (float)i * 0.0174532925199433f;
		pos[0] = (org[0] + sin(ang) * radius * forward[0]) +
			cos(ang) * radius * right[0];
		pos[1] = (org[1] + sin(ang) * radius * forward[1]) +
			cos(ang) * radius * right[1];
		pos[2] = (org[2] + sin(ang) * radius * forward[2]) +
			cos(ang) * radius * right[2];

		if (i > 0)
		{
			if (*ri.numDebugLines >= r_numdebuglines->integer) {
				ri.Printf(PRINT_ALL, "R_DebugCircle: Exceeded MAX_DEBUG_LINES\n");
				return;
			}

			line = &(*ri.DebugLines)[*ri.numDebugLines];
			(*ri.numDebugLines)++;
			VectorCopy(lastpos, line->start);
			VectorCopy(pos, line->end);
			VectorSet(line->color, r, g, b);
			line->alpha = alpha;
			line->width = 1.0;
			line->factor = 1;
			line->pattern = -1;
		}
	}
}

/*
================
R_DebugLine
================
*/
void R_DebugLine(const vec3_t start, const vec3_t end, float r, float g, float b, float alpha) {
	debugline_t* line;

	if (!ri.DebugLines || !ri.numDebugLines) {
		return;
	}

	if (*ri.numDebugLines >= r_numdebuglines->integer) {
		ri.Printf(PRINT_ALL, "R_DebugLine: Exceeded MAX_DEBUG_LINES\n");
		return;
	}

	line = &(*ri.DebugLines)[*ri.numDebugLines];
	(*ri.numDebugLines)++;
	VectorCopy(start, line->start);
	VectorCopy(end, line->end);
	VectorSet(line->color, r, g, b);
	line->alpha = alpha;
	line->width = 1.0;
	line->factor = 1;
	line->pattern = -1;
}

/*
================
R_DrawDebugLines
================
*/
void R_DrawDebugLines(void) {
	debugline_t* line;
	int i;
	float width;
	int factor;
	unsigned short pattern;

	if (!ri.DebugLines || !ri.numDebugLines) {
		return;
	}

	if (!*ri.numDebugLines) {
		return;
	}

	if (tr.refdef.rdflags & RDF_NOWORLDMODEL) {
		return;
	}

	if (tr.portalsky.inUse) {
		return;
	}

    R_SyncRenderThread();
    GL_Bind(tr.whiteImage);
	if (r_debuglines_depthmask->integer) {
		GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);
	} else {
		GL_State(GLS_POLYMODE_LINE);
	}

    qglDisableClientState(GL_COLOR_ARRAY);
    qglDisableClientState(GL_TEXTURE_COORD_ARRAY);

    width = 1.0;
    factor = 4;
    pattern = -1;

    if (r_stipplelines->integer)
    {
        qglEnable(GL_LINE_STIPPLE);
        qglLineStipple(4, -1);
        qglLineWidth(1.0f);
    }

	qglBegin(GL_LINES);

	for (i = 0; i < *ri.numDebugLines; i++) {
		line = &(*ri.DebugLines)[i];

		if (r_stipplelines->integer) {
			if (line->width != width || line->factor != factor || line->pattern != pattern) {
				qglEnd();
				qglLineStipple(line->factor, line->pattern);
                qglLineWidth(line->width);
                qglBegin(GL_LINES);
                factor = line->factor;
                width = line->width;
                pattern = line->pattern;
			}
		}

		qglColor4f(line->color[0], line->color[1], line->color[2], line->alpha);
        qglVertex3fv(line->start);
        qglVertex3fv(line->end);
	}

    qglEnd();

    if (r_stipplelines->integer)
    {
        qglDisable(GL_LINE_STIPPLE);
        qglLineStipple(1, -1);
        qglLineWidth(1.0);
    }

    qglDepthRange(0.0, 1.0);
}

/*
================
R_DrawDebugLines
================
*/
void R_DrawDebugStrings(void) {
	int i;
	debugstring_t* string;

	if (!ri.DebugStrings || !ri.numDebugStrings) {
		return;
	}

	if (!*ri.numDebugStrings) {
		return;
	}

    if (tr.refdef.rdflags & RDF_NOWORLDMODEL) {
        return;
    }

    if (tr.portalsky.inUse) {
        return;
    }

	for (i = 0; i < *ri.numDebugStrings; i++) {
		string = &(*ri.DebugStrings)[i];

		R_DrawFloatingString(
			tr.pFontDebugStrings,
			string->szText,
			string->pos,
			string->color,
			string->scale,
			64
		);
	}
}

/*
================
R_RenderView

A view may be either the actual camera view,
or a mirror / remote location
================
*/
void R_RenderView (viewParms_t *parms) {
	int		firstDrawSurf;
	int		firstSpriteSurf;

	if ( parms->viewportWidth <= 0 || parms->viewportHeight <= 0 ) {
		return;
	}

	tr.viewCount++;

	tr.viewParms = *parms;
	tr.viewParms.frameSceneNum = tr.frameSceneNum;
	tr.viewParms.frameCount = tr.frameCount;

	firstDrawSurf = tr.refdef.numDrawSurfs;
	firstSpriteSurf = tr.refdef.numSpriteSurfs;

	tr.viewCount++;

	// set viewParms.world
	R_RotateForViewer ();

	R_SetupFrustum ();

	R_Sky_Reset();

	R_DrawDebugStrings();

	R_GenerateDrawSurfs();

    R_SortDrawSurfs(
		tr.refdef.drawSurfs + firstDrawSurf, tr.refdef.numDrawSurfs - firstDrawSurf,
		tr.refdef.spriteSurfs + firstSpriteSurf, tr.refdef.numSpriteSurfs - firstSpriteSurf
	);

	R_DrawDebugLines();

	// draw main system development information (surface outlines, etc)
	R_DebugGraphics();
}



