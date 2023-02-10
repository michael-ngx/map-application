/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <set>
#include <unordered_map>
#include <list>
#include <cmath>
#include <bits/stdc++.h>
// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection 
// data that is higher-level than the raw OSM data). 
// This file name will always end in ".streets.bin" and you 
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1 
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the 
// name of the ".osm.bin" file that matches your map -- just change 
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.

/*********************************************************************************
 * GLOBAL VARIABLES
 **********************************************************************************/

std::vector<std::vector<StreetSegmentIdx>> intersection_street_segments;
std::vector<StreetSegmentInfo> st_segment_info;

// class for street information
class StreetInfo{
public:
    std::string name;
    std::vector<IntersectionIdx> intersectionId;
    std::vector<StreetSegmentIdx> segmentId;
};

// class of distance from intersection to a location
class IntersectionDistance{
public:
    int intersectionId;
    int distance;
    //IntersectionDistance();
};

std::vector<StreetSegmentIdx> StreetSegmentIndices;
std::vector<std::vector<StreetSegmentIdx>> streetsSegment;
std::vector<StreetIdx> streets;
std::unordered_map<StreetSegmentIdx, std::vector<IntersectionIdx>> streetSegmentsIntersection;

// *********************************
// Numbers
// *********************************

int intersectionNum;
int segmentNum;
int streetNum;

//*********************************
// Street Segments
//*********************************

// Processed information of each street segment
class StreetSegmentDetailedInfo{
    public:
        IntersectionIdx from, to;  // intersection ID this segment runs from/to
        double length;
        double travel_time;
};
// Index: Segment id, Value: Processed information of the segment
std::vector<StreetSegmentDetailedInfo> Segment_SegmentDetailedInfo;

//*********************************
// Intersections
//*********************************

// Index: Intersection id, Value: vector of all segments that cross through the intersection
std::vector<std::vector<StreetSegmentIdx>> Intersection_AllStreetSegments;

//*********************************
// Streets
//*********************************

// Keys: Street id, Value: vector of all segments corresponding to that Street
std::unordered_map<StreetIdx, std::vector<StreetSegmentIdx>> Streets_AllSegments;

/*********************************************************************************
 * HELPER FUNCTIONS
 **********************************************************************************/
void m1_init(){

    // Retrive total numbers from API
    segmentNum = getNumStreetSegments();
    streetNum = getNumStreets();
    intersectionNum = getNumIntersections();
//
    for(int i = 0; i < streetNum; i++){
        std::vector<StreetSegmentIdx> streetsSegmentPlaceHolder;
        streetsSegment.push_back(streetsSegmentPlaceHolder);
    }
//    
    for(int i = 0; i < segmentNum; i++){
        StreetSegmentInfo info = getStreetSegmentInfo(i);
        st_segment_info.push_back(info);
        StreetIdx stIdx = info.streetID;
//        
        IntersectionIdx from = info.from;
        IntersectionIdx to = info.to;
        StreetSegmentIndices.push_back(i); // contains all the relevent street segment's index
//        
        std::vector<IntersectionIdx> new_intersectionIndex;
        new_intersectionIndex.push_back(from);
        new_intersectionIndex.push_back(to);
//        
        streetSegmentsIntersection[i] = new_intersectionIndex; // the beginning and end of intersections of street segment stored
        streetsSegment[stIdx].push_back(i); //street information that stores corresponding segments       
    }
    
    // Vector of StreetSegmentDetailedInfo (StreetSegmentIdx - StreetSegmentDetailedInfo)
    for (int segment = 0; segment < segmentNum; segment++){         // Corresponds to id of all street segments
        StreetSegmentInfo rawInfo = getStreetSegmentInfo(segment);  // Raw info object
        StreetSegmentDetailedInfo processedInfo;                    // Processed info object
        
        processedInfo.from = rawInfo.from;
        processedInfo.to = rawInfo.to;

        // Pre-calculate length of each street segments (including curve points)
        if (rawInfo.numCurvePoints == 0){
            LatLon point_1 = getIntersectionPosition(rawInfo.from);
            LatLon point_2 = getIntersectionPosition(rawInfo.to);
            processedInfo.length = findDistanceBetweenTwoPoints(point_1, point_2);
        }
        else{
            LatLon point_1 = getIntersectionPosition(rawInfo.from);
            processedInfo.length = 0.0; // Starting length
            // Iterate through all curve points
            for (int i = 0; i < rawInfo.numCurvePoints; i++){
                LatLon point_2 = getStreetSegmentCurvePoint(segment, i);
                double templength = findDistanceBetweenTwoPoints(point_1, point_2);
                processedInfo.length += templength;
                point_1 = point_2;
            }
            LatLon point_2 = getIntersectionPosition(rawInfo.to);                   // Destination (to) point
            processedInfo.length += findDistanceBetweenTwoPoints(point_1, point_2);
        }

        // Pre-calculate travel time of each street segments
        processedInfo.travel_time = processedInfo.length/rawInfo.speedLimit;        // TODO: Optimize division

        // Push processed info into vector
        Segment_SegmentDetailedInfo.push_back(processedInfo);
    }

    // Vector for Intersection (IntersectionIdx - Vector of Segments)
    for (IntersectionIdx intersection = 0; intersection < intersectionNum; intersection++){
        std::vector<StreetSegmentIdx> allSegments;
        for(int segment = 0; segment < getNumIntersectionStreetSegment(intersection); segment++) {
            StreetSegmentIdx ss_id = getIntersectionStreetSegment(intersection, segment);
            allSegments.push_back(ss_id);
        }
        Intersection_AllStreetSegments.push_back(allSegments);
    }

    // Unordered Map for Streets (StreetIdx - Vector of Segments)
    for(int j = 0; j < segmentNum; ++j){
        StreetSegmentInfo tempInfo = getStreetSegmentInfo(j);
        if (Streets_AllSegments.find(tempInfo.streetID) == Streets_AllSegments.end()){
            std::vector<StreetSegmentIdx> streetSegmentIndex;
            streetSegmentIndex.push_back(j);
            Streets_AllSegments.insert(std::make_pair(tempInfo.streetID, streetSegmentIndex));
        }
        else {
            Streets_AllSegments.at(tempInfo.streetID).push_back(j);
        }
    }
}

