#include "ui_callbacks/custom_callback.hpp"


/*******************************************************************************************************************************
 * UI CALLBACKS
 ********************************************************************************************************************************/

void city_change_cbk (GtkComboBoxText* self, ezgl::application* application)
{
    //Getting current text content
    auto text = gtk_combo_box_text_get_active_text(self);
    std::string text_string = text;
    free(text);

    // Get new math path based on input
    std::string new_map_path = get_new_map_path(text_string);
    
    if (text_string.empty() || text_string == " " || new_map_path == "None")
    {   // Returning if the combo box is currently empty
        // Or if the input map is unexpected
        // Or Always check to avoid errors
        return;
    } else if (new_map_path != CURRENT_MAP_PATH)
    {
        CURRENT_MAP_PATH = new_map_path;

        // Closes current map and loads the new city
        pin_display.clear();
        closeMap();
        loadMap(new_map_path);

        // Clear subway switches if new city doesn't have subways
        if (AllSubwayRoutes.size() == 0)
        {
            gtk_switch_set_active(subway_station_switch, false);
            gtk_switch_set_state(subway_station_switch, false);
            gtk_switch_set_active(subway_line_switch, false);
            gtk_switch_set_state(subway_line_switch, false);
        }
        // Clear navigation switch
        gtk_switch_set_active(navigation_switch, false);
        gtk_switch_set_state(navigation_switch, false);

        // Clear GtkListStores of old city. Load GtkListStore for new city
        gtk_list_store_clear(list_store);
        for (auto& pair : IntersectionName_IntersectionIdx_no_repeat)
        {
            if (pair.first.find('&') == std::string::npos)
            {
                continue;   // Do not input intersections that isn't intersection of at least 2 streets
            }
            gtk_list_store_append(list_store, &iter);
            gtk_list_store_set(list_store, &iter, 0, (pair.first).c_str(), -1);
        }
        // Clear the current text in GtkSearchEntry
        gtk_entry_set_text(GTK_ENTRY(SearchBar), "");
        gtk_entry_set_text(GTK_ENTRY(SearchBarDestination), "");
        
        // Reset the world based on new map
        ezgl::rectangle new_world(xy_from_latlon(latlon_bound.min),
                                    xy_from_latlon(latlon_bound.max));
       
        application->change_canvas_world_coordinates("MainCanvas", new_world);
        application->refresh_drawing();

        // Announce to user
        application->update_message("Loaded new map!");
    }
}

// Callback function for selecting filter type
void poi_filter_cbk (GtkComboBoxText* self, ezgl::application* application)
{
    // Get current text from list
    auto text = gtk_combo_box_text_get_active_text(self);
    std::string text_string = text;
    free(text);
    
    if(!text || text_string == "All")
    {  // Returning if the combo box is currently empty/Turning off filter
        filtered = false;
        application->refresh_drawing();
        return;
    } else if (text_string != "All")
    {
        filtered = true;
        CURRENT_FILTER = text_string;       
        application->refresh_drawing();
    }
}

// Callback function for switching ON/OFF night mode
void night_mode_cbk (GtkSwitch* /*self*/, gboolean state, ezgl::application* application){
    if(state)
    {
        application->update_message("Night mode turned on");
        night_mode = true;
        application->refresh_drawing();
    } else
    {
        application->update_message("Night mode turned off");
        night_mode = false;
        application->refresh_drawing();
    }   
}

// Callback function for switching ON/OFF subway station and subway line mode
void subway_station_cbk (GtkSwitch* self, gboolean state, ezgl::application* application)
{
    if(state)
    {
        if (AllSubwayRoutes.size() == 0)
        {
            application->create_popup_message("Error","City has no subway!");
            gtk_switch_set_active(self, false);
            gtk_switch_set_state(self, false);
            return;
        } else
        {
            application->update_message("Displaying Subway Stations");
            subway_station_mode = true;
            pin_display.clear();
            application->refresh_drawing();
        }
    } else
    {
        application->update_message("Hid Subway Stations");
        subway_station_mode = false;
        application->refresh_drawing();
    }
}

void subway_line_cbk (GtkSwitch* self, gboolean state, ezgl::application* application)
{
    if(state)
    {
        if (AllSubwayRoutes.size() == 0)
        {
            application->create_popup_message("Error","City has no subway!");
            gtk_switch_set_active(self, false);
            gtk_switch_set_state(self, false);
            return;
        } else
        {
            application->update_message("Displaying Subway Lines");
            subway_line_mode = true;
            application->refresh_drawing();
        }
    } else
    {
        application->update_message("Hid Subway Lines");
        subway_line_mode = false;
        application->refresh_drawing();
    }   
}

// Callback function for navigation mode switch
void navigation_switch_cbk (GtkSwitch* /*self*/, gboolean state, ezgl::application* application)
{
    if(state)
    {
        application->update_message("Navigation mode turned on");
        navigation_mode = true;
        // Unhide second search bar
        gtk_widget_show(GTK_WIDGET(SearchBarDestination));
        // Change placeholder of first search bar
        gtk_entry_set_placeholder_text(GTK_ENTRY(SearchBar), "Choose starting point, or click on the map");
        // Grab the focus to destination search bar if the content of the first search bar is not empty
        const gchar *search_text;
        search_text = gtk_entry_get_text(GTK_ENTRY(SearchBar));
        std::string input_1(search_text);
        if (!input_1.empty())
        {
            gtk_widget_grab_focus(GTK_WIDGET(SearchBarDestination));
        }
        application->refresh_drawing();
    } else
    {
        application->update_message("Navigation mode turned off");
        navigation_mode = false;
        // Hide second search bar
        gtk_widget_hide(GTK_WIDGET(SearchBarDestination));
        // Change placeholder of first search bar
        gtk_entry_set_placeholder_text(GTK_ENTRY(SearchBar), "Search Intersections");
        // Clear the currently selected pins
        pin_display.clear();
        application->refresh_drawing();
    }
}

