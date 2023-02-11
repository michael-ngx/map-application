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
#include <cctype>
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

/*******************************************************************************************************************************
 * GLOBAL VARIABLES AND HELPER FUNCTION DECLARATION
 ********************************************************************************************************************************/

// *******************************************************************
// Helper function Declaration
// *******************************************************************
void m1_init();

// *******************************************************************
// Numbers
// *******************************************************************
int intersectionNum;
int segmentNum;
int streetNum;

// *******************************************************************
// Street Segments
// *******************************************************************

// Processed information of each street segment
class StreetSegmentDetailedInfo{
    public:
        OSMID wayOSMID;             // OSM ID of the source way
                                    // NOTE: Multiple segments may match a single OSM way ID
        IntersectionIdx from, to;   // intersection ID this segment runs from/to
        bool oneWay;
        double length;
        double travel_time;
        StreetIdx streetID;         // index of street this segment belongs to
};
// Index: Segment id, Value: Processed information of the segment
std::vector<StreetSegmentDetailedInfo> Segment_SegmentDetailedInfo;

// *******************************************************************
// Intersections
// *******************************************************************

// class of distance from intersection to a location
class IntersectionDistance{
public:
    int intersectionId;
    double distance;
}; // TODO: consider changing to vector for performance
// Index: Intersection id, Value: vector of all segments that cross through the intersection
std::vector<std::vector<StreetSegmentIdx>> Intersection_AllStreetSegments;

// *******************************************************************
// Streets
// *******************************************************************

// Keys: Street id, Value: vector of all segments corresponding to that Street
std::unordered_map<StreetIdx, std::vector<StreetSegmentIdx>> Streets_AllSegments;
// Keys: Street id, Value: vector of all intersections within that Street
std::unordered_map<StreetIdx, std::vector<IntersectionIdx>> Streets_AllIntersections;
// Keys: Street id, Value: length of the street
std::unordered_map<StreetIdx, double> streetAllLength;
// Keys: Street names, Value: street index
std::multimap<std::string, StreetIdx> StreetName_StreetIdx;

// *******************************************************************
// OSMNode
// *******************************************************************
// Keys: OSMID, Value: vector of (tag, value) pairs
std::unordered_map<OSMID, std::vector<std::pair<std::string, std::string>>> OSM_AllTagPairs;

/*******************************************************************************************************************************
 * STREET MAP LIBRARY
 ********************************************************************************************************************************/
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
    load_successful = loadStreetsDatabaseBIN(map_streets_database_filename) &&
                    loadOSMDatabaseBIN(map_osm_database_filename);

    if (load_successful){
        m1_init();
    }
    
    delete[] temp;
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

// TODO: have not done the corner case
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    std::vector<IntersectionIdx> adjacentIntersections;
    std::vector<StreetSegmentIdx> stSegments = findStreetSegmentsOfIntersection(intersection_id);

    for(auto& i : stSegments){
        if(Segment_SegmentDetailedInfo[i].to != intersection_id)
            adjacentIntersections.push_back(Segment_SegmentDetailedInfo[i].to);
        if(!Segment_SegmentDetailedInfo[i].oneWay)
            if(Segment_SegmentDetailedInfo[i].from != intersection_id)
                adjacentIntersections.push_back(Segment_SegmentDetailedInfo[i].from);                        
    }
    
    sort(adjacentIntersections.begin(), adjacentIntersections.end());
    adjacentIntersections.erase(unique(adjacentIntersections.begin(), adjacentIntersections.end()), adjacentIntersections.end());
    
    return adjacentIntersections;
}

// Returns the geographically nearest intersection (i.e. as the crow flies) to 
// the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    std::list<IntersectionDistance> distanceContainer;
    
    //get distance from every intersection to IntersectionPosition
    for(int i = 0; i < intersectionNum; i++){       
        IntersectionDistance intersectionDistance;
        double distance = findDistanceBetweenTwoPoints(getIntersectionPosition(i), my_position);
        intersectionDistance.distance = distance;
        intersectionDistance.intersectionId = i;
        distanceContainer.push_back(intersectionDistance);
    }
    
    IntersectionDistance shortestDistance; 
    shortestDistance = *distanceContainer.begin();
    // choose the nearest position
    for(auto& i : distanceContainer){
        if(i.distance < shortestDistance.distance){
            shortestDistance.distance = i.distance;
            shortestDistance.intersectionId = i.intersectionId;
          } 
        }
              
    return shortestDistance.intersectionId;
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
    
    std::vector<IntersectionIdx> street1Intersection = Streets_AllIntersections[street_id1];
    std::vector<IntersectionIdx> street2Intersection = Streets_AllIntersections[street_id2];

    std::set_intersection(street1Intersection.begin(), street1Intersection.end(),
                        street2Intersection.begin(), street2Intersection.end(),
                        std::back_inserter(intersectionTwoSt));
    
    return intersectionTwoSt;
}

