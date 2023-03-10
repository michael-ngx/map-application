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
#include "globals.h"
#include "OSMDatabaseAPI.h"
#include <cmath>
#include <chrono>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>

/*******************************************************************************************************************************
 * GLOBAL VARIABLES
 ********************************************************************************************************************************/
std::string CURRENT_CITY = " ";

// Zoom limits for curr_world_width, in meters
const float ZOOM_LIMIT_0 = 50000;
const float ZOOM_LIMIT_1 = 15000;
const float ZOOM_LIMIT_2 = 5000;
const float ZOOM_LIMIT_3 = 2000;
// Current world width in meters
double curr_world_width;

std::string stName1, stName2;

// Short info for segments whose street name will be displayed
struct SegShortInfo
{
    std::string street_name;
    ezgl::point2d from_xy;
    ezgl::point2d to_xy;
};
// All segments whose street name will be displayed
std::vector<SegShortInfo> seg_names_to_display;

/*******************************************************************************************************************************
 * FUNCTION DECLARATIONS
 ********************************************************************************************************************************/

/*************************************************************
 * Draw to the main canvas using the provided graphics object. 
 * Runs every time graphics are refreshed/image zooms or pans
 * The graphics object expects that x and y values will be in the main canvas' world coordinate system.
 *************************************************************/
void draw_main_canvas(ezgl::renderer *g);

/*************************************************************
 * Initial Setup is a mandatory function for any EZGL application, and is run whenever a window is opened. 
 *************************************************************/
void initial_setup(ezgl::application *application, bool new_window);

/*************************************************************
 * EVENT CALLBACK FUNCTIONS
 * 
 * These functions run whenever their corresponding event (key press, mouse move, or mouse click) occurs.
 *************************************************************/
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);
void act_on_mouse_move(ezgl::application *application, GdkEventButton *event, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name);

/*************************************************************
 * UI CALLBACK FUNCTIONS
 * 
 * These are callback functions for the UI elements
 *************************************************************/
void city_change_cbk(GtkComboBoxText* self, ezgl::application* app);
void input_streets_cbk(GtkWidget */*widget*/, ezgl::application* app);
void dialog_cbk(GtkDialog* self, gint response_id, ezgl::application* app);

/************************************************************
 * HELPER FUNCTIONS 
 ************************************************************/
void draw_feature_area(ezgl::renderer *g, FeatureDetailedInfo tempFeatureInfo);
void draw_street_segments_pixel(ezgl::renderer *g, StreetSegmentIdx seg_id, 
                          ezgl::point2d from_xy, ezgl::point2d to_xy, 
                          std::string street_type);
int get_line_width(std::string street_type);
void draw_street_segment_names(ezgl::renderer *g, std::string street_name, ezgl::point2d from_xy, ezgl::point2d to_xy);
void draw_highlighted_intersections(ezgl::renderer* g, ezgl::point2d inter_xy);
std::string get_new_map_path(std::string text_string);

/*******************************************************************************************************************************
 * DRAW MAP
 ********************************************************************************************************************************/
void drawMap()
{
    // Set up the ezgl graphics window and hand control to it, as shown in the
    // ezgl example program.
    // This function will be called by both the unit tests (ece297exercise)
    // and your main() function in main/src/main.cpp.
    // The unit tests always call loadMap() before calling this function
    // and call closeMap() after this function returns.

    // Draw map
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";

    ezgl::application application(settings);

    ezgl::rectangle initial_world(xy_from_latlon(latlon_bound.min),
                                  xy_from_latlon(latlon_bound.max));

    application.add_canvas("MainCanvas", draw_main_canvas, initial_world, ezgl::color(240, 240, 240));


    application.run(initial_setup, act_on_mouse_click,
                    nullptr, nullptr);
}

/*******************************************************************************************************************************
 * DRAW MAIN CANVAS
 ********************************************************************************************************************************/
