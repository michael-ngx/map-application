/* 
 *
 * HEADER FILE FOR GRID BOX
 * 
 */

#ifndef GRID_H
#define GRID_H

#include "globals.h"

struct Grid
{
    ezgl::rectangle Grid_Rectangle;
    std::vector<StreetSegmentDetailedInfo> Gird_Segments;
    std::vector<IntersectionInfo> Grid_Intersections;
    std::unordered_map<StreetIdx, StreetInfo> Grid_Streets;
    std::vector<FeatureDetailedInfo> Grid_Features;
    std::vector<POIDetailedInfo> Grid_POIs;
};

extern Grid MapGrids[NUM_GRIDS][NUM_GRIDS];

#endif /* GRID_H */