// Returns all street ids corresponding to street names that start with the 
// given prefix
// Speed Requirement --> high
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    std::vector<StreetIdx> result;

    if (street_prefix.empty()) return result;

    // Manipulate street_prefix
    std::string prefix = "";
    for (auto& c : street_prefix){
        if (c == ' ') continue;
        prefix.push_back(char(tolower(c)));
    }

    // Find street by street prefix
    std::multimap<std::string,StreetIdx>::iterator node = StreetName_StreetIdx.lower_bound(prefix); // Iterator to first candidate

    // Street with prefix will always be larger
    while (node->first.compare(0, prefix.size(), prefix) == 0){
        result.push_back(node->second);
        node++;
    }
    return result;
}

// Returns the length of a given street in meters
// Speed Requirement --> high 
double findStreetLength(StreetIdx street_id){
    //use unorderedmap to get the street length from street id
    return streetAllLength.at(street_id);
}

// Returns the nearest point of interest of the given type (e.g. "restaurant") 
// to the given position
// Speed Requirement --> none 
POIIdx findClosestPOI(LatLon my_position, std::string POItype){
    POIIdx closestPOI = 0; //return value
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
    int numberOfPoints = getNumFeaturePoints(feature_id); //total number of points
    //variables used for area calculation
    double lat1, lon1, lat2, lon2, latavg;
    double x1, y1, x2, y2;
    double featureArea = 0; //return value
    //check for non-closed polygon
    LatLon firstPoint = getFeaturePoint(feature_id, 0);
    LatLon secondPoint = getFeaturePoint(feature_id, numberOfPoints - 1);
    if ((firstPoint.latitude() == secondPoint.latitude()) && (firstPoint.longitude() == secondPoint.longitude())){
        
        //Calculate the area of the polygon
        for (int index = 0; index < numberOfPoints-1; index++){
            firstPoint = getFeaturePoint(feature_id, index);
            secondPoint = getFeaturePoint(feature_id, index + 1);
            lat1 = firstPoint.latitude() * kDegreeToRadian;
            lon1 = firstPoint.longitude() * kDegreeToRadian;
            lat2 = secondPoint.latitude() * kDegreeToRadian;
            lon2 = secondPoint.longitude() * kDegreeToRadian;
            latavg = (lat1 + lat2)/2;
            x1 = kEarthRadiusInMeters * lon1 * cos(latavg);
            y1 = kEarthRadiusInMeters * lat1;
            x2 = kEarthRadiusInMeters * lon2 * cos(latavg);
            y2 = kEarthRadiusInMeters * lat2;
            
            featureArea = featureArea + (y2 - y1) * std::abs((x1+x2)/2);
        }
        return std::abs(featureArea);
    } else {
        return 0.0;
    }
}

// Return the value associated with this key on the specified OSMNode.
// If this OSMNode does not exist in the current map, or the specified key is 
// not set on the specified OSMNode, return an empty string.
// Speed Requirement --> high
std::string getOSMNodeTagValue (OSMID OSMid, std::string key){
    //use try and catch block to check for out of range OSMid
    try{
        //use the OSM_AllTagPairs container to find given OSMid
        auto tempVector = OSM_AllTagPairs.at(OSMid);
        for (auto tempPair = tempVector.begin(); tempPair != tempVector.end(); ++tempPair){
            //return value corresponding to a key
            if ((*tempPair).first == key){
                return (*tempPair).second;
            }
        }
        return "";
    }
    catch (const std::out_of_range& oor){
        return "";
    }
}

void closeMap() {
    //Clean-up your map related data structures here
    Segment_SegmentDetailedInfo.clear();
    Intersection_AllStreetSegments.clear();
    Streets_AllSegments.clear();
    Streets_AllIntersections.clear();
    streetAllLength.clear();
    StreetName_StreetIdx.clear();

    closeStreetDatabase();
    closeOSMDatabase();
}

