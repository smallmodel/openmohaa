/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

// navigation -- Modern navigation system using Recast and Detour

#include "g_local.h"
#include "navigation_recast.h"
#include "../script/scriptexception.h"
#include "navigate.h"
#include "debuglines.h"
#include "level.h"

#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourCrowd.h"
#include "DetourNode.h"

static const float recastCellSize   = 12.5f;
static const float recastCellHeight = 1.0;
static const float agentHeight      = CROUCH_VIEWHEIGHT;
static const float agentMaxClimb    = STEPSIZE;
static const float agentMaxSlope = 45.5729942f; // normal of { 0.714142799, 0, 0.700000048 }, or an angle of -44.4270058
static const float agentRadius   = 0.5;
static const int   regionMinSize = 5;
static const int   regionMergeSize      = 20;
static const float edgeMaxLen           = 100.0;
static const float edgeMaxError         = 1.3f;
static const int   vertsPerPoly         = 6;
static const float detailSampleDist     = 12.0;
static const float detailSampleMaxError = 1.0;

static const float worldScale = 30.5 / 16.0;

navMap_t prev_navMap;

rcPolyMesh     *navPolyMesh;
dtNavMesh      *navMeshDt;
dtNavMeshQuery *navMeshQuery;
dtCrowd        *navCrowd;
int             navAgentId = -1;

static Vector ai_startpath;
static Vector ai_endpath;
static Vector ai_pathlist[256];
static int    ai_numPaths = 0;
static int    ai_lastpath = 0;

/// Recast build context.
class RecastBuildContext : public rcContext
{
protected:
    virtual void doResetLog() override {}

    virtual void doLog(const rcLogCategory category, const char *msg, const int len) override
    {
        gi.DPrintf("Recast (category %d): %s\n", (int)category, msg);
    }
};

/*
============
ConvertFromGameCoord
============
*/
static void ConvertFromGameCoord(const float *in, float *out)
{
#if 0
    vec3_t mat[3];
    vec3_t angles = { 0, 0, -90 };
    AnglesToAxis(angles, mat);

    MatrixTransformVector(in, mat, out);
#endif

    // Same as above
    out[0] = in[0];
    out[1] = in[2];
    out[2] = -in[1];
}

/*
============
ConvertToGameCoord
============
*/
static void ConvertToGameCoord(const float *in, float *out)
{
#if 0
    vec3_t mat[3];
    vec3_t angles = { 0, 0, 90 };
    AnglesToAxis(angles, mat);

    MatrixTransformVector(in, mat, out);
#endif

    out[0] = in[0];
    out[1] = -in[2];
    out[2] = in[1];
}

void TestAgent(const Vector& start, const Vector& end, Vector *paths, int *numPaths, int maxPaths)
{
    vec3_t        half = {64, 64, 64};
    vec3_t        startNav;
    vec3_t        endNav;
    dtQueryFilter filter;
    dtPolyRef     nearestStartRef, nearestEndRef;
    vec3_t        nearestStartPt, nearestEndPt;

    ConvertFromGameCoord(start, startNav);
    ConvertFromGameCoord(end, endNav);

    if (navAgentId != -1) {
        navCrowd->removeAgent(navAgentId);
    }

    dtCrowdAgentParams ap {0};

    ap.radius                = agentRadius;
    ap.height                = agentHeight;
    ap.maxAcceleration       = 8.0f;
    ap.maxSpeed              = 3.5f;
    ap.collisionQueryRange   = ap.radius * 12.0f;
    ap.pathOptimizationRange = ap.radius * 30.0f;
    ap.updateFlags           = 0;
    navAgentId               = navCrowd->addAgent(startNav, &ap);

    navMeshQuery->findNearestPoly(startNav, half, &filter, &nearestStartRef, nearestStartPt);
    navMeshQuery->findNearestPoly(endNav, half, &filter, &nearestEndRef, nearestEndPt);

    navCrowd->requestMoveTarget(navAgentId, nearestEndRef, nearestEndPt);

    dtPolyRef polys[256];
    int       nPolys;
    navMeshQuery->findPath(nearestStartRef, nearestEndRef, nearestStartPt, nearestEndPt, &filter, polys, &nPolys, 256);
    navMeshQuery->findStraightPath(
        nearestStartPt, nearestEndPt, polys, nPolys, (float *)paths, NULL, NULL, numPaths, maxPaths
    );

    for (int i = 0; i < *numPaths; i++) {
        Vector converted;

        ConvertToGameCoord(paths[i], converted);
        paths[i] = converted;
    }
}

