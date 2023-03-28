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
#include "m1.h"
#include "globals.h"
#include "OSMDatabaseAPI.h"
#include "draw/utilities.hpp"
#include <iostream>
#include <set>
#include <unordered_map>
#include <cmath>
#include <bits/stdc++.h>
#include <cctype>


/*******************************************************************************************************************************
 * GLOBAL VARIABLES AND HELPER FUNCTION DECLARATION
 ********************************************************************************************************************************/

// *******************************************************************
// Helper function Declaration
// *******************************************************************
void m1_init();
void init_segments();
void init_intersections();
void init_streets();
void init_features();
void init_POI();
void init_osm_nodes();
void init_osm_ways();
bool compareFeatureArea (FeatureDetailedInfo F1, FeatureDetailedInfo F2);
void init_osm_relations_subways();
ezgl::color get_rgb_color(std::string osm_color);

// *******************************************************************
// Latlon bounds of current city
// *******************************************************************
LatLonBounds latlon_bound;
double max_lat, max_lon;
double min_lat, min_lon;
double lat_avg;

// *******************************************************************
// Numbers
// *******************************************************************
int intersectionNum;
int segmentNum;
int streetNum;
int featureNum;
int POINum;

// *******************************************************************
// Shared variables
// *******************************************************************
// Initialize first city for city switching
std::string CURRENT_MAP_PATH = " ";
// Distance of closest POI/Intersection on-click to determine which to hightlight
double clicked_intersection_distance;
double clicked_POI_distance;

// *******************************************************************
// Street Segments
// *******************************************************************
// Index: Segment id, Value: Processed information of the segment
std::vector<StreetSegmentDetailedInfo> Segment_SegmentDetailedInfo;

// *******************************************************************
// Intersections
// *******************************************************************
// Index: Intersection id, Value: vector of all segments that cross through the intersection
std::vector<std::vector<StreetSegmentIdx>> Intersection_AllStreetSegments;
// Index: Intersection id, Value: Pre-processed Intersection info
std::vector<IntersectionInfo> Intersection_IntersectionInfo;
// Key: Intersection name, Value: IntersectionIdx (no repeating intersection names)
std::unordered_map<std::string, IntersectionIdx> IntersectionName_IntersectionIdx_no_repeat;
// Key: Intersection name, Value: IntersectionIdx
std::unordered_multimap<std::string, IntersectionIdx> IntersectionName_IntersectionIdx;
// Key: Intersection name (lower case, no space), Value: IntersectionIdx
std::multimap<std::string, IntersectionIdx> IntersectionName_lower_IntersectionIdx;

// *******************************************************************
// Streets
// *******************************************************************
// Keys: Street idx, Value: Street Info struct
std::unordered_map<StreetIdx, StreetInfo> Street_StreetInfo;
// Keys: Street names (lower case, no space), Value: street index
std::multimap<std::string, StreetIdx> StreetName_lower_StreetIdx;

// *******************************************************************
// Features
// *******************************************************************
//Index: FeatureIdx, Value: structure that stores all feature information
std::vector<FeatureDetailedInfo> Features_AllInfo;

// *******************************************************************
// POI
// *******************************************************************
//Index: POIIdx, Value: structure that stores all POI information
std::vector<POIDetailedInfo> POI_AllInfo;
// Key: POI Name, Value: All Food POI locations
std::multimap<std::string, POIDetailedInfo> POI_AllFood;

// *******************************************************************
// OSMNode
// *******************************************************************
// Keys: OSMID, Value: vector of (tag, value) pairs
std::unordered_map<OSMID, std::vector<std::pair<std::string, std::string>>> OSMID_Nodes_AllTagPairs;
// Keys: OSMID, Value: Type of highway of corresponding wayOSMID (only for segments)
std::unordered_map<OSMID, std::string> OSMID_Highway_Type;
// Keys: index, Value: Subway relations of current world
std::vector<SubwayRoutes> AllSubwayRoutes;

// Return Node Index and Way Index from OSMID
std::unordered_map<OSMID, int> OSMID_NodeIndex;
std::unordered_map<OSMID, int> OSMID_WayIndex;

/*******************************************************************************************************************************
 * STREET MAP LIBRARY
 ********************************************************************************************************************************/
bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = false; //Indicates whether the map has loaded successfully

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    // String Manipulation for OSMDatabase
    char *temp = new char[map_streets_database_filename.length() + 1];
    strcpy(temp, map_streets_database_filename.c_str());
    std::string map_osm_database_filename;
    char *tempChar = strtok(temp, ".");
    map_osm_database_filename = tempChar;
    map_osm_database_filename.append(".osm.bin");

    // load both StreetsDatabase and OSMDatabase
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
    // Convert to cartesian coordinates
    x1 = kEarthRadiusInMeters * lon1 * cos(latavg);
    y1 = kEarthRadiusInMeters * lat1;
    x2 = kEarthRadiusInMeters * lon2 * cos(latavg);
    y2 = kEarthRadiusInMeters * lat2;
    // Find distance between (x1, y1) and (x2, y2)
    double distance = sqrt(pow((y2-y1),2) + pow((x2-x1),2));
    return distance;
}

double findDistanceBetweenTwoPoints (ezgl::point2d point_1, ezgl::point2d point_2)
{
    return sqrt(pow((point_2.y - point_1.y), 2) + pow((point_2.x - point_1.x),2));
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
    std::vector<IntersectionIdx> adjacentIntersections;
    std::vector<StreetSegmentIdx> stSegments = findStreetSegmentsOfIntersection(intersection_id);   // All segments of intersection

    for(auto& i : stSegments){
        if(Segment_SegmentDetailedInfo[i].from == Segment_SegmentDetailedInfo[i].to){
            adjacentIntersections.push_back(Segment_SegmentDetailedInfo[i].from);       // Corner case
            continue;
        }
        if(Segment_SegmentDetailedInfo[i].to != intersection_id)                        // Can travel "to" -> always add
            adjacentIntersections.push_back(Segment_SegmentDetailedInfo[i].to);
        if(!Segment_SegmentDetailedInfo[i].oneWay && 
            Segment_SegmentDetailedInfo[i].from != intersection_id)                     // If not one way and labelled as 
                adjacentIntersections.push_back(Segment_SegmentDetailedInfo[i].from);   // from: intersection_id -> to: id_to_add
    }
    // Remove duplicate intersections (2 segments lead to 1 adjacent intersection)
    sort(adjacentIntersections.begin(), adjacentIntersections.end());
    adjacentIntersections.erase(unique(adjacentIntersections.begin(),adjacentIntersections.end()), 
                                adjacentIntersections.end());
    
    return adjacentIntersections;
}

// Returns the geographically nearest intersection (i.e. as the crow flies) to 
// the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    std::vector<IntersectionIdx> distanceContainer;
    
    // Get distance from every intersection to IntersectionPosition
    for(int i = 0; i < intersectionNum; i++){
        double distance = findDistanceBetweenTwoPoints(getIntersectionPosition(i), my_position);
        distanceContainer.push_back(distance);
    }
    
    IntersectionIdx closestIntersection = 0;                            // First intersection
    double closestDistance = distanceContainer[closestIntersection];    // Closest distance so far
    // Choose the nearest position
    for(int i = 0; i < distanceContainer.size(); i++){
        if (distanceContainer[i] < closestDistance){
            closestDistance = distanceContainer[i];
            closestIntersection = i;
        }
    }

    return closestIntersection;
}

IntersectionIdx findClosestIntersection(ezgl::point2d my_position)
{
    // Closest_distance found so far
    double closest_distance = findDistanceBetweenTwoPoints(Intersection_IntersectionInfo[0].position_xy, my_position);
    IntersectionIdx closestIntersection = 0;                            // First intersection

    // Get distance from every intersection to IntersectionPosition
    for(int i = 1; i < intersectionNum; i++){
        double distance = findDistanceBetweenTwoPoints(Intersection_IntersectionInfo[i].position_xy, my_position);
        if (distance < closest_distance)
        {
            closest_distance = distance;
            closestIntersection = i;
        }
    }
    clicked_intersection_distance = closest_distance;
    return closestIntersection;
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
    return Street_StreetInfo.at(street_id).all_intersections;
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
    // Get all intersections of 2 streets
    // Intersections are already sorted (required for set_intersection function
    std::vector<IntersectionIdx> street1Intersection = Street_StreetInfo.at(street_id1).all_intersections;
    std::vector<IntersectionIdx> street2Intersection = Street_StreetInfo.at(street_id2).all_intersections;

    // Find union of 2 vectors
    // 2 vectors are already sorted in ascending order
    std::set_intersection(street1Intersection.begin(), street1Intersection.end(),
                        street2Intersection.begin(), street2Intersection.end(),
                        std::back_inserter(intersectionTwoSt));

    return intersectionTwoSt;
}

