#include "grid.h"
#include "draw/draw.hpp"

/********************************************************************************
* Draw features. Features are sorted by descending area
********************************************************************************/
void Grid::draw_grid_features (ezgl::renderer* g, double limit)
{
    // Displaying features with area > area limit
    for (auto feature : this->Grid_Features)
    {
        if (feature.featureArea > limit
            && !check_feature_drawn[feature.id])
        {
            check_feature_drawn[feature.id] = true;
            draw_feature_area(g, feature);
        } else if (feature.featureArea <= limit)
        {
            return;
        }
    }
}

/********************************************************************************
* Draw street segments
********************************************************************************/
void Grid::draw_grid_segments (ezgl::renderer* g)
{
    for (auto segment : this->Grid_Segments_Non_Motorway)
    {
        // Skip segment if segment is part of found_path (will be drawn later)
        // Skip segment if it's already drawn (by other grids)
        if (std::find(found_path.begin(), found_path.end(), segment.id) != found_path.end()
            || check_segment_drawn[segment.id])
        {
            continue;
        }
        check_segment_drawn[segment.id] = true;

        // Draws different amount of data based on different zoom levels
        if (curr_world_width >= ZOOM_LIMIT_0)
        {
            if (segment.highway_type == "primary")
            {
                draw_street_segment_pixel(g, segment);
            }
        } else if (ZOOM_LIMIT_1 <= curr_world_width && curr_world_width < ZOOM_LIMIT_0)
        {
            if (segment.highway_type == "primary" || segment.highway_type == "trunk" || segment.highway_type == "secondary")
            {   
                draw_street_segment_pixel(g, segment);
            }
        } else if (ZOOM_LIMIT_2 <= curr_world_width && curr_world_width < ZOOM_LIMIT_1)
        {
            if (segment.highway_type == "primary" || segment.highway_type == "trunk" 
                || segment.highway_type == "secondary" || segment.highway_type == "tertiary")
            {
                draw_street_segment_pixel(g, segment);
            }
        } else
        {
            // Only display all types of street (except for highway for later) when < ZOOM_LIMIT_3
            if (ZOOM_LIMIT_3 <= curr_world_width && curr_world_width < ZOOM_LIMIT_2)
            {
                if (segment.highway_type == "primary" || segment.highway_type == "trunk"
                    || segment.highway_type == "secondary" || segment.highway_type == "tertiary" 
                    || segment.highway_type == "unclassified" || segment.highway_type == "residential")
                {
                    draw_street_segment_meters(g, segment);
                }
            } else if (curr_world_width < ZOOM_LIMIT_3 && segment.highway_type != "motorway" && segment.highway_type != "motorway_link")
            {
                draw_street_segment_meters(g, segment);
            }
        }
    }

    // Draw motorway and motorway-link (highways) above other streets
    for (auto segment : this->Grid_Segments_Motorway)
    {
        // Skip segment if segment is part of found_path (will be drawn later)
        // Skip segment if it's already drawn (by other grids)
        if (std::find(found_path.begin(), found_path.end(), segment.id) != found_path.end()
            || check_segment_drawn[segment.id])
        {
            continue;
        }
        check_segment_drawn[segment.id] = true;

        if (curr_world_width >= ZOOM_LIMIT_2 && segment.highway_type == "motorway")
        {
            draw_street_segment_pixel(g, segment);
        } else if (curr_world_width < ZOOM_LIMIT_2)
        {
            draw_street_segment_meters(g, segment);
        }
    }
}

/********************************************************************************
* Draw street names and arrows
********************************************************************************/
// Draw street names in visible regions if region is available
void Grid::draw_grid_names (ezgl::renderer *g)
{
    for (auto segment : this->Grid_Segments_Names)
    {
        // Skip segment if it's already drawn (by other grids)
        if (check_name_drawn[segment.id])
        {
            continue;
        }
        check_name_drawn[segment.id] = true;
        if (std::find(found_path.begin(), found_path.end(), segment.id) != found_path.end())
        {
            draw_seg_name(g, segment, true);
        } else
        {
            draw_seg_name(g, segment);
        }
    }
}

/********************************************************************************
* Draw POIs and Icons
********************************************************************************/
void Grid::draw_grid_POIs(ezgl::renderer* g)
{
    // Track number of POIs drawn within the current grid
    int count = 0;
    // "Step" for skipping POI by id. This is based on the fact that close POIs tend to have close POIIdx
    int step = POI_STEP;
    for (auto POI : this->Grid_POIs)
    {
        if (POI.id % step == 0 && count <= MAX_GRID_POI && visible_world.contains(POI.POIPoint))
        {
            draw_POIs(g, POI);
            count++;
        }
    }
}

/********************************************************************************
* Draw Subway Stations
********************************************************************************/
void Grid::draw_grid_subway_stations (ezgl::renderer *g)
{
    for (auto station : this->Grid_Subway_Stations)
    {
        ezgl::point2d point = station.position_xy;
        if (!visible_world.contains(point))
        {
            continue;
        }
        draw_png(g, point, "subway_station");
        point.y += 10;
        g->set_color(ezgl::RED);
        g->format_font("monospace", ezgl::font_slant::normal, ezgl::font_weight::normal, 12);
        g->draw_text(point, station.name);
    }
}
