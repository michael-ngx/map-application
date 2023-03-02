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
    g->set_color(0,0,0);
    
    for(size_t i = 0; i < IntersectionInfoVec.size(); i++){
          float x = x_from_lon(IntersectionInfoVec[i].position.longitude());
          float y = y_from_lat(IntersectionInfoVec[i].position.latitude());
          float width = 100;
          float height = width;
        
          g->fill_rectangle({x,y}, {x + width, y + height});
    }
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