void draw_main_canvas(ezgl::renderer *g)
{
    // Check for current zoom level through visible width of world
    ezgl::rectangle visible_world = g->get_visible_world();
    curr_world_width = visible_world.width();
    std::cout << curr_world_width << std::endl;
    
    // Draw features
    for (int j = 0; j < featureNum; j++)
    {
        FeatureDetailedInfo tempFeatureInfo = Features_AllInfo[j];
        draw_feature_area(g, tempFeatureInfo);
    }
    
    // Loop through streets
    for (StreetIdx street_id = 0; street_id < streetNum; street_id++)
    {
        // Get vector of all segments
        std::vector<StreetSegmentIdx> all_segments = Streets_AllSegments.find(street_id)->second;         // Vector of all segments
        int total_segment_amount = all_segments.size();

        // Number of segments to display for each street (= fraction of total segment amount)           // TODO: Step based on total segments and zoom levels
        // int num_seg_to_add = total_segment_amount * 0.00005;
        // Avoid dividing by zero
        // if (!num_seg_to_add) continue;
        // 
        // int step = total_segment_amount / num_seg_to_add;
        int step = 20;
        // Draw street names based on number of segments of each street and zoom levels
        // Draw street names starting at lowest 2 zoom level
        std::string street_name = Segment_SegmentDetailedInfo[all_segments[0]].streetName;
        for (int i = 0; i < total_segment_amount; i++)
        {
            StreetSegmentIdx seg_id = all_segments[i];
            // Get LatLon information of from and to intersections from each segments
            IntersectionIdx from_id = Segment_SegmentDetailedInfo[seg_id].from;
            IntersectionIdx to_id = Segment_SegmentDetailedInfo[seg_id].to;
            ezgl::point2d from_xy = Intersection_IntersectionInfo[from_id].position_xy;
            ezgl::point2d to_xy = Intersection_IntersectionInfo[to_id].position_xy;

            // Check the type of street this segment belongs to through wayOSMID
            OSMID wayOSMID = Segment_SegmentDetailedInfo[seg_id].wayOSMID;
            std::string highway_type = OSMID_Highway_Type.at(wayOSMID);
            
            // Draws different amount of data based on different zoom levels
            if (curr_world_width >= ZOOM_LIMIT_0)
            {
                if (highway_type == "motorway" || highway_type == "primary")
                {   
                    draw_street_segments_pixel(g, seg_id, from_xy, to_xy, highway_type);
                }
            } else if (ZOOM_LIMIT_1 <= curr_world_width && curr_world_width < ZOOM_LIMIT_0)
            {
                if (highway_type == "motorway" || highway_type == "trunk"
                        || highway_type == "primary" || highway_type == "secondary")
                {   
                    draw_street_segments_pixel(g, seg_id, from_xy, to_xy, highway_type);
                }
                        
            } else if (ZOOM_LIMIT_2 <= curr_world_width && curr_world_width < ZOOM_LIMIT_1)
            {
                if (highway_type == "motorway" || highway_type == "motorway_link" || highway_type == "trunk"
                    || highway_type == "primary" || highway_type == "secondary" || highway_type == "tertiary")
                {
                    draw_street_segments_pixel(g, seg_id, from_xy, to_xy, highway_type);
                }
            } else if (ZOOM_LIMIT_3 <= curr_world_width && curr_world_width < ZOOM_LIMIT_2)
            {
                if (highway_type == "motorway" || highway_type == "motorway_link" || highway_type == "trunk"
                    || highway_type == "primary" || highway_type == "secondary" || highway_type == "tertiary" 
                    || highway_type == "unclassified" || highway_type == "residential" )
                {
                    draw_street_segments_pixel(g, seg_id, from_xy, to_xy, highway_type);
                    // If display street name on segment, get street names and position
                    if (i % step == 2 && street_name != "<unknown>")
                    {
                        SegShortInfo short_info;
                        short_info.street_name = street_name;
                        short_info.from_xy = from_xy;
                        short_info.to_xy = to_xy;
                        seg_names_to_display.push_back(short_info);
                    }
                }
            } else
            {
                draw_street_segments_pixel(g, seg_id, from_xy, to_xy, highway_type);
                // If display street name on segment, get street names and position
                if (i % step == 2 && street_name != "<unknown>")
                {
                    SegShortInfo short_info;
                    short_info.street_name = street_name;
                    short_info.from_xy = from_xy;
                    short_info.to_xy = to_xy;
                    seg_names_to_display.push_back(short_info);
                }
            }
        }
    }

    // Draw street names
    if ((ZOOM_LIMIT_3 <= curr_world_width && curr_world_width < ZOOM_LIMIT_2)
        || (curr_world_width < ZOOM_LIMIT_3))
    {
        for (auto &seg_info : seg_names_to_display)
        {
            draw_street_segment_names(g, seg_info.street_name, seg_info.from_xy, seg_info.to_xy);
        }
        // Clear vector every frame to avoid multiple drawings
        seg_names_to_display.clear();
    }

    // Draw highlighted intersection(s)
    for (int i = 0; i < intersectionNum; i++)
    {
        if (Intersection_IntersectionInfo[i].highlight)
            draw_highlighted_intersections(g, Intersection_IntersectionInfo[i].position_xy);
    }
}
 
 /*******************************************************************************************************************************
 * EVENT CALLBACKS
 ********************************************************************************************************************************/