// Callback function for Search bars (shared by both starting point and destination bars)
void search_activate_cbk (GtkSearchEntry */*self*/, ezgl::application *application)
{
    const gchar *search_text;
    // Get the text from the first search entry
    search_text = gtk_entry_get_text(GTK_ENTRY(SearchBar));
    std::string input_1(search_text);

    // Turn off subway station mode when searching/navigating something
    gtk_switch_set_active(subway_station_switch, false);
    gtk_switch_set_state(subway_station_switch, false);

    // Determine how UI responses based on inputs and whether navigation mode is ON
    if (navigation_mode)
    {
        // Get the text from the second search entry
        search_text = gtk_entry_get_text(GTK_ENTRY(SearchBarDestination));
        std::string input_2(search_text);
        navigation_response(input_1, input_2, application);
    } else
    {
        search_response(input_1, application);
    }
}

/*******************************************************************************************************************************
 * UI CALLBACK HELPER FUNCTIONS
 ********************************************************************************************************************************/
/************************************************************
// Get map path for reloading from string of city name
*************************************************************/
std::string get_new_map_path (std::string text_string)
{
    std::string new_map_path;
    if (text_string == "Toronto")
    {
        new_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    } else if (text_string == "Beijing")
    {
        new_map_path = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
    } else if (text_string == "Cairo") 
    {
        new_map_path = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
    } else if (text_string == "Cape Town")
    {
        new_map_path = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
    } else if (text_string == "Golden Horseshoe")
    {
        new_map_path = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
    } else if (text_string == "Hamilton")
    {
        new_map_path = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
    } else if (text_string == "Hong Kong")
    {
        new_map_path = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
    } else if (text_string == "Iceland")
    {
        new_map_path = "/cad2/ece297s/public/maps/iceland.streets.bin";
    } else if (text_string == "Interlaken")
    {
        new_map_path = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
    } else if (text_string == "Kyiv") 
    {
        new_map_path = "/cad2/ece297s/public/maps/kyiv_ukraine.streets.bin";
    } else if (text_string == "London")
    {
        new_map_path = "/cad2/ece297s/public/maps/london_england.streets.bin";
    } else if (text_string == "New Delhi") 
    {
        new_map_path = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
    } else if (text_string == "New York")
    {
        new_map_path = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
    } else if (text_string == "Rio de Janeiro") 
    {
        new_map_path = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
    } else if (text_string == "Saint Helena") 
    {
        new_map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
    } else if (text_string == "Singapore") 
    {
        new_map_path = "/cad2/ece297s/public/maps/singapore.streets.bin";
    } else if (text_string == "Sydney") 
    {
        new_map_path = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
    } else if (text_string == "Tehran") 
    {
        new_map_path = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
    } else if (text_string == "Tokyo")
    {
        new_map_path = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
    } else
    {
        new_map_path = "None";
    }
    return new_map_path;
}

// Algorithm for fuzzy searching
gboolean fuzzy_match_func (GtkEntryCompletion */*completion*/, const gchar *user_input, GtkTreeIter *iterr, gpointer /*user_data*/)
{
    GtkTreeModel *model;
    gchar *user_input_lower, *data_text, *data_text_lower;
    gboolean result = FALSE;
    
    model = gtk_entry_completion_get_model(completion);
    gtk_tree_model_get(model, iterr, 0, &data_text, -1);

    if (data_text == NULL || user_input == NULL)
    {
        return result;
    }
    // Convert both data_text and user_input to lowercase
    data_text_lower = g_utf8_strdown(data_text, -1);
    user_input_lower = g_utf8_strdown(user_input, -1);

    // Tokenize data_text_lower and user_input_lower using space as delimiter (Split into words)
    gchar **data_tokens = g_strsplit(data_text_lower, " ", -1);
    gchar **user_input_tokens = g_strsplit(user_input_lower, " ", -1);

    // Iterate through each user_input token and check if it's a substring of any entry token
    for (int i = 0; user_input_tokens[i] != NULL; i++)
    {
        gboolean token_matched = FALSE;
        for (int j = 0; data_tokens[j] != NULL; j++)
        {
            if (strstr(data_tokens[j], user_input_tokens[i]))
            {
                token_matched = TRUE;
                break;
            }
        }
        if (!token_matched)
        {
            // If at least one user_input token didn't match any entry token, return FALSE
            result = FALSE;
            goto exit;
        }
    }

    // If all user_input tokens matched at least one entry token, return TRUE
    result = TRUE;

exit:
    g_free(user_input_lower);
    g_strfreev(user_input_tokens);
    g_free(data_text);
    g_free(data_text_lower);
    g_strfreev(data_tokens);
    return result;
}