/*********************************************************************************
 * STREET MAP LIBRARY
 **********************************************************************************/
bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = false; //Indicates whether the map has loaded 
                                  //successfully
    
    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    // Load your map related data structures here.
    //String Manipulation for OSMDatabase
    char *temp = new char[map_streets_database_filename.length() + 1];
    strcpy(temp, map_streets_database_filename.c_str());
    std::string map_osm_database_filename;
    char *tempChar = strtok(temp, ".");
    map_osm_database_filename = tempChar;
    map_osm_database_filename.append(".osm.bin");
    
    //load both StreetsDatabase and OSMDatabase
    load_successful = loadStreetsDatabaseBIN(map_streets_database_filename)&&
                      loadOSMDatabaseBIN(map_osm_database_filename);
    
    //Initialize helper functions and process loaded data if loading was successful
    if(load_successful) m1_init();

    return load_successful;
}

// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2){
    double lat1, lon1, lat2, lon2, latavg;
    double x1, y1, x2, y2;
    // Get latitude and longitude, in radians
    lat1 = point_1.latitude() * kDegreeToRadian;
    lon1 = point_1.longitude() * kDegreeToRadian;
    lat2 = point_2.latitude() * kDegreeToRadian;
    lon2 = point_2.longitude() * kDegreeToRadian;
    latavg = (lat1 + lat2)/2;
    
    x1 = kEarthRadiusInMeters * lon1 * cos(latavg);
    y1 = kEarthRadiusInMeters * lat1;
    x2 = kEarthRadiusInMeters * lon2 * cos(latavg);
    y2 = kEarthRadiusInMeters * lat2;
    // Find distance between (x1, y1) and (x2, y2)
    double distance = sqrt(pow((y2-y1),2) + pow((x2-x1),2));
    return distance;
}

// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    return Segment_SegmentDetailedInfo[street_segment_id].length;
}

// Returns the travel time to drive from one end of a street segment 
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high 
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    return Segment_SegmentDetailedInfo[street_segment_id].travel_time;
}

// Returns all intersections reachable by traveling down one street segment 
// from the given intersection (hint: you can't travel the wrong way on a 
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Corner case: cul-de-sacs can connect an intersection to itself 
// (from and to intersection on  street segment are the same). In that case
// include the intersection in the returned vector (no special handling needed).
// Speed Requirement --> high 
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    std::vector<IntersectionIdx> stub;
    return stub;
}