// Function called before the activation of the application
void initial_setup(ezgl::application *application, bool /*new_window*/)
{
  // Update the status bar message
  application->update_message("EZGL Application");
  
  // Setting our starting row for insertion at 6 (Default zoom/pan buttons created by EZGL take up first five rows);
  // We will increment row each time we insert a new element. 
  int row = 6;
  // Ask user to input names of two streets              // TODO: segmentation fault if street name not found
  application->create_button("Search intersections", row++, input_streets_cbk);
  
  application->create_label(row++, "Select city:"); 
  //Creating drop-down list for different cities, connected to city_change_cbk
  application->create_combo_box_text(
    "CitySelect", 
    row++,
    city_change_cbk,
    {" ", "Toronto", "Beijing", "Cairo", "Cape Town", "Golden Horseshoe", 
    "Hamilton", "Hong Kong", "Iceland", "Interlaken", "Kyiv",
    "London", "New Delhi", "New York", "Rio de Janeiro", "Saint Helena",
    "Singapore", "Sydney", "Tehran", "Tokyo"}
  );
}

// Storing state of mouse clicks
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y)
{
    LatLon pos = LatLon(latlon_from_xy(x, y));
    // Highlight / Unhighlight closest intersections to mouseclick
    int id = findClosestIntersection(pos);
    Intersection_IntersectionInfo[id].highlight = !Intersection_IntersectionInfo[id].highlight;
    if (Intersection_IntersectionInfo[id].highlight)
    {   // Update mesasge if newly highlight an intersection
        app->update_message("Intersection selected: " + Intersection_IntersectionInfo[id].name);
    }
    app->refresh_drawing();
}

/*******************************************************************************************************************************
 * UI CALLBACKS
 ********************************************************************************************************************************/

// Callback function for the city change drop down list.
// Function trigerred when currently selected option changes. 
void city_change_cbk(GtkComboBoxText* self, ezgl::application* app){
    //Getting current text content
    auto text = gtk_combo_box_text_get_active_text(self);
    std::string text_string = text;
    if(!text || text_string == " ")
    {  //Returning if the combo box is currently empty (Always check to avoid errors)
        return;
    } else if (text_string != CURRENT_CITY)
    {                   // TODO: Bug current city is reloaded if user select current city as first choice
        CURRENT_CITY = text_string;
        std::string new_map_path = get_new_map_path(text_string);

        // Closes current map and loads the new city
        closeMap();
        loadMap(new_map_path);
        std::cout << "Loaded new map" << std::endl;
        ezgl::rectangle new_world(xy_from_latlon(latlon_bound.min),
                                    xy_from_latlon(latlon_bound.max));
       
        app->change_canvas_world_coordinates("MainCanvas", new_world);
        app->refresh_drawing();
    }
}

// asks user to input street names
void input_streets_cbk(GtkWidget */*widget*/, ezgl::application* app){
    //app->create_dialog_window(dialog_cbk, "Notice", "Please input street names!");
    std::cout << "Please enter 2 street names (separated by 'enter'): " << std::endl;
    while(1){
        getline(std::cin, stName1);
        getline(std::cin, stName2);
        std::vector<StreetIdx> FoundInterIdx1 = findStreetIdsFromPartialStreetName(stName1);
        std::vector<StreetIdx> FoundInterIdx2 = findStreetIdsFromPartialStreetName(stName2);
        
        if(!FoundInterIdx1.size() || !FoundInterIdx2.size()){
            std::cout << "No match!" << std::endl;
            break;
        }
        
        std::vector<IntersectionIdx> FoundInters 
                                     = findIntersectionsOfTwoStreets(FoundInterIdx1[0], FoundInterIdx2[0]);
        
        if(!FoundInters.size()){
            std::cout << "No match!" << std::endl;
            break;
        }
        
        for(auto interIdx : FoundInters){
            std::cout << std::endl;
            std::cout << "Info of this intersection --------" << std::endl;
            std::cout << "Name of Intersection: " << Intersection_IntersectionInfo[interIdx].name << std::endl;
            std::cout << "X position: " << Intersection_IntersectionInfo[interIdx].position_xy.x << std::endl
                     << "Y position: " << Intersection_IntersectionInfo[interIdx].position_xy.y << std::endl;
        }
        break;
    }   
}

/**
 * Callback function for dialog window created by "Create Dialog Window" button. 
 * Updates application message to reflect user answer to dialog window. 
 */
