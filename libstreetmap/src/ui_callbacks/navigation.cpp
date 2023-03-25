#include "ui_callbacks/input_response.hpp" 

/****************************************************************************************************************************
// Response to navigation callback
*****************************************************************************************************************************/
void navigation_response (std::string input_1, std::string input_2, ezgl::application *application)
{
    pin_display.clear();

    // No response if input if empty
    if (input_1.empty())
    {
        std::string to_be_converted = "Choose starting point, or click on the map";
        const char* message = to_be_converted.c_str();
        application->create_popup_message("Error", message);
        application->refresh_drawing();
        return;
    } else if (input_2.empty())
    {
        std::string to_be_converted = "Choose destination, or click on the map";
        const char* message = to_be_converted.c_str();
        application->create_popup_message("Error", message);
        application->refresh_drawing();
        return;
    }

    // Center of new camera, to be set to see the path
    ezgl::point2d center;
    // Starting point and destination point
    ezgl::point2d start_point;
    ezgl::point2d destination_point;
    // User feedback popup messages
    std::string to_be_converted = "";
    std::string headline = "";

    // Get the intersections from the inputs
    auto input_1_it = IntersectionName_IntersectionIdx.find(input_1);
    auto input_2_it = IntersectionName_IntersectionIdx.find(input_2);

    //*****************************************************************************************************
    // Input 1: a) User selected from the drop-down list - perfectly matches intersection names between 2 streeets
    //*****************************************************************************************************
    if (input_1_it != IntersectionName_IntersectionIdx.end())
    {
        // There may be multiple intersections with the same name
        auto range = IntersectionName_IntersectionIdx.equal_range(input_1);

        // TODO: If there are multiple intersections with the same name, user should get to choose which one they want
        // M3: If there are multiple intersections, choose the first one found and put a note to user
        if (std::distance(range.first, range.second) != 1)
        {
            to_be_converted += "Start point name is not unique. First name occurence was chosen.\n";
            headline = "Note";
        }
        // Save the position of the (first) intersection found as starting point
        start_point = Intersection_IntersectionInfo[range.first->second].position_xy;
        // Highlight the starting point on the map
        pin_display.push_back(start_point);
    }
    // Input 1: b) User entered only 1 street name or 2 street names not separated by '&'
    else if (input_1.find('&') == std::string::npos)
    {
        std::vector<IntersectionIdx> partials = findIntersectionIdsFromPartialIntersectionName(input_1);
        if (partials.size() == 0)
        {
            to_be_converted = "Starting point: Intersection not found";
            const char* message = to_be_converted.c_str();
            application->create_popup_message("Error", message);
            application->refresh_drawing();
            return;
        } else if (partials.size() == 1)
        {
            // Save the position of the (first) intersection found as starting point
            start_point = Intersection_IntersectionInfo[partials[0]].position_xy;
            // Highlight the found intersection on the map
            pin_display.push_back(start_point);
        } else
        {
            // There are multiple intersections with the chosen name --> choose and display first intersection
            to_be_converted += "Start point name is not unique. First name occurence was chosen.\n";
            headline = "Note";
            // Save the position of the (first) intersection found as starting point
            start_point = Intersection_IntersectionInfo[partials[0]].position_xy;
            // Highlight the found intersection on the map
            pin_display.push_back(start_point);
        }
        // Change the content of the search bar
        std::string new_search_bar_content = Intersection_IntersectionInfo[partials[0]].name;
        const gchar* cstr = new_search_bar_content.c_str();
        gtk_entry_set_text(GTK_ENTRY(SearchBar), cstr);
    }
    // Input 1: c) User did enter something with a "&". 
    // Split the string to get street names, then find streets and intersections by partial name
    else
    {
        // Vector to store all streets parsed (lower cased, no space)
        std::vector<std::string> split_streets;
        // Vector to store all streets selected to be considered
        std::vector<std::string> streets_selected;
        // Index: count of streets being considered; Value: vector of all intersections of that street
        std::vector<std::vector<IntersectionIdx>> intersections_selected;
        // Vector of flags to note if a street selected from partials uniquely defines a street
        std::vector<bool> street_unique_flags;

        // Parse all streets input from the user. Streets are separated by "&" character
        input_1.erase(std::remove(input_1.begin(), input_1.end(), ' '), input_1.end());
        int pos = 0;
        std::string street;
        while ((pos = input_1.find("&")) != std::string::npos)
        {
            street = input_1.substr(0, pos);
            split_streets.push_back(street);
            input_1.erase(0, pos + 1);
        }
        split_streets.push_back(input_1); // add the last street

        // Do not allow enter same inputs (avoid showing all intersections of one street)
        if (split_streets.size() == 2 && split_streets[0] == split_streets[1] && split_streets[0] != "")
        {
            to_be_converted = "Starting point: Enter two different streets";
            const char* message = to_be_converted.c_str();
            application->create_popup_message("Error", message);
            application->refresh_drawing();
            return;
        }

        for (int i = 0; i < split_streets.size(); i++)
        {
            std::vector<StreetIdx> partials = findStreetIdsFromPartialStreetName(split_streets[i]);
            // No partials were found for given token
            if (partials.size() == 0)
            {
                to_be_converted = "Starting point: Token " + std::to_string(i + 1) + " entered does not match any streets";
                const char* message = to_be_converted.c_str();
                application->create_popup_message("Error", message);
                application->refresh_drawing();
                return;
            } else
            {
                // Set flags for uniqueness of each street entered
                if (partials.size() == 1)
                {
                    street_unique_flags.push_back(true);
                } else
                {
                    street_unique_flags.push_back(false);
                }
                // Select the matching name as the first name found in partial name
                // This name may also not be unique. We record all intersections of streets with the same name
                StreetIdx selected_idx = partials[0];
                std::string selected_name = Street_StreetInfo.at(selected_idx).name;
                // Keep track of which street names are selected to find intersections
                streets_selected.push_back(selected_name);

                // Remove space and lowercase of selected name (since StreetName_lower_StreetIdx has keys as non-space and lowercase)
                std::string selected_name_lower = lower_no_space (selected_name);

                // Record all intersections of all streets with the same name into a same vector (all_inter_same_name)
                auto range = StreetName_lower_StreetIdx.equal_range(selected_name_lower);
                std::vector<IntersectionIdx> all_inter_same_name;
                for (auto it = range.first; it != range.second; it++)
                {
                    all_inter_same_name.insert(all_inter_same_name.end(),
                                               Street_StreetInfo.at(it->second).all_intersections.begin(),
                                               Street_StreetInfo.at(it->second).all_intersections.end());
                }
                // Add this vector
                sort(all_inter_same_name.begin(), all_inter_same_name.end());
                intersections_selected.push_back(all_inter_same_name);
            }
        }

        // All streets entered have been found through partial street name
        // Change the current text on the search bar to street names that are selected
        std::string new_search_bar_content = "";
        for (int i = 0; i < streets_selected.size(); i++)
        {
            new_search_bar_content += streets_selected[i];
            if (i != streets_selected.size() - 1)
            {
                new_search_bar_content += " & ";
            }
        }
        const gchar* cstr = new_search_bar_content.c_str();
        gtk_entry_set_text(GTK_ENTRY(SearchBar), cstr);

        // Now find union of all vectors of intersections
        auto first_vect = intersections_selected[0];
        for (auto curr_vect : intersections_selected)
        {
            std::vector<IntersectionIdx> intersection_vect;
            std::set_intersection(first_vect.begin(), first_vect.end(),
                                  curr_vect.begin(), curr_vect.end(),
                                  std::back_inserter(intersection_vect));
            first_vect = intersection_vect;
        }

        // 1.c.i. No intersections found between the streets
        if (first_vect.size() == 0)
        {
            to_be_converted += "Starting point: No intersections found between selected streets";
            headline = "Error";
        }
        // 1.c.ii. One intersection found between streets
        else if (first_vect.size() == 1)
        {
            // Note
            to_be_converted += "Starting point: Unique intersections found between selected streets\n";
            headline = "Note";
            // Save the position of the (first) intersection found as starting point
            start_point = Intersection_IntersectionInfo[first_vect[0]].position_xy;
            // Highlight the starting point on the map
            pin_display.push_back(start_point);
        }
        // 1.c.iii. More than 1 intersections found between streets
        // TODO: User will be prompted to choose a specific intersection when direction mode is turned ON
        // Prototype: Choose the first intersection found
        else
        {
            // Note
            to_be_converted += "Multiple intersections found between streets. First intersection was chosen.\n";
            headline = "Note";
            // Save the position of the (first) intersection found as starting point
            start_point = Intersection_IntersectionInfo[first_vect[0]].position_xy;
            // Highlight the starting point on the map
            pin_display.push_back(start_point);
        }

        // For all situations in 1.c (user entered street names with '&'),
        // add warning if any street name entered isn't unique 
        auto flag_it = std::find(street_unique_flags.begin(), street_unique_flags.end(), false);
        if (flag_it != street_unique_flags.end())
        {
            to_be_converted += "\n\n";
            to_be_converted += "Starting point: Token " + std::to_string(std::distance(street_unique_flags.begin(), flag_it) + 1) + " does not uniquely define a street\n";
        }
        // Show message and return if an error occurred. Else, move on to input 2
        if (headline == "Error")
        {
            const char* message = to_be_converted.c_str();
            const char* headline_ctr = headline.c_str();
            application->create_popup_message(headline_ctr, message);
            application->refresh_drawing();
            return;
        }
    }

    //*****************************************************************************************************
    // Input 2: a) User selected from the drop-down list - perfectly matches intersection names between 2 streeets
    //*****************************************************************************************************
    if (input_2_it != IntersectionName_IntersectionIdx.end())
    {
        // There may be multiple intersections with the same name
        auto range = IntersectionName_IntersectionIdx.equal_range(input_2);

        // TODO: If there are multiple intersections with the same name, user should get to choose which one they want
        // M3: If there are multiple intersections, choose the first one found and put a note to user
        if (std::distance(range.first, range.second) != 1)
        {
            to_be_converted += "Destination name is not unique. First name occurence was chosen.\n";
            headline = "Note";
        }
        // Save the position of the (first) intersection found as destination point
        destination_point = Intersection_IntersectionInfo[range.first->second].position_xy;
        // Highlight the destination point on the map
        pin_display.push_back(destination_point);
    }
    // Input 2: b) User did not select from drop-down list. User entered only 1 street name or 2 street names not separated by '&'
    else if (input_2.find('&') == std::string::npos)
    {
        std::vector<IntersectionIdx> partials = findIntersectionIdsFromPartialIntersectionName(input_2);
        if (partials.size() == 0)
        {
            to_be_converted = "Destination point: Intersection not found";
            const char* message = to_be_converted.c_str();
            application->create_popup_message("Error", message);
            application->refresh_drawing();
            return;
        } else if (partials.size() == 1)
        {
            // Save the position of the (first) intersection found as starting point
            start_point = Intersection_IntersectionInfo[partials[0]].position_xy;
            // Highlight the found intersection on the map
            pin_display.push_back(start_point);
        } else
        {
            // There are multiple intersections with the chosen name --> choose and display first intersection
            to_be_converted += "Destination name is not unique. First name occurence was chosen.\n";
            headline = "Note";
            // Save the position of the (first) intersection found as starting point
            start_point = Intersection_IntersectionInfo[partials[0]].position_xy;
            // Highlight the found intersection on the map
            pin_display.push_back(start_point);
        }
        // Change the content of the search bar
        std::string new_search_bar_content = Intersection_IntersectionInfo[partials[0]].name;
        const gchar* cstr = new_search_bar_content.c_str();
        gtk_entry_set_text(GTK_ENTRY(SearchBarDestination), cstr);
    }
    // Input 2: c) User did enter something with a "&". 
    // Split the string to get street names, then find streets and intersections by partial name
    else
    {
        // Vector to store all streets parsed (lower cased, no space)
        std::vector<std::string> split_streets;
        // Vector to store all streets selected to be considered
        std::vector<std::string> streets_selected;
        // Index: count of streets being considered; Value: vector of all intersections of that street
        std::vector<std::vector<IntersectionIdx>> intersections_selected;
        // Vector of flags to note if a street selected from partials uniquely defines a street
        std::vector<bool> street_unique_flags;

        // Parse all streets input from the user. Streets are separated by "&" character
        input_2.erase(std::remove(input_2.begin(), input_2.end(), ' '), input_2.end());
        int pos = 0;
        std::string street;
        while ((pos = input_2.find("&")) != std::string::npos)
        {
            street = input_2.substr(0, pos);
            split_streets.push_back(street);
            input_2.erase(0, pos + 1);
        }
        split_streets.push_back(input_2); // add the last street

        // Do not allow enter same inputs (avoid showing all intersections of one street)
        if (split_streets.size() == 2 && split_streets[0] == split_streets[1] && split_streets[0] != "")
        {
            to_be_converted = "Destination point: Enter two different streets";
            const char* message = to_be_converted.c_str();
            application->create_popup_message("Error", message);
            application->refresh_drawing();
            return;
        }

        for (int i = 0; i < split_streets.size(); i++)
        {
            std::vector<StreetIdx> partials = findStreetIdsFromPartialStreetName(split_streets[i]);
            // No partials were found for given token
            if (partials.size() == 0)
            {
                to_be_converted = "Destination point: Token " + std::to_string(i + 1) + " entered does not match any streets";
                const char* message = to_be_converted.c_str();
                application->create_popup_message("Error", message);
                application->refresh_drawing();
                return;
            } else
            {
                // Set flags for uniqueness of each street entered
                if (partials.size() == 1)
                {
                    street_unique_flags.push_back(true);
                } else
                {
                    street_unique_flags.push_back(false);
                }
                // Select the matching name as the first name found in partial name
                // This name may also not be unique. We record all intersections of streets with the same name
                StreetIdx selected_idx = partials[0];
                std::string selected_name = Street_StreetInfo.at(selected_idx).name;
                // Keep track of which street names are selected to find intersections
                streets_selected.push_back(selected_name);

                // Remove space and lowercase of selected name (since StreetName_lower_StreetIdx has keys as non-space and lowercase)
                std::string selected_name_lower = lower_no_space (selected_name);

                // Record all intersections of all streets with the same name into a same vector (all_inter_same_name)
                auto range = StreetName_lower_StreetIdx.equal_range(selected_name_lower);
                std::vector<IntersectionIdx> all_inter_same_name;
                for (auto it = range.first; it != range.second; it++)
                {
                    all_inter_same_name.insert(all_inter_same_name.end(),
                                               Street_StreetInfo.at(it->second).all_intersections.begin(),
                                               Street_StreetInfo.at(it->second).all_intersections.end());
                }
                // Add this vector
                sort(all_inter_same_name.begin(), all_inter_same_name.end());
                intersections_selected.push_back(all_inter_same_name);
            }
        }

        // All streets entered have been found through partial street name
        // Change the current text on the search bar to street names that are selected
        std::string new_search_bar_content = "";
        for (int i = 0; i < streets_selected.size(); i++)
        {
            new_search_bar_content += streets_selected[i];
            if (i != streets_selected.size() - 1)
            {
                new_search_bar_content += " & ";
            }
        }
        const gchar* cstr = new_search_bar_content.c_str();
        gtk_entry_set_text(GTK_ENTRY(SearchBarDestination), cstr);

        // Now find union of all vectors of intersections
        auto first_vect = intersections_selected[0];
        for (auto curr_vect : intersections_selected)
        {
            std::vector<IntersectionIdx> intersection_vect;
            std::set_intersection(first_vect.begin(), first_vect.end(),
                                  curr_vect.begin(), curr_vect.end(),
                                  std::back_inserter(intersection_vect));
            first_vect = intersection_vect;
        }

        // 2.c.i. No intersections found between the streets
        if (first_vect.size() == 0)
        {
            to_be_converted += "Destination point: No intersections found between given streets";
            headline = "Error";
        }
        // 2.c.ii. One intersection found between streets
        else if (first_vect.size() == 1)
        {
            // Note
            to_be_converted += "Destination point: Unique intersections found between selected streets\n";
            headline = "Note";
            // Save the position of the (first) intersection found as starting point
            destination_point = Intersection_IntersectionInfo[first_vect[0]].position_xy;
            // Highlight the starting point on the map
            pin_display.push_back(destination_point);
        }
        // 2.c.iii. More than 1 intersections found between streets
        // TODO: User will be prompted to choose a specific intersection when direction mode is turned ON
        // Prototype: Choose the first intersection found
        else
        {
            // Note
            to_be_converted += "Multiple intersections found between streets. First intersection was chosen.\n";
            headline = "Note";
            // Save the position of the (first) intersection found as starting point
            destination_point = Intersection_IntersectionInfo[first_vect[0]].position_xy;
            // Highlight the starting point on the map
            pin_display.push_back(destination_point);
        }

        // For all situations in 2.c (user entered street names with '&'),
        // add warning if any street name entered isn't unique 
        auto flag_it = std::find(street_unique_flags.begin(), street_unique_flags.end(), false);
        if (flag_it != street_unique_flags.end())
        {
            to_be_converted += "\n\n";
            to_be_converted += "Destination point: Token " + std::to_string(std::distance(street_unique_flags.begin(), flag_it) + 1) + " does not uniquely define a street";
        }
    }
    // Show message and return if an error or note occurred.
    if (to_be_converted != "" && headline != "")
    {
        const char* message = to_be_converted.c_str();
        const char* headline_ctr = headline.c_str();
        application->create_popup_message(headline_ctr, message);
    }
    application->refresh_drawing();
    return;
}