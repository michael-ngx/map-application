/*************************************************************
 * APPLICATION EVENT SETUP
 *************************************************************/

#ifndef SETUP_H
#define SETUP_H

#include "ui_callbacks/widgets.hpp"

// Initial Setup is run whenever a window is opened. 
void initial_setup (ezgl::application *application, bool new_window);
// Register mouse click
void act_on_mouse_click (ezgl::application *application, GdkEventButton */*event*/, double x, double y);

#endif /* SETUP_H */ 