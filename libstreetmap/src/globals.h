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
    bool highlight = false;
};

// Index: Intersection id, Value: Pre-processed Intersection info
extern std::vector<IntersectionInfo> Intersection_IntersectionInfo;

// *******************************************************************
// Streets
// *******************************************************************

// Keys: Street id, Value: vector of all segments corresponding to that Street
extern std::unordered_map<StreetIdx, std::vector<StreetSegmentIdx>> Streets_AllSegments;
// Keys: Street id, Value: vector of all intersections within that Street
extern std::unordered_map<StreetIdx, std::vector<IntersectionIdx>> Streets_AllIntersections;
// Keys: Street id, Value: length of the street
extern std::unordered_map<StreetIdx, double> streetAllLength;
// Keys: Street names (lower case, no space), Value: street index
extern std::multimap<std::string, StreetIdx> StreetName_lower_StreetIdx;
// Keys: Street names (full), Value: street index
extern std::multimap<std::string, StreetIdx> StreetName_full_StreetIdx;

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
struct POIDetailedInfo{
    std::string POIType;
    std::string POIName;
    ezgl::point2d POIPoint;
    OSMID POIOSMID;
};
// Index: POIIdx, Value: structure that stores all POI information
extern std::vector<POIDetailedInfo> POI_AllInfo;
// Key: POI Name, Value: All Food POI locations
extern std::unordered_map<std::string, ezgl::point2d> POI_AllFood;

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

