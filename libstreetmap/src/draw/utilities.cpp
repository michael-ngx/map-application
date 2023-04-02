#include "draw/utilities.hpp"
#include "StreetsDatabaseAPI.h"
#include <math.h>
#include <cmath>


/*******************************************************************************************************************************
 * OTHER HELPER FUNCTIONS
 ********************************************************************************************************************************/
// Check if 2 rectangles collides with each other
bool check_collides (ezgl::rectangle rec_1, ezgl::rectangle rec_2)
{
    bool x_overlap = (rec_1.left() <= rec_2.right()) && (rec_2.left() <= rec_1.right());
    bool y_overlap = (rec_1.bottom() <= rec_2.top()) && (rec_2.bottom() <= rec_1.top());
    return x_overlap && y_overlap;
}
// Check if r1 fully contains r2
bool check_contains (ezgl::rectangle rec_1, ezgl::rectangle rec_2)
{
    bool x_contain = (rec_1.left() <= rec_2.left()) && (rec_2.right() <= rec_1.right());
    bool y_contain = (rec_1.bottom() <= rec_2.bottom()) && (rec_2.top() <= rec_1.top());
    return x_contain && y_contain;
}

// Move camera to new center
void move_camera (ezgl::point2d center, double new_width, ezgl::application* application)
{
    // Get aspect ratio of current world (viewable region)
    ezgl::renderer* g = application->get_renderer();
    visible_world = g->get_visible_world();
    double width = visible_world.width();
    double height = visible_world.height();
    double map_aspect_ratio = width / height;
    // Set aspect ratio of new camera
    double new_height = new_width / map_aspect_ratio;
    ezgl::rectangle new_rect({center.x - new_width / 2, center.y - new_height / 2}, new_width, new_height);
    g->set_visible_world(new_rect);

    // Redraw the main canvas
    application->refresh_drawing();
}

void view_path (ezgl::application* application, double camera_level)
{
    
    if (found_path.size() == 0)
    {
        std::string pathDirections = "Please Enter Two Valid Locations for Direction\n";
        const char* message = pathDirections.c_str();
        gtk_text_buffer_set_text(DirectionTextBuffer, message, -1);
    } else
    {
        double max_y = Segment_SegmentDetailedInfo[found_path[0]].segmentRectangle.top();
        double min_y = Segment_SegmentDetailedInfo[found_path[0]].segmentRectangle.bottom();
        double max_x = Segment_SegmentDetailedInfo[found_path[0]].segmentRectangle.right();
        double min_x = Segment_SegmentDetailedInfo[found_path[0]].segmentRectangle.left();
        for(auto i : found_path)
        {
            max_y = std::max(max_y, Segment_SegmentDetailedInfo[i].segmentRectangle.top());
            min_y = std::min(min_y, Segment_SegmentDetailedInfo[i].segmentRectangle.bottom());
            max_x = std::max(max_x, Segment_SegmentDetailedInfo[i].segmentRectangle.right());
            min_x = std::min(min_x, Segment_SegmentDetailedInfo[i].segmentRectangle.left());
        }
        double new_width = std::max((max_y - min_y), (max_x - min_x));
        ezgl::point2d center = {(min_x + max_x) / 2, (max_y + min_y) / 2};
        move_camera(center, new_width * camera_level, application);
    }
}

