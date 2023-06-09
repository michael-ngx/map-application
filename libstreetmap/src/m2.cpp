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
#include "grid.h"
#include "ui_callbacks/widgets.hpp"
#include "ui_callbacks/setup.hpp"
#include "draw/draw.hpp"
#include "draw/utilities.hpp"
#include <cmath>
#include <algorithm>
#include <chrono>

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
GObject *DirectionDisplay;
GObject *DirectionWindow;
GtkTextBuffer *DirectionTextBuffer;

GObject *FilterComboBox;
GObject *CityChangeComboBox;

GObject *SearchBar;
GObject *SearchBarDestination;
GtkListStore *list_store;
GtkTreeIter iter;
GtkEntryCompletion *completion;
GtkEntryCompletion *completion_destination;

/**********************************************
 * Feature states
 *********************************************/
// Check current filter for applying filters
std::string CURRENT_FILTER = "Filters";
// Checks if night mode is on
bool night_mode = false;
// Checks if filter is on
bool filtered = false;
// Checks if the subway mode if turned on
bool subway_mode = false;
// Checks if the navigation mode if turned on (to allow navigation)
bool navigation_mode = false;
// Checks if the direction display is on
bool direction_display_on = false;

/**********************************************
 * Drawing - zoom levels & grids
 *********************************************/
// Rectangle of current visible world, in meters
ezgl::rectangle visible_world;
double curr_world_width;
double curr_world_height;

// Index: FeatureIdx, value: boolean to check if a feature has been drawn
std::vector<bool> check_feature_drawn(featureNum);
// Index: StreetSegmentIdx, value: boolean to check if a segment has been drawn
std::vector<bool> check_segment_drawn(segmentNum);
// Index: StreetSegmentIdx, value: boolean to check if a segment name has been drawn
std::vector<bool> check_name_drawn(segmentNum);

