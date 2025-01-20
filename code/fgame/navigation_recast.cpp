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

#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"

static const float recastCellSize = 16.0;
static const float recastCellHeight = 0.2;
static const float agentHeight = 0.5;
static const float agentMaxClimb = 18.0;
static const float agentRadius = 0.5;
static const int regionMinSize = 8;
static const int regionMergeSize = 20;
static const float edgeMaxLen = 12.0;
static const float edgeMaxError = 1.3;
static const int vertsPerPoly = 6;
static const float detailSampleDist = 16;
static const float detailSampleMaxError = 1.0;

static const float worldScale = 30.5 / 16.0;

/// Recast build context.
class RecastBuildContext : public rcContext
{
protected:	
    virtual void doResetLog() override
    {
    }

    virtual void doLog(const rcLogCategory category, const char* msg, const int len) override
    {
        gi.DPrintf("Recast (category %d): %s\n", (int)category, msg);
    }
};

/*
============
G_Navigation_BuildRecastMesh
============
*/
void G_Navigation_BuildRecastMesh(navMap_t& navigationMap)
{
    Vector minBounds, maxBounds;
    int gridSizeX, gridSizeZ;
    RecastBuildContext buildContext;
    const size_t numIndexes = navigationMap.indices.NumObjects();
    const size_t numTris = numIndexes / 3;
    const size_t numVertices = navigationMap.vertices.NumObjects();
    const int* trisBuffer = (const int*)navigationMap.indices.AddressOfObjectAt(1);
    const unsigned int walkableHeight = (int)ceilf(agentHeight / recastCellHeight);
    const unsigned int walkableClimb = (int)floorf(agentMaxClimb / recastCellHeight);
    const unsigned int walkableRadius = (int)ceilf(agentRadius / recastCellSize);

    //
    // Recreate the vertice buffer so it's compatible
    // with Recast's right-handed Y-up coordinate system
    float* vertsBuffer = new float[numVertices * 3];

    for (size_t i = 0; i < numVertices; i++) {
        const Vector& inVertice = navigationMap.vertices.ObjectAt(i + 1);
        vertsBuffer[i * 3 + 0] = inVertice[0] * worldScale;
        vertsBuffer[i * 3 + 1] = inVertice[2] * worldScale;
        vertsBuffer[i * 3 + 2] = inVertice[1] * worldScale;
    }

    //
    // Calculate the grid size
    //
    rcCalcBounds(vertsBuffer, numVertices, minBounds, maxBounds);
    rcCalcGridSize(minBounds, maxBounds, recastCellSize, &gridSizeX, &gridSizeZ);

    rcHeightfield* heightfield = rcAllocHeightfield();

    //
    // Rasterize polygons
    //

    rcCreateHeightfield(&buildContext, *heightfield, gridSizeX, gridSizeZ, minBounds, maxBounds, recastCellSize, recastCellHeight);

    unsigned char* triAreas = new unsigned char[numTris];
    memset(triAreas, 0, sizeof(unsigned char) * numTris);

    rcMarkWalkableTriangles(&buildContext, 45, vertsBuffer, numVertices, trisBuffer, numTris, triAreas);
    rcRasterizeTriangles(&buildContext, vertsBuffer, numVertices, trisBuffer, triAreas, numTris, *heightfield);

    delete[] triAreas;

    //
    // Filter walkable surfaces
    //

    rcFilterLowHangingWalkableObstacles(&buildContext, walkableClimb, *heightfield);
    rcFilterLedgeSpans(&buildContext, walkableHeight, walkableClimb, *heightfield);
    rcFilterWalkableLowHeightSpans(&buildContext, walkableHeight, *heightfield);

    // Partition walkable surfaces

    rcCompactHeightfield *compactedHeightfield = rcAllocCompactHeightfield();

    rcBuildCompactHeightfield(&buildContext, walkableHeight, walkableClimb, *heightfield, *compactedHeightfield);

    // The heightfield is not needed anymore
    rcFreeHeightField(heightfield);

    rcErodeWalkableArea(&buildContext, walkableRadius, *compactedHeightfield);

    rcBuildDistanceField(&buildContext, *compactedHeightfield);
    rcBuildRegions(&buildContext, *compactedHeightfield, 0, Square(regionMinSize), Square(regionMergeSize));

    //
    // Simplify region contours
    //

    rcContourSet *contourSet = rcAllocContourSet();

    rcBuildContours(&buildContext, *compactedHeightfield, edgeMaxError, edgeMaxLen, *contourSet);

    //
    // Build polygon mesh from contours
    //

    rcPolyMesh *polyMesh = rcAllocPolyMesh();
    
    rcBuildPolyMesh(&buildContext, *contourSet, vertsPerPoly, *polyMesh);

    //
    // Create detail mesh to access approximate height for each polygon
    //

    rcPolyMeshDetail* polyMeshDetail = rcAllocPolyMeshDetail();

    rcBuildPolyMeshDetail(&buildContext, *polyMesh, *compactedHeightfield, recastCellSize * detailSampleDist, detailSampleMaxError, *polyMeshDetail);

    rcFreeCompactHeightfield(compactedHeightfield);
    rcFreeContourSet(contourSet);

    delete[] vertsBuffer;

    //
    // Create detour data
    //

    unsigned char* navData = NULL;
    int navDataSize = 0;

    dtNavMeshCreateParams dtParams{ 0 };

    dtParams.verts = polyMesh->verts;
    dtParams.vertCount = polyMesh->nverts;
    dtParams.polys = polyMesh->polys;
    dtParams.polyAreas = polyMesh->areas;
    dtParams.polyFlags = polyMesh->flags;
    dtParams.polyCount = polyMesh->npolys;
    dtParams.nvp = polyMesh->nvp;
    dtParams.detailMeshes = polyMeshDetail->meshes;
    dtParams.detailVerts = polyMeshDetail->verts;
    dtParams.detailVertsCount = polyMeshDetail->nverts;
    dtParams.detailTris = polyMeshDetail->tris;
    dtParams.detailTriCount = polyMeshDetail->ntris;
    dtParams.walkableHeight = agentHeight;
    dtParams.walkableRadius = agentRadius;
    dtParams.walkableClimb = agentMaxClimb;
    rcVcopy(minBounds, polyMesh->bmin);
    rcVcopy(maxBounds, polyMesh->bmax);
    dtParams.cs = recastCellSize;
    dtParams.ch = recastCellHeight;
    dtParams.buildBvTree = true;

    dtCreateNavMeshData(&dtParams, &navData, &navDataSize);

    dtNavMesh* navMesh = dtAllocNavMesh();

    dtStatus status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);

    dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
    navQuery->init(navMesh, 2048);


    if (dtStatusFailed(status))
    {
        buildContext.log(RC_LOG_ERROR, "Could not init Detour navmesh query");
        return;
    }

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
                const Vector org = Vector(x / worldScale, z / worldScale, y / worldScale + 16);
                int l;

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
}

/*
============
G_Navigation_LoadWorldMap
============
*/
void G_Navigation_LoadWorldMap(const char *mapname)
{
    navMap_t navigationMap;
    int start, end;

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
