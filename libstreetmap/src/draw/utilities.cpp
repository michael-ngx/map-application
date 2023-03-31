#include "draw/utilities.hpp"

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

void view_path (ezgl::application* application)
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
    move_camera(center, new_width * 2, application);
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