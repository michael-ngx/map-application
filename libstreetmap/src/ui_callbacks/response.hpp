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
bool search_response (std::string input, ezgl::application *application);
bool navigation_response (std::string input, bool start_search_bar, ezgl::application *application);

#endif /* RESPONSE_H */