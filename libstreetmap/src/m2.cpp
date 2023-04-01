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
#include "m2.h"
#include "ui_callbacks/widgets.hpp"
#include "ui_callbacks/setup.hpp"
#include "draw/draw.hpp"
#include "draw/utilities.hpp"
#include <cmath>
#include <algorithm>

/*******************************************************************************************************************************
 * GLOBAL VARIABLES (M2)
 ********************************************************************************************************************************/
/**********************************************
 * Gtk pointers
 *********************************************/
GObject *SubwayButton;
GObject *SubwayOffButton;
GObject *TutorialButton;
GObject *NavigationButton;
GObject *EndNavigationButton;
GObject *NightModeButton;
GObject *DayModeButton;
GObject *DirectionButton;


GObject *FilterComboBox;
GObject *CityChangeComboBox;

GObject *SearchBar;
GObject *SearchBarDestination;
GtkListStore *list_store;
GtkTreeIter iter;
GtkEntryCompletion *completion;
GtkEntryCompletion *completion_destination;

// Check current filter for applying filters
std::string CURRENT_FILTER = "Filters";
// Checks if night mode is on
bool night_mode = false;
// Checks if filter is on
bool filtered = false;
// Checks if the subway station mode if turned on (to show subway stations)
bool subway_station_mode = false;
// Checks if the subway line mode if turned on (to show subway lines)
bool subway_line_mode = false;
// Checks if the navigation mode if turned on (to allow navigation)
bool navigation_mode = false;

// Rectangle of current visible world, in meters
ezgl::rectangle visible_world;
// Starting point and destination point (Initialized to 0, 0) and id to -1, -1
ezgl::point2d start_point = ezgl::point2d(0, 0);
ezgl::point2d destination_point = ezgl::point2d(0, 0);
IntersectionIdx start_point_id = -1;
IntersectionIdx destination_point_id = -1;
// Bool to check if an intersection in a search bar is "Set"
// "Set" means clicked directly on the map/Pressed Enter to search
// "Unset" is when user modified text in the search bar
// Navigations are executed only when both text fields are set
bool start_point_set = false;
bool destination_point_set = false;
// Bool to check if the content of the search bar is being changed 
// by autocomplete (search_response and navigation_response)
// or by user (adding/deleting characters, etc.) 
// If done by autocomplete, "changed" signal from GtkSearchEntry 
// should not modify the start_point_set or destination_point_set
bool search_1_forced_change = false;
bool search_2_forced_change = false;

// Short info for segments whose street name will be displayed
struct SegShortInfo
{
    std::string street_name;
    ezgl::point2d from_xy;
    ezgl::point2d to_xy;
    StreetSegmentIdx seg_id;
    bool arrow;
    std::string street_type;
};

// All POI whose name will be displayed
std::vector<std::vector<POIDetailedInfo>> poi_display;
// All points where pin will be drawn on
std::vector<ezgl::point2d> pin_display_start;
std::vector<ezgl::point2d> pin_display_dest;

// All street segments of the path found (to be drawn)
std::vector<StreetSegmentIdx> found_path;

/*******************************************************************************************************************************
 * FUNCTION DECLARATIONS
 ********************************************************************************************************************************/
/*************************************************************
 * Draw to the main canvas using the provided graphics object. 
 * Runs every time graphics are refreshed/image zooms or pans
 * The graphics object expects that x and y values will be in the main canvas' world coordinate system.
 *************************************************************/
void draw_main_canvas (ezgl::renderer *g);


/*******************************************************************************************************************************
 * DRAW MAP
 ********************************************************************************************************************************/
void drawMap ()
{
    // Set up the ezgl graphics window and hand control to it, as shown in the
    // ezgl example program.
    // This function will be called by both the unit tests (ece297exercise)
    // and your main() function in main/src/main.cpp.
    // The unit tests always call loadMap() before calling this function
    // and call closeMap() after this function returns.
    
    // Create our EZGL application.
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    ezgl::application application(settings);

    // Set initial world rectangle
    ezgl::rectangle initial_world(xy_from_latlon(latlon_bound.min),
                                  xy_from_latlon(latlon_bound.max));
    
    // Set some parameters for the main sub-window (MainCanvas), where 
    // visualization graphics are draw. Set the callback function that will be 
    // called when the main window needs redrawing, and define the (world) 
    // coordinate system we want to draw in.
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world, ezgl::color(240, 240, 240)); 

    // Run the application until the user quits.
    // This hands over all control to the GTK runtime---after this point
    // you will only regain control based on callbacks you have setup.
    // Three callbacks can be provided to handle mouse button presses,
    // mouse movement and keyboard button presses in the graphics area,
    // respectively. Also, an initial_setup function can be passed that will
    // be called before the activation of the application and can be used
    // to create additional buttons, initialize the status message, or
    // connect added widgets to their callback functions.
    // Those callbacks are optional, so we can pass nullptr if
    // we don't need to take any action on those events
    application.run(initial_setup,
                    act_on_mouse_click,
                    nullptr,
                    nullptr);
}