// Returns the geographically nearest intersection (i.e. as the crow flies) to 
// the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    std::list<IntersectionDistance> distanceContainer;
    
    //get distance from every intersection to IntersectionPosition
    for(int i = 0; i < intersectionNum; i++){
        int distance = findDistanceBetweenTwoPoints(getIntersectionPosition(i), my_position);
        IntersectionDistance intersectionDistance;
        intersectionDistance.distance = distance;
        intersectionDistance.intersectionId = i;
        distanceContainer.push_back(intersectionDistance);
    }
    
    IntersectionDistance shortestDistance; 
    shortestDistance = *distanceContainer.begin();
    // choose the nearest position
    for(auto& i : distanceContainer)
        if(i.distance < shortestDistance.distance)
            shortestDistance.distance = i.distance;
    
    return shortestDistance.distance;
}

// Returns the street segments that connect to the given intersection 
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return Intersection_AllStreetSegments[intersection_id];
}

// Returns all intersections along the a given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    // Vector to be returned
    std::vector<IntersectionIdx> IntersectionsOfStreet;
    // Get vector of all street segments in the street
    std::vector<StreetSegmentIdx> allSegments = Streets_AllSegments.find(street_id)->second;
    
    // Get all intersections for each "segment", then push_back to IntersectionOfStreet
    for (auto segment : allSegments){
        IntersectionsOfStreet.push_back(Segment_SegmentDetailedInfo[segment].from);
        IntersectionsOfStreet.push_back(Segment_SegmentDetailedInfo[segment].to);
    }
    
    //sort + unique to remove all the duplicates
    sort(IntersectionsOfStreet.begin(), IntersectionsOfStreet.end());
    IntersectionsOfStreet.erase(unique(IntersectionsOfStreet.begin(), IntersectionsOfStreet.end()), IntersectionsOfStreet.end());
    
    //using set to remove duplicates (somehow slower than the first method)
//    std::set<IntersectionIdx> IntersectionsOfStreetSet (IntersectionsOfStreet.begin(), IntersectionsOfStreet.end());
//    IntersectionsOfStreet.assign(IntersectionsOfStreetSet.begin(), IntersectionsOfStreetSet.end());
    
    return IntersectionsOfStreet;
    
}

// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual 
// curved streets it is possible to have more than one intersection at which 
// two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(StreetIdx street_id1, StreetIdx street_id2){
    std::vector<IntersectionIdx> intersectionTwoSt;
    
    std::vector<IntersectionIdx> street1Intersection = findIntersectionsOfStreet(street_id1);
    std::vector<IntersectionIdx> street2Intersection = findIntersectionsOfStreet(street_id2);
    
    // IntersectionIdx street1IntersectionNum = street1Intersection.size();
    // IntersectionIdx street2IntersectionNum = street2Intersection.size();

    // IntersectionIdx cmp_num = std::min(street1IntersectionNum, street2IntersectionNum);

    // for(IntersectionIdx i = 0; i < cmp_num; i++){
    //     for(IntersectionIdx j = 0; j < cmp_num; j++)
    //         if(street1Intersection[i] == street2Intersection[j])
    //             intersectionTwoSt.push_back(street2Intersection[j]);
    // }
    //  for(auto& i : street1Intersection){
    //      for(auto& j : street2Intersection)
    //          if(i == j)
    //              intersectionTwoSt.push_back(j);
    //  }

    for(auto& i : street1Intersection){
         auto it = std::find(street2Intersection.begin(), street2Intersection.end(), i);
            if(it != street2Intersection.end())
                intersectionTwoSt.push_back(i);
     }
    
    return intersectionTwoSt;
}

// Returns all street ids corresponding to street names that start with the 
// given prefix 
// The function should be case-insensitive to the street prefix. 
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to 
// "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty 
// (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty 
// (length 0) string, but your program must not crash if street_prefix is a 
// length 0 string.
// Speed Requirement --> high 
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    std::vector<StreetIdx> stub;
    return stub;
}

