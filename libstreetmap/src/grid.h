/* 
 *
 * HEADER FILE FOR GRID BOX
 * 
 */

#ifndef GRID_H
#define GRID_H

#include "globals.h"

class Grid
{
    public:
        std::vector<FeatureDetailedInfo> Grid_Features;
        std::vector<POIDetailedInfo> Grid_POIs;
        std::vector<StreetSegmentDetailedInfo> Grid_Segments;
        std::vector<IntersectionInfo> Grid_Intersections;

        void draw_grid_features (ezgl::renderer *g, float factor);
        void draw_grid_segments (ezgl::renderer *g);
        void draw_grid_POIs (ezgl::renderer *g);
};

extern Grid MapGrids[NUM_GRIDS][NUM_GRIDS];
extern std::vector<bool> check_feature_drawn;

#endif /* GRID_H */