// Returns all street ids corresponding to street names that start with the 
// given prefix
// Speed Requirement --> high
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix)
{
    std::vector<StreetIdx> result;
    // Avoid crashing with empty input.
    if (street_prefix.empty())
    {
        return result;
    }

    // Manipulate street_prefix to lowercase, ignore space
    std::string prefix = lower_no_space(street_prefix);

    // Find street by street prefix
    std::multimap<std::string, StreetIdx>::iterator node = StreetName_lower_StreetIdx.lower_bound(prefix); // Iterator to first candidate

    // Increase iterator through ordered map will always reach larger strings by string comparision
    while (node->first.compare(0, prefix.size(), prefix) == 0){
        result.push_back(node->second);
        node++;
    }
    return result;
}

// Returns all intersection ids corresponding to intersection names that start with the given prefix
std::vector<IntersectionIdx> findIntersectionIdsFromPartialIntersectionName(std::string intersection_prefix)
{
    std::vector<IntersectionIdx> result;
    // Avoid crashing with empty input.
    if (intersection_prefix.empty())
    {
        return result;
    }

    // Manipulate intersection_prefix to lowercase, ignore space
    std::string prefix = lower_no_space(intersection_prefix);

    // Find intersection by intersection prefix
    std::multimap<std::string, IntersectionIdx>::iterator node = IntersectionName_lower_IntersectionIdx.lower_bound(prefix); // Iterator to first candidate

    // Increase iterator through ordered map will always reach larger strings by string comparision
    while (node->first.compare(0, prefix.size(), prefix) == 0)
    {
        result.push_back(node->second);
        node++;
    }
    return result;
}

// Returns the length of a given street in meters
// Speed Requirement --> high 
double findStreetLength (StreetIdx street_id)
{
    // Get street length from street id
    return Street_StreetInfo.at(street_id).length;
}

