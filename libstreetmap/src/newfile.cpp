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
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

// global structures and variables
struct IntersectionInfo{
    LatLon position;
    std::string name;
};

std::vector<IntersectionInfo> IntersectionInfoVec;
int intersectionNumber = getNumIntersections();
// helper function declaration
void draw_main_canvas (ezgl::renderer *g);
void draw_map_blank_canvas();
//void intersection_init();

/*void intersection_init(){
    IntersectionInfoVec.resize(intersectionNumber);
    for (IntersectionIdx intersection = 0; intersection < intersectionNumber; intersection++){
        
        IntersectionInfoVec[intersection].position = getIntersectionPosition(intersection);
        IntersectionInfoVec[intersection].name = getIntersectionName(intersection);
    }
}*/

void draw_map_blank_canvas(){
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    ezgl::rectangle initial_world({0,0}, {1000,1000});
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    application.run(nullptr, nullptr, 
                    nullptr, nullptr);
    
}

void draw_main_canvas(ezgl::renderer *g){
    g->draw_rectangle({0,0}, {1000,1000});
    
    /*for(size_t intersection = 0; intersection < IntersectionInfoVec.size(); intersection++){
        float x = IntersectionInfoVec[intersection].position.longitude();
        float y = IntersectionInfoVec[intersection].position.latitude();
        
        float width  = 2;
        float hight = width;
        
        g->fill_rectangle({x,y}, {x + width, y + hight});
    }*/
}


void drawMap() {
   // Set up the ezgl graphics window and hand control to it, as shown in the 
   // ezgl example program. 
   // This function will be called by both the unit tests (ece297exercise) 
   // and your main() function in main/src/main.cpp.
   // The unit tests always call loadMap() before calling this function
   // and call closeMap() after this function returns.
   draw_map_blank_canvas();
}
