/*************************************************************
 * 
 * UTILITIES HELPER FUNCTIONS
 * 
 *************************************************************/

#ifndef UTILITIES_H
#define UTILITIES_H

#include "ezgl/application.hpp"
#include "StreetsDatabaseAPI.h"
#include "globals.h"

void move_camera (ezgl::point2d center, ezgl::application* application);
bool check_collides (ezgl::rectangle rec_1, ezgl::rectangle rec_2);
bool check_contains (ezgl::rectangle rec_1, ezgl::rectangle rec_2);
std::string lower_no_space (std::string input);

#endif /* UTILITIES_H */