/*******************************************************************************************************************************
 * DRAW MAIN CANVAS
 ********************************************************************************************************************************/
void draw_main_canvas (ezgl::renderer *g)
{
    /********************************************************************************
    * Local variables of current canvas
    ********************************************************************************/
    // Check for current zoom level through visible width (in meters) of world
    visible_world = g->get_visible_world();
    double curr_world_width = visible_world.width();
    // highways to be displayed after all other segments
    std::vector<SegShortInfo> highway_segments; 
    // All segments whose street name or arrows will be displayed
    std::vector<SegShortInfo> seg_names_and_arrows; 
    // Defining 3x4 or 2x3 regions on the screen based on visible world
    std::vector<ezgl::rectangle> visible_regions;
    // For each region, allow showing 1 name, 1 arrow, and 1 subway station
    std::vector<std::vector<int>> available_region = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, 
                                                      {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1},
                                                      {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    
    // Populating the visible_region vector to divide the window into:
    // 6 segments (2x3) at far zoom level, or 12 segments (3x4) at low zoom level
    if (curr_world_width < ZOOM_LIMIT_2 || subway_station_mode)
    {
        // 1/4 world width
        double fourth_curr_world_width = curr_world_width * 0.25;
        // 1/3 world width
        double third_curr_world_height = visible_world.height() * 0.333333;
        ezgl::rectangle RECT_0_0({visible_world.left(), 
                                visible_world.top() - third_curr_world_height}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_0_1({visible_world.left() + fourth_curr_world_width, 
                                visible_world.top() - third_curr_world_height}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_0_2({visible_world.left() + fourth_curr_world_width * 2, 
                                visible_world.top() - third_curr_world_height}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_0_3({visible_world.right() - fourth_curr_world_width, 
                                visible_world.top() - third_curr_world_height}, 
                                fourth_curr_world_width, third_curr_world_height);

        ezgl::rectangle RECT_1_0({visible_world.left(), 
                                visible_world.bottom() + third_curr_world_height}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_1_1({visible_world.left() + fourth_curr_world_width, 
                                visible_world.bottom() + third_curr_world_height}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_1_2({visible_world.left() + fourth_curr_world_width * 2, 
                                visible_world.bottom() + third_curr_world_height}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_1_3({visible_world.right() - fourth_curr_world_width, 
                                visible_world.bottom() + third_curr_world_height}, 
                                fourth_curr_world_width, third_curr_world_height);

        ezgl::rectangle RECT_2_0({visible_world.left(), 
                                visible_world.bottom()}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_2_1({visible_world.left() + fourth_curr_world_width, 
                                visible_world.bottom()}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_2_2({visible_world.left() + fourth_curr_world_width * 2, 
                                visible_world.bottom()}, 
                                fourth_curr_world_width, third_curr_world_height);
        ezgl::rectangle RECT_2_3({visible_world.right() - fourth_curr_world_width, 
                                visible_world.bottom()}, 
                                fourth_curr_world_width, third_curr_world_height);
        visible_regions.push_back(RECT_0_0);
        visible_regions.push_back(RECT_0_1);
        visible_regions.push_back(RECT_0_2);
        visible_regions.push_back(RECT_0_3);
        visible_regions.push_back(RECT_1_0);
        visible_regions.push_back(RECT_1_1);
        visible_regions.push_back(RECT_1_2);
        visible_regions.push_back(RECT_1_3);
        visible_regions.push_back(RECT_2_0);
        visible_regions.push_back(RECT_2_1);
        visible_regions.push_back(RECT_2_2);
        visible_regions.push_back(RECT_2_3);
    }

    //Draw the canvas for Night Mode
    if (night_mode)
    {
        ezgl::rectangle visible_world_new = g->get_visible_world();
        g->set_color(43, 56, 70);
        g->fill_rectangle(visible_world_new);
    }

    /********************************************************************************
    * Draw features. Features are sorted by descending areas in Features_AllInfo
    ********************************************************************************/
    // Determine number of features to be drawn to screen based on zoom levels
    int numOfFeatureDisplay = featureNum;
    if (curr_world_width >= ZOOM_LIMIT_0)
    {
        numOfFeatureDisplay = featureNum * FEATURE_ZOOM_0;
    } else if (ZOOM_LIMIT_1 <= curr_world_width && curr_world_width < ZOOM_LIMIT_0)
    {
        numOfFeatureDisplay = featureNum * FEATURE_ZOOM_1;
    } else if (ZOOM_LIMIT_2 <= curr_world_width && curr_world_width < ZOOM_LIMIT_1)
    {
        numOfFeatureDisplay = featureNum * FEATURE_ZOOM_2;
    } else if (ZOOM_LIMIT_3 <= curr_world_width && curr_world_width < ZOOM_LIMIT_2)
    {
        numOfFeatureDisplay = featureNum * FEATURE_ZOOM_3;
    }
    // Displaying features
    for (int j = 0; j < numOfFeatureDisplay; j++)
    {
        FeatureDetailedInfo tempFeatureInfo = Features_AllInfo[j];
        // Skip if feature is outside of current visible world (not colliding or containing)
        if (!(check_collides(tempFeatureInfo.featureRectangle, visible_world)
              || check_contains(tempFeatureInfo.featureRectangle, visible_world)
              || check_contains(visible_world,tempFeatureInfo.featureRectangle))) 
        {
            continue;
        }
        draw_feature_area(g, tempFeatureInfo);
    }
    
    /********************************************************************************
    * Draw street segments
    ********************************************************************************/
    for (StreetIdx street_id = 0; street_id < streetNum; street_id++)
    {
        // Get vector of all segments
        std::vector<StreetSegmentIdx> all_segments = Street_StreetInfo.find(street_id)->second.all_segments;
        int total_segment_amount = all_segments.size();
        
        // Street name of current street
        std::string street_name = Segment_SegmentDetailedInfo[all_segments[0]].streetName;

        for (int i = 0; i < total_segment_amount; i++)
        {
            StreetSegmentIdx seg_id = all_segments[i];

            // Skip segment if segment rectangle is outside of current visible window
            ezgl::rectangle segment_rect = Segment_SegmentDetailedInfo[seg_id].segmentRectangle;
            if (!(check_collides(segment_rect, visible_world)
                || check_contains(segment_rect, visible_world)
                || check_contains(visible_world, segment_rect))) 
            {
                continue;
            }
            // Skip segment if segment is part of found_path (will be drawn later)
            if (std::find(found_path.begin(), found_path.end(), seg_id) != found_path.end())
            {
                continue;
            }

            // Get LatLon information of from and to intersections from each segments
            IntersectionIdx from_id = Segment_SegmentDetailedInfo[seg_id].from;
            IntersectionIdx to_id = Segment_SegmentDetailedInfo[seg_id].to;
            ezgl::point2d from_xy = Intersection_IntersectionInfo[from_id].position_xy;
            ezgl::point2d to_xy = Intersection_IntersectionInfo[to_id].position_xy;

            // Check the type of street this segment belongs to through wayOSMID
            OSMID wayOSMID = Segment_SegmentDetailedInfo[seg_id].wayOSMID;
            std::string highway_type = OSMID_Highway_Type.at(wayOSMID);
            
            // To store information of current street segment (that will be sent to either highway_segments or seg_names_and_arrows)
            SegShortInfo segment_short_info;

            // Stores highways into vector to be drawn later
            if ((highway_type == "motorway" || highway_type == "motorway_link"))
            {   
                segment_short_info.from_xy = from_xy;
                segment_short_info.to_xy = to_xy;
                segment_short_info.seg_id = seg_id;
                segment_short_info.street_type = highway_type;
                highway_segments.push_back(segment_short_info);
                
                // Highway contributes to name display if appropriate
                if (curr_world_width < ZOOM_LIMIT_2 && street_name != "<unknown>" 
                    && highway_type == "motorway" && (i != 0) && (i % 2 == 0))
                {
                    segment_short_info.street_name = street_name;
                    segment_short_info.arrow = Segment_SegmentDetailedInfo[seg_id].oneWay;
                    seg_names_and_arrows.push_back(segment_short_info);
                }
                continue;
            }

            // Draws different amount of data based on different zoom levels
            if (curr_world_width >= ZOOM_LIMIT_0)
            {
                if (highway_type == "primary")
                {
                    draw_street_segment_pixel(g, seg_id, from_xy, to_xy, highway_type);
                }
            } else if (ZOOM_LIMIT_1 <= curr_world_width && curr_world_width < ZOOM_LIMIT_0)
            {
                if (highway_type == "trunk" || highway_type == "primary" || highway_type == "secondary")
                {   
                    draw_street_segment_pixel(g, seg_id, from_xy, to_xy, highway_type);
                }
            } else if (ZOOM_LIMIT_2 <= curr_world_width && curr_world_width < ZOOM_LIMIT_1)
            {
                if (highway_type == "trunk" || highway_type == "primary" || highway_type == "secondary" || highway_type == "tertiary")
                {
                    draw_street_segment_pixel(g, seg_id, from_xy, to_xy, highway_type);
                }
            } else // Displaying street names and arrows start here
            {                
                // Display immediately based on street type and zoom levels
                // Only display all types of street (except for highway for later) when < ZOOM_LIMIT_3
                if (ZOOM_LIMIT_3 <= curr_world_width && curr_world_width < ZOOM_LIMIT_2)
                {
                    if (highway_type == "trunk" || highway_type == "primary"
                        || highway_type == "secondary" || highway_type == "tertiary" 
                        || highway_type == "unclassified" || highway_type == "residential")
                    {
                        draw_street_segment_meters(g, seg_id, from_xy, to_xy, highway_type);
                    }
                } else if (curr_world_width < ZOOM_LIMIT_3 && highway_type != "motorway" && highway_type != "motorway_link")
                {
                    draw_street_segment_meters(g, seg_id, from_xy, to_xy, highway_type);
                }

                // Get street names and position of segments chosen to display name/arrow
                // Only display names for "main" streets
                if ((ZOOM_LIMIT_3 <= curr_world_width && curr_world_width < ZOOM_LIMIT_2
                    && street_name != "<unknown>" && (i != 0) && (i % 2 == 0)
                    && (highway_type == "primary" || highway_type == "secondary"))
                    ||
                    (curr_world_width < ZOOM_LIMIT_3
                    && street_name != "<unknown>" && (i != 0) && (i % 2 == 0)
                    && (highway_type == "primary" || highway_type == "secondary"
                       || highway_type == "tertiary" || highway_type == "residential")))
                {
                    if (Segment_SegmentDetailedInfo[seg_id].length < 100) 
                    {
                        continue;        // Skip segments that are too short
                    }
                    segment_short_info.from_xy = from_xy;
                    segment_short_info.to_xy = to_xy;
                    segment_short_info.street_name = street_name;
                    // Separate between name-displaying segments and arrow-displaying segments
                    segment_short_info.arrow = Segment_SegmentDetailedInfo[seg_id].oneWay;
                    seg_names_and_arrows.push_back(segment_short_info);
                }
            }
        }
    }

    // Draw motorway and motorway-link (highways) above other streets
    for (int i = 0; i < highway_segments.size(); i++)
    {
        SegShortInfo segInfo = highway_segments[i];
        if (curr_world_width >= ZOOM_LIMIT_2 && segInfo.street_type == "motorway")
        {
            draw_street_segment_pixel(g, segInfo.seg_id, segInfo.from_xy, segInfo.to_xy, "motorway");
        } else if (curr_world_width < ZOOM_LIMIT_2)
        {
            draw_street_segment_meters(g, segInfo.seg_id, segInfo.from_xy, segInfo.to_xy, "motorway");
        }
    }

    /********************************************************************************
    * Draw Subways
    ********************************************************************************/
    // Display subway lines
    if (subway_line_mode || subway_station_mode)
    {
        g->set_line_width(4);
        int count = NUM_REGIONS;
        for (int route = 0; route < AllSubwayRoutes.size(); route++)
        {
            // Display subway route
            if (subway_line_mode)
            {
                if (AllSubwayRoutes[route].track_points.size() == 0)
                {
                    continue;
                }
                // Set subway line to processed color
                g->set_color((AllSubwayRoutes[route].colour));
                for (int way = 0; way < (AllSubwayRoutes[route].track_points.size()); way++)
                {
                    for (int node = 0; node < AllSubwayRoutes[route].track_points[way].size() - 1; node++)
                    {
                        g->draw_line(AllSubwayRoutes[route].track_points[way][node], 
                                    AllSubwayRoutes[route].track_points[way][node + 1]);
                    }
                }
            }
            // Display subway stations
            if (subway_station_mode)        // TODO: Avoid redundant stations
            {
                if (AllSubwayRoutes[route].station_points.size() == 0)
                {
                    continue;
                }
                for (int i = 0; i < AllSubwayRoutes[route].station_points.size() && count; i++)
                {
                    // Skips if subway is not in visible world
                    if (visible_world.contains(AllSubwayRoutes[route].station_points[i]))
                    {
                        // Draw all if at very low zoom level
                        if (curr_world_width < ZOOM_LIMIT_4)
                        {
                            draw_pin(g, AllSubwayRoutes[route].station_points[i]);
                        } else
                        {
                            for (int j = 0; j < NUM_REGIONS; j++)
                            {
                                // Draw 12 stations for high zoom levels
                                if (available_region[j][2] && visible_regions[j].contains(AllSubwayRoutes[route].station_points[i]))
                                {
                                    available_region[j][2]--;
                                    draw_pin(g, AllSubwayRoutes[route].station_points[i]);
                                    count--;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /********************************************************************************
    * Draw result path of navigation mode
    ********************************************************************************/
    for (auto seg_id : found_path)
    {
        IntersectionIdx from_id = Segment_SegmentDetailedInfo[seg_id].from;
        IntersectionIdx to_id = Segment_SegmentDetailedInfo[seg_id].to;
        ezgl::point2d from_xy = Intersection_IntersectionInfo[from_id].position_xy;
        ezgl::point2d to_xy = Intersection_IntersectionInfo[to_id].position_xy;

        if (ZOOM_LIMIT_2 <= curr_world_width)
        {
            draw_street_segment_pixel(g, seg_id, from_xy, to_xy, "path");
        } else
        {
            draw_street_segment_meters(g, seg_id, from_xy, to_xy, "path");
        }
    }

    /********************************************************************************
    * Draw street names and arrows
    ********************************************************************************/
    // Draw street names in rectangular regions if region is available
    if (curr_world_width < ZOOM_LIMIT_2)
    {
        // If all regions are unavailable, break for-loop early
        int count_names = NUM_REGIONS;
        int count_arrows = NUM_REGIONS;
        for (int i = 0; i < seg_names_and_arrows.size() && count_names && count_arrows; i++)
        {
            SegShortInfo seg_info = seg_names_and_arrows[i];
            for (int region = 0; region < NUM_REGIONS; region++)
            {
                if (visible_regions[region].contains(seg_info.from_xy) && 
                    visible_regions[region].contains(seg_info.to_xy))
                {
                    if (available_region[region][1] && seg_info.arrow)
                    {   // Display arrow
                        draw_name_or_arrow(g, seg_info.street_name, seg_info.arrow, 
                                            seg_info.from_xy, seg_info.to_xy);
                        available_region[region][1] = 0;
                        count_arrows--;
                        break;
                    } else if (available_region[region][0] && !seg_info.arrow)
                    {   // Display street name
                        draw_name_or_arrow(g, seg_info.street_name, seg_info.arrow, 
                                            seg_info.from_xy, seg_info.to_xy);
                        available_region[region][0] = 0;
                        count_names--;
                        break;
                    }
                }
            }
        }
    }
    
    /********************************************************************************
    * Draw POIs and Icons
    ********************************************************************************/
    poi_display.clear();
    poi_display.resize(NUM_REGIONS);
    // Get POI names and position chosen to display
    for (int tempIdx = 0; tempIdx < POINum; tempIdx++)
    {
        POIDetailedInfo tempPOI = POI_AllInfo[tempIdx];
        if (curr_world_width < ZOOM_LIMIT_4)
        {
            for (int regionIdx = 0; regionIdx < NUM_REGIONS; regionIdx++)
            {
                if (visible_regions[regionIdx].contains(tempPOI.POIPoint))
                {
                    poi_display[regionIdx].push_back(tempPOI);
                    break;
                }
            }
        }
    }
    // Display the selected POIs and Icons
    if (curr_world_width < ZOOM_LIMIT_4)
    {
        for (int regionIdx = 0; regionIdx < NUM_REGIONS; regionIdx++)
        {
            draw_POIs(g, regionIdx);
        }
    }

    /********************************************************************************
    * Draw pins for currently selected Intersections/POIs
    ********************************************************************************/
    for (auto& point : pin_display_start)
    {
        if (!visible_world.contains(point))
        {
            continue;
        }
        draw_pin(g, point);
    }
    for (auto& point : pin_display_dest)
    {
        if (!visible_world.contains(point))
        {
            continue;
        }
        draw_pin(g, point);
    }
    
    /********************************************************************************
    * Draw the distance scale
    ********************************************************************************/
    draw_distance_scale (g, visible_world);
}
