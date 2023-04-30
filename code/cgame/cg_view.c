/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#include "cg_local.h"

//============================================================================

/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect (void) {
	int		size;

	// the intermission should allways be full screen
	if ( cg.snap->ps.pm_flags & PMF_INTERMISSION ) {
		size = 100;
	} else {
		// bound normal viewsize
		if (cg_viewsize->integer < 30) {
			cgi.Cvar_Set ("viewsize","30");
			size = 30;
		} else if (cg_viewsize->integer > 100) {
			cgi.Cvar_Set ("viewsize","100");
			size = 100;
		} else {
			size = cg_viewsize->integer;
		}

	}
	cg.refdef.width = cgs.glconfig.vidWidth*size/100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight*size/100;
	cg.refdef.height &= ~1;

	cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
	cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define CAMERA_MINIMUM_DISTANCE 40
static void CG_OffsetThirdPersonView( void )
{
	vec3_t forward;
	vec3_t original_camera_position;
	vec3_t new_vieworg;
	trace_t trace;
	vec3_t min, max;
   float *look_offset;
   float *target_angles;
   float *target_position;
   vec3_t delta;
   vec3_t original_angles;
   qboolean lookactive, resetview;
   static vec3_t saved_look_offset;
   vec3_t camera_offset;

   target_angles = cg.refdefViewAngles;
   target_position = cg.refdef.vieworg;

   // see if angles are absolute
   if ( cg.predicted_player_state.camera_flags & CF_CAMERA_ANGLES_ABSOLUTE )
      {
      VectorClear( target_angles );
      }

   // see if we need to ignore yaw
   if ( cg.predicted_player_state.camera_flags & CF_CAMERA_ANGLES_IGNORE_YAW )
      {
      target_angles[ YAW ] = 0;
      }

   // see if we need to ignore pitch
   if ( cg.predicted_player_state.camera_flags & CF_CAMERA_ANGLES_IGNORE_PITCH )
      {
      target_angles[ PITCH ] = 0;
      }

   // offset the current angles by the camera offset
   VectorSubtract( target_angles, cg.predicted_player_state.camera_offset, target_angles );

   // Get the position of the camera after any needed rotation
   look_offset = cgi.get_camera_offset( &lookactive, &resetview );

   if ( 
         ( !resetview ) &&
         (
            ( cg.predicted_player_state.camera_flags & CF_CAMERA_ANGLES_ALLOWOFFSET ) || 
            ( lookactive )
         )
      )
      {
      VectorSubtract( look_offset, saved_look_offset, camera_offset );
      VectorAdd( target_angles, camera_offset, target_angles );
      if ( target_angles[ PITCH ] > 90 )
         target_angles[ PITCH ] = 90;
      else if ( target_angles[ PITCH ] < -90 )
         target_angles[ PITCH ] = -90;
      }
   else
      {
      VectorCopy( look_offset, saved_look_offset );
      }

   target_angles[ YAW ] = AngleNormalize360( target_angles[ YAW ] );
   target_angles[ PITCH ] = AngleNormalize180( target_angles[ PITCH ] );

	// Move reference point up

	target_position[2] += cg_cameraheight->value;

	VectorCopy(target_position, original_camera_position);

	// Move camera back from reference point

	AngleVectors(target_angles, forward, NULL, NULL);

	VectorMA(target_position, -cg_cameradist->value, forward, new_vieworg);

   new_vieworg[ 2 ] += cg_cameraverticaldisplacement->value;

	// Create a bounding box for our camera

	min[0] = -5;
	min[1] = -5;
	min[2] = -5;

	max[0] = 5;
	max[1] = 5;
	max[2] = 5;

	// Make sure camera does not collide with anything
	CG_Trace(&trace, cg.playerHeadPos, min, max, new_vieworg, 0, MASK_CAMERASOLID, qfalse, qtrue, "ThirdPersonTrace 1" );

	VectorCopy(trace.endpos, target_position);

   // calculate distance from end position to head position
   VectorSubtract( target_position, cg.playerHeadPos, delta );
   // kill any negative z difference in delta
   if ( delta[ 2 ] < CAMERA_MINIMUM_DISTANCE )
      delta[ 2 ] = 0;
   if ( VectorLength( delta ) < CAMERA_MINIMUM_DISTANCE )
      {
      VectorNormalize( delta);
/*
      // see if we are going straight up
      if ( ( delta[ 2 ] > 0.75 ) && ( height > 0.85f * cg.predicted_player_state.viewheight ) )
         {
         // we just need to lower our start position slightly, since we are on top of the player
         new_vieworg[ 2 ] -= 16;
	      CG_Trace(&trace, cg.playerHeadPos, min, max, new_vieworg, 0, MASK_CAMERASOLID, qfalse, true, "ThirdPersonTrace 2" );
	      VectorCopy(trace.endpos, target_position);
         }
      else
*/
         {
         // we are probably up against the wall so we want the camera to pitch up on top of the player
         // save off the original angles
         VectorCopy( target_angles, original_angles );
         // start cranking up the target angles, pitch until we are the correct distance away from the player
         while ( target_angles[ PITCH ] < 90 )
            {
            target_angles[ PITCH ] += 2;

	         AngleVectors(target_angles, forward, NULL, NULL);

	         VectorMA( original_camera_position, -cg_cameradist->value, forward, new_vieworg);

            new_vieworg[ 2 ] += cg_cameraverticaldisplacement->value;

	         CG_Trace(&trace, cg.playerHeadPos, min, max, new_vieworg, 0, MASK_CAMERASOLID, qfalse, qtrue, "ThirdPersonTrace 3" );

	         VectorCopy(trace.endpos, target_position);

            // calculate distance from end position to head position
            VectorSubtract( target_position, cg.playerHeadPos, delta );
            // kill any negative z difference in delta
            if ( delta[ 2 ] < 0 )
               delta[ 2 ] = 0;
            if ( VectorLength( delta ) >= CAMERA_MINIMUM_DISTANCE )
               {
               target_angles[ PITCH ] = ( 0.25f * target_angles[ PITCH ] ) + ( 0.75f * original_angles[ PITCH ] );
               // set the pitch to be that of the angle we are currently looking
               //target_angles[ PITCH ] = original_angles[ PITCH ];
               break;
               }
            }
         if ( target_angles[ PITCH ] > 90 )
            {
            // if we failed, go with the original angles
            target_angles[ PITCH ] = original_angles[ PITCH ];
            }
         }
      }
   }

/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView(refEntity_t* pREnt, qboolean bUseWorldPosition)
{
	float* origin;
	centity_t* pCent;
	dtiki_t* tiki;
	int iTag;
	int i;
	int iMask;
	vec3_t vDelta;
	float mat[3][3];
	vec3_t vOldOrigin;
	vec3_t vStart, vEnd, vMins, vMaxs;
	trace_t trace;

	// FIXME: UNIMPLEMENTED
	assert(qfalse);
}

/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static int CG_CalcFov( void )
{
    float	x;
    float	phase;
    float	v;
    int		contents;
    float	fov_x, fov_y;
    int		inwater;

    fov_x = cg.camera_fov;
    x = cg.refdef.width / tan(fov_x / 360 * M_PI);
    fov_y = atan2(cg.refdef.height, x);
    fov_y = fov_y * 360 / M_PI;

    // warp if underwater
    contents = CG_PointContents(cg.refdef.vieworg, -1);
    if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA)) {
        phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
        v = WAVE_AMPLITUDE * sin(phase);
        fov_x += v;
        fov_y -= v;
        inwater = qtrue;
    }
    else {
        inwater = qfalse;
    }


    // set it
    cg.refdef.fov_x = fov_x;
    cg.refdef.fov_y = fov_y;
    cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
    return inwater;
}


