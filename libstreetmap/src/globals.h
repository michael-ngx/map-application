/* 
 *
 * HEADER FILE FOR DECLARING SHARED GLOBAL VARIABLES.
 * Global variables are initialized in m1.cpp.
 * 
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "ezgl/application.hpp"
#include "m1.h"
#include <unordered_map>

// *********************************************************************************************************
// Global GTK pointers - M2
// *********************************************************************************************************
extern GObject *SubwayButton;
extern GObject *SubwayOffButton;
extern GObject *TutorialButton;
extern GObject *NavigationButton;
extern GObject *EndNavigationButton;
extern GObject *NightModeButton;
extern GObject *DayModeButton;
extern GObject *DirectionButton;
extern GObject *DirectionDisplay;
extern GObject *DirectionWindow;
extern GtkTextBuffer *DirectionTextBuffer;

extern GObject *FilterComboBox;
extern GObject *CityChangeComboBox;

extern GObject *SearchBar;
extern GObject *SearchBarDestination;
extern GtkListStore *list_store;
extern GtkTreeIter iter;
extern GtkEntryCompletion *completion;
extern GtkEntryCompletion *completion_destination;

// *********************************************************************************************************
// Gtk Feature States
// *********************************************************************************************************
// Check current map path for city switching - Main
extern std::string CURRENT_MAP_PATH;
// Check current filter for applying filters - M2
extern std::string CURRENT_FILTER;
// Checks if night mode is on - M2
extern bool night_mode;
// Checks if filter is on - M2
extern bool filtered;
// Checks if the subway mode if turned on - M2
extern bool subway_mode;
// Checks if the navigation mode if turned on (to allow navigation) - M2
extern bool navigation_mode;
// Checks if the direction display is on
extern bool direction_display_on;

// Distance of closest intersection/POI, calculated in M1 - used in M2
// Distances are used to determine whether user selected an intersection or a POI
extern double clicked_intersection_distance;
extern double clicked_POI_distance;

// *********************************************************************************************************
// Drawing & zooming variables
// *********************************************************************************************************
// Rectangle for visible world - Updated every frame in M2
extern ezgl::rectangle visible_world;
extern double curr_world_width;
extern double curr_world_height;

// Zoom limits for curr_world_width, in meters
const float ZOOM_LIMIT_0 = 50000;
const float ZOOM_LIMIT_1 = 15000;
const float ZOOM_LIMIT_2 = 5000;
const float ZOOM_LIMIT_3 = 2000;
const float ZOOM_LIMIT_4 = 1500;

// Minimum area a feature must have to be displayed by different zoom levels
const double FEATURE_AREA_LIMIT_0 = 500000;
const double FEATURE_AREA_LIMIT_1 = 200000;
const double FEATURE_AREA_LIMIT_2 = 30000;
const double FEATURE_AREA_LIMIT_3 = 7000;
const double FEATURE_AREA_LIMIT_4 = 1000;

// Camera zoom levels
const double CAMERALVL_SMALL = 2.5;
const double CAMERALVL_LARGE = 2;

// Width of new world to be zoomed to after searching
const double FIND_ZOOM_WIDTH = 1000.0;

// Total number of map grids to initialize data to
const int NUM_GRIDS = 20;

// Maximum number of POIs that can be drawn in 1 grid
const int MAX_GRID_POI = 10;
// Default step for skipping POIIdx to avoid collision
const int POI_STEP = 7;

// *********************************************************************************************************
// Overload functions from M1
// *********************************************************************************************************
// Returns all intersection ids corresponding to intersection names that start with the given prefix
std::vector<IntersectionIdx> findIntersectionIdsFromPartialIntersectionName(std::string intersection_prefix);

// *********************************************************************************************************
// Bounds of the city & conversions
// *********************************************************************************************************
extern ezgl::point2d world_top_right, world_bottom_left;
extern double lat_avg;
extern double world_height, world_width;
extern double grid_height, grid_width;

ezgl::point2d xy_from_latlon(LatLon latlon);
LatLon latlon_from_xy(double x, double y);

// *********************************************************************************************************
// Total counts of objects
// *********************************************************************************************************
extern int intersectionNum;
extern int segmentNum;
extern int streetNum;
extern int featureNum;
extern int POINum;

// *********************************************************************************************************
// Street Segments
// ********************************************************************************************************
// Pre-processed information of each street segment
struct StreetSegmentDetailedInfo
{
    StreetSegmentIdx id;        // id of the segment
    OSMID wayOSMID;             // OSM ID of the source way
                                // NOTE: Multiple segments may match a single OSM way ID
    std::string highway_type;   // Street type of street segment
    IntersectionIdx from, to;   // Intersection ID this segment runs from/to
    ezgl::point2d from_xy, to_xy;
    bool oneWay;
    double length;              // Real length (in meters) of segment
    int width;                  // Real half-width (in meters) of segment
    double travel_time;         // Travel time, in seconds
    float speedLimit;           // Speed limit of current segment, in m/s
    StreetIdx streetID;         // Index of street this segment belongs to
    std::string streetName;     // Name of the street this segment belongs to
    std::string streetName_arrow;   // Name of the street this segment belongs to, arrow included
    double angle_degree;         // Angle to be rotated to draw street segment name and arrow, in degrees
    int numCurvePoints;         // number of curve points between the ends
    std::vector<ezgl::point2d> curvePoints_xy; // Vector of xy of all curvepoints (not containing from and to)
    std::vector<std::vector<ezgl::point2d>> poly_points; // Each index is a vector of polygon points needed to draw 
                                                        // small curve segments in world coordinates
    ezgl::rectangle segmentRectangle;       // Rectangle for checking display & navigation zooming
};
// Index: Segment id, Value: Processed information of the segment
extern std::vector<StreetSegmentDetailedInfo> Segment_SegmentDetailedInfo;

// *******************************************************************
// Intersections
// *******************************************************************
// Struct for preprocessed information of Intersections
struct IntersectionInfo
{
    ezgl::point2d position_xy;
    LatLon position_latlon;
    std::string name;
    // Vector of all segments 
    std::vector<StreetSegmentIdx> all_segments;
    // Vector of neighboring IntersectionIdx - SegmentIdx pairs
    // Neighboring intersections are intersections that the current intersection can travel to,
    // taking into consideration one-way street and self-connecting intersection (included)
    // The segment ids are segments that can be taken to travel to the neighboring intersection
    std::vector<std::pair<IntersectionIdx, std::vector<StreetSegmentIdx>>> neighbors_and_segments;
};

// Index: Intersection id, Value: Pre-processed Intersection info
extern std::vector<IntersectionInfo> Intersection_IntersectionInfo;
// Key: Intersection name, Value: IntersectionIdx (no repeating intersection names)
extern std::unordered_map<std::string, IntersectionIdx> IntersectionName_IntersectionIdx_no_repeat;
// Key: Intersection name, Value: IntersectionIdx
extern std::unordered_multimap<std::string, IntersectionIdx> IntersectionName_IntersectionIdx;
// Key: Intersection name (lower case, no space), Value: IntersectionIdx
extern std::multimap<std::string, IntersectionIdx> IntersectionName_lower_IntersectionIdx;

// *********************************************************************************************************
// Streets
// *********************************************************************************************************
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
extern std::multimap<std::string, StreetIdx> StreetName_lower_StreetIdx;

// *********************************************************************************************************
// Features
// *********************************************************************************************************
//Stores pre-processed information of features
struct FeatureDetailedInfo{
    FeatureIdx id;                              // Feature id
    FeatureType featureType;                    // Type of the feature
    TypedOSMID  featureOSMID;                   // OSMID of the feature
    std::vector<ezgl::point2d> featurePoints;   // Coordinates of the feature in point2d
    double featureArea;

    double temp_max_lat, temp_max_lon;          // For temporary storage only
    double temp_min_lat, temp_min_lon;          
};
//Index: FeatureIdx, Value: structure that stores all feature information
extern std::vector<FeatureDetailedInfo> Features_AllInfo;

// *********************************************************************************************************
// POI
// *********************************************************************************************************
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

// *********************************************************************************************************
// OSM
// *********************************************************************************************************
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
};
// Stores subway station information
struct SubwayStation
{
    ezgl::point2d position_xy;
    std::string name;
};
// Keys: index, Value: Subway relations of current world
extern std::vector<SubwayRoutes> AllSubwayRoutes;

extern std::unordered_map<OSMID, int> OSMID_NodeIndex;
extern std::unordered_map<OSMID, int> OSMID_WayIndex;

// *********************************************************************************************************
// A* Path finding
// *********************************************************************************************************
// Turn penalty default for map
const double DEFAULT_TURN_PENALTY = 15;

// Max speed limit of a street in the city
extern double MAX_SPEED_LIMIT;

// A struct to represent a node in the search graph
struct Node
{
    IntersectionIdx id;
    double g;       // g-value (cost of path from start node to this node)
    double h;       // h-value (heuristic estimate of cost from this node to goal node)
                    // NOTE that h-value must never overestimate the cost to reach the goal
                    // Therefore, h = Euclidean distance to goal node / Largest speed limit in the city

    IntersectionIdx parent;     // node that leads to this node on the shortest path found so far
    StreetSegmentIdx parent_segment;      // segment (with least travel time) that leads to this node
    // Compare the f-value (f = g + h) between 2 nodes
    // Used to set up ascending priority queue (pops the smallest value first)
    bool operator< (const Node& other) const
    {
        return (g + h) > (other.g + other.h);
    }
};

// All points where pins will be drawn on - Cleared and Modified based on user input
extern std::vector<ezgl::point2d> pin_display_start;
extern std::vector<ezgl::point2d> pin_display_dest;
// Starting point and destination point
extern ezgl::point2d start_point;
extern ezgl::point2d destination_point;
extern IntersectionIdx start_point_id;
extern IntersectionIdx destination_point_id;
// Bool to check if an intersection in a search bar is "Set"
// "Set" means clicked directly on the map/Pressed Enter to search
// "Unset" is when user modified text in the search bar
// Navigations are executed only when both text fields are set
extern bool start_point_set;
extern bool destination_point_set;
// Bool to check if the content of the search bar is being changed 
// by autocomplete (search_response and navigation_response)
// or by user (adding/deleting characters, etc.) 
// If done by autocomplete, "changed" signal from GtkSearchEntry 
// should not modify the start_point_set or destination_point_set
extern bool search_1_forced_change;
extern bool search_2_forced_change;
// Storing vector for found path for special display
extern std::vector<StreetSegmentIdx> found_path;

#endif /* GLOBALS_H */

