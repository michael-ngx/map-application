/* 
 *
 * HEADER FILE FOR DECLARING SHARED GLOBAL VARIABLES.
 * Global variables are initialized in m1.cpp.
 * 
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <unordered_map>
#include <map>
#include <vector>

// *******************************************************************
// Other cross-milestone access constants
// *******************************************************************
// Check current map path for city switching
extern std::string CURRENT_MAP_PATH;
extern double clicked_intersection_distance;
extern double clicked_POI_distance;

// *******************************************************************
// Overloaded functions from M1
// *******************************************************************
// Find distance between 2 points based on xy
double findDistanceBetweenTwoPoints (ezgl::point2d point_1, ezgl::point2d point_2);
// Function to find closest intersection based on xy, not LatLon
IntersectionIdx findClosestIntersection(ezgl::point2d my_position);
// Function to find closest POI of any type, based on xy, not LatLon
POIIdx findClosestPOI(ezgl::point2d my_position);

// *******************************************************************
// Latlon bounds
// *******************************************************************
extern LatLonBounds latlon_bound;
extern double max_lat, max_lon;
extern double min_lat, min_lon;
extern double lat_avg;

ezgl::point2d xy_from_latlon(LatLon latlon);
LatLon latlon_from_xy(double x, double y);

// *******************************************************************
// Numbers (counts)
// *******************************************************************
extern int intersectionNum;
extern int segmentNum;
extern int streetNum;
extern int featureNum;
extern int POINum;

// *******************************************************************
// Street Segments
// *******************************************************************

// Pre-processed information of each street segment
struct StreetSegmentDetailedInfo{
    OSMID wayOSMID;             // OSM ID of the source way
                                // NOTE: Multiple segments may match a single OSM way ID
    IntersectionIdx from, to;   // Intersection ID this segment runs from/to
    bool oneWay;
    double length;
    double travel_time;
    StreetIdx streetID;         // Index of street this segment belongs to
    std::string streetName;
    int numCurvePoints;      // number of curve points between the ends
    std::vector<ezgl::point2d> curvePoints_xy; // Vector of xy for all curvepoints
    ezgl::rectangle segmentRectangle;       // Rectangle for checking display
};
// Index: Segment id, Value: Processed information of the segment
extern std::vector<StreetSegmentDetailedInfo> Segment_SegmentDetailedInfo;

// *******************************************************************
// Intersections
// *******************************************************************

// Index: Intersection id, Value: vector of all segments that cross through the intersection
extern std::vector<std::vector<StreetSegmentIdx>> Intersection_AllStreetSegments;

// Struct for preprocessed information of Intersections
struct IntersectionInfo{
    ezgl::point2d position_xy;
    std::string name;
};

// Index: Intersection id, Value: Pre-processed Intersection info
extern std::vector<IntersectionInfo> Intersection_IntersectionInfo;
// Key: Intersection name, Value: IntersectionIdx (no repeating intersection names)
extern std::unordered_map<std::string, IntersectionIdx> IntersectionName_IntersectionIdx_no_repeat;
// Key: Intersection name, Value: IntersectionIdx (Allow repeating intersection names)
extern std::unordered_multimap<std::string, IntersectionIdx> IntersectionName_IntersectionIdx;

// *******************************************************************
// Streets
// *******************************************************************
struct StreetInfo
{
    StreetIdx id;   // Street Idx
    std::string name;   // Street name (Full, without suffix)
    std::vector<StreetSegmentIdx> all_segments; // Vector of all street segment ids within that street
    std::vector<IntersectionIdx> all_intersections; // Vector of all intersections within that street
    double length;  // Length of street
};
extern std::unordered_map<StreetIdx, StreetInfo> Street_StreetInfo;

// Keys: Street names (lower case, no space), Value: street id
// If street name == "<unknown>", street name has no suffix
extern std::multimap<std::string, StreetIdx> StreetName_lower_StreetIdx;

// *******************************************************************
// Features
// *******************************************************************
//Stores pre-processed information of features
struct FeatureDetailedInfo{
    FeatureType featureType;                    // Type of the feature
    TypedOSMID  featureOSMID;                   // OSMID of the feature
    std::vector<ezgl::point2d> featurePoints;   // Coordinates of the feature in point2d
    ezgl::rectangle featureRectangle;           // Rectangle for checking display
    double featureArea;

    double temp_max_lat, temp_max_lon;          // For temporary storage only
    double temp_min_lat, temp_min_lon;          
};
//Index: FeatureIdx, Value: structure that stores all feature information
extern std::vector<FeatureDetailedInfo> Features_AllInfo;

// *******************************************************************
// POI
// *******************************************************************
struct POIDetailedInfo
{
    POIIdx id;
    std::string POIType;
    std::string POIName;
    ezgl::point2d POIPoint;
    OSMID POIOSMID;
};
// Index: POIIdx, Value: structure that stores all POI information
extern std::vector<POIDetailedInfo> POI_AllInfo;
// Key: POI Name, Value: All Food POI locations
extern std::multimap<std::string, POIDetailedInfo> POI_AllFood;

// *******************************************************************
// OSM
// *******************************************************************
// Keys: OSMID, Value: vector of (tag, value) pairs
extern std::unordered_map<OSMID, std::vector<std::pair<std::string, std::string>>> OSMID_Nodes_AllTagPairs;
// Keys: OSMID, Value: Type of highway of corresponding wayOSMID (only for segments)
extern std::unordered_map<OSMID, std::string> OSMID_Highway_Type;
// Stores subway relation information
struct SubwayRoutes
{
    OSMID route_id;                     // Unique OSMID of current relation
    ezgl::color colour;                 // Colour of subway route
    // Members - ordered (same index and same size)
    std::vector<std::string> roles;     // Roles of each member
    std::vector<TypedOSMID> members;    // TypedOSMID of each members. Can check type (Way/Node/Relations)
    std::vector<std::vector<ezgl::point2d>> track_points;    // Vector of vector of point2d for points along all raiway=subway
    std::vector<ezgl::point2d> station_points;  // Vector of point2d for all stations
};
// Keys: index, Value: Subway relations of current world
extern std::vector<SubwayRoutes> AllSubwayRoutes;

extern std::unordered_map<OSMID, int> OSMID_NodeIndex;
extern std::unordered_map<OSMID, int> OSMID_WayIndex;

#endif /* GLOBALS_H */

