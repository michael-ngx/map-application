/*************************************************************
 * UI CALLBACK FUNCTIONS
 * 
 * These are callback functions for the UI elements
 *************************************************************/

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "UI callbacks/input_response.hpp"

void input_streets_cbk(GtkWidget *widget, ezgl::application* application);
void search_activate_cbk(GtkSearchEntry *self, ezgl::application* application);
void night_mode_cbk(GtkSwitch* self, gboolean state, ezgl::application* application);
void subway_station_cbk(GtkSwitch* self, gboolean state, ezgl::application* application);
void subway_line_cbk(GtkSwitch* self, gboolean state, ezgl::application* application);
void navigation_switch_cbk(GtkSwitch* self, gboolean state, ezgl::application* application);
void poi_filter_cbk(GtkComboBoxText* self, ezgl::application* application);
void city_change_cbk(GtkComboBoxText* self, ezgl::application* application);

std::string get_new_map_path (std::string text_string);
gboolean fuzzy_match_func(GtkEntryCompletion */*completion*/, const gchar *user_input, GtkTreeIter *iterr, gpointer /*user_data*/);

#endif /* CALLBACKS_H */ 