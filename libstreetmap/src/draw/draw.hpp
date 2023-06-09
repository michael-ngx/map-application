/************************************************************
 * DRAWING HELPER FUNCTIONS
 ************************************************************/

#ifndef DRAW_H
#define DRAW_H

#include "ezgl/application.hpp"
#include "m1.h"
#include "globals.h"

void draw_feature_area (ezgl::renderer *g, FeatureDetailedInfo tempFeatureInfo);
void draw_POIs (ezgl::renderer* g, POIDetailedInfo POI);
void draw_street_segment_pixel (ezgl::renderer *g, StreetSegmentDetailedInfo& segment, bool on_path = false);
void draw_street_segment_meters (ezgl::renderer *g, StreetSegmentDetailedInfo& segment, bool on_path = false);
void draw_line_meters (ezgl::renderer *g, ezgl::point2d from_xy,
                      ezgl::point2d to_xy, int& width_meters);
int get_street_width_pixel (std::string& street_type);
int get_street_width_meters (std::string& street_type);
void draw_seg_name (ezgl::renderer *g, StreetSegmentDetailedInfo& segment, bool on_path = false);

void draw_png (ezgl::renderer* g, ezgl::point2d inter_xy, std::string pin_type);
void draw_subway_lines (ezgl::renderer* g);
void draw_distance_scale (ezgl::renderer *g, ezgl::rectangle current_window);

#endif /* DRAW_H */ 