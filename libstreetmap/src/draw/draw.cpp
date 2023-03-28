#include "draw/draw.hpp"

/************************************************************
// Draw street segments
*************************************************************/
// Draw street segments with pixels (for far zoom levels)
void draw_street_segment_pixel (ezgl::renderer *g, StreetSegmentIdx seg_id, 
                          ezgl::point2d from_xy, ezgl::point2d to_xy, 
                          std::string street_type)
{
    // Set colors and line width according to street type
    if (street_type == "motorway" || street_type == "motorway_link")
    {
        if (!night_mode)
        {
            g->set_color(255, 212, 124);
        } else
        {
            g->set_color(58, 128, 181);
        }
    } else 
    {
        if (!night_mode)
        {
            g->set_color(ezgl::WHITE);
        } else
        {
            g->set_color(96, 96, 96);
        }
    }
    if (street_type == "path")
    {
         g->set_color(ezgl::RED);
    }
    // Set line width based on current zoom level and street type    
    int line_width = get_street_width_pixel(street_type); 
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

// Draw street segments with meters (for close zoom levels)
void draw_street_segment_meters (ezgl::renderer *g, StreetSegmentIdx seg_id, 
                                ezgl::point2d from_xy, ezgl::point2d to_xy, 
                                std::string street_type)
{
    // Set colors according to street type
    if (street_type == "motorway" || street_type == "motorway_link")
    {
        if (!night_mode)
        {
            g->set_color(255, 212, 124);
        } else
        {
            g->set_color(58, 128, 181);    
        }
    } else
    {
        if (!night_mode)
        {
            g->set_color(ezgl::WHITE);
        } else
        {
            g->set_color(96, 96, 96);
        }
    }
    if (street_type == "path")
    {
         g->set_color(ezgl::RED);
    }
    // Set street width (in meters) based on current zoom level and street type 
    int width_meters = get_street_width_meters(street_type);

    // Draw street segments including curvepoints
    ezgl::point2d curve_pt_xy; // Temp xy for current curve point.
                               // Starts drawing at from_xy to first curve point.
    
    // Connecting curvepoints. Increment from_xy.
    for (int i = 0; i < Segment_SegmentDetailedInfo[seg_id].numCurvePoints; i++)
    {
        curve_pt_xy = Segment_SegmentDetailedInfo[seg_id].curvePoints_xy[i];
        draw_line_meters(g, from_xy, curve_pt_xy, width_meters);
        from_xy = curve_pt_xy;
    }
    // Connect last curve point to (x_to, y_to)
    draw_line_meters(g, from_xy, to_xy, width_meters);
}

// Draw lines in world coordinates with fill_polygon
void draw_line_meters (ezgl::renderer *g, ezgl::point2d from_xy,
                       ezgl::point2d to_xy, int& width_meters)
{
    // Circles around intersections (or curvepoints)
    g->fill_arc(from_xy, width_meters, 0, 360);
    g->fill_arc(to_xy, width_meters, 0, 360);
    if (to_xy.y == from_xy.y)
    {   
        g->fill_rectangle({from_xy.x, from_xy.y + width_meters},
                          {to_xy.x, to_xy.y - width_meters});
        return;
    } else 
    {   
        double orthog_slope = - ((to_xy.x - from_xy.x) / (to_xy.y - from_xy.y));
        // delta_x and delta_y > 0
        double delta_x = abs(width_meters / sqrt(1 + pow(orthog_slope, 2)));
        double delta_y = abs(orthog_slope * delta_x);
        
        if (orthog_slope < 0)
        {
            ezgl::point2d point_1(from_xy.x + delta_x, from_xy.y - delta_y);
            ezgl::point2d point_2(to_xy.x + delta_x, to_xy.y - delta_y);
            ezgl::point2d point_3(to_xy.x - delta_x, to_xy.y + delta_y);
            ezgl::point2d point_4(from_xy.x - delta_x, from_xy.y + delta_y);
            // Vector of polygon points
            std::vector<ezgl::point2d> points;
            points.push_back(point_1);
            points.push_back(point_2);
            points.push_back(point_3);
            points.push_back(point_4);
            g->fill_poly(points);
            g->fill_arc(from_xy, width_meters, 0, 360);
            g->fill_arc(to_xy, width_meters, 0, 360);
        } else
        {
            ezgl::point2d point_1(from_xy.x + delta_x, from_xy.y + delta_y);
            ezgl::point2d point_2(to_xy.x + delta_x, to_xy.y + delta_y);
            ezgl::point2d point_3(to_xy.x - delta_x, to_xy.y - delta_y);
            ezgl::point2d point_4(from_xy.x - delta_x, from_xy.y - delta_y);
            // Vector of polygon points
            std::vector<ezgl::point2d> points;
            points.push_back(point_1);
            points.push_back(point_2);
            points.push_back(point_3);
            points.push_back(point_4);
            g->fill_poly(points);
            g->fill_arc(from_xy, width_meters, 0, 360);
            g->fill_arc(to_xy, width_meters, 0, 360);
        }
    }
}

// Manually fix street width with pixels according to zoom levels (far zoom levels)
int get_street_width_pixel (std::string& street_type)
{
    // If the segment is part of path
    if (street_type == "path")
    {
        return 5;
    }
    // Else, determine width based on street type and zoom levels
    double curr_world_width = visible_world.width();
    if (curr_world_width > ZOOM_LIMIT_0)
    {
        if (street_type == "motorway") 
        {
            return 4;
        } else
        {
            return 2;
        }
    } else if (ZOOM_LIMIT_1 < curr_world_width && curr_world_width < ZOOM_LIMIT_0)
    {
        if (street_type == "motorway")
        {
            return 5;
        } else if (street_type == "primary")
        {
            return 3;
        } else if (street_type == "trunk")
        {
            return 0;
        } else if (street_type == "secondary") 
        {
            return 0;
        }
    } else if (ZOOM_LIMIT_2 < curr_world_width && curr_world_width < ZOOM_LIMIT_1)
    {
        if (street_type == "motorway") 
        {
            return 5;
        } else if (street_type == "motorway_link") 
        {
            return 5;
        } else if (street_type == "primary")
        {
            return 5;
        } else if (street_type == "trunk") 
        {
            return 3;
        } else if (street_type == "secondary") 
        {
            return 2;
        } else if (street_type == "tertiary") 
        {
            return 2;
        }
    }
    return 0;
}

// Manually fix street width with meters according to zoom levels (close zoom levels)
int get_street_width_meters (std::string& street_type)
{
    if (street_type == "motorway") 
    {
        return 5;
    } else if (street_type == "motorway_link") 
    {
        return 5;
    } else if (street_type == "primary") 
    {
        return 5;
    } else if (street_type == "trunk") 
    {
        return 4;
    } else if (street_type == "secondary")
    {
        return 4;
    } else if (street_type == "tertiary") 
    {
        return 3;
    } else if (street_type == "unclassified")
    {
        return 3;
    } else if (street_type == "residential") 
    {
        return 3;
    } else if (street_type == "path")
    {
        return 5;
    } else {
        return 1;
    }
    return 0;
}

/************************************************************
// Draw street names
*************************************************************/
// Draws text on street segments
void draw_name_or_arrow (ezgl::renderer *g, std::string street_name, bool arrow,
                        ezgl::point2d from_xy, ezgl::point2d to_xy)
{
    // Calculate text rotation based on segment slope                   // TODO: Curved segments!
                                                                        // TODO: Set boundaries for street names based on street length/street width
    double angle_degree;
    if (from_xy.x == to_xy.x)
    {
        if (from_xy.y > to_xy.y && arrow)
        {
            angle_degree = 270;
        } else if (from_xy.y == to_xy.y && arrow)
        {
            return;
        } else 
        {
            angle_degree = 90;
        }
    } else
    {
        double slope = (to_xy.y - from_xy.y) / (to_xy.x - from_xy.x);
        if (slope >= 0)
        {
            angle_degree = atan2(abs(to_xy.y - from_xy.y), abs(to_xy.x - from_xy.x)) / kDegreeToRadian;   // 1,0 to 0,1
            if (arrow && from_xy.y > to_xy.y) 
            {
                angle_degree = angle_degree + 180;    // 0,1 to 1,0
            }
        } else
        {
            angle_degree = 360 - atan2(abs(to_xy.y - from_xy.y), abs(to_xy.x - from_xy.x)) / kDegreeToRadian; // 1,1 to 0,0
            if (arrow && from_xy.y < to_xy.y)
            {
                angle_degree = angle_degree - 180;    // 0,0 to 1,1
            }
        }
    }
    g->set_text_rotation(angle_degree);
    // Draw name or arrow at position between from_xy and to_xy
    ezgl::point2d mid_xy = {(from_xy.x + to_xy.x) / 2, (from_xy.y + to_xy.y) / 2};
    if (arrow)
    {      
        if(!night_mode){
            g->set_color(0, 0, 0);
            g->set_font_size(12);
        }
        else{
            g->set_color(255, 255, 255);
            g->set_font_size(12);
        }
        g->draw_text(mid_xy, "->");
    } else 
    {
        if(!night_mode){
            g->set_color(0, 0, 0);
            g->set_font_size(12);
        }
        else{
            g->set_color(255, 255, 255);
            g->set_font_size(12);
        }
        g->draw_text(mid_xy, street_name);
    }
}

/************************************************************
// Draw Features
*************************************************************/
void draw_feature_area (ezgl::renderer *g, FeatureDetailedInfo tempFeatureInfo)
{
    //Store feature information in temp variables for checking
    FeatureType tempType = tempFeatureInfo.featureType;
    std::vector<ezgl::point2d> tempPoints = tempFeatureInfo.featurePoints;
    //Draw different types of features with different colors
    if (tempType == PARK)
    {
        if (tempPoints.size() > 1)
        {         
            if(!night_mode)
            {
                g->set_color(206, 234, 214);
            } else
            {
                g->set_color(66, 75, 69);
            }
            g->fill_poly(tempPoints);
        }
    } else if (tempType == BEACH)
    {
        if (tempPoints.size() > 1)
        {
            g->set_color(255, 235, 205);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == LAKE)
    {
        if (tempPoints.size() > 1)
        {            
            if(!night_mode){
                g->set_color(153, 204, 255);
            } else
            {
                g->set_color(0, 0, 0);
            }
            g->fill_poly(tempPoints);
        }
    } else if (tempType == ISLAND)
    {
        if (tempPoints.size() > 1)
        {           
            if(!night_mode)
            {
                g->set_color(168, 218, 181);  
            } else
            {
                g->set_color(89, 110, 89);
            }
            g->fill_poly(tempPoints);
        }
    } else if (tempType == BUILDING)
    {
        if (tempPoints.size() > 1)
        {
            if(!night_mode)
            {
                g->set_color(230, 230, 230);
            } else
            {
                g->set_color(63, 81, 98);
            }
            g->fill_poly(tempPoints);
        }
    } else if (tempType == GREENSPACE)
    {
        if (tempPoints.size() > 1)
        {
            if(!night_mode)
            {
                g->set_color(206, 234, 214);
            } else
            {
                g->set_color(79, 91, 83);
            }
            g->fill_poly(tempPoints);
        }
    } else if (tempType == GOLFCOURSE)
    {
        if (tempPoints.size() > 1)
        {   
            if(!night_mode)
            {
                g->set_color(168, 218, 181);
            } else
            {
                g->set_color(58, 74, 62);  
            }
            g->fill_poly(tempPoints);
        }
    } else if (tempType == GLACIER)
    {
        if (tempPoints.size() > 1)
        {
            g->set_color(ezgl::WHITE);
            g->fill_poly(tempPoints);
        }
    } else if (tempType == RIVER)
    {
        auto tempPointIdx = tempPoints.begin();
        if (!night_mode)
        {
            g->set_color(153, 204, 255);
        } else
        {
            g->set_color(75, 97, 119);
        }
        g->set_line_width(10);
        for (int count = 0; count < (tempPoints.size() - 1); count++)
        {
            g->draw_line(*tempPointIdx, *(tempPointIdx + 1));
            tempPointIdx++;
        }
    } else if (tempType == STREAM)
    {
        auto tempPointIdx = tempPoints.begin();
        g->set_color(153, 204, 255);
        g->set_line_width(0);
        for (int count = 0; count < (tempPoints.size() - 1); count++)
        {
            g->draw_line(*tempPointIdx, *(tempPointIdx + 1));
            tempPointIdx++;
        }
    } else if (tempType == UNKNOWN)
    {
        if (tempPoints.begin() == tempPoints.end())
        {
            if (tempPoints.size() > 1)
            {
                g->set_color(230, 230, 230);
                g->fill_poly(tempPoints);
            }
        } else
        {
            auto tempPointIdx = tempPoints.begin();
            g->set_color(153, 204, 255);
            g->set_line_width(0);
            for (int count = 0; count < (tempPoints.size() - 1); count++)
            {
                g->draw_line(*tempPointIdx, *(tempPointIdx + 1));
                tempPointIdx++;
            }
        }
    }
    return;
}

/************************************************************
// Draw POIs
*************************************************************/
void draw_POIs (ezgl::renderer* g, int regionIdx)
{
    int regionSize = poi_display[regionIdx].size();
    if (!regionSize)
    {
        return;                                     // Skip if no POI is in the region
    }
    int middlePOIIdx = regionSize / 2;
    std::string tempPOIName = poi_display[regionIdx][middlePOIIdx].POIName;
    if (tempPOIName.size() > 50)
    {
        return;                                     // Skip if the POI name is too long 
    }

    // Store POI information in temp variables
    ezgl::point2d tempDrawPoint = poi_display[regionIdx][middlePOIIdx].POIPoint;
    std::string tempType = poi_display[regionIdx][middlePOIIdx].POIType;

    // Treat CURRENT_FILTER as lowercase with space as underscore -> current_filter
    std::string current_filter;
    for (auto& c : CURRENT_FILTER){
        if (c == ' ')
        {
            current_filter.push_back('_');
            continue;
        }
        current_filter.push_back(char(tolower(c))); // Save names as lowercase, no space
    }

    // Skip if something is filtered out
    if (filtered && tempType != current_filter)
    {
        return;
    }

    // Drawing the icon
    g->set_text_rotation(0); 
    g->set_color(0, 0, 0, 50);
    g->format_font("Emoji", ezgl::font_slant::normal, ezgl::font_weight::normal, 20);
    std::string icon = "\U00002B50";
    if (tempType == "fast_food")
    {
        icon = "\U0001F354";
    } else if (tempType == "bar")
    {
        icon = "\U0001F37A";
    } else if (tempType == "restaurant")
    {
        icon = "\U0001F37D";
    } else if (tempType == "cafe")
    {
        icon = "\U00002615";
    } else if (tempType == "ice_cream")
    {
        icon = "\U0001F366";
    } else if (tempType == "hospital" || tempType == "clinic" ||
               tempType == "doctor" || tempType == "dentist")
    {
        icon = "\U0001FA7A";
    } else if (tempType == "bbq")
    {
        icon = "\U0001F356";
    } else if (tempType == "post_office")
    {
        icon = "\U00002709";
    } else if (tempType == "bank")
    {
        icon = "\U0001F4B0";
    } else if (tempType == "police")
    {
        icon = "\U0001F46E";
    } else if (tempType == "school" || tempType == "university")
    {
        icon = "\U0001F393";
    } else if (tempType == "toilets")
    {
        icon = "\U0001F6BD";
    } else if (tempType == "fuel")
    {
        icon = "\U000026FD";
    }
    g->draw_text(tempDrawPoint, icon);
    
    // Drawing the POI name
    g->format_font("monospace", ezgl::font_slant::normal, ezgl::font_weight::normal, 12);
    tempDrawPoint.y = tempDrawPoint.y + 6;          // Move the POI text up  from Icons
    if (!night_mode)
    {
        g->set_color(51,102,0);
    } else
    {
        g->set_color(118,215,150);
    }
    g->draw_text(tempDrawPoint, tempPOIName);       // Draw the POI name
}

/************************************************************
// Draw Pins
*************************************************************/
// Display pins (intersection or POI if highlighted)
void draw_pin (ezgl::renderer* g, ezgl::point2d inter_xy)
{
    ezgl::surface *png_surface = g->load_png("libstreetmap/resources/red_pin.png");
    g->draw_surface(png_surface, inter_xy);
    g->free_surface(png_surface);
}

/*******************************************************************************************************************************
// Draw the distance scale
 ********************************************************************************************************************************/
void draw_distance_scale (ezgl::renderer *g, ezgl::rectangle current_window)
{
    //initialize scale variables
    unsigned scaleNameIdx = 0;
    double current_width = current_window.right() - current_window.left();
    double current_height = current_window.top() - current_window.bottom();
    const int scale_num[12] = {5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    const std::string scale_name[12] = {"5m", "10m", "20m", "50m", "100m", "200m", "500m", 
                                        "1km", "2km", "5km", "10km", "20km"};
    const std::vector<int> scaleNum (scale_num, scale_num + sizeof(scale_num) / sizeof(int));
    const std::vector<std::string> scaleName (scale_name, 
                                              scale_name + sizeof(scale_name) / sizeof(std::string));
    //find the proper scale for the current window
    auto scale = std::upper_bound (scaleNum.begin(), scaleNum.end(), int(current_width) / 20);
    if (*scale > 20000)
    {
        return;                         //protect the program from corner case
    }
    for (unsigned scaleIdx = 0; scaleIdx < scaleNum.size(); scaleIdx++)
    {
        if (scaleNum[scaleIdx] == *scale)
        {
            scaleNameIdx = scaleIdx;
            break;
        }
    }
    ezgl::point2d rightPoint;
    ezgl::point2d leftPoint;
    rightPoint.x = current_window.right() - current_width / 20;
    rightPoint.y = current_window.bottom() + current_height / 20;
    leftPoint.x = rightPoint.x - *scale;
    leftPoint.y = rightPoint.y;
    if (night_mode)
    {
        g->set_color(255,255,25);
    } else
    {
        g->set_color(0,0,0);
    }
    g->set_line_width(5);
    g->set_text_rotation(0);
    g->draw_line(leftPoint, rightPoint);
    g->draw_text({(leftPoint.x + rightPoint.x) / 2, 
                   current_window.bottom() + current_height / 25}, 
                   scaleName[scaleNameIdx]);
}
