#include "draw/draw.hpp"

/************************************************************
// Draw street segments
*************************************************************/
// Draw street segments with pixels (for far zoom levels)
void draw_street_segment_pixel (ezgl::renderer *g, StreetSegmentDetailedInfo& segment, bool on_path)
{
    // Set colors according to street type
    if (on_path)
    {
        if (!night_mode)
        {
            g->set_color(ezgl::DARK_SLATE_BLUE);
        } else
        {
            g->set_color(ezgl::RED);
        }
        // Set line width based on current zoom level and street type
        g->set_line_width(5);
    } else if (segment.highway_type == "motorway" || segment.highway_type == "motorway_link")
    {
        if (!night_mode)
        {
            g->set_color(255, 212, 124);
        } else
        {
            g->set_color(58, 128, 181);
        }
        // Set line width based on current zoom level and street type
        g->set_line_width(get_street_width_pixel(segment.highway_type));
    } else 
    {
        if (!night_mode)
        {
            g->set_color(ezgl::WHITE);
        } else
        {
            g->set_color(96, 96, 96);
        }
        // Set line width based on current zoom level and street type
        g->set_line_width(get_street_width_pixel(segment.highway_type));
    }

    // Round street ends
    g->set_line_cap(ezgl::line_cap(1));
    // Draw street segments including curvepoints
    ezgl::point2d from_xy = segment.from_xy;
    ezgl::point2d curve_pt_xy; // Temp xy for current curve point.
                               // Starts drawing at from_xy to first curve point.
    
    // Connecting curvepoints. Increment from_xy.
    for (int i = 0; i < segment.numCurvePoints; i++)
    {
        curve_pt_xy = segment.curvePoints_xy[i];
        g->draw_line(from_xy, curve_pt_xy);
        from_xy = curve_pt_xy;
    }
    // Connect last curve point to (x_to, y_to)
    g->draw_line(from_xy, segment.to_xy);
}

// Draw street segments with meters (for close zoom levels)
void draw_street_segment_meters (ezgl::renderer *g, StreetSegmentDetailedInfo& segment, bool on_path)
{
    // Set colors according to street type
    if (on_path)
    {
        if (!night_mode)
        {
            g->set_color(ezgl::DARK_SLATE_BLUE);
        } else
        {
            g->set_color(ezgl::RED);
        }
    } else if (segment.highway_type == "motorway" || segment.highway_type == "motorway_link")
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

    // Draw street segments including curvepoints
    ezgl::point2d from_xy = segment.from_xy;
    ezgl::point2d curve_pt_xy; // Temp xy for current curve point.
                               // Starts drawing at from_xy to first curve point.
    // Circle around "from" intersection
    g->fill_arc(from_xy, segment.width, 0, 360);
    // Drawing circles around curvepoints and polygons leading to THAT curvepoint
    for (int i = 0; i < segment.poly_points.size() - 1; i++)
    {
        g->fill_arc(segment.curvePoints_xy[i], segment.width, 0, 360);
        g->fill_poly(segment.poly_points[i]);
    }
    // Draw last segment in poly_points
    g->fill_poly(segment.poly_points[segment.poly_points.size() - 1]);
    // Circle around "to" intersection
    g->fill_arc(segment.to_xy, segment.width, 0, 360);
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
void draw_seg_name (ezgl::renderer *g, StreetSegmentDetailedInfo& segment, bool on_path)
{
    g->set_text_rotation(segment.angle_degree);
    // Draw name or arrow at position between from_xy and to_xy (temporary)
    ezgl::point2d mid_xy = {(segment.from_xy.x + segment.to_xy.x) / 2, (segment.from_xy.y + segment.to_xy.y) / 2};
    if(!night_mode)
    {
        if (on_path)
        {
            g->set_color(ezgl::WHITE);
        } else
        {
            g->set_color(0, 0, 0);
        }
    }
    else
    {
        if (on_path)
        {
            g->set_color(ezgl::YELLOW);
        } else
        {
            g->set_color(255, 255, 255);
        }
    }
    g->set_font_size(10);
    g->draw_text(mid_xy, segment.streetName_arrow, segment.length * 0.5, segment.width * 1.8);
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
                g->set_color(153, 212, 150);
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
        g->set_line_cap(ezgl::line_cap(1));
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
        g->set_line_cap(ezgl::line_cap(1));
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
            g->set_line_cap(ezgl::line_cap(1));
            for (int count = 0; count < (tempPoints.size() - 1); count++)
            {
                g->draw_line(*tempPointIdx, *(tempPointIdx + 1));
                tempPointIdx++;
            }
        }
    }
    return;
}

/********************************************************************************
* Draw Subway Lines
********************************************************************************/
void draw_subway_lines (ezgl::renderer* g)
{
    g->set_line_width(4);
    g->set_line_cap(ezgl::line_cap(1));
    for (int route = 0; route < AllSubwayRoutes.size(); route++)
    {
        // Display subway route
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
}

/************************************************************
* Draw POIs
*************************************************************/
void draw_POIs (ezgl::renderer* g, POIDetailedInfo POI)
{
    // Store information of current POI
    std::string tempPOIName = POI.POIName;
    // Skip if the POI name is too long 
    if (tempPOIName.size() > 50)
    {
        return;                                     
    }
    ezgl::point2d tempDrawPoint = POI.POIPoint;
    std::string tempType = POI.POIType;
    

    // Treat CURRENT_FILTER as lowercase with space as underscore -> current_filter
    // This is because POIType is stored as lowercase and space as underscore
    std::string current_filter;
    for (auto& c : CURRENT_FILTER){
        if (c == ' ')
        {
            current_filter.push_back('_');
            continue;
        }
        current_filter.push_back(char(tolower(c))); // Save names as lowercase, no space
    }

    // Skip if current POI is filtered out
    if (filtered && tempType != current_filter)
    {
        return;
    }

    // Drawing the icon
    g->set_text_rotation(0); 
    g->set_color(0, 0, 0);
    g->format_font("Emoji", ezgl::font_slant::normal, ezgl::font_weight::normal, 25);
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
    tempDrawPoint.y = tempDrawPoint.y + 8;          // Move the POI text up  from Icons
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
// Display pins
void draw_png (ezgl::renderer* g, ezgl::point2d inter_xy, std::string pin_type)
{
    std::string to_be_converted = "libstreetmap/resources/" + pin_type + ".png";
    ezgl::surface *png_surface = g->load_png(to_be_converted.c_str());
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