// Returns the length of a given street in meters
// Speed Requirement --> high 
double findStreetLength(StreetIdx street_id){
    std::vector<StreetSegmentIdx> allSegments = Streets_AllSegments.at(street_id);
    LatLon tempLatLonA, tempLatLonB;
    double streetLength = 0;
    int x1, x2, y1, y2;
    for (auto it = allSegments.begin(); it != allSegments.end(); ++it){
        auto strInfo = getStreetSegmentInfo(*it);
        if (strInfo.numCurvePoints == 0){
            tempLatLonA = getIntersectionPosition(strInfo.from);
            tempLatLonB = getIntersectionPosition(strInfo.to);
            x1 = kEarthRadiusInMeters*tempLatLonA.longitude() * 
                     cos((tempLatLonA.latitude() + tempLatLonB.latitude()) / 2);
            y1 = kEarthRadiusInMeters*tempLatLonA.latitude();
            x2 = kEarthRadiusInMeters*tempLatLonB.longitude() * 
                     cos((tempLatLonA.latitude() + tempLatLonB.latitude()) / 2);
            y2 = kEarthRadiusInMeters*tempLatLonB.latitude();
            streetLength += sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2));
        } else {
            tempLatLonA = getIntersectionPosition(strInfo.from);
            tempLatLonB = getStreetSegmentCurvePoint(*it, 0);
            x1 = kEarthRadiusInMeters*tempLatLonA.longitude() * 
                     cos((tempLatLonA.latitude() + tempLatLonB.latitude()) / 2);
            y1 = kEarthRadiusInMeters*tempLatLonA.latitude();
            x2 = kEarthRadiusInMeters*tempLatLonB.longitude() * 
                     cos((tempLatLonA.latitude() + tempLatLonB.latitude()) / 2);
            y2 = kEarthRadiusInMeters*tempLatLonB.latitude();
            streetLength += sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2));
            for (int i = 0; i < strInfo.numCurvePoints - 1; ++i){
                tempLatLonA = getStreetSegmentCurvePoint(*it, i);
                tempLatLonB = getStreetSegmentCurvePoint(*it, i+1);
                x1 = kEarthRadiusInMeters*tempLatLonA.longitude() * 
                         cos((tempLatLonA.latitude() + tempLatLonB.latitude()) / 2);
                y1 = kEarthRadiusInMeters*tempLatLonA.latitude();
                x2 = kEarthRadiusInMeters*tempLatLonB.longitude() * 
                         cos((tempLatLonA.latitude() + tempLatLonB.latitude()) / 2);
                y2 = kEarthRadiusInMeters*tempLatLonB.latitude();
                streetLength += sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2));
            }
            tempLatLonA = getStreetSegmentCurvePoint(*it, strInfo.numCurvePoints - 1);
            tempLatLonB = getIntersectionPosition(strInfo.to);
            x1 = kEarthRadiusInMeters*tempLatLonA.longitude() * 
                     cos((tempLatLonA.latitude() + tempLatLonB.latitude()) / 2);
            y1 = kEarthRadiusInMeters*tempLatLonA.latitude();
            x2 = kEarthRadiusInMeters*tempLatLonB.longitude() * 
                     cos((tempLatLonA.latitude() + tempLatLonB.latitude()) / 2);
            y2 = kEarthRadiusInMeters*tempLatLonB.latitude();
            streetLength += sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2));
        }
    }
    return streetLength;
}

// Returns the nearest point of interest of the given type (e.g. "restaurant") 
// to the given position
// Speed Requirement --> none 
POIIdx findClosestPOI(LatLon my_position, std::string POItype){
    POIIdx closestPOI; //return value
    double tempDistance; //store temporary distance
    bool initialized = false; //flag for smallest distance initialization
    double smallestDistance; //store the smallest distance
    
    for(POIIdx POI = 0; POI <= getNumPointsOfInterest() - 1; POI++){
        //check for POI type
        if(POItype == getPOIType(POI)){
            //initialize smallest distance and the return value
            if (!initialized){
                smallestDistance = findDistanceBetweenTwoPoints(my_position, getPOIPosition(POI));
                closestPOI = POI;
                initialized = true;
            }
            tempDistance = findDistanceBetweenTwoPoints(my_position, getPOIPosition(POI));
            if (tempDistance < smallestDistance){
                smallestDistance = tempDistance;
                closestPOI = POI;
            }
        }
    }
    return closestPOI;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id){
    return 0.0;
}

// Return the value associated with this key on the specified OSMNode.
// If this OSMNode does not exist in the current map, or the specified key is 
// not set on the specified OSMNode, return an empty string.
// Speed Requirement --> high
std::string getOSMNodeTagValue (OSMID OSMid, std::string key){
    return "stub";
}

void closeMap() {
    //Clean-up your map related data structures here
    closeStreetDatabase();
    //closeOSMDatabase();
}
