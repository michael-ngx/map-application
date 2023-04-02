#include "grid.h"
#include "draw/draw.hpp"

/********************************************************************************
* Draw features. Features are sorted by descending area
********************************************************************************/
void Grid::draw_grid_features(ezgl::renderer* g, float factor)
{
    // Display limit, based on feature zoom factor
    int limit = this->Grid_Features.size() * factor;
    // Displaying features
    for (int i = 0; i < limit; i++)
    {
        if (!check_feature_drawn[this->Grid_Features[i].id])
        {
            check_feature_drawn[this->Grid_Features[i].id] = true;
            draw_feature_area(g, this->Grid_Features[i]);
        }
    }
}

/********************************************************************************
* Draw street segments
********************************************************************************/
void Grid::draw_grid_segments(ezgl::renderer* g)
{
    for (auto segment : this->Grid_Non_Motorway_Segments)
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
                draw_street_segment_pixel(g, segment.id, segment.from_xy, segment.to_xy, segment.highway_type);
            }
        } else if (ZOOM_LIMIT_1 <= curr_world_width && curr_world_width < ZOOM_LIMIT_0)
        {
            if (segment.highway_type == "primary" || segment.highway_type == "trunk" || segment.highway_type == "secondary")
            {   
                draw_street_segment_pixel(g, segment.id, segment.from_xy, segment.to_xy, segment.highway_type);
            }
        } else if (ZOOM_LIMIT_2 <= curr_world_width && curr_world_width < ZOOM_LIMIT_1)
        {
            if (segment.highway_type == "primary" || segment.highway_type == "trunk" 
                || segment.highway_type == "secondary" || segment.highway_type == "tertiary")
            {
                draw_street_segment_pixel(g, segment.id, segment.from_xy, segment.to_xy, segment.highway_type);
            }
        } else // Displaying street names and arrows start here
        {
            // Only display all types of street (except for highway for later) when < ZOOM_LIMIT_3
            if (ZOOM_LIMIT_3 <= curr_world_width && curr_world_width < ZOOM_LIMIT_2)
            {
                if (segment.highway_type == "primary" || segment.highway_type == "trunk"
                    || segment.highway_type == "secondary" || segment.highway_type == "tertiary" 
                    || segment.highway_type == "unclassified" || segment.highway_type == "residential")
                {
                    draw_street_segment_meters(g, segment.id, segment.from_xy, segment.to_xy, segment.highway_type);
                }
            } else if (curr_world_width < ZOOM_LIMIT_3 && segment.highway_type != "motorway" && segment.highway_type != "motorway_link")
            {
                draw_street_segment_meters(g, segment.id, segment.from_xy, segment.to_xy, segment.highway_type);
            }
        }
    }

    // Draw motorway and motorway-link (highways) above other streets
    for (auto segment : this->Grid_Motorway_Segments)
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
            draw_street_segment_pixel(g, segment.id, segment.from_xy, segment.to_xy, segment.highway_type);
        } else if (curr_world_width < ZOOM_LIMIT_2)
        {
            draw_street_segment_meters(g, segment.id, segment.from_xy, segment.to_xy, segment.highway_type);
        }
    }
}

/********************************************************************************
* Draw street names and arrows
********************************************************************************/
// Draw street names in visible regions if region is available
void Grid::draw_grid_names_or_arrows (ezgl::renderer *g)
{
    for (auto segment : this->Grid_Names_And_Arrows_Segments)
    {
        if ((curr_world_width >= ZOOM_LIMIT_3 && (segment.highway_type == "primary" || segment.highway_type == "secondary"))
            || curr_world_width < ZOOM_LIMIT_3)
        {
            draw_name_and_arrow(g, segment.streetName, segment.oneWay, 
                                segment.from_xy, segment.to_xy, false);
        }
    }
}

/********************************************************************************
* Draw POIs and Icons
********************************************************************************/
void Grid::draw_grid_POIs(ezgl::renderer* g)
{
    for (auto POI : this->Grid_POIs)
    {
        draw_POIs(g, POI);
    }
}