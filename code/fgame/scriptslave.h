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

// scriptslave.h: Standard scripted objects.  Controlled by scriptmaster.  These objects
// are bmodel objects created in the editor and controlled by an external
// text based script.  Commands are interpretted on by one and executed
// upon a signal from the script master.  The base script object can
// perform several different relative and specific rotations and translations
// and can cause other parts of the script to be executed when touched, damaged,
// touched, or used.
//

#pragma once

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "mover.h"
#include "script.h"
#include "scriptmaster.h"
#include "misc.h"
#include "bspline.h"
#include "spline.h"

class ScriptSlave : public Mover
{
protected:
    float attack_finished;

    int dmg;
    int dmg_means_of_death;

public:
    qboolean commandswaiting;

    Vector TotalRotation;
    Vector NewAngles;
    Vector NewPos;
    Vector ForwardDir;
    float  speed;

    Waypoint *waypoint;
    float     traveltime;
    BSpline  *splinePath;
    float     splineTime;

    cSpline<4, 512> *m_pCurPath;
    int              m_iCurNode;

    float  m_fCurSpeed;
    float  m_fIdealSpeed;
    Vector m_vIdealPosition;
    Vector m_vIdealDir;
    float  m_fIdealAccel;
    float  m_fIdealDistance;
    float  m_fLookAhead;

    qboolean splineangles;
    qboolean ignoreangles;
    qboolean ignorevelocity;
    qboolean moving; // is the script object currently moving?

protected:
    void CheckNewOrders(void);
    void NewMove(void);

    CLASS_PROTOTYPE(ScriptSlave);

    ScriptSlave();
    ~ScriptSlave();

    void BindEvent(Event *ev);
    void EventUnbind(Event *ev);
    void DoMove(Event *ev);
    void WaitMove(Event *ev);
    void MoveEnd(Event *ev);
    void SetAnglesEvent(Event *ev);
    void SetAngleEvent(Event *ev);
    void SetModelEvent(Event *ev);
    void TriggerEvent(Event *ev) override;
    void GotoNextWaypoint(Event *ev);
    void JumpTo(Event *ev);
    void MoveToEvent(Event *ev);
    void SetSpeed(Event *ev);
    void SetTime(Event *ev);
    void MoveUp(Event *ev);
    void MoveDown(Event *ev);
    void MoveNorth(Event *ev);
    void MoveSouth(Event *ev);
    void MoveEast(Event *ev);
    void MoveWest(Event *ev);
    void MoveForward(Event *ev);
    void MoveBackward(Event *ev);
    void MoveLeft(Event *ev);
    void MoveRight(Event *ev);
    void RotateXdownto(Event *ev);
    void RotateYdownto(Event *ev);
    void RotateZdownto(Event *ev);
    void RotateAxisdownto(Event *ev);
    void RotateXupto(Event *ev);
    void RotateYupto(Event *ev);
    void RotateZupto(Event *ev);
    void RotateAxisupto(Event *ev);
    void Rotateupto(Event *ev);
    void Rotatedownto(Event *ev);
    void Rotateto(Event *ev);
    void RotateXdown(Event *ev);
    void RotateYdown(Event *ev);
    void RotateZdown(Event *ev);
    void RotateAxisdown(Event *ev);
    void RotateXup(Event *ev);
    void RotateYup(Event *ev);
    void RotateZup(Event *ev);
    void RotateAxisup(Event *ev);
    void RotateX(Event *ev);
    void RotateY(Event *ev);
    void RotateZ(Event *ev);
    void RotateAxis(Event *ev);
    void SetDamage(Event *ev);
    void SetMeansOfDeath(Event *ev);
    void FollowPath(Event *ev);
    void EndPath(Event *ev);
    void FollowingPath(Event *ev);
    void CreatePath(SplinePath *path, splinetype_t type);
    void Explode(Event *ev);
    void NotShootable(Event *ev);
    void OpenPortal(Event *ev);
    void ClosePortal(Event *ev);
    void PhysicsOn(Event *ev);
    void PhysicsOff(Event *ev);
    void PhysicsVelocity(Event *ev);
    void DamageFunc(Event *ev);
    void EventFlyPath(Event *ev);
    void EventModifyFlyPath(Event *ev);
    void SetupPath(cSpline<4, 512> *pPath, SimpleEntity *se);

    void Archive(Archiver& arc) override;
};

inline void ScriptSlave::Archive(Archiver& arc)
{
    int tempInt;

    Mover::Archive(arc);

    arc.ArchiveFloat(&attack_finished);
    arc.ArchiveInteger(&dmg);
    arc.ArchiveInteger(&dmg_means_of_death);

    arc.ArchiveBoolean(&commandswaiting);
    arc.ArchiveVector(&TotalRotation);
    arc.ArchiveVector(&NewAngles);
    arc.ArchiveVector(&NewPos);
    arc.ArchiveVector(&ForwardDir);
    arc.ArchiveFloat(&speed);
    arc.ArchiveObjectPointer((Class **)&waypoint);
    arc.ArchiveFloat(&traveltime);

    if (arc.Saving()) {
        // if it exists, archive it, otherwise place a special NULL ptr tag
        if (splinePath) {
            tempInt = ARCHIVE_POINTER_VALID;
        } else {
            tempInt = ARCHIVE_POINTER_NULL;
        }
        arc.ArchiveInteger(&tempInt);
        if (tempInt == ARCHIVE_POINTER_VALID) {
            splinePath->Archive(arc);
        }
    } else {
        arc.ArchiveInteger(&tempInt);
        if (tempInt == ARCHIVE_POINTER_VALID) {
            splinePath = new BSpline;
            splinePath->Archive(arc);
        } else {
            splinePath = NULL;
        }
    }
    arc.ArchiveFloat(&splineTime);
    arc.ArchiveBoolean(&splineangles);
    arc.ArchiveBoolean(&ignoreangles);
    arc.ArchiveBoolean(&moving);

    if (arc.Saving()) {
        // if it exists, archive it, otherwise place a special NULL ptr tag
        if (m_pCurPath) {
            tempInt = ARCHIVE_POINTER_VALID;
        } else {
            tempInt = ARCHIVE_POINTER_NULL;
        }
        arc.ArchiveInteger(&tempInt);
        if (tempInt == ARCHIVE_POINTER_VALID) {
            m_pCurPath->Archive(arc);
        }
    } else {
        arc.ArchiveInteger(&tempInt);
        if (tempInt == ARCHIVE_POINTER_VALID) {
            m_pCurPath = new cSpline<4, 512>;
            m_pCurPath->Archive(arc);
        } else {
            m_pCurPath = NULL;
        }
    }
}

class ScriptModel : public ScriptSlave
{
private:
    void GibEvent(Event *ev);

public:
    CLASS_PROTOTYPE(ScriptModel);

    ScriptModel();
    void SetAngleEvent(Event *ev);
    void SetModelEvent(Event *ev);
    void SetAnimEvent(Event *ev);
};

class ScriptOrigin : public ScriptSlave
{
public:
    CLASS_PROTOTYPE(ScriptOrigin);
    ScriptOrigin();
};

class ScriptSkyOrigin : public ScriptSlave
{
public:
    CLASS_PROTOTYPE(ScriptSkyOrigin);
    ScriptSkyOrigin();
};