void dialog_cbk(GtkDialog* self, gint response_id, ezgl::application* app){
  //Response_id is an integer/enumeration, so we can use a switch to read its value and act accordingly
  switch(response_id){
    case GTK_RESPONSE_ACCEPT:
      app->update_message("USER ACCEPTED");
      break;
    case GTK_RESPONSE_REJECT:
      app->update_message("USER REJECTED");
      break;
    case GTK_RESPONSE_DELETE_EVENT:
      app->update_message("USER CLOSED WINDOW");
      break;
    default:
      app->update_message("YOU SHOULD NOT SEE THIS");
  }

  //We always have to destroy the dialog window in the callback function or it will never close
  gtk_widget_destroy(GTK_WIDGET(self));
}

/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/
// Draw street segments
void draw_street_segments_pixel(ezgl::renderer *g, StreetSegmentIdx seg_id, 
                          ezgl::point2d from_xy, ezgl::point2d to_xy, 
                          std::string street_type)
{
    // Set colors and line width according to street type
    if (street_type == "motorway" || street_type == "motorway_link")
        g->set_color(255, 212, 124);
    else 
        g->set_color(ezgl::WHITE);

    // Set line width based on current zoom level and street type    
    int line_width = get_line_width(street_type); 
    g->set_line_width(line_width);

    // Round street ends
    g->set_line_cap(ezgl::line_cap(1));
    // Draw street segments including curvepoints
    ezgl::point2d curve_pt_xy; // Temp xy for current curve point.
                               // Starts drawing at from_xy to first curve point.
    
    // Connecting curvepoints. Increment from_xy.
    for (int i = 0; i < Segment_SegmentDetailedInfo[seg_id].numCurvePoints; i++)
    {
        curve_pt_xy = Segment_SegmentDetailedInfo[seg_id].curvePoints_xy[i];
        g->draw_line(from_xy, curve_pt_xy);
        from_xy = curve_pt_xy;
    }
    // Connect last curve point to (x_to, y_to)
    g->draw_line(from_xy, to_xy);
}

// Manually fix street width with pixels according to zoom levels
int get_line_width(std::string street_type)
{
    if (curr_world_width > ZOOM_LIMIT_0)
    {
        if (street_type == "motorway") return 4;
        else return 2;
    } else if (ZOOM_LIMIT_1 < curr_world_width && curr_world_width < ZOOM_LIMIT_0)
    {
        if (street_type == "motorway") return 5;
        else if (street_type == "primary") return 3;
        else if (street_type == "trunk") return 0;
        else if (street_type == "secondary") return 0;
    } else if (ZOOM_LIMIT_2 < curr_world_width && curr_world_width < ZOOM_LIMIT_1)
    {
        if (street_type == "motorway") return 5;
        if (street_type == "motorway_link") return 5;
        else if (street_type == "primary") return 5;
        else if (street_type == "trunk") return 3;
        else if (street_type == "secondary") return 2;
        else if (street_type == "tertiary") return 2;
    } else if (ZOOM_LIMIT_3 < curr_world_width && curr_world_width < ZOOM_LIMIT_2)
    {
        if (street_type == "motorway") return 6;
        if (street_type == "motorway_link") return 6;
        else if (street_type == "primary") return 6;
        else if (street_type == "trunk") return 4;
        else if (street_type == "secondary") return 5;
        else if (street_type == "tertiary") return 4;
        else if (street_type == "unclassified") return 4;
        else if (street_type == "residential") return 4;
    } else
    {
        if (street_type == "motorway") return 8;
        if (street_type == "motorway_link") return 8;
        else if (street_type == "primary") return 8;
        else if (street_type == "trunk") return 5;
        else if (street_type == "secondary") return 7;
        else if (street_type == "tertiary") return 6;
        else if (street_type == "unclassified") return 5;
        else if (street_type == "residential") return 5;
        else return 0;
    }
    return 0;
}

// Display intersection if highlighted
void draw_highlighted_intersections(ezgl::renderer* g, ezgl::point2d inter_xy)
{
    ezgl::surface *png_surface = g->load_png("libstreetmap/resources/red_pin.png");
    g->draw_surface(png_surface, inter_xy);
    g->free_surface(png_surface);
}

// Draws text on street segments
void draw_street_segment_names(ezgl::renderer *g, std::string street_name, ezgl::point2d from_xy, ezgl::point2d to_xy)
{
    // Calculate text rotation based on segment slope                   // TODO: Curved segments!
                                                                        // TODO: Boundaries and overlapping
    double angle_degree;
    if (to_xy.x == from_xy.x)
        angle_degree = 90;
    else
    {
        double slope = (to_xy.y - from_xy.y)/(to_xy.x - from_xy.x);
        if (slope >= 0)
            angle_degree = atan2(abs(to_xy.y - from_xy.y), abs(to_xy.x - from_xy.x))/kDegreeToRadian;
        else
            angle_degree = 360 - atan2(abs(to_xy.y - from_xy.y), abs(to_xy.x - from_xy.x))/kDegreeToRadian;
    }
    
    g->set_text_rotation(angle_degree);
    g->set_color(0, 0, 0);
    g->set_font_size(7);
    ezgl::point2d mid_xy = {(from_xy.x + to_xy.x) / 2, (from_xy.y + to_xy.y) / 2};
    g->draw_text(mid_xy, street_name);

}