// Returns the nearest point of interest of the given type (e.g. "restaurant") 
// to the given position
// Speed Requirement --> none 
POIIdx findClosestPOI (LatLon my_position, std::string POItype)
{
    POIIdx closestPOI = 0; //return value
    double tempDistance; //store temporary distance
    bool initialized = false; //flag for smallest distance initialization
    double smallestDistance; //store the smallest distance
    
    for(POIIdx POI = 0; POI <= getNumPointsOfInterest() - 1; POI++){
        // Check for POI type
        if(POItype == getPOIType(POI)){
            // Initialize smallest distance and the return value
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

// Returns the nearest point of interest of any type, in xy
POIIdx findClosestPOI(ezgl::point2d my_position)
{
    POIIdx closest_POI = 0;
    double smallestDistance = findDistanceBetweenTwoPoints(my_position, POI_AllInfo[0].POIPoint); // Store the smallest distance (Starts at POIidx = 0)
    
    for(POIIdx id = 1; id < POINum; id++)
    {
        double tempDistance = findDistanceBetweenTwoPoints(my_position, POI_AllInfo[id].POIPoint);
        if (tempDistance < smallestDistance)
        {
            smallestDistance = tempDistance;
            closest_POI = id;
        }
    }
    clicked_POI_distance = smallestDistance;
    return closest_POI;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id){
    int numberOfPoints = getNumFeaturePoints(feature_id); // Total number of points
    // Variables used for area calculation
    double lat1, lon1, lat2, lon2, latavg;
    double x1, y1, x2, y2;
    double featureArea = 0; //return value
    // Check for non-closed polygon
    LatLon firstPoint = getFeaturePoint(feature_id, 0);
    LatLon secondPoint = getFeaturePoint(feature_id, numberOfPoints - 1);
    // If polygon is closed
    if ((firstPoint.latitude() == secondPoint.latitude()) && (firstPoint.longitude() == secondPoint.longitude())){
        // Calculate the area of the polygon
        for (int index = 0; index < numberOfPoints - 1; index++){
            firstPoint = getFeaturePoint(feature_id, index);
            secondPoint = getFeaturePoint(feature_id, index + 1);
            lat1 = firstPoint.latitude() * kDegreeToRadian;
            lon1 = firstPoint.longitude() * kDegreeToRadian;
            lat2 = secondPoint.latitude() * kDegreeToRadian;
            lon2 = secondPoint.longitude() * kDegreeToRadian;
            latavg = (lat1 + lat2) / 2;

            x1 = kEarthRadiusInMeters * lon1 * cos(latavg);
            y1 = kEarthRadiusInMeters * lat1;
            x2 = kEarthRadiusInMeters * lon2 * cos(latavg);
            y2 = kEarthRadiusInMeters * lat2;
            
            featureArea = featureArea + (y2 - y1) * std::abs((x1 + x2) / 2);
        }
        return std::abs(featureArea);
    } 
    else {
        return 0.0;     // If polygon is non-closed
    }
}

// Return the value associated with this key on the specified OSMNode.
// If this OSMNode does not exist in the current map, or the specified key is 
// not set on the specified OSMNode, return an empty string.
// Speed Requirement --> high
std::string getOSMNodeTagValue (OSMID OSMid, std::string key){
    //use try and catch block to check for out of range OSMid
    try{
        //use the OSMID_Nodes_AllTagPairs container to find given OSMid
        auto tempVector = OSMID_Nodes_AllTagPairs.at(OSMid);
        for (auto tempPair = tempVector.begin(); tempPair != tempVector.end(); ++tempPair){
            //return value corresponding to a key
            if ((*tempPair).first == key){
                return (*tempPair).second;
            }
        }
        return "";  // No value for given key
    }
    catch (const std::out_of_range& oor){
        return "";  // No OSMNode exist in current map
    }
}

void closeMap() {
    //Clean-up your map related data structures here
    Segment_SegmentDetailedInfo.clear();
    Intersection_AllStreetSegments.clear();
    Intersection_IntersectionInfo.clear();
    IntersectionName_IntersectionIdx_no_repeat.clear();
    IntersectionName_IntersectionIdx.clear();
    IntersectionName_lower_IntersectionIdx.clear();
    Street_StreetInfo.clear();
    StreetName_lower_StreetIdx.clear();
    Features_AllInfo.clear();
    POI_AllInfo.clear();
    POI_AllFood.clear();
    OSMID_Nodes_AllTagPairs.clear();
    OSMID_Highway_Type.clear();
    AllSubwayRoutes.clear();
    OSMID_NodeIndex.clear();
    OSMID_WayIndex.clear();

    closeStreetDatabase();
    closeOSMDatabase();
}

/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/
void m1_init(){
    // Retrive total numbers from API
    segmentNum = getNumStreetSegments();
    streetNum = getNumStreets();
    intersectionNum = getNumIntersections();
    featureNum = getNumFeatures();
    POINum = getNumPointsOfInterest();
    // Init database
    init_features();
    init_POI();
    init_intersections();
    init_segments();
    init_streets();    
    init_osm_nodes();
    init_osm_ways();
    init_osm_relations_subways();
}

// *******************************************************************
// Street Segments
// *******************************************************************
void init_segments()
{
    // Vector of StreetSegmentDetailedInfo (StreetSegmentIdx - StreetSegmentDetailedInfo)
    for (int segment = 0; segment < segmentNum; segment++)                  // Corresponds to id of all street segments
    {                 
        StreetSegmentInfo rawInfo = getStreetSegmentInfo(segment);          // Raw info object   
        StreetSegmentDetailedInfo processedInfo;                            // Processed info object
        
        processedInfo.wayOSMID = rawInfo.wayOSMID;
        processedInfo.from = rawInfo.from;
        processedInfo.to = rawInfo.to;
        processedInfo.oneWay = rawInfo.oneWay;
        processedInfo.streetID = rawInfo.streetID;
        processedInfo.numCurvePoints = rawInfo.numCurvePoints;
        processedInfo.streetName = getStreetName(processedInfo.streetID);       // (get the name of the street that each segment belongs to - for m2)
        
        // Find max and min x, y for defining rectangles of each segment
        ezgl::point2d point_xy = xy_from_latlon(getIntersectionPosition(rawInfo.from));
        double max_x = point_xy.x;
        double max_y = point_xy.y;
        double min_x = max_lat;
        double min_y = max_lon;

        // Pre-calculate length of each street segments (including curve points)
        // Determine rectangle bounds of each segment
        if (rawInfo.numCurvePoints == 0){
            LatLon point_1 = getIntersectionPosition(rawInfo.from);
            LatLon point_2 = getIntersectionPosition(rawInfo.to);
            processedInfo.length = findDistanceBetweenTwoPoints(point_1, point_2);

            ezgl::point2d point_1_xy = xy_from_latlon(point_1);
            ezgl::point2d point_2_xy = xy_from_latlon(point_2);
            processedInfo.segmentRectangle = ezgl::rectangle(point_1_xy, point_2_xy);
        } else{
            LatLon point_1 = getIntersectionPosition(rawInfo.from);
            processedInfo.length = 0.0; // Starting length
            // Iterate through all curve points
            for (int i = 0; i < rawInfo.numCurvePoints; i++){
                LatLon point_2 = getStreetSegmentCurvePoint(segment, i);
                double templength = findDistanceBetweenTwoPoints(point_1, point_2);
                processedInfo.length += templength;
                ezgl::point2d point_2_xy = xy_from_latlon(point_2);
                processedInfo.curvePoints_xy.push_back(point_2_xy);    // Save the xy of curve points (for m2)
                point_1 = point_2;
                // Compare to get max min xy of each segment
                max_x = std::max(point_2_xy.x, max_x);
                max_y = std::max(point_2_xy.y, max_y);
                min_x = std::min(point_2_xy.x, min_x);
                min_y = std::min(point_2_xy.y, min_y);
            }
            LatLon point_2 = getIntersectionPosition(rawInfo.to);                   // Destination (to) point
            processedInfo.length += findDistanceBetweenTwoPoints(point_1, point_2);
            ezgl::point2d point_2_xy = xy_from_latlon(point_2);
            max_x = std::max(point_2_xy.x, max_x);
            max_y = std::max(point_2_xy.y, max_y);
            min_x = std::min(point_2_xy.x, min_x);
            min_y = std::min(point_2_xy.y, min_y);
            
            // Record the rectangle that bounds segment
            processedInfo.segmentRectangle = ezgl::rectangle(ezgl::point2d(min_x, min_y),
                                                             ezgl::point2d(max_x, max_y));
        }

        // Pre-calculate travel time of each street segments
        processedInfo.travel_time = processedInfo.length/rawInfo.speedLimit;

        // Push processed info into vector
        Segment_SegmentDetailedInfo.push_back(processedInfo);
    }
}

// *******************************************************************
// Intersections and LatLon
// *******************************************************************
void init_intersections(){
    Intersection_IntersectionInfo.resize(intersectionNum);

    for (IntersectionIdx id = 0; id < intersectionNum; id++)
    {        
        // Initialize vector for Intersection (IntersectionIdx - Vector of Segments)
        std::vector<StreetSegmentIdx> allSegments;
        for(int segment = 0; segment < getNumIntersectionStreetSegment(id); segment++) {
            StreetSegmentIdx ss_id = getIntersectionStreetSegment(id, segment);
            allSegments.push_back(ss_id);
        }
        Intersection_AllStreetSegments.push_back(allSegments);

        // Pre-process xy position for all intersections
        std::string name = getIntersectionName(id);
        Intersection_IntersectionInfo[id].name = name;
        Intersection_IntersectionInfo[id].position_xy = xy_from_latlon(getIntersectionPosition(id));

        // Populate data structures to allow searching for intersection by name
        IntersectionName_IntersectionIdx_no_repeat.insert(std::make_pair(name, id));
        IntersectionName_IntersectionIdx.insert(std::make_pair(name, id));
        IntersectionName_lower_IntersectionIdx.insert(std::make_pair(lower_no_space(name), id));
    }
}

// *******************************************************************
// Streets
// *******************************************************************
void init_streets()
{
    for(int seg_id = 0; seg_id < segmentNum; seg_id++){
        // Info of current segment
        StreetSegmentDetailedInfo segmentInfo = Segment_SegmentDetailedInfo[seg_id];
        StreetIdx street_id = segmentInfo.streetID;
        // Populate Street_StreetInfo based on streetID
        if (Street_StreetInfo.find(street_id) == Street_StreetInfo.end())
        {
            StreetInfo street_info;
            street_info.id = street_id;
            street_info.name = getStreetName(street_id);
            street_info.length = segmentInfo.length;

            street_info.all_segments.push_back(seg_id);
            street_info.all_intersections.push_back(Segment_SegmentDetailedInfo[seg_id].from);
            street_info.all_intersections.push_back(Segment_SegmentDetailedInfo[seg_id].to);
            Street_StreetInfo.insert(std::make_pair(street_id, street_info));
        } else 
        {
            // Push segment into street info
            Street_StreetInfo.at(street_id).all_segments.push_back(seg_id);
            // Push intersections into street info
            // Intersections will appear duplicates here. Intersections will be sorted and duplicates will be removed in next for loop
            Street_StreetInfo.at(street_id).all_intersections.push_back(Segment_SegmentDetailedInfo[seg_id].from);
            Street_StreetInfo.at(street_id).all_intersections.push_back(Segment_SegmentDetailedInfo[seg_id].to);

            // Add segment length to street
            Street_StreetInfo.at(street_id).length += Segment_SegmentDetailedInfo[seg_id].length;
        }
    }

    for (auto& pair : Street_StreetInfo)
    {
        // Sort + unique to remove all the duplicating intersections for each streets
        // Sorting is required for finding union between 2 vectors (to find intersections between 2 streets) later
        sort(pair.second.all_intersections.begin(), pair.second.all_intersections.end());
        auto remove_duplicates = unique(pair.second.all_intersections.begin(), pair.second.all_intersections.end());
        pair.second.all_intersections.erase(remove_duplicates, pair.second.all_intersections.end());
        
        // Populate ordered multimap for Streets (StreetName - Street index)
        // Names as lowercase, no space
        StreetName_lower_StreetIdx.insert(std::make_pair(lower_no_space(pair.second.name), pair.first));
    }
}

// *******************************************************************
// Features
// *******************************************************************
void init_features()
{
    // Find max and min lat, lon of feature points in city
    // Initialize for comparision
    max_lat = getFeaturePoint(0, 0).latitude();
    max_lon = getFeaturePoint(0, 0).longitude();
    min_lat = max_lat;
    min_lon = max_lon;
    
    for (int featureIdx = 0; featureIdx < featureNum; featureIdx++)
    {
        FeatureDetailedInfo tempFeatureInfo;
        tempFeatureInfo.featureType = getFeatureType(featureIdx);
        tempFeatureInfo.featureOSMID = getFeatureOSMID(featureIdx);

        // To get max min lat lon of current featureIdx
        double temp_min_lat, temp_min_lon, temp_max_lat, temp_max_lon;
        temp_max_lat = getFeaturePoint(featureIdx, 0).latitude();
        temp_max_lon = getFeaturePoint(featureIdx, 0).longitude();
        temp_min_lat = temp_max_lat;
        temp_min_lon = temp_max_lon;
        for (int pointIdx = 0; pointIdx < getNumFeaturePoints(featureIdx); pointIdx++)
        {
            LatLon temp_latlon = getFeaturePoint(featureIdx, pointIdx);
            temp_max_lat = std::max(temp_max_lat, temp_latlon.latitude());
            temp_max_lon = std::max(temp_max_lon, temp_latlon.longitude());
            temp_min_lat = std::min(temp_min_lat, temp_latlon.latitude());
            temp_min_lon = std::min(temp_min_lon, temp_latlon.longitude());
        }
        tempFeatureInfo.temp_max_lat = temp_max_lat;
        tempFeatureInfo.temp_max_lon = temp_max_lon;
        tempFeatureInfo.temp_min_lat = temp_min_lat;
        tempFeatureInfo.temp_min_lon = temp_min_lon;

        // Add Feature Info to Features_AllInfo
        Features_AllInfo.push_back(tempFeatureInfo);

        // Check if feature has max min lat lon of current world
        max_lat = std::max(temp_max_lat, max_lat);
        max_lon = std::max(temp_max_lon, max_lon);
        min_lat = std::min(temp_min_lat, min_lat);
        min_lon = std::min(temp_min_lon, min_lon);
    }

    // Already have max min lat lon of the whole world
    // xy_from_latlon conversion function needs max_lat and min_lat
    lat_avg = (max_lat + min_lat) / 2;
    latlon_bound.max = LatLon(max_lat, max_lon);
    latlon_bound.min = LatLon(min_lat, min_lon);

    //Load pre-processed data into Features_AllPoints
    for (int featureIdx = 0; featureIdx < featureNum; featureIdx++)
    {
        Features_AllInfo[featureIdx].featureRectangle = 
                    ezgl::rectangle(xy_from_latlon(LatLon(Features_AllInfo[featureIdx].temp_min_lat,
                                                        Features_AllInfo[featureIdx].temp_min_lon)), 
                                    xy_from_latlon(LatLon(Features_AllInfo[featureIdx].temp_max_lat,
                                                        Features_AllInfo[featureIdx].temp_max_lon)));
        for (int pointIdx = 0; pointIdx < getNumFeaturePoints(featureIdx); pointIdx++)
        {
            ezgl::point2d tempPoint = xy_from_latlon(getFeaturePoint(featureIdx, pointIdx));
            Features_AllInfo[featureIdx].featurePoints.push_back(tempPoint);
        }
        Features_AllInfo[featureIdx].featureArea = findFeatureArea(featureIdx);
    }
    std::sort(Features_AllInfo.begin(), Features_AllInfo.end(), compareFeatureArea);
}

//Helper function for sorting feature areas
bool compareFeatureArea (FeatureDetailedInfo F1, FeatureDetailedInfo F2)
{
    return (F1.featureArea > F2.featureArea);
}

// Converts LatLon to xy
ezgl::point2d xy_from_latlon(LatLon latlon)
{
    double x = kEarthRadiusInMeters * latlon.longitude() * kDegreeToRadian * cos(lat_avg * kDegreeToRadian);
    double y = kEarthRadiusInMeters * latlon.latitude() * kDegreeToRadian;
    return ezgl::point2d(x, y);
}

// Converts xy to LatLon(
LatLon latlon_from_xy(double x, double y)
{
    double lon = x / (kEarthRadiusInMeters * kDegreeToRadian* cos(lat_avg * kDegreeToRadian));
    double lat = y / (kEarthRadiusInMeters * kDegreeToRadian);
    return LatLon(lat, lon);
}

// *******************************************************************
// POI
// *******************************************************************
void init_POI(){
    for (int tempIdx = 0; tempIdx < POINum; tempIdx++){
        POIDetailedInfo tempPOIInfo;
        tempPOIInfo.POIPoint = xy_from_latlon(getPOIPosition(tempIdx));
        tempPOIInfo.POIType = getPOIType(tempIdx);
        tempPOIInfo.POIName = getPOIName(tempIdx);
        tempPOIInfo.id = tempIdx;
        POI_AllInfo.push_back(tempPOIInfo);
        
        // If POI is a food place, add to POI_AllFood
        if (tempPOIInfo.POIType == "bar" || tempPOIInfo.POIType == "beer" || tempPOIInfo.POIType == "cafe" || tempPOIInfo.POIType == "cafe;fast_food" 
            || tempPOIInfo.POIType == "cater" || tempPOIInfo.POIType == "fast_food" || tempPOIInfo.POIType == "food_court" || tempPOIInfo.POIType == "ice_cream"
            || tempPOIInfo.POIType == "old_restaurant" || tempPOIInfo.POIType == "pub" || tempPOIInfo.POIType == "restaurant" || tempPOIInfo.POIType == "veterinary")
        {
            POI_AllFood.insert(std::make_pair(tempPOIInfo.POIName + " - " + std::to_string(tempIdx), tempPOIInfo));
        }
    }
}


// *******************************************************************
// OSMNode
// *******************************************************************
void init_osm_nodes()
{
    // Unordered Map for OSMDatabase (OSMID - vector of pair(tag, value)
    // Pre-load data for OSMNodes
    for (int index = 0; index < getNumberOfNodes(); ++index){
        const OSMNode* tempOSMNode = getNodeByIndex(index);
        OSMID tempOSMID = tempOSMNode->id();
        // For getting OSMNode given OSMID
        OSMID_NodeIndex.insert(std::make_pair(tempOSMID, index));
        for (int tagIdx = 0; tagIdx < getTagCount(tempOSMNode); ++tagIdx){
            if (OSMID_Nodes_AllTagPairs.find(tempOSMID) == OSMID_Nodes_AllTagPairs.end()){
                std::vector<std::pair<std::string, std::string>> tempVector;
                tempVector.push_back(getTagPair(tempOSMNode, tagIdx));
                OSMID_Nodes_AllTagPairs.insert(std::make_pair(tempOSMID, tempVector));
            } else {
                OSMID_Nodes_AllTagPairs.at(tempOSMID).push_back(getTagPair(tempOSMNode, tagIdx));
            }
        }
    }
}

// Pre-load data for OSMWays - Only consider OSMIDs having a tag of highway, record highway type
void init_osm_ways()
{
    for (int way = 0; way < getNumberOfWays(); ++way)
    {
        const OSMWay* tempOSMWay = getWayByIndex(way);
        OSMID tempOSMID = tempOSMWay->id();
        // For getting OSMWay given OSMID
        OSMID_WayIndex.insert(std::make_pair(tempOSMID, way));
        // Check highway tag for street type
        for (int tagIdx = 0; tagIdx < getTagCount(tempOSMWay); ++tagIdx)
        {
            auto tag_pair = getTagPair(tempOSMWay, tagIdx);
            if (tag_pair.first == "highway")
            {
                OSMID_Highway_Type.insert(std::make_pair(tempOSMID, tag_pair.second));
            }
        }
    }
}

// Initialize necessary data for subway lines and stations
void init_osm_relations_subways()
{
    // Loop thourgh all relations
    for (int relation = 0; relation < getNumberOfRelations(); ++relation)
    {
        bool is_route = false;
        bool is_subway = false;
        const OSMRelation* tempOSMRelation = getRelationByIndex(relation);
        // Check all tags to see if current relation is subway
        for (int tagIdx = 0; tagIdx < getTagCount(tempOSMRelation); ++tagIdx)
        {
            auto tag_pair = getTagPair(tempOSMRelation, tagIdx);
            if (tag_pair.first == "type" && tag_pair.second == "route")
            {
                is_route = true;
            }
            if (tag_pair.first == "route" && tag_pair.second == "subway")
            {
                is_subway = true;
            }
        }
        // If the current relation is a subway relation - start extracting data
        if (is_route && is_subway)
        {   
            SubwayRoutes subway;
            subway.route_id = tempOSMRelation->id();
            subway.roles = getRelationMemberRoles(tempOSMRelation);     // A vector of roles
            subway.members = getRelationMembers(tempOSMRelation);       // A vector of members
            subway.colour = ezgl::RED;                                  // Default color to red
            // Get colour of current subway relation
            for (int tagIdx = 0; tagIdx < getTagCount(tempOSMRelation); ++tagIdx)
            {
                auto tag_pair = getTagPair(tempOSMRelation, tagIdx);
                if (tag_pair.first == "colour")
                {
                    subway.colour = get_rgb_color(tag_pair.second);
                }
            }
            // Populate track points
            for (int i = 0; i < subway.members.size(); i++)     // Assuming that subway.members.size() == subway.roles.size()
            {
                // Tracks railway=subway
                if ((subway.roles[i] == "") && (subway.members[i].type() == TypedOSMID::Way))
                {
                    // Get the actual Way based on TypedOSMID (Way)
                    const OSMWay *currWay = getWayByIndex(OSMID_WayIndex.at(subway.members[i]));
                    // Get xy of all nodes forming ways of a subway
                    std::vector<OSMID> way_nodes = getWayMembers(currWay);

                    std::vector<ezgl::point2d> curr_way_track_points;
                    for (auto id : way_nodes)
                    {
                        const OSMNode* tempOSMNode = getNodeByIndex(OSMID_NodeIndex.at(id));
                        curr_way_track_points.push_back(xy_from_latlon(getNodeCoords(tempOSMNode)));
                    }
                    subway.track_points.push_back(curr_way_track_points);
                } else if ((subway.roles[i] == "stop") && (subway.members[i].type() == TypedOSMID::Node))
                {   // Subway stations
                    const OSMNode* tempOSMNode = getNodeByIndex(OSMID_NodeIndex.at(subway.members[i]));
                    subway.station_points.push_back(xy_from_latlon(getNodeCoords(tempOSMNode)));
                }
            }
            AllSubwayRoutes.push_back(subway);
        }
    }
}

// Get ezgl::color from OSM color (string)
ezgl::color get_rgb_color(std::string osm_color)
{
    if (osm_color[0] != '#')
    {
        if (osm_color == "black") return ezgl::BLACK;
        else if (osm_color == "white") return ezgl::WHITE;
        else if (osm_color == "gray" || osm_color == "grey" || osm_color == "silver") return ezgl::GREY_55;
        else if (osm_color == "maroon" || osm_color == "red") return ezgl::RED;
        else if (osm_color == "olive" || osm_color == "yellow") return ezgl::ORANGE;
        else if (osm_color == "green" || osm_color == "lime") return ezgl::DARK_GREEN;
        else if (osm_color == "teal" || osm_color == "aqua" || osm_color == "cyan") return ezgl::LIGHT_SKY_BLUE;
        else if (osm_color == "navy" || osm_color == "blue") return ezgl::LIGHT_MEDIUM_BLUE;
        else if (osm_color == "purple" || osm_color == "fuchsia" || osm_color == "magenta") return ezgl::MEDIUM_PURPLE;
    } else 
    {
        // remove the '#'
        osm_color.erase(0, 1);
        if (osm_color.length() == 3)
        {
            osm_color = osm_color.substr(0, 1) + osm_color.substr(0, 1) +
                        osm_color.substr(1, 1) + osm_color.substr(1, 1) +
                        osm_color.substr(2, 1) + osm_color.substr(2, 1);
        }
        // Convert HEX string to RGB vector of integers
        int red, green, blue;
        red = std::stoi(osm_color.substr(0, 2), nullptr, 16);
        green = std::stoi(osm_color.substr(2, 2), nullptr, 16);
        blue = std::stoi(osm_color.substr(4, 2), nullptr, 16);
        return ezgl::color(red, green, blue);
    }
    return ezgl::DARK_KHAKI;
}
