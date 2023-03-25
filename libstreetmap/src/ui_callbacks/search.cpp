#include "ui_callbacks/input_response.hpp" 

/****************************************************************************************************************************
// Response to search callback
*****************************************************************************************************************************/
void search_response (std::string input, ezgl::application *application)
{
    pin_display.clear();

    // No response if input is empty
    if (input.empty())
    {
        std::string to_be_converted = "Enter intersection in the input field";
        const char* message = to_be_converted.c_str();
        application->create_popup_message("Error", message);
        application->refresh_drawing();
        return;
    }

    // Center of new camera, to be set to either intersection or food place
    ezgl::point2d center;

    //*****************************************************************************************************
    // 1: User selected from the drop-down list - perfectly matches an intersection name between 2 streeets
    //*****************************************************************************************************
    if (IntersectionName_IntersectionIdx.find(input) != IntersectionName_IntersectionIdx.end())
    {
        // There may be multiple intersections with the same name
        // If there are multiple, only allow a maximum of 5 intersections to be added to the pop-up window
        auto range = IntersectionName_IntersectionIdx.equal_range(input);

        int count = 0;  // Count the number of values
        std::string to_be_converted = "Intersection(s) " + input + ":\n";
        for (auto it = range.first; it != range.second; ++it) {
            // Highlight the found intersections on the map
            pin_display.push_back(Intersection_IntersectionInfo[it->second].position_xy);
            // Add data for all intersections to pop-up message if count < 5
            if (count < 5)
            {
                to_be_converted += "Latitude: " + std::to_string(getIntersectionPosition(it->second).latitude()) + "\n"
                                    + "Longitude: " + std::to_string(getIntersectionPosition(it->second).longitude()) + "\n"
                                    + "------------------------\n";
            }
            count++;
        }
        if (count >= 5)
        {
            to_be_converted += "More not shown...\n";
        }
        const char* message = to_be_converted.c_str();
        application->create_popup_message("Intersection(s) found: ", message);
        application->update_message("Found intersection!");

        // Move the camera to focus the intersection
        // Center of new location (centered at first intersection found)
        center = Intersection_IntersectionInfo[range.first->second].position_xy;
        move_camera(center, application);
    }

    //*****************************************************************************************************
    // 2. User entered something without a '&' - can either be intersection on just 1 street, or 2 streets without '&'
    //*****************************************************************************************************
    else if (input.find('&') == std::string::npos)
    {
        // Response if there's a perfect match, unique partial match, or multiple partial match
        // If there are multiple partial match, choose the first one
        std::vector<IntersectionIdx> partials = findIntersectionIdsFromPartialIntersectionName(input);
        if (partials.size() == 0)
        {
            std::string to_be_converted = "Intersection not found";
            const char* message = to_be_converted.c_str();
            application->create_popup_message("Error", message);
            application->refresh_drawing();
            return;
        } else if (partials.size() == 1)
        {
            std::string to_be_converted = "Intersection(s) on " + Intersection_IntersectionInfo[partials[0]].name + ":\n\n";
            to_be_converted += "Latitude: " + std::to_string(getIntersectionPosition(partials[0]).latitude()) + "\n"
                                + "Longitude: " + std::to_string(getIntersectionPosition(partials[0]).longitude()) + "\n";
            const char* message = to_be_converted.c_str();
            application->create_popup_message("Intersection found: ", message);
            application->update_message("Found intersection!");

            // Move the camera to focus the intersection
            // Center of new location (centered at first intersection found)
            center = Intersection_IntersectionInfo[partials[0]].position_xy;
            // Highlight the found intersection on the map
            pin_display.push_back(center);
            move_camera(center, application);
        } else
        {
            // There are multiple intersections with the chosen name --> display all intersections
            IntersectionIdx selected_inter = partials[0];
            std::string selected_name = Intersection_IntersectionInfo[selected_inter].name;
            
            std::string to_be_converted = "Intersection on " + Intersection_IntersectionInfo[selected_inter].name + ":\n";
            // If there are multiple, only allow a maximum of 5 intersections to be added to the pop-up window
            auto range = IntersectionName_IntersectionIdx.equal_range(selected_name);
            int count = 0;  // Count the number of values
            for (auto it = range.first; it != range.second; ++it) {
                // Highlight the found intersections on the map
                pin_display.push_back(Intersection_IntersectionInfo[it->second].position_xy);
                // Add data for all intersections to pop-up message if count < 5
                if (count < 5)
                {
                    to_be_converted += "Latitude: " + std::to_string(getIntersectionPosition(it->second).latitude()) + "\n"
                                        + "Longitude: " + std::to_string(getIntersectionPosition(it->second).longitude()) + "\n"
                                        + "------------------------\n";
                }
                count++;
            }
            if (count >= 5)
            {
                to_be_converted += "More not shown...\n";
            }
            to_be_converted += "\n\n";
            to_be_converted += "Note: Input does not uniquely identify an intersection. First name match chosen";
            const char* message = to_be_converted.c_str();
            application->create_popup_message("Intersection(s) found: ", message);
            application->update_message("Found intersection!");

            // Move the camera to focus the intersection
            // Center of new location (centered at first intersection found)
            center = Intersection_IntersectionInfo[range.first->second].position_xy;
            move_camera(center, application);

            // Change the content of the search bar to first partials found
            std::string new_search_bar_content = Intersection_IntersectionInfo[partials[0]].name;
            const gchar* cstr = new_search_bar_content.c_str();
            gtk_entry_set_text(GTK_ENTRY(SearchBar), cstr);
        }
    }

    //*****************************************************************************************************
    // 3. User did enter something with a "&". Now, split the string to get street names, then find streets and intersections by partial name
    //*****************************************************************************************************
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
        // String for pop-up message
        std::string to_be_converted = "";

        // Parse all streets input from the user. Streets are separated by "&" character
        input.erase(std::remove(input.begin(), input.end(), ' '), input.end());
        int pos = 0;
        std::string street;
        while ((pos = input.find("&")) != std::string::npos)
        {
            street = input.substr(0, pos);
            split_streets.push_back(street);
            input.erase(0, pos + 1);
        }
        split_streets.push_back(input); // add the last street

        // Do not allow enter same inputs (avoid showing all intersections of one street)
        if (split_streets.size() == 2 && split_streets[0] == split_streets[1] && split_streets[0] != "")
        {
            to_be_converted = "Enter two different streets";
            const char* message = to_be_converted.c_str();
            application->create_popup_message("Error", message);
            application->refresh_drawing();
            return;
        }

        for (int i = 0; i < split_streets.size(); i++)
        {
            std::vector<StreetIdx> partials = findStreetIdsFromPartialStreetName(split_streets[i]);
            if (partials.size() == 0)
            {
                to_be_converted = "Token " + std::to_string(i + 1) + " does not match any streets";
                const char* message = to_be_converted.c_str();
                application->create_popup_message("Error", message);
                application->refresh_drawing();
                return;
            } else
            {
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

        // 3.1. No intersections found between the streets
        if (first_vect.size() == 0)
        {
            to_be_converted += "No intersections found between ";
            for (int i = 0; i < streets_selected.size(); i++)
            {
                to_be_converted += streets_selected[i];
                if (i != streets_selected.size() - 1)
                {
                    to_be_converted += " & ";
                }
            }
            application->update_message("No intersections found!");

        }
        // 3.2. One intersection found between streets
        else if (first_vect.size() == 1)
        {
            // Highlight the found intersections on the map
            pin_display.push_back(Intersection_IntersectionInfo[first_vect[0]].position_xy);

            to_be_converted += "Intersection found between ";
            for (int i = 0; i < streets_selected.size(); i++)
            {
                to_be_converted += streets_selected[i];
                if (i != streets_selected.size() - 1)
                {
                    to_be_converted += " & ";
                }
            }
            to_be_converted += ":\n";
            to_be_converted += "Latitude: " + std::to_string(getIntersectionPosition(first_vect[0]).latitude()) + "\n"
                                + "Longitude: " + std::to_string(getIntersectionPosition(first_vect[0]).longitude()) + "\n"
                                + "------------------------\n";

            // Move the camera to focus the intersection
            center = Intersection_IntersectionInfo[first_vect[0]].position_xy;
            move_camera(center, application);

            application->update_message("Found intersection!");
        }
        // 3.3. More than 1 intersections found between streets
        // TODO: User will be prompted to choose a specific intersection when direction mode is turned ON
        else
        {
            int count = 0;  // Count the number of intersections
            to_be_converted += "Multiple intersections found between ";
            for (int i = 0; i < streets_selected.size(); i++)
            {
                to_be_converted += streets_selected[i];
                if (i != streets_selected.size() - 1)
                {
                    to_be_converted += " & ";
                }
            }
            to_be_converted += ":\n";
            for (auto it = first_vect.begin(); it != first_vect.end(); it++)
            {
                // Highlight the found intersections on the map
                pin_display.push_back(Intersection_IntersectionInfo[*it].position_xy);
                // Add data for all intersections to pop-up message if count < 5
                if (count < 5)
                {
                    to_be_converted += "Latitude: " + std::to_string(getIntersectionPosition(*it).latitude()) + "\n"
                                        + "Longitude: " + std::to_string(getIntersectionPosition(*it).longitude()) + "\n"
                                        + "------------------------\n";
                }
                count++;
            }
            if (count >= 5)
            {
                to_be_converted += "More not shown...\n";
            }
            // Set center of new camera (centered at first intersection found)
            center = Intersection_IntersectionInfo[first_vect[0]].position_xy;
            move_camera(center, application);

            application->update_message("Found intersection!");
        }

        // For all situations (that user entered street names with '&')
        // add warning if any street name entered isn't unique 
        auto flag_it = std::find(street_unique_flags.begin(), street_unique_flags.end(), false);
        if (flag_it != street_unique_flags.end())
        {
            to_be_converted += "\n\n";
            to_be_converted += "Note: Token " + std::to_string(std::distance(street_unique_flags.begin(), flag_it) + 1) + " does not uniquely define a street";
        }
        const char* message = to_be_converted.c_str();
        application->create_popup_message("Search intersection", message);
    }

    // Check for food places - Temporarily disabled for M3
    // else if (POI_AllFood.find(input) != POI_AllFood.end())
    // {
    //     auto POIit = POI_AllFood.find(input);
    //     // Highlight the location to be displayed
    //     pin_display.push_back(POI_AllInfo[POIit->second.id].POIPoint);

    //     // Center of new location
    //     center = POI_AllInfo[POIit->second.id].POIPoint;
    //     move_camera(center,application);
    //     application->update_message("Found food place!");
    // }
}