/**********************************************
 * Navigation & search
 *********************************************/
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
    ezgl::rectangle initial_world(world_bottom_left,
                                  world_top_right);
    
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
    // Calculate FPS
    auto start_time = std::chrono::high_resolution_clock::now();
    /********************************************************************************
    * Local variables of current canvas
    ********************************************************************************/
    // Check for current zoom level through visible width (in meters) of world
    visible_world = g->get_visible_world();
    curr_world_width = visible_world.width();
    curr_world_height = visible_world.height();
    // Reset all features to be not drawn
    check_feature_drawn.assign(featureNum, false);
    // Reset all segments to be not drawn
    check_segment_drawn.assign(segmentNum, false);
    // Reset all segments to be not drawn
    check_name_drawn.assign(segmentNum, false);

    //Draw the canvas for Night Mode
    if (night_mode)
    {
        ezgl::rectangle visible_world_new = g->get_visible_world();
        g->set_color(43, 56, 70);
        g->fill_rectangle(visible_world_new);
    }

    /********************************************************************************
    * Determine which grids can be drawn
    ********************************************************************************/
    // Determine bounds of MapGrid to be drawn
    int col_max = (visible_world.right() - world_bottom_left.x) / grid_width;
    int col_min = (visible_world.left() - world_bottom_left.x) / grid_width;
    int row_max = (visible_world.top() - world_bottom_left.y) / grid_height;
    int row_min = (visible_world.bottom() - world_bottom_left.y) / grid_height;
    // We will draw contents within the grids (col_min - 1 -> col_max + 1) and (row_min - 1 -> row_max + 1)
    // +-1 is done for smooth transition between grids
    if (col_max > NUM_GRIDS - 2)
    {
        col_max = NUM_GRIDS - 2;
        if (col_min > NUM_GRIDS)
        {
            col_min = NUM_GRIDS;
        }
    }
    if (row_max > NUM_GRIDS - 2)
    {
        row_max = NUM_GRIDS - 2;
        if (row_min > NUM_GRIDS)
        {
            row_min = NUM_GRIDS;
        }
    }
    if (col_min < 1)
    {
        col_min = 1;
        if (col_max < -1)
        {
            col_max = -1;
        }
    }
    if (row_min < 1)
    {
        row_min = 1;
        if (row_max < -1)
        {
            row_max = -1;
        }
    }

    /********************************************************************************
    * Draw features
    ********************************************************************************/
    double limit = 0;
    // Determine area limit of features to be drawn to screen based on zoom levels
    // Only areas larger than the limit can be drawn to the screen
    if (curr_world_width >= ZOOM_LIMIT_0)
    {
        limit = FEATURE_AREA_LIMIT_0;
    } else if (ZOOM_LIMIT_1 <= curr_world_width && curr_world_width < ZOOM_LIMIT_0)
    {
        limit = FEATURE_AREA_LIMIT_1;
    } else if (ZOOM_LIMIT_2 <= curr_world_width && curr_world_width < ZOOM_LIMIT_1)
    {
        limit = FEATURE_AREA_LIMIT_2;
    } else if (ZOOM_LIMIT_3 <= curr_world_width && curr_world_width < ZOOM_LIMIT_2)
    {
        limit = FEATURE_AREA_LIMIT_3;
    } else if (ZOOM_LIMIT_4 <= curr_world_width && curr_world_width < ZOOM_LIMIT_3)
    {
        limit = FEATURE_AREA_LIMIT_4;
    }
    
    for (int i = row_min - 1; i <= row_max + 1; i++)
    {
        for (int j = col_min - 1; j <= col_max + 1; j++)
        {
            if (MapGrids[i][j].Grid_Features.size() != 0)
            {
                MapGrids[i][j].draw_grid_features(g, limit);
            }
        }
    }

    /********************************************************************************
    * Draw street segments
    ********************************************************************************/
    for (int i = row_min - 1; i <= row_max + 1; i++)
    {
        for (int j = col_min - 1; j <= col_max + 1; j++)
        {
            MapGrids[i][j].draw_grid_segments(g);
        }
    }

    /********************************************************************************
    * Draw subway if in subway mode
    ********************************************************************************/
    if (subway_mode)
    {
        draw_subway_lines(g);
        for (int i = row_min - 1; i <= row_max + 1; i++)
        {
            for (int j = col_min - 1; j <= col_max + 1; j++)
            {
                MapGrids[i][j].draw_grid_subway_stations(g);
            }
        }
    }

    /********************************************************************************
    * Draw result path of navigation mode
    ********************************************************************************/
    for (int i = 0; i < found_path.size(); i++)
    {
        StreetSegmentDetailedInfo segment = Segment_SegmentDetailedInfo[found_path[i]];
        if (ZOOM_LIMIT_2 <= curr_world_width)
        {
            draw_street_segment_pixel(g, segment, true);
        } else
        {
            draw_street_segment_meters(g, segment, true);
        }
    }

    /********************************************************************************
    * Draw street names and arrows if zoomed in enough
    * Names on top of found path is included
    ********************************************************************************/
    if (curr_world_width < ZOOM_LIMIT_1)
    {
        for (int i = row_min - 1; i <= row_max + 1; i++)
        {
            for (int j = col_min - 1; j <= col_max + 1; j++)
            {
                MapGrids[i][j].draw_grid_names(g);                
            }
        }
    }

    /********************************************************************************
    * Draw POIs if zoomed in enough
    ********************************************************************************/
    if (curr_world_width < ZOOM_LIMIT_4)
    {
        for (int i = row_min - 1; i <= row_max + 1; i++)
        {
            for (int j = col_min - 1; j <= col_max + 1; j++)
            {
                if (MapGrids[i][j].Grid_POIs.size() != 0)
                {
                    MapGrids[i][j].draw_grid_POIs(g);
                }
            }
        }
    }

    /********************************************************************************
    * Draw pins for currently selected Intersections/POIs
    ********************************************************************************/
    for (auto& point : pin_display_start)
    {
        draw_png(g, point, "red_pin");
    }
    for (auto& point : pin_display_dest)
    {
        draw_png(g, point, "dest_flag");
    }
    
    /********************************************************************************
    * Draw the distance scale
    ********************************************************************************/
    draw_distance_scale (g, visible_world);

    // End time
    auto current_time = std::chrono::high_resolution_clock::now();
    auto period = std::chrono::duration_cast<std::chrono::duration<double>> (current_time - start_time);
    std::cout << "FPS: " << 1/period.count() << std::endl;
}
