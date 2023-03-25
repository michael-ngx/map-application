/* 
 *
 * HANDLING UI RESPONSES TO USER TEXT INPUT
 * 
 */

#ifndef RESPONSE_H
#define RESPONSE_H

#include "ezgl/application.hpp"
#include "m1.h"
#include "globals.h"
#include "draw/utilities.hpp"

// Response to user input for searching/navigation
void search_response (std::string input, ezgl::application *application);
void navigation_response (std::string input_1, std::string input_2, ezgl::application *application);

#endif /* RESPONSE_H */