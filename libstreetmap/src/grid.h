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
        std::vector<StreetSegmentDetailedInfo> Grid_Non_Motorway_Segments;
        std::vector<StreetSegmentDetailedInfo> Grid_Motorway_Segments;
        std::vector<StreetSegmentDetailedInfo> Grid_Names_And_Arrows_Segments;
        std::vector<IntersectionInfo> Grid_Intersections;

        void draw_grid_features (ezgl::renderer *g, float factor);
        void draw_grid_segments (ezgl::renderer *g);
        void draw_grid_POIs (ezgl::renderer *g);
        void draw_grid_names_or_arrows (ezgl::renderer *g);
};
extern Grid MapGrids[NUM_GRIDS][NUM_GRIDS];

// Index: FeatureIdx, value: boolean to check if a feature has been drawn
extern std::vector<bool> check_feature_drawn;
// Index: StreetSegmentIdx, value: boolean to check if a segment has been drawn
extern std::vector<bool> check_segment_drawn;

#endif /* GRID_H */