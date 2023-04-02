/*************************************************************
 * UI CALLBACK FUNCTIONS
 * 
 * These are callback functions for the UI elements
 *************************************************************/

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "ui_callbacks/response.hpp"

// Search bars
void search_activate_cbk_start (GtkSearchEntry */*self*/, ezgl::application *application);
void search_activate_cbk_dest (GtkSearchEntry */*self*/, ezgl::application* application);
void search_changed_cbk_start (GtkSearchEntry */*self*/, ezgl::application *application);
void search_changed_cbk_dest (GtkSearchEntry */*self*/, ezgl::application* application);
// Drop-down lists
void poi_filter_cbk (GtkComboBoxText* self, ezgl::application* application);
void city_change_cbk (GtkComboBoxText* self, ezgl::application* application);
// Buttons
void subway_cbk (GtkButton* /*self*/, ezgl::application* application);
void subway_off_cbk (GtkButton* /*self*/, ezgl::application* application);
void tutorial_cbk (GtkButton* /*self*/, ezgl::application* application);
void direction_cbk (GtkButton* /*self*/);
void navigation_cbk (GtkButton* /*self*/, ezgl::application* application);
void end_navigation_cbk (GtkButton* /*self*/, ezgl::application* application);
void night_mode_cbk (GtkButton* /*self*/, ezgl::application* application);
void day_mode_cbk (GtkButton* /*self*/, ezgl::application* application);

// Callback helper functions
std::string get_new_map_path (std::string text_string);
gboolean fuzzy_match_func (GtkEntryCompletion */*completion*/, const gchar *user_input,
                           GtkTreeIter *iterr, gpointer /*user_data*/);

#endif /* CALLBACKS_H */ 