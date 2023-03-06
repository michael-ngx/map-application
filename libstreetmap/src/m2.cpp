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

double WORLD_AREA;
float ZOOM_LIMIT_1 = 30;
float ZOOM_LIMIT_2 = 5;
float ZOOM_LIMIT_3 = 1;
double world_percent;
/*******************************************************************************************************************************
 * HELPER FUNCTION DECLARATION
 ********************************************************************************************************************************/
void draw_main_canvas(ezgl::renderer *g);
void draw_street_segments(ezgl::renderer *g, StreetSegmentIdx seg_id, ezgl::point2d from_xy, ezgl::point2d to_xy);
void draw_street_segment_names(ezgl::renderer *g, StreetSegmentIdx seg_id, ezgl::point2d mid_xy);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);
void highlight_intersection(ezgl::renderer* g);
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

    WORLD_AREA = abs(xy_from_latlon(latlon_bound.max).x - xy_from_latlon(latlon_bound.min).x)
                      * abs(xy_from_latlon(latlon_bound.max).y - xy_from_latlon(latlon_bound.min).y);

    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);

    application.run(nullptr, act_on_mouse_click,
                    nullptr, nullptr);
}

/*******************************************************************************************************************************
 * DRAW MAIN CANVAS
 ********************************************************************************************************************************/
void draw_main_canvas(ezgl::renderer *g)
{
    //auto startTime = std::chrono::high_resolution_clock::now();
    
    // Check for current zoom level through area of visible world
    ezgl::rectangle world = g->get_visible_world();
    world_percent = world.area()/WORLD_AREA*100;
    std::cout << "world area percent: " << world_percent << "%" << std::endl;
    
    highlight_intersection(g);
    
    for (StreetSegmentIdx seg_id = 0; seg_id < segmentNum; seg_id++)
    {
        // Get LatLon information of from and to intersections from each segments
        IntersectionIdx from_id = Segment_SegmentDetailedInfo[seg_id].from;
        IntersectionIdx to_id = Segment_SegmentDetailedInfo[seg_id].to;
        ezgl::point2d from_xy = Intersection_IntersectionInfo[from_id].position_xy;
        ezgl::point2d to_xy = Intersection_IntersectionInfo[to_id].position_xy;
        ezgl::point2d mid_xy = {(from_xy.x + to_xy.x) / 2, (from_xy.y + to_xy.y) / 2};
        
        // Check the type of street this segment belongs to through wayOSMID
        OSMID wayOSMID = Segment_SegmentDetailedInfo[seg_id].wayOSMID;
        auto temp_vector = OSM_AllTagPairs.at(wayOSMID);
        for (auto tag : temp_vector)
        {
            if (tag.first == "highway")
            {   // Draws different amount of data based on different zoom levels
                if (world_percent > ZOOM_LIMIT_1)
                {
                    if (tag.second == "trunk" || tag.second == "motorway" 
                         || tag.second == "primary" || tag.second == "secondary")
                    {
                        draw_street_segments(g, seg_id, from_xy, to_xy);
                    }
                           
                } else if (ZOOM_LIMIT_2 < world_percent && world_percent < ZOOM_LIMIT_1)
                {
                    if (tag.second == "trunk" || tag.second == "motorway" || tag.second == "primary" 
                        || tag.second == "secondary" || tag.second == "tertiary")
                    {
                        draw_street_segments(g, seg_id, from_xy, to_xy);
                    }
                } else if (ZOOM_LIMIT_3 < world_percent && world_percent < ZOOM_LIMIT_2)
                {
                    if (tag.second == "trunk" || tag.second == "motorway" || tag.second == "primary" 
                        || tag.second == "secondary" || tag.second == "tertiary" 
                        || tag.second == "unclassified" || tag.second == "residential")
                    {
                        draw_street_segments(g, seg_id, from_xy, to_xy);
                    }
                } else if (world_percent <= ZOOM_LIMIT_3)
                {
                    draw_street_segments(g, seg_id, from_xy, to_xy);                   
                    draw_street_segment_names(g, seg_id, mid_xy);
                }
            }
        }
    }
//    auto currTime = std::chrono::high_resolution_clock::now();
//    auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>>(currTime - startTime);
//    std::cout << "draw main cavas took " << wallClock.count() << " seconds" << std::endl;
}

/*******************************************************************************************************************************
 *  function that stores state of mouse clicks
 ********************************************************************************************************************************/
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y){
    LatLon pos = LatLon(latlon_from_xy(x, y));   
    int id = findClosestIntersection(pos);   
    Intersection_IntersectionInfo[id].highligh = true;
    //std::cout << Intersection_IntersectionInfo[id].highligh << std::endl;  
    std::stringstream ss;
    ss << "Intersection selected: " << Intersection_IntersectionInfo[id].name;
    app->update_message(ss.str());
    app->refresh_drawing();
}

/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/

// Draw street segments
void draw_street_segments(ezgl::renderer *g, StreetSegmentIdx seg_id, ezgl::point2d from_xy, ezgl::point2d to_xy)
{
    // Draw street segments
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

// Draws text on street segments
void draw_street_segment_names(ezgl::renderer *g, StreetSegmentIdx seg_id, ezgl::point2d mid_xy)
{
    ezgl::rectangle rec;
    rec = g->get_visible_screen();
    std::string stName = getStreetName(Segment_SegmentDetailedInfo[seg_id].streetID);
    // g->set_color(0, 0, 0, 100);
    g->set_font_size(7);
    g->draw_text(mid_xy, stName, rec.m_second.x, rec.m_second.y);
}

// highlights selected intersection, and draws intersection like normal if nothing is selected
void highlight_intersection(ezgl::renderer* g){
    for(IntersectionIdx inter_id = 0; inter_id < intersectionNum; inter_id++){
        if(Intersection_IntersectionInfo[inter_id].highligh){
            g->set_color(ezgl::RED);
        }else{
           g->set_color(0, 0, 0); 
        }
              
        float width = 10;
        float height = width;
        ezgl::point2d inter_loc = Intersection_IntersectionInfo[inter_id].position_xy
                                  - ezgl::point2d{width / 2, height / 2};
                                  
        if (world_percent <= ZOOM_LIMIT_3){   
                g->fill_rectangle(inter_loc, width, height);
        }
    }
}
 