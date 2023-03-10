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
#include <deque>
#include <unordered_map>
#include <map>
#include <vector>

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
// Keys: Street names, Value: street index
extern std::multimap<std::string, StreetIdx> StreetName_StreetIdx;

// *******************************************************************
// Features
// *******************************************************************
//Stores pre-processed information of features
struct FeatureDetailedInfo{
    FeatureType featureType;                    //Type of the feature
    TypedOSMID  featureOSMID;                   //OSMID of the feature
    std::vector<ezgl::point2d> featurePoints;   //Coordinates of the feature in point2d
};
//Index: FeatureIdx, Value: structure that stores all feature information
extern std::deque<FeatureDetailedInfo> Features_AllInfo;

// *******************************************************************
// OSMNode
// *******************************************************************
// Keys: OSMID, Value: vector of (tag, value) pairs
extern std::unordered_map<OSMID, std::vector<std::pair<std::string, std::string>>> OSM_AllTagPairs;
// Keys: OSMID, Value: Type of highway of corresponding wayOSMID (only for segments)
extern std::unordered_map<OSMID, std::string> OSMID_Highway_Type;

#endif /* GLOBALS_H */

