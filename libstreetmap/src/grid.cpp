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
            draw_feature_area(g, this->Grid_Features[i]);
            check_feature_drawn[this->Grid_Features[i].id] = true;
        }
    }
}


void Grid::draw_grid_segments(ezgl::renderer* g)
{
    
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