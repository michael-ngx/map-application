/* 
 *
 * HEADER FILE FOR DECLARING SHARED GLOBAL VARIABLES.
 * Global variables are initialized in m1.cpp.
 * 
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <unordered_map>
#include <map>
#include <vector>

// *******************************************************************
// Numbers (counts)
// *******************************************************************
extern int intersectionNum;
extern int segmentNum;
extern int streetNum;

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

    int numCurvePoints;      // number of curve points between the ends
    std::vector<LatLon> curvePoints;    // Vector of LatLon for all curvepoints
};
// Index: Segment id, Value: Processed information of the segment
extern std::vector<StreetSegmentDetailedInfo> Segment_SegmentDetailedInfo;

// *******************************************************************
// Intersections
// *******************************************************************

// Index: Intersection id, Value: vector of all segments that cross through the intersection
extern std::vector<std::vector<StreetSegmentIdx>> Intersection_AllStreetSegments;

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
// OSMNode
// *******************************************************************
// Keys: OSMID, Value: vector of (tag, value) pairs
extern std::unordered_map<OSMID, std::vector<std::pair<std::string, std::string>>> OSM_AllTagPairs;

#endif /* GLOBALS_H */