/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues(void)
{
    playerState_t* ps;
    float SoundAngles[3];

    memset(&cg.refdef, 0, sizeof(cg.refdef));

    // calculate size of 3D view
    CG_CalcVrect();

    // setup fog and far clipping plane
    cg.refdef.farplane_distance = cg.farplane_distance;
    VectorCopy(cg.farplane_color, cg.refdef.farplane_color);
    cg.refdef.farplane_cull = cg.farplane_cull;

    // setup portal sky
    cg.refdef.sky_alpha = cg.sky_alpha;
    cg.refdef.sky_portal = cg.sky_portal;
    memcpy(cg.refdef.sky_axis, cg.sky_axis, sizeof(cg.sky_axis));
    VectorCopy(cg.sky_origin, cg.refdef.sky_origin);

    ps = &cg.predicted_player_state;

    VectorCopy(ps->origin, cg.refdef.vieworg);
    VectorCopy(ps->viewangles, cg.refdefViewAngles);

    if (cg.snap->ps.stats[STAT_HEALTH] > 0)
    {
        // FIXME
        // TODO: damage Angles
        // TODO: viewkick
    }

    // add error decay
    if (cg_errorDecay->value > 0)
    {
        int		t;
        float	f;

        t = cg.time - cg.predictedErrorTime;
        f = (cg_errorDecay->value - t) / cg_errorDecay->value;
        if (f > 0 && f < 1)
        {
            VectorMA(cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg);
        }
        else
        {
            cg.predictedErrorTime = 0;
        }
    }

    // calculate position of player's head
    cg.refdef.vieworg[2] += cg.predicted_player_state.viewheight;
    // save off the position of the player's head
    VectorCopy(cg.refdef.vieworg, cg.playerHeadPos);

    // Set the aural position of the player
    VectorCopy(cg.playerHeadPos, cg.SoundOrg);

    // Set the aural axis of the player
    VectorCopy(cg.refdefViewAngles, SoundAngles);
    // yaw is purposely inverted because of the miles sound system
    SoundAngles[YAW] = -SoundAngles[YAW];
    AnglesToAxis(SoundAngles, cg.SoundAxis);

    // decide on third person view
    cg.renderingThirdPerson = cg_3rd_person->integer;

    if (cg.renderingThirdPerson)
    {
        // back away from character
        CG_OffsetThirdPersonView();
    }

    // if we are in a camera view, we take our audio cues directly from the camera
    if (ps->pm_flags & PMF_CAMERA_VIEW)
    {
        // Set the aural position to that of the camera
        VectorCopy(cg.camera_origin, cg.refdef.vieworg);

        // Set the aural axis to the camera's angles
        VectorCopy(cg.camera_angles, cg.refdefViewAngles);

        if (ps->camera_posofs[0] || ps->camera_posofs[1] || ps->camera_posofs[2])
        {
            vec3_t vAxis, vOrg;
            AnglesToAxis(cg.refdefViewAngles, vAxis);
            MatrixTransformVector(ps->camera_posofs, vAxis, vOrg);
            VectorAdd(cg.refdef.vieworg, vOrg, cg.refdef.vieworg);
        }

        // copy view values
        VectorCopy(cg.refdef.vieworg, cg.currentViewPos);
        VectorCopy(cg.refdefViewAngles, cg.currentViewAngles);
    }

    // position eye reletive to origin
    AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

    if (cg.hyperspace) {
        cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
    }

    // field of view
    return CG_CalcFov();
}