/*
============
G_Navigation_BuildRecastMesh
============
*/
void G_Navigation_BuildRecastMesh(navMap_t& navigationMap)
{
    Vector             minBounds, maxBounds;
    int                gridSizeX, gridSizeZ;
    RecastBuildContext buildContext;
    const size_t       numIndexes     = navigationMap.indices.NumObjects();
    const size_t       numTris        = numIndexes / 3;
    const size_t       numVertices    = navigationMap.vertices.NumObjects();
    const unsigned int walkableHeight = (int)ceilf(agentHeight / recastCellHeight);
    const unsigned int walkableClimb  = (int)floorf(agentMaxClimb / recastCellHeight);
    const unsigned int walkableRadius = (int)ceilf(agentRadius / recastCellSize);
    const unsigned int maxEdgeLen     = (int)(edgeMaxLen / recastCellSize);

    //
    // Recreate the vertice buffer so it's compatible
    // with Recast's right-handed Y-up coordinate system
    float *vertsBuffer = new float[numVertices * 3];

    for (size_t i = 0; i < numVertices; i++) {
        const navVertice_t& inVertice = navigationMap.vertices.ObjectAt(i + 1);
        ConvertFromGameCoord(inVertice.xyz, &vertsBuffer[i * 3]);
    }

    int *indexesBuffer = new int[numIndexes];
    for (size_t i = 0; i < numIndexes; i += 3) {
        indexesBuffer[i + 0] = navigationMap.indices[i + 2];
        indexesBuffer[i + 1] = navigationMap.indices[i + 1];
        indexesBuffer[i + 2] = navigationMap.indices[i + 0];
    }

    //
    // Calculate the grid size
    //
    rcCalcBounds(vertsBuffer, numVertices, minBounds, maxBounds);
    rcCalcGridSize(minBounds, maxBounds, recastCellSize, &gridSizeX, &gridSizeZ);

    rcHeightfield *heightfield = rcAllocHeightfield();

    //
    // Rasterize polygons
    //

    rcCreateHeightfield(
        &buildContext, *heightfield, gridSizeX, gridSizeZ, minBounds, maxBounds, recastCellSize, recastCellHeight
    );

    unsigned char *triAreas = new unsigned char[numTris];
    memset(triAreas, 0, sizeof(unsigned char) * numTris);

    rcMarkWalkableTriangles(&buildContext, agentMaxSlope, vertsBuffer, numVertices, indexesBuffer, numTris, triAreas);
    rcRasterizeTriangles(
        &buildContext, vertsBuffer, numVertices, indexesBuffer, triAreas, numTris, *heightfield, walkableClimb
    );

    delete[] triAreas;
    delete[] vertsBuffer;
    delete[] indexesBuffer;

    //
    // Filter walkable surfaces
    //

    rcFilterLowHangingWalkableObstacles(&buildContext, walkableClimb, *heightfield);

    //rcFilterLedgeSpans(&buildContext, walkableHeight, walkableClimb, *heightfield);
    rcFilterWalkableLowHeightSpans(&buildContext, walkableHeight, *heightfield);

    // Partition walkable surfaces

    rcCompactHeightfield *compactedHeightfield = rcAllocCompactHeightfield();

    rcBuildCompactHeightfield(&buildContext, walkableHeight, walkableClimb, *heightfield, *compactedHeightfield);

    // The heightfield is not needed anymore
    rcFreeHeightField(heightfield);

    rcErodeWalkableArea(&buildContext, walkableRadius, *compactedHeightfield);

    rcBuildDistanceField(&buildContext, *compactedHeightfield);
    rcBuildRegions(&buildContext, *compactedHeightfield, 0, Square(regionMinSize), Square(regionMergeSize));

    //rcBuildRegionsMonotone(&buildContext, *compactedHeightfield, 0, Square(regionMinSize), Square(regionMergeSize));

    //
    // Simplify region contours
    //

    rcContourSet *contourSet = rcAllocContourSet();

    rcBuildContours(&buildContext, *compactedHeightfield, edgeMaxError, maxEdgeLen, *contourSet);

    //
    // Build polygon mesh from contours
    //

    rcPolyMesh *polyMesh = rcAllocPolyMesh();

    rcBuildPolyMesh(&buildContext, *contourSet, vertsPerPoly, *polyMesh);

    //
    // Create detail mesh to access approximate height for each polygon
    //

    rcPolyMeshDetail *polyMeshDetail = rcAllocPolyMeshDetail();

    rcBuildPolyMeshDetail(
        &buildContext,
        *polyMesh,
        *compactedHeightfield,
        recastCellSize * detailSampleDist,
        detailSampleMaxError,
        *polyMeshDetail
    );

    rcFreeCompactHeightfield(compactedHeightfield);
    rcFreeContourSet(contourSet);

    // Update poly flags from areas.
    for (int i = 0; i < polyMesh->npolys; ++i) {
        if (polyMesh->areas[i] == RC_WALKABLE_AREA) {
            polyMesh->flags[i] = 1;
        }
    }

    //
    // Create detour data
    //

    unsigned char *navData     = NULL;
    int            navDataSize = 0;

    dtNavMeshCreateParams dtParams {0};

    dtParams.verts            = polyMesh->verts;
    dtParams.vertCount        = polyMesh->nverts;
    dtParams.polys            = polyMesh->polys;
    dtParams.polyAreas        = polyMesh->areas;
    dtParams.polyFlags        = polyMesh->flags;
    dtParams.polyCount        = polyMesh->npolys;
    dtParams.nvp              = polyMesh->nvp;
    dtParams.detailMeshes     = polyMeshDetail->meshes;
    dtParams.detailVerts      = polyMeshDetail->verts;
    dtParams.detailVertsCount = polyMeshDetail->nverts;
    dtParams.detailTris       = polyMeshDetail->tris;
    dtParams.detailTriCount   = polyMeshDetail->ntris;
    dtParams.walkableHeight   = agentHeight;
    dtParams.walkableRadius   = agentRadius;
    dtParams.walkableClimb    = agentMaxClimb;
    rcVcopy(dtParams.bmin, polyMesh->bmin);
    rcVcopy(dtParams.bmax, polyMesh->bmax);
    dtParams.cs          = recastCellSize;
    dtParams.ch          = recastCellHeight;
    dtParams.buildBvTree = true;

    dtCreateNavMeshData(&dtParams, &navData, &navDataSize);

    dtNavMesh *navMesh = dtAllocNavMesh();

    dtStatus status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);

    rcFreePolyMeshDetail(polyMeshDetail);
    rcFreePolyMesh(polyMesh);

    navMeshQuery = dtAllocNavMeshQuery();
    navMeshQuery->init(navMesh, 2048);

    if (dtStatusFailed(status)) {
        buildContext.log(RC_LOG_ERROR, "Could not init Detour navmesh query");
        return;
    }

    navCrowd = dtAllocCrowd();
    navCrowd->init(MAX_CLIENTS, agentRadius, navMesh);

    navPolyMesh = polyMesh;
    navMeshDt   = navMesh;
    prev_navMap = navigationMap;

    /*
    for (int i = 0; i < polyMesh->npolys; ++i)
    {
        const unsigned short* p = &polyMesh->polys[i* polyMesh->nvp*2];
        const unsigned char area = polyMesh->areas[i];

        if (area != RC_WALKABLE_AREA) {
            continue;
        }

        unsigned short vi[3];
        for (int j = 2; j < polyMesh->nvp; ++j)
        {
            if (p[j] == RC_MESH_NULL_IDX) break;
            vi[0] = p[0];
            vi[1] = p[j-1];
            vi[2] = p[j];
            for (int k = 0; k < 3; ++k)
            {
                const unsigned short* v = &polyMesh->verts[vi[k]*3];
                const float x = polyMesh->bmin[0] + v[0]*polyMesh->cs;
                const float y = polyMesh->bmin[1] + (v[1]+1)*polyMesh->ch;
                const float z = polyMesh->bmin[2] + v[2]*polyMesh->cs;

                Vector org;
                ConvertToGameCoord(Vector(x, y, z), org);

                for (l = 0; l < PathSearch::nodecount; l++) {
                    if (PathSearch::pathnodes[l] && PathSearch::pathnodes[l]->origin == org) {
                        break;
                    }
                }

                if (l != PathSearch::nodecount) {
                    continue;
                }

                PathNode* pathNode = new PathNode;
                pathNode->origin = org;
            }
        }
    }
    */
}