/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/
void m1_init(){
    // *******************************************************************
    // Numbers
    // *******************************************************************
    // Retrive total numbers from API
    segmentNum = getNumStreetSegments();
    streetNum = getNumStreets();
    intersectionNum = getNumIntersections();

    // *******************************************************************
    // Street Segments
    // *******************************************************************
    // Vector of StreetSegmentDetailedInfo (StreetSegmentIdx - StreetSegmentDetailedInfo)
    for (int segment = 0; segment < segmentNum; segment++){                 // Corresponds to id of all street segments
        StreetSegmentInfo rawInfo = getStreetSegmentInfo(segment);          // Raw info object   
        StreetSegmentDetailedInfo processedInfo;                            // Processed info object
        
        processedInfo.wayOSMID = rawInfo.wayOSMID;
        processedInfo.from = rawInfo.from;
        processedInfo.to = rawInfo.to;
        processedInfo.oneWay = rawInfo.oneWay;
        processedInfo.streetID = rawInfo.streetID;

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

    // *******************************************************************
    // Intersections
    // *******************************************************************
    // Vector for Intersection (IntersectionIdx - Vector of Segments)
    for (IntersectionIdx intersection = 0; intersection < intersectionNum; intersection++){
        std::vector<StreetSegmentIdx> allSegments;
        for(int segment = 0; segment < getNumIntersectionStreetSegment(intersection); segment++) {
            StreetSegmentIdx ss_id = getIntersectionStreetSegment(intersection, segment);
            allSegments.push_back(ss_id);
        }
        Intersection_AllStreetSegments.push_back(allSegments);
    }

    // *******************************************************************
    // Streets
    // *******************************************************************
    for(int j = 0; j < segmentNum; ++j){
        // Unordered Map for Streets (StreetIdx - Vector of All Segments)
        StreetSegmentDetailedInfo segmentInfo = Segment_SegmentDetailedInfo[j];
        if (Streets_AllSegments.find(segmentInfo.streetID) == Streets_AllSegments.end()){
            std::vector<StreetSegmentIdx> segmentsVector;
            segmentsVector.push_back(j);
            Streets_AllSegments.insert(std::make_pair(segmentInfo.streetID, segmentsVector));
        }
        else {
            Streets_AllSegments.at(segmentInfo.streetID).push_back(j);
        }
        
        //Unordered Map for Streets (StreetIdx - length of street)
        double tempLength = Segment_SegmentDetailedInfo[j].length;
        if (streetAllLength.find(segmentInfo.streetID) == streetAllLength.end()){
            streetAllLength.insert(std::make_pair(segmentInfo.streetID, tempLength));
        }
        else {
            streetAllLength.at(segmentInfo.streetID) += tempLength;
        }
    }

    for (auto& pair : Streets_AllSegments){
        // Populate ordered multimap for Streets (StreetName - Street index)
        std::string str = getStreetName(pair.first);
        std::string streetName = "";
        for (auto& c : str){
            if (c == ' ') continue;
            streetName.push_back(char(tolower(c))); // Save names as lowercase, no space
        }
        StreetName_StreetIdx.insert(std::make_pair(streetName, pair.first)); // Add (name, streetidx) pair

        // 2D Vector for Streets (StreetIdx - Vector of All Intersections)
        Streets_AllIntersections[pair.first] = findIntersectionsOfStreet(pair.first);
    }

    // Unordered Map for OSMDatabase (OSMID - vector of pair(tag, value)
    for (int index = 0; index < getNumberOfNodes(); ++index){
        const OSMNode* tempOSMNode = getNodeByIndex(index);
        OSMID tempOSMID = tempOSMNode->id();
        for (int tagIdx = 0; tagIdx < getTagCount(tempOSMNode); ++tagIdx){
            if (OSM_AllTagPairs.find(tempOSMID) == OSM_AllTagPairs.end()){
                std::vector<std::pair<std::string, std::string>> tempVector;
                tempVector.push_back(getTagPair(tempOSMNode, 0));
                OSM_AllTagPairs.insert(std::make_pair(tempOSMID, tempVector));
            } else {
                OSM_AllTagPairs.at(tempOSMID).push_back(getTagPair(tempOSMNode, tagIdx));
            }
        }
    }
}