void CG_EyePosition(vec3_t* o_vPos)
{
    // FIXME: unimplemented
}

void CG_EyeOffset(vec3_t* o_vOfs)
{
    // FIXME: unimplemented
}

void CG_EyeAngles(vec3_t* o_vAngles)
{
    // FIXME: unimplemented
}

float CG_SensitivityScale()
{
    // FIXME: unimplemented
}

void CG_AddLightShow()
{
    // FIXME: unimplemented
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, int frameTime, stereoFrame_t stereoView, qboolean demoPlayback )
{
    cg.time = serverTime;
    cg.frametime = frameTime;
    cg.demoPlayback = demoPlayback;

    // any looped sounds will be respecified as entities
    // are added to the render list
    cgi.S_ClearLoopingSounds();

    // clear all the render lists
    cgi.R_ClearScene();

    // set up cg.snap and possibly cg.nextSnap
    CG_ProcessSnapshots();

    // if we haven't received any snapshots yet, all
    // we can draw is the information screen
    if (!cg.snap || (cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE))
    {
        return;
    }

    // this counter will be bumped for every valid scene we generate
    cg.clientFrame++;

    // set cg.frameInterpolation
    if (cg.nextSnap && r_lerpmodels->integer) {
        int		delta;

        delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
        if (delta == 0) {
            cg.frameInterpolation = 0;
        }
        else {
            cg.frameInterpolation = (float)(cg.time - cg.snap->serverTime) / delta;
        }
    }
    else {
        cg.frameInterpolation = 0;	// actually, it should never be used, because 
        // no entities should be marked as interpolating
    }

    // update cg.predicted_player_state
    CG_PredictPlayerState();

    // build cg.refdef
    CG_CalcViewValues();

    // build the render lists
    if (!cg.hyperspace)
    {
        CG_AddPacketEntities();	// after calcViewValues, so predicted player state is correct
        CG_AddMarks();
    }

    // finish up the rest of the refdef
#if 0
    if (cg.testModelEntity.hModel)
    {
        CG_AddTestModel();
    }
#endif

    cg.refdef.time = cg.time;
    memcpy(cg.refdef.areamask, cg.snap->areamask, sizeof(cg.refdef.areamask));

    // update audio positions
    cgi.S_Respatialize(cg.snap->ps.clientNum, cg.SoundOrg, cg.SoundAxis);

    // make sure the lagometerSample and frame timing isn't done twice when in stereo
    if (stereoView != STEREO_RIGHT)
    {
        cg.frametime = cg.time - cg.oldTime;
        if (cg.frametime < 0)
        {
            cg.frametime = 0;
        }
        cg.oldTime = cg.time;
        CG_AddLagometerFrameInfo();
    }

    CG_UpdateTestEmitter();

    if (!cg_hidetempmodels->integer)
        CG_AddTempModels();

    CG_AddBeams();

    // actually issue the rendering calls
    CG_DrawActive(stereoView);

    if (cg_stats->integer)
    {
        cgi.Printf("cg.clientFrame:%i\n", cg.clientFrame);
    }
}