void G_Navigation_DebugDraw()
{
    rcPolyMesh *polyMesh = navPolyMesh;
    Entity     *ent      = g_entities[0].entity;

    if (!navPolyMesh || !navMeshDt) {
        return;
    }

    if (!ent) {
        return;
    }

    if (ai_showallnode->integer) {
        for (int i = 0; i < navMeshDt->getMaxTiles(); i++) {
            const dtMeshTile *tile = static_cast<const dtNavMesh *>(navMeshDt)->getTile(i);

            for (int j = 0; j < tile->header->vertCount; j++) {
                const float *pvert = &tile->verts[j * 3];

                Vector org;
                ConvertToGameCoord(pvert, org);

                org.z += 16;

                if (org.z < ent->origin.z - 94 || org.z > ent->origin.z + 94) {
                    continue;
                }

                G_DebugBBox(org, Vector(-8, -8, -8), Vector(8, 8, 8), 1.0, 0.0, 0.5, 1.0);
            }
        }

#if 0
        for (int i = 0; i < polyMesh->nverts; ++i) {
            const unsigned short *v = &polyMesh->verts[i * 3];
            const float           x = polyMesh->bmin[0] + v[0] * polyMesh->cs;
            const float           y = polyMesh->bmin[1] + (v[1] + 1) * polyMesh->ch + 0.1f;
            const float           z = polyMesh->bmin[2] + v[2] * polyMesh->cs;

            Vector org;
            ConvertToGameCoord(Vector(x, y, z), org);

            org.z += 16;

            if (org.z < ent->origin.z - 94 || org.z > ent->origin.z + 94) {
                continue;
            }

            G_DebugBBox(org, Vector(-8, -8, -8), Vector(8, 8, 8), 1.0, 0.0, 0.5, 1.0);
        }
#endif
    }

    switch (ai_showroutes->integer) {
    case 0:
    default:
        break;
    case 1:
        {
            const float maxDistSquared = Square(ai_showroutes_distance->integer);

            for (int i = 0; i < navMeshDt->getMaxTiles(); i++) {
                const dtMeshTile *tile = static_cast<const dtNavMesh *>(navMeshDt)->getTile(i);

                for (int j = 0; j < tile->header->polyCount; j++) {
                    const dtPoly *poly = &tile->polys[j];

                    for (int k = 0; k < poly->vertCount; ++k) {
                        const float *pv1 = &tile->verts[poly->verts[k] * 3];
                        const float *pv2 = &tile->verts[poly->verts[(k + 1) % poly->vertCount] * 3];

                        Vector v1, v2;
                        ConvertToGameCoord(pv1, v1);
                        ConvertToGameCoord(pv2, v2);

                        const Vector delta = v1 - ent->origin;

                        if (delta.lengthSquared() >= maxDistSquared) {
                            continue;
                        }

                        G_DebugLine(v1, v2, 0, 1, 0, 1);
                    }
                }
            }
#if 0
        for (int i = 0; i < polyMesh->npolys; ++i) {
            const unsigned short *p    = &polyMesh->polys[i * polyMesh->nvp * 2];
            const unsigned char   area = polyMesh->areas[i];

            if (area != RC_WALKABLE_AREA) {
                continue;
            }

            unsigned short vi[3];
            for (int j = 2; j < polyMesh->nvp; ++j) {
                if (p[j] == RC_MESH_NULL_IDX) {
                    break;
                }
                vi[0] = p[0];
                vi[1] = p[j - 1];
                vi[2] = p[j];

                Vector vertices[3];

                for (int k = 0; k < 3; ++k) {
                    const unsigned short *v = &polyMesh->verts[vi[k] * 3];

                    vec3_t pos;
                    pos[0] = polyMesh->bmin[0] + v[0] * polyMesh->cs;
                    pos[1] = polyMesh->bmin[1] + (v[1] + 1) * polyMesh->ch;
                    pos[2] = polyMesh->bmin[2] + v[2] * polyMesh->cs;

                    ConvertToGameCoord(pos, vertices[k]);
                }

                for (int k = 0; k < 3; ++k) {
                    const Vector delta = vertices[k] - ent->origin;

                    if (delta.lengthSquared() >= maxDistSquared) {
                        continue;
                    }

                    G_DebugLine(vertices[k], vertices[(k + 1) % 3], 0, 1, 0, 1);
                }
            }
        }
#endif
        }
        break;
    case 2:
        {
            const float maxDistSquared = Square(ai_showroutes_distance->integer);

            for (int i = 0; i < prev_navMap.indices.NumObjects(); i += 3) {
                const navVertice_t& v1 = prev_navMap.vertices[prev_navMap.indices[0]];
                const navVertice_t& v2 = prev_navMap.vertices[prev_navMap.indices[1]];
                const navVertice_t& v3 = prev_navMap.vertices[prev_navMap.indices[2]];

                for (int k = 0; k < 3; ++k) {
                    const Vector delta = prev_navMap.vertices[prev_navMap.indices[i + k]].xyz - ent->origin;

                    if (delta.lengthSquared() >= maxDistSquared) {
                        continue;
                    }

                    G_DebugLine(
                        prev_navMap.vertices[prev_navMap.indices[i + k]].xyz,
                        prev_navMap.vertices[prev_navMap.indices[i + ((k + 1) % 3)]].xyz,
                        0,
                        1,
                        0,
                        1
                    );
                }
            }
        }
        break;
    }

    if (ai_showpath->integer && navMeshQuery) {
        switch (ai_showpath->integer) {
        default:
        case 0:
            ai_startpath = ai_endpath = vec_zero;
            break;
        case 1:
            ai_startpath = ent->origin;
            break;
        case 2:
            ai_endpath = ent->origin;
            break;
        case 3:
            if (ai_lastpath != ai_showpath->integer) {
                TestAgent(ai_startpath, ai_endpath, ai_pathlist, &ai_numPaths, ARRAY_LEN(ai_pathlist));
            }
            break;
        }

        ai_lastpath = ai_showpath->integer;

        navCrowd->update(level.frametime, NULL);

        for (int i = 0; i < ai_numPaths - 1; i++) {
            const Vector v1 = ai_pathlist[i] + Vector(0, 0, 64);
            const Vector v2 = ai_pathlist[i + 1] + Vector(0, 0, 64);

            G_DebugLine(v1, v2, 1.0, 0.0, 1.0, 1.0);
        }

#if 0
        if (navAgentId != -1) {
            const dtCrowdAgent* agent = navCrowd->getAgent(navAgentId);

            const dtPolyRef* path = agent->corridor.getPath();
            const int npath = agent->corridor.getPathCount();
            for (int i = 0; i < npath; ++i) {
                const dtMeshTile* tile;
                const dtPoly* poly;
                navMeshDt->getTileAndPolyByRef(path[i], &tile, &poly);

                const unsigned int tileId = (unsigned int)(poly - tile->polys);

                const dtPolyDetail* dm = &tile->detailMeshes[tileId];

                for (int j = 0; j < dm->triCount; ++j)
                {
                    const unsigned char* t = &tile->detailTris[(dm->triBase + i) * 4];
                    for (int k = 0; k < 3; ++k)
                    {
                        const float* pv1;
                        const float* pv2;

                        if (t[k] < poly->vertCount) {
                            pv1 = &tile->verts[poly->verts[t[k]] * 3];
                            pv2 = &tile->verts[poly->verts[t[(k + 1) % 3]] * 3];
                        } else {
                            pv1 = &tile->detailVerts[(dm->vertBase + t[k] - poly->vertCount) * 3];
                            pv2 = &tile->detailVerts[(dm->vertBase + t[(k + 1) % 3] - poly->vertCount) * 3];
                        }

                        Vector v1, v2;
                        ConvertToGameCoord(pv1, v1);
                        ConvertToGameCoord(pv2, v2);

                        G_DebugLine(v1, v2, 0.0, 1.0, 0.0, 1.0);
                    }
                }
            }
        }
#endif

#if 0
        const dtNodePool* pool = navMeshQuery->getNodePool();

        if (pool) {
            for (int i = 0; i < pool->getHashSize(); ++i)
            {
                for (dtNodeIndex j = pool->getFirst(i); j != DT_NULL_IDX; j = pool->getNext(j))
                {
                    const dtNode* node = pool->getNodeAtIdx(j + 1);
                    if (!node) continue;

                    Vector pos;
                    ConvertToGameCoord(node->pos, pos);

                    G_DebugBBox(pos, Vector(-8, -8, -8), Vector(8, 8, 8), 0.0, 1.0, 0.0, 1.0);
                }
            }
        }
#endif
    }
}

/*
============
G_Navigation_LoadWorldMap
============
*/
void G_Navigation_LoadWorldMap(const char *mapname)
{
    navMap_t navigationMap;
    int      start, end;

    gi.Printf("---- Recast Navigation ----\n");

    if (!sv_maxbots->integer) {
        gi.Printf("No bots, skipping navigation\n");
        return;
    }

    try {
        start = gi.Milliseconds();

        G_Navigation_ProcessBSPForNavigation(mapname, navigationMap);
    } catch (const ScriptException& e) {
        gi.Printf("Failed to load BSP for navigation: %s\n", e.string.c_str());
        return;
    }

    end = gi.Milliseconds();

    gi.Printf("BSP file loaded and parsed in %.03f seconds\n", (float)((end - start) / 1000.0));

    try {
        start = gi.Milliseconds();

        G_Navigation_BuildRecastMesh(navigationMap);
    } catch (const ScriptException& e) {
        gi.Printf("Couldn't build recast navigation mesh: %s\n", e.string.c_str());
        return;
    }

    end = gi.Milliseconds();

    gi.Printf("Recast navigation mesh generated in %.03f seconds\n", (float)((end - start) / 1000.0));
}
