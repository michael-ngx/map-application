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
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "LatLon.h"
#include <cmath>
#include <chrono>
/*******************************************************************************************************************************
 * GLOBAL VARIABLES AND HELPER FUNCTION DECLARATION
 ********************************************************************************************************************************/
struct IntersectionInfo{
    LatLon position;
    std::string name;
};

std::vector<IntersectionInfo> IntersectionInfoVec;

double max_lat;
double max_lon;
double min_lat;
double min_lon;
double latavg;
// *************************************************
// Function Declarations
// *************************************************
void draw_main_canvas (ezgl::renderer *g);
void draw_map_blank_canvas();
void intersection_init();
float x_from_lon(float lon);
float y_from_lat(float lat);
float lon_from_x(float x);
float lat_from_y(float y);
/*******************************************************************************************************************************
 * DRAW MAP
 ********************************************************************************************************************************/
void drawMap() {
   // Set up the ezgl graphics window and hand control to it, as shown in the 
   // ezgl example program. 
   // This function will be called by both the unit tests (ece297exercise) 
   // and your main() function in main/src/main.cpp.
   // The unit tests always call loadMap() before calling this function
   // and call closeMap() after this function returns.
    
    
   // Demo 1 
    intersection_init();
    draw_map_blank_canvas();
   // Demo 2
}


/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/

// Initialize IntersectionInfoVec
void intersection_init(){
    IntersectionInfoVec.resize(intersectionNum);
    max_lat = getIntersectionPosition(0).latitude();
    max_lon = getIntersectionPosition(0).longitude();
    min_lat = max_lat;
    min_lon = max_lon;
    
    for (IntersectionIdx intersection = 0; intersection < intersectionNum; intersection++){
        IntersectionInfoVec[intersection].position = getIntersectionPosition(intersection);
        IntersectionInfoVec[intersection].name = getIntersectionName(intersection);
        
        max_lat = std::max(max_lat, IntersectionInfoVec[intersection].position.latitude());
        max_lon = std::max(max_lon, IntersectionInfoVec[intersection].position.longitude());
        min_lat = std::min(min_lat, IntersectionInfoVec[intersection].position.latitude());
        min_lon = std::min(min_lon, IntersectionInfoVec[intersection].position.longitude());
    }
}

void draw_map_blank_canvas(){
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
        
    ezgl::rectangle initial_world({x_from_lon(min_lon), y_from_lat(min_lat)}, 
                                    {x_from_lon(max_lon), y_from_lat(max_lat)});
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    application.run(nullptr, nullptr, 
                    nullptr, nullptr);
}

void draw_main_canvas(ezgl::renderer *g){
    auto startTime = std::chrono::high_resolution_clock::now();
    g->set_color(0,0,0);
    
    for (int seg_id = 0; seg_id < segmentNum; seg_id++){
        // Get LatLon information of from and to intersections from each segments
        IntersectionIdx from_id = Segment_SegmentDetailedInfo[seg_id].from;
        IntersectionIdx to_id = Segment_SegmentDetailedInfo[seg_id].to;
        float x_from = x_from_lon(IntersectionInfoVec[from_id].position.longitude());
        float y_from = y_from_lat(IntersectionInfoVec[from_id].position.latitude());
        float x_to = x_from_lon(IntersectionInfoVec[to_id].position.longitude());
        float y_to = y_from_lat(IntersectionInfoVec[to_id].position.latitude());
            
        // Temp x and y for current curve point. Starts drawing at (x_from, y_from) to first curve point.
        float curve_from_x = x_from;
        float curve_from_y = y_from;
        float curve_to_x, curve_to_y;

        // Draw street segments by drawing between curvepoints
        for (int i = 0; i < Segment_SegmentDetailedInfo[seg_id].numCurvePoints; i++){
            curve_to_x = x_from_lon(Segment_SegmentDetailedInfo[seg_id].curvePoints[i].longitude());
            curve_to_y = y_from_lat(Segment_SegmentDetailedInfo[seg_id].curvePoints[i].latitude());
            g->draw_line({curve_from_x, curve_from_y}, {curve_to_x, curve_to_y});
            curve_from_x = curve_to_x;
            curve_from_y = curve_to_y;
        }
        // Connect last curve point to (x_to, y_to)
        g->draw_line({curve_from_x, curve_from_y}, {x_to, y_to});
        
        // fixing the centre of intersection
        x_from -= 5;
        y_from -= 5;
        x_to -= 5;
        y_to -= 5;
        
                // Draw intersections corresponding to segment. Not drawing curve points. 
        float width = 10;
        float height = width;
        g->fill_rectangle({x_from, y_from}, {x_from + width, y_from + height});
        // Only draw once if from == to
        if (x_from != x_to && y_from != y_to) g->fill_rectangle({x_to, y_to}, {x_to + width, y_to + height});
    }
      
    auto currTime = std::chrono::high_resolution_clock::now();
    auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>>(currTime - startTime);
    std::cout << "draw main cavas took " << wallClock.count() << " seconds" << std::endl;
}

// converts longitude to float value
float x_from_lon(float lon){
    float min_lat1 = min_lat * kDegreeToRadian;
    float max_lat1 = max_lat * kDegreeToRadian;
    latavg = (min_lat1 + max_lat1) / 2;
    float lon1 = lon * kDegreeToRadian;
    
    float x = kEarthRadiusInMeters * lon1 * cos(latavg);
    return x;
}

// converts latitude to float value
float y_from_lat(float lat){
    float lat1 = lat * kDegreeToRadian;
    
    float y = kEarthRadiusInMeters * lat1;
    return y;
}

//float lon_from_x(float x){
//    return 0;
//}
//
//float lat_from_y(float y){
//    return 0;
//}