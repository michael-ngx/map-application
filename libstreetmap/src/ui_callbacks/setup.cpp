#include "ui_callbacks/setup.hpp"
#include "draw/utilities.hpp"
/*******************************************************************************************************************************
 * INITIAL SETUP
 * 
 * Function called before the activation of the application
 ********************************************************************************************************************************/
void initial_setup (ezgl::application *application, bool /*new_window*/)
{
    // Update the status bar message
    application->update_message("Welcome!");

    // Connects to Subway button
    SubwayButton = application->get_object("SubwayButton");
    g_signal_connect(
        SubwayButton,
        "clicked",
        G_CALLBACK(subway_cbk),
        application
    );

    // Connects to Subway Off button
    SubwayOffButton = application->get_object("SubwayOffButton");
    g_signal_connect(
        SubwayOffButton,
        "clicked",
        G_CALLBACK(subway_off_cbk),
        application
    );
    // Default: Hides Subway Off button
    gtk_widget_hide(GTK_WIDGET(SubwayOffButton));

    // Connects to Navigation button
    NavigationButton = application->get_object("NavigationButton");
    g_signal_connect(
        NavigationButton,
        "clicked",
        G_CALLBACK(navigation_cbk),
        application
    );

    // Connects to EndNavigation button
    EndNavigationButton = application->get_object("EndNavigationButton");
    g_signal_connect(
        EndNavigationButton,
        "clicked",
        G_CALLBACK(end_navigation_cbk),
        application
    );
    // Default: Hides EndNavigation button
    gtk_widget_hide(GTK_WIDGET(EndNavigationButton));

    // Connects to Tutorial button
    TutorialButton = application->get_object("Tutorial");
    g_signal_connect(
        TutorialButton,
        "clicked",
        G_CALLBACK(tutorial_cbk),
        application
    );
    // Default: Hides Tutorial button
    gtk_widget_hide(GTK_WIDGET(TutorialButton));
    
     // Connects to Direction button
    DirectionButton = application->get_object("Direction");
    g_signal_connect(
        DirectionButton,
        "clicked",
        G_CALLBACK(direction_cbk),
        application
    );
    // Default: Hides Direction button
    gtk_widget_hide(GTK_WIDGET(DirectionButton));
    
    // Default: Hides Direction Display
    DirectionWindow = application->get_object("DirectionWindow");
    DirectionDisplay = application->get_object("DirectionTextDisplay");
    gtk_widget_set_size_request(GTK_WIDGET(DirectionWindow), 325, 600);
    gtk_widget_set_size_request(GTK_WIDGET(DirectionDisplay), 325, 600);
    DirectionTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(DirectionDisplay));
    gtk_widget_hide(GTK_WIDGET(DirectionDisplay));
    gtk_widget_hide(GTK_WIDGET(DirectionWindow));
    
    // Connects to Night Mode button
    NightModeButton = application->get_object("NightModeButton");
    g_signal_connect(
        NightModeButton,
        "clicked",
        G_CALLBACK(night_mode_cbk),
        application
    );

    // Connects to Day Mode button
    DayModeButton = application->get_object("DayModeButton");
    g_signal_connect(
        DayModeButton,
        "clicked",
        G_CALLBACK(day_mode_cbk),
        application
    );
    // Default: Hides Day Mode button
    gtk_widget_hide(GTK_WIDGET(DayModeButton));

    // Connects to Filters list
    FilterComboBox = application->get_object("FilterComboBox");
    g_signal_connect(
        FilterComboBox,
        "changed",
        G_CALLBACK(poi_filter_cbk),
        application
    );
    // Connects to City change List
    CityChangeComboBox = application->get_object("CityChangeComboBox");
    g_signal_connect(
        CityChangeComboBox,
        "changed",
        G_CALLBACK(city_change_cbk),
        application
    );
    
    /***********************************************
     * Sets up entry completion for search bar
     ************************************************/
    // Connects to first SearchBar
    SearchBar = application->get_object("SearchBar");
    g_signal_connect(
        SearchBar, // pointer to the UI widget
        "activate", // signal representing "Enter" has been pressed or user clicked search icon
        G_CALLBACK(search_activate_cbk_start),
        application // passing an application pointer to callback function
    );
    // When the text is modified by the user --> Change the "Set" to false, unless forced change by autocomplete
    g_signal_connect(SearchBar, "changed", G_CALLBACK(search_changed_cbk_start), application);
    
    // Connects to the second search bar
    SearchBarDestination = application->get_object("SearchBarDestination");
    g_signal_connect(
        SearchBarDestination, // pointer to the UI widget
        "activate", // signal representing "Enter" has been pressed or user clicked search icon
        G_CALLBACK(search_activate_cbk_dest),
        application // passing an application pointer to callback function
    );
    // When the text is modified by the user --> Change the "Set" to false, unless forced change by autocomplete
    g_signal_connect(SearchBarDestination, "changed", G_CALLBACK(search_changed_cbk_dest), application);
    // Default: Hides second search bar
    gtk_widget_hide(GTK_WIDGET(SearchBarDestination));

    // Connect to FullSearchList
    list_store = GTK_LIST_STORE(application->get_object("FullSearchList"));
    // Load all intersection names
    // Intersection names may or may not contain '&'
    for (auto& pair : IntersectionName_IntersectionIdx_no_repeat)
    {
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, (pair.first).c_str(), -1);
    }
    // Load all food place names - Temporarily disabled for M3
    // for (auto& pair : POI_AllFood)
    // {
    //     gtk_list_store_append(list_store, &iter);
    //     gtk_list_store_set(list_store, &iter, 0, (pair.first).c_str(), -1);
    // }

    // Change entry completion algorithm to support fuzzy search        // TODO: Prioritize closer match
    completion = GTK_ENTRY_COMPLETION(application->get_object("FullEntryCompletion"));
    completion_destination = GTK_ENTRY_COMPLETION(application->get_object("FullEntryCompletionDestination"));
    gtk_entry_completion_set_match_func(completion, fuzzy_match_func, NULL, NULL);
    gtk_entry_completion_set_match_func(completion_destination, fuzzy_match_func, NULL, NULL);
}

                                            
/*******************************************************************************************************************************
 * ACT ON MOUSE CLICK
 ********************************************************************************************************************************/
