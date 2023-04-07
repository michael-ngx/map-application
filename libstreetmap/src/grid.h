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
        std::vector<StreetSegmentDetailedInfo> Grid_Segments_Non_Motorway;
        std::vector<StreetSegmentDetailedInfo> Grid_Segments_Motorway;
        std::vector<StreetSegmentDetailedInfo> Grid_Segments_Names;
        std::vector<IntersectionInfo> Grid_Intersections;
        std::vector<SubwayStation> Grid_Subway_Stations;

        void draw_grid_features (ezgl::renderer *g, double limit);
        void draw_grid_segments (ezgl::renderer *g);
        void draw_grid_POIs (ezgl::renderer *g);
        void draw_grid_names (ezgl::renderer *g);
        void draw_grid_subway_stations (ezgl::renderer *g);
};
extern Grid MapGrids[NUM_GRIDS][NUM_GRIDS];

// Index: FeatureIdx, value: boolean to check if a feature has been drawn
extern std::vector<bool> check_feature_drawn;
// Index: StreetSegmentIdx, value: boolean to check if a segment has been drawn
extern std::vector<bool> check_segment_drawn;
// Index: StreetSegmentIdx, value: boolean to check if a segment name has been drawn
extern std::vector<bool> check_name_drawn;

#endif /* GRID_H */