//Input: vector of street segment index of the optimized path. Output: a string of directions
void generate_directions ()
{
    std::string pathDirections;
    bool continueOnStreet = true;
    ezgl::point2d pointX1;
    ezgl::point2d pointX2;
    ezgl::point2d pointX3;
    ezgl::point2d pointX4;
    ezgl::point2d pointFrom;
    ezgl::point2d pointMiddle;
    ezgl::point2d pointTo;
    double deltaXSeg1;
    double deltaYSeg1;
    double deltaXSeg2;
    double deltaYSeg2;
    double deltaXSeg3;
    double deltaYSeg3;
    double direction;
    
    if (found_path.size() == 0)
    {
        pathDirections = "Please Enter Two Valid Locations for Direction\n";
    } else if (found_path.size() == 1)
    {
        pathDirections = "Your Destination is right Ahead\n";
    } else
    {
        for (StreetSegmentIdx tempIndex = 0; tempIndex < found_path.size() - 1; tempIndex++)
        {
            StreetSegmentDetailedInfo tempSegInfo = Segment_SegmentDetailedInfo[found_path[tempIndex]];
            StreetSegmentDetailedInfo nextTempSegInfo = Segment_SegmentDetailedInfo[found_path[tempIndex + 1]];
            StreetIdx tempSegIdx = tempSegInfo.streetID;
            StreetIdx nextTempSegIdx = nextTempSegInfo.streetID;
            pointX1 = Intersection_IntersectionInfo[tempSegInfo.from].position_xy;
            pointX2 = Intersection_IntersectionInfo[tempSegInfo.to].position_xy;
            pointX3 = Intersection_IntersectionInfo[nextTempSegInfo.from].position_xy;
            pointX4 = Intersection_IntersectionInfo[nextTempSegInfo.to].position_xy;
            if (pointX1 == pointX3)
            {
                pointFrom = pointX2;
                pointMiddle = pointX1;
                pointTo = pointX4;
            } else if (pointX1 == pointX4)
            {
                pointFrom = pointX2;
                pointMiddle = pointX1;
                pointTo = pointX3;
            } else if (pointX2 == pointX3)
            {
                pointFrom = pointX1;
                pointMiddle = pointX2;
                pointTo = pointX4;
            } else if (pointX2 == pointX4)
            {
                pointFrom = pointX1;
                pointMiddle = pointX2;
                pointTo = pointX3;
            }
            deltaXSeg1 = pointMiddle.x - pointFrom.x;
            deltaYSeg1 = pointMiddle.y - pointFrom.y;
            deltaXSeg2 = pointTo.x - pointFrom.x;
            deltaYSeg2 = pointTo.y - pointFrom.y;
            deltaXSeg3 = pointTo.x - pointMiddle.x;
            deltaYSeg3 = pointTo.y - pointMiddle.y;
            direction = deltaXSeg2 * deltaYSeg1 - deltaYSeg2 * deltaXSeg1;

            if (tempSegIdx == nextTempSegIdx)           //still on the same street
            {
                if (tempIndex == 0)
                {
                    pathDirections += "Get on " + getStreetName(tempSegIdx) + ".\n";
                }
                //use the law of cosine to see if the turn is a u-turn
                if (acos((pow(deltaXSeg1,2) + pow(deltaYSeg1,2) + pow(deltaXSeg3,2) + 
                     pow(deltaYSeg3,2) - pow(deltaXSeg2,2) - pow(deltaYSeg2,2)) / 
                     (2 * pow(pow(deltaXSeg1,2) + pow(deltaYSeg1,2),0.5) * 
                     pow(pow(deltaXSeg3,2) + pow(deltaYSeg3,2),0.5))) < M_PI_4)
                {
                    pathDirections += "Make a U-turn.\n";
                    continueOnStreet = true;
                } else {
                    if (continueOnStreet)
                    {
                        pathDirections += "Continue on " + getStreetName(nextTempSegIdx) + ".\n";
                        continueOnStreet = false;
                    }
                }
            } else if (tempSegIdx != nextTempSegIdx)    //change of street
            {
                continueOnStreet = true;
                if (tempIndex == 0)
                {
                    pathDirections += "Get on " + getStreetName(tempSegIdx) + ".\n";
                }
                if (direction > 0)
                {
                    if (acos((pow(deltaXSeg1,2) + pow(deltaYSeg1,2) + pow(deltaXSeg3,2) + 
                     pow(deltaYSeg3,2) - pow(deltaXSeg2,2) - pow(deltaYSeg2,2)) / 
                     (2 * pow(pow(deltaXSeg1,2) + pow(deltaYSeg1,2),0.5) * 
                     pow(pow(deltaXSeg3,2) + pow(deltaYSeg3,2),0.5))) > (M_PI_4 * 3))
                    {
                        pathDirections += "Make a slight right turn onto " + getStreetName(nextTempSegIdx) + ".\n";
                    } else
                    {
                        pathDirections += "Make a right turn onto " + getStreetName(nextTempSegIdx) + ".\n";
                    }
                } else if (direction < 0)
                {
                    if (acos((pow(deltaXSeg1,2) + pow(deltaYSeg1,2) + pow(deltaXSeg3,2) + 
                     pow(deltaYSeg3,2) - pow(deltaXSeg2,2) - pow(deltaYSeg2,2)) / 
                     (2 * pow(pow(deltaXSeg1,2) + pow(deltaYSeg1,2),0.5) * 
                     pow(pow(deltaXSeg3,2) + pow(deltaYSeg3,2),0.5))) > (M_PI_4 * 3))
                    {
                        pathDirections += "Make a slight left turn onto " + getStreetName(nextTempSegIdx) + ".\n";
                    } else
                    {
                        pathDirections += "Make a left turn onto " + getStreetName(nextTempSegIdx) + ".\n";
                    }
                } else if (direction == 0)
                {
                    pathDirections += "Continue onto " + getStreetName(nextTempSegIdx) + ".\n";
                }
            }
            if (tempIndex == found_path.size() - 2)
            {
                pathDirections += "You will see your destination ahead.\n";
            }
        }
    }

    const char* message = pathDirections.c_str();
    gtk_text_buffer_set_text(DirectionTextBuffer, message, -1);
}


// Input: original string. Output: string lowercased and space removed
std::string lower_no_space (std::string input)
{
    std::string result = "";
    if (input.empty())
    {
        return result;
    }
    for (auto& c : input){
        if (c == ' ')
        {
            continue;
        }
        result.push_back(char(tolower(c)));
    }
    return result;
}