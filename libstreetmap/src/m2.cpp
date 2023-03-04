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
#include <cmath>
#include <chrono>
/*******************************************************************************************************************************
 * HELPER FUNCTION DECLARATION
 ********************************************************************************************************************************/
void set_up_windows();
void draw_main_canvas (ezgl::renderer *g);

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
    set_up_windows();
}

/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/

void set_up_windows()
{
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    ezgl::rectangle initial_world(xy_from_latlon(latlon_bound.min), 
                                    xy_from_latlon(latlon_bound.max));
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    application.run(nullptr, nullptr, 
                    nullptr, nullptr);
}

void draw_main_canvas(ezgl::renderer *g){
    auto startTime = std::chrono::high_resolution_clock::now();
    g->set_color(0,0,0);
    
    for (int seg_id = 0; seg_id < segmentNum; seg_id++)
    {
        // Get LatLon information of from and to intersections from each segments
        IntersectionIdx from_id = Segment_SegmentDetailedInfo[seg_id].from;
        IntersectionIdx to_id = Segment_SegmentDetailedInfo[seg_id].to;

        // TODO: fix the centre of intersection
        float width = 10;
        float height = width;

        // Draw street segments
        ezgl::point2d from_xy = Intersection_IntersectionInfo[from_id].position_xy;
        ezgl::point2d to_xy = Intersection_IntersectionInfo[to_id].position_xy;
        ezgl::point2d mid_xy = {(from_xy.x + to_xy.x) / 2, (from_xy.y + to_xy.y) / 2};
        ezgl::point2d curve_pt_xy;          // Temp xy for current curve point. 
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
        
//        Intersection_IntersectionInfo[from_id].position_xy.x -= 5;
//        Intersection_IntersectionInfo[from_id].position_xy.y -= 5;
        // Draw intersections corresponding to each segment. Not drawing curve points. 
        g->fill_rectangle({Intersection_IntersectionInfo[from_id].position_xy.x - 5, 
                           Intersection_IntersectionInfo[from_id].position_xy.y - 5}, 
                           width, height);
        g->fill_rectangle({Intersection_IntersectionInfo[to_id].position_xy.x - 5, 
                           Intersection_IntersectionInfo[to_id].position_xy.y - 5}, 
                           width, height);
                           
        // draws text on street segments
//        std::string stName = getStreetName(Segment_SegmentDetailedInfo[seg_id].streetID);
//        //g->set_color(0, 0, 0, 100);
//        g->set_font_size(7);
//        g->draw_text(mid_xy, stName);                 
    }
             
   auto currTime = std::chrono::high_resolution_clock::now();
   auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>>(currTime - startTime);
   std::cout << "draw main cavas took " << wallClock.count() << " seconds" << std::endl;
}