// Get map path for reloading from string of city name
std::string get_new_map_path(std::string text_string)
{
    std::string new_map_path;
    if (text_string == "Toronto") new_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    else if (text_string == "Beijing") new_map_path = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
    else if (text_string == "Cairo") new_map_path = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
    else if (text_string == "Cape Town") new_map_path = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
    else if (text_string == "Golden Horseshoe") new_map_path = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
    else if (text_string == "Hamilton") new_map_path = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
    else if (text_string == "Hong Kong") new_map_path = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
    else if (text_string == "Iceland") new_map_path = "/cad2/ece297s/public/maps/iceland.streets.bin";
    else if (text_string == "Interlaken") new_map_path = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
    else if (text_string == "Kyiv") new_map_path = "/cad2/ece297s/public/maps/kyiv_ukraine.streets.bin";
    else if (text_string == "London") new_map_path = "/cad2/ece297s/public/maps/london_england.streets.bin";
    else if (text_string == "New Delhi") new_map_path = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
    else if (text_string == "New York") new_map_path = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
    else if (text_string == "Rio de Janeiro") new_map_path = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
    else if (text_string == "Saint Helena") new_map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
    else if (text_string == "Singapore") new_map_path = "/cad2/ece297s/public/maps/singapore.streets.bin";
    else if (text_string == "Sydney") new_map_path = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
    else if (text_string == "Tehran") new_map_path = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
    else if (text_string == "Tokyo") new_map_path = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
    return new_map_path;
}

void draw_feature_area(ezgl::renderer *g, FeatureDetailedInfo tempFeatureInfo)
{
    FeatureType tempType = tempFeatureInfo.featureType;
    std::vector<ezgl::point2d> tempPoints = tempFeatureInfo.featurePoints;
    if (tempType == PARK)
    {
        if (tempPoints.size() > 1){
            g->set_color(206, 234, 214);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == BEACH)
    {
        if (tempPoints.size() > 1){
            g->set_color(255, 235, 205);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == LAKE)
    {
        if (tempPoints.size() > 1){
            g->set_color(153, 204, 255);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == ISLAND)
    {
        if (tempPoints.size() > 1){
            g->set_color(168, 218, 181);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == BUILDING)
    {
        if (tempPoints.size() > 1){
            g->set_color(230, 230, 230);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == GREENSPACE)
    {
        if (tempPoints.size() > 1){
            g->set_color(206, 234, 214);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == GOLFCOURSE)
    {
        if (tempPoints.size() > 1){
            g->set_color(168, 218, 181);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == GLACIER)
    {
        if (tempPoints.size() > 1){
            g->set_color(114, 157, 200);
            g->fill_poly(tempPoints);
        }
    } 
    return;
}

// Algorithm for coordinate street display
// draw(g, from_xy, to_xy, half_lane_width);

// void draw(ezgl::renderer *g, ezgl::point2d from_xy, ezgl::point2d to_xy, float width)
// {
//     g->fill_arc(from_xy, width, 0, 360);
//     g->fill_arc(to_xy, width, 0, 360);
//     if (to_xy.x == from_xy.x)
//     {
//         return;
//     } else if ((to_xy.y == from_xy.y))
//     {
//         return;
//     } else 
//     {
//         double slope = -abs(to_xy.x - from_xy.x)/abs(to_xy.y - from_xy.y);
//         double delta_x = width / sqrt(1 + pow(slope, 2));
//         double delta_y = slope * delta_x;
//         ezgl::point2d point_1(from_xy.x + delta_x, from_xy.y + delta_y);
//         ezgl::point2d point_2(from_xy.x - delta_x, from_xy.y - delta_y);
//         ezgl::point2d point_3(to_xy.x + delta_x, to_xy.y + delta_y);
//         ezgl::point2d point_4(to_xy.x - delta_x, to_xy.y - delta_y);
//         std::vector<ezgl::point2d> a;
//         a.push_back(point_1);
//         a.push_back(point_3);
//         a.push_back(point_4);
//         a.push_back(point_2);
//         g->fill_poly(a);
//     }
// }