void act_on_mouse_click (ezgl::application* application, GdkEventButton* /*event*/, double x, double y)
{
    // If not in navigation mode --> Exploration mode --> Allow highlighting only one intersection/POI at a time
    if (!navigation_mode)
    {
        // Check whether the mouse clicked closer to a POI or an intersection
        IntersectionIdx inter_id = findClosestIntersection(ezgl::point2d(x, y));
        // POIIdx POI_id = findClosestPOI(ezgl::point2d(x, y)); - temporarily disablied for M3
        
        // User selected intersection
        // if (clicked_intersection_distance <= clicked_POI_distance)
        // {
            auto inter_it = std::find(pin_display_start.begin(), pin_display_start.end(), Intersection_IntersectionInfo[inter_id].position_xy);
            // Highlight closest intersections by adding point to pin_display_start
            if (inter_it == pin_display_start.end())
            {
                // Clear all pins if newly select intersection. 
                pin_display_start.clear();
                // Set start point to selected position
                start_point = Intersection_IntersectionInfo[inter_id].position_xy;
                start_point_id = inter_id;
                start_point_set = true;
                // Highlight only selected intersection
                pin_display_start.push_back(start_point);
                // Change the text in the SearchBar to intersection selected
                // This should not change the start_point_set to false
                search_1_forced_change = true;
                gtk_entry_set_text(GTK_ENTRY(SearchBar), Intersection_IntersectionInfo[inter_id].name.c_str());
            } else  
            {
                // Unhighlight intersection by removing from pin_display_start
                pin_display_start.erase(inter_it);
                // Unset the starting point and clear search bar
                start_point_set = false;
                gtk_entry_set_text(GTK_ENTRY(SearchBar), "");
            }
        // } else  // User selected POI - temporarily disablied for M3
        // {   
        //     auto POI_it = std::find(pin_display_start.begin(), pin_display_start.end(), POI_AllInfo[POI_id].POIPoint);
        //     if (POI_it == pin_display_start.end())
        //     {   
        //         // Clear all pins if newly select food place.
        //         pin_display_start.clear();
        //         // Set start point to selected position
        //         start_point = POI_AllInfo[POI_id].POIPoint;
        //         // Highlight only selected POI
        //         pin_display_start.push_back(start_point);
                
        //         // Set Search bar to contain intersection name
        //         std::string POIname = POI_AllInfo[POI_id].POIName;
        //         const gchar* cstr = POIname.c_str();
        //         gtk_entry_set_text(GTK_ENTRY(SearchBar), cstr);
        //     } else
        //     {
        //         pin_display_start.erase(POI_it);
        //     }
        // }
    }
    // Navigation mode
    else
    {
        // Record closest intersection
        IntersectionIdx inter_id = findClosestIntersection(ezgl::point2d(x, y));
        const gchar* cstr = Intersection_IntersectionInfo[inter_id].name.c_str();
        
        // Put the intersection name into input field, depending on which field is hightlighted
        if (gtk_widget_has_focus(GTK_WIDGET(SearchBarDestination)))
        {
            // Change the text in the SearchBar to intersection selected
            // This should not change the destination_point_set to false
            search_2_forced_change = true;
            gtk_entry_set_text(GTK_ENTRY(SearchBarDestination), cstr);
            // Set destination point
            destination_point = Intersection_IntersectionInfo[inter_id].position_xy;
            destination_point_id = inter_id;
            destination_point_set = true;

            // If the other field is not empty but not "set", call navigation_response on it
            const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(SearchBar));
            std::string input_1(search_text);
            if (!input_1.empty() && !start_point_set)
            {
                start_point_set = navigation_response(input_1, true, application);
            }
        } else
        {
            // Change the text in the SearchBar to intersection selected
            // This should not change the start_point_set to false
            search_1_forced_change = true;
            gtk_entry_set_text(GTK_ENTRY(SearchBar), cstr);
            // Set starting point
            start_point = Intersection_IntersectionInfo[inter_id].position_xy;
            start_point_id = inter_id;
            start_point_set = true;

            // If the other field is not empty but not "set", call navigation_response on it
            const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(SearchBarDestination));
            std::string input_2(search_text);
            if (!input_2.empty() && !destination_point_set)
            {
                destination_point_set = navigation_response(input_2, false, application);
            }
        }
        // Change pin display for new start and destination if they are set
        pin_display_start.clear();
        pin_display_dest.clear();
        if (start_point_set)
        {
            pin_display_start.push_back(start_point);
        }
        if (destination_point_set)
        {
            pin_display_dest.push_back(destination_point);
        }
        
        // Start navigate if both fields are set
        if (start_point_set && destination_point_set)
        {
            found_path.clear();
            found_path = findPathBetweenIntersections(std::make_pair(start_point_id, destination_point_id), DEFAULT_TURN_PENALTY);
            if (found_path.size() == 0)
            {
                std::string to_be_converted = "No path found between 2 points";
                application->refresh_drawing();
                application->create_popup_message("Error", to_be_converted.c_str());
                return;
            }
            
            //Generate Direction Text for display
            generate_directions();
            
            // Rezoom the map to view the path
            view_path(application, 2);
        }
    }
    application->refresh_drawing();
}
