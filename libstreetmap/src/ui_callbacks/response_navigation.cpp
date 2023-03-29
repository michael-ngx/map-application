#include "ui_callbacks/response.hpp" 

/****************************************************************************************************************************
* Response to navigation callback
*
* bool start_search_bar determines whether the "activate" is coming from starting search bar or destination search bar
*****************************************************************************************************************************/
bool navigation_response (std::string input, bool start_search_bar, ezgl::application *application)
{
    // Clear current pin display
    if (start_search_bar)
    {
        pin_display_start.clear();
    } else
    {
        pin_display_dest.clear();
    }

    // No response if input if empty
    if (input.empty())
    {
        std::string to_be_converted;
        if (start_search_bar) 
        {
            to_be_converted = "Choose starting point, or click on the map";
        } else
        {
            to_be_converted = "Choose destination, or click on the map";
        }
        application->create_popup_message("Error", to_be_converted.c_str());
        application->refresh_drawing();
        return false;
    }

    // Center of new camera, to be set to see the path
    ezgl::point2d center;
    // User feedback popup message
    std::string to_be_converted = "";
    // User feedback headline
    std::string headline = "";

    // First check if input perfectly matches an intersection
    auto input_it = IntersectionName_IntersectionIdx.find(input);

    //*****************************************************************************************************
    // a) Input perfectly matches intersection names between 2 streeets
    //*****************************************************************************************************
    if (input_it != IntersectionName_IntersectionIdx.end())
    {
        // There may be multiple intersections with the same name
        auto range = IntersectionName_IntersectionIdx.equal_range(input);

        // TODO: If there are multiple intersections with the same name, user should get to choose which one they want
        // M3: If there are multiple intersections, choose the first one found and put a note to user
        if (start_search_bar) 
        {
            if (std::distance(range.first, range.second) != 1)
            {
                to_be_converted += "Start name not unique. First occurence chosen.\n";
            }
            // Save the position of the (first) intersection found as starting point
            start_point = Intersection_IntersectionInfo[range.first->second].position_xy;
            start_point_id = range.first->second;
            // Highlight the starting point on the map
            pin_display_start.push_back(start_point);
        } else
        {
            if (std::distance(range.first, range.second) != 1)
            {
                to_be_converted += "Destination name not unique. First occurence chosen.\n";
            }
            // Save the position of the (first) intersection found as destination point
            destination_point = Intersection_IntersectionInfo[range.first->second].position_xy;
            destination_point_id = range.first->second;
            // Highlight the destination point on the map
            pin_display_dest.push_back(destination_point);
        }
    }
    //*****************************************************************************************************
    // b) User entered something without "&"
    // --> Only 1 street name (intersection on 1 street) or 2 street names not separated (error)
    //*****************************************************************************************************
    else if (input.find('&') == std::string::npos)
    {
        std::vector<IntersectionIdx> partials = findIntersectionIdsFromPartialIntersectionName(input);
        if (partials.size() == 0)
        {
            if (start_search_bar) 
            {
                to_be_converted = "Starting point: Intersection not found";
            } else
            {
                to_be_converted = "Destination: Intersection not found";
            }
            application->create_popup_message("Error", to_be_converted.c_str());
            application->refresh_drawing();
            return false;
        } else if (partials.size() == 1)
        {
            if (start_search_bar) 
            {
                // Save the position of the unique intersection found as starting point
                start_point = Intersection_IntersectionInfo[partials[0]].position_xy;
                start_point_id = partials[0];
                // Highlight the starting point
                pin_display_start.push_back(start_point);
            } else
            {
                // Save the position of the unique intersection found as destination
                destination_point = Intersection_IntersectionInfo[partials[0]].position_xy;
                destination_point_id = partials[0];
                // Highlight the destination
                pin_display_dest.push_back(destination_point);
            }
        } else  // TODO: Allow selecting which intersection
        {
            if (start_search_bar) 
            {
                // There are multiple intersections with the chosen name --> choose and display first intersection
                to_be_converted += "Start name is not unique. First occurence chosen.\n";
                // Save the position of the (first) intersection found as starting point
                start_point = Intersection_IntersectionInfo[partials[0]].position_xy;
                start_point_id = partials[0];
                // Highlight the found intersection on the map
                pin_display_start.push_back(start_point);
            } else
            {
                // There are multiple intersections with the chosen name --> choose and display first intersection
                to_be_converted += "Destination name is not unique. First name occurence was chosen.\n";
                // Save the position of the (first) intersection found as destination point
                destination_point = Intersection_IntersectionInfo[partials[0]].position_xy;
                destination_point_id = partials[0];
                // Highlight the destination point on the map
                pin_display_dest.push_back(destination_point);
            }
        }
        // Change the content of the search bar
        // This should not change the starting_point_set or destination_point_set to false
        std::string new_search_bar_content = Intersection_IntersectionInfo[partials[0]].name;
        if (start_search_bar) 
        {
            search_1_forced_change = true;
            gtk_entry_set_text(GTK_ENTRY(SearchBar), new_search_bar_content.c_str());
        } else
        {
            search_2_forced_change = true;
            gtk_entry_set_text(GTK_ENTRY(SearchBarDestination), new_search_bar_content.c_str());
        }
    }
    //*****************************************************************************************************
    // c) User did enter something with a "&"
    // --> Parse string to get street names, then find streets by partial name
    // --> Find intersections between those streets
    //*****************************************************************************************************
    else
    {
        // All streets parsed (lowercase, no space)
        std::vector<std::string> split_streets;
        // All streets selected to be considered (First occurence of partial street name)
        std::vector<std::string> streets_selected;
        // Flags of all streets to note if a street selected from partials uniquely defines a street
        std::vector<bool> street_unique_flags;
        // Index: count of streets being considered; Value: vector of all intersections of that street
        std::vector<std::vector<IntersectionIdx>> intersections_selected;

        // Parse all streets input from the user. Streets should be separated by "&" character
        // TODO: Edge cases streets that have "&" in name
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
            if (start_search_bar) 
            {
                to_be_converted = "Starting point: Enter 2 different streets";
            } else
            {
                to_be_converted = "Destination: Enter 2 different streets";
            }
            application->create_popup_message("Error", to_be_converted.c_str());
            application->refresh_drawing();
            return false;
        }
        
        // Search partials & get intersections for each streets parsed
        for (int i = 0; i < split_streets.size(); i++)
        {
            std::vector<StreetIdx> partials = findStreetIdsFromPartialStreetName(split_streets[i]);
            // No partials were found for given token
            if (partials.size() == 0)
            {
                if (start_search_bar) 
                {
                    to_be_converted = "Starting point: ";
                } else
                {
                    to_be_converted = "Destination: ";
                }
                to_be_converted += "Token " + std::to_string(i + 1) + " does not match any streets";
                application->create_popup_message("Error", to_be_converted.c_str());
                application->refresh_drawing();
                return false;
            } else
            {
                // Set flags for uniqueness of each street entered to give hints
                if (partials.size() == 1)
                {
                    street_unique_flags.push_back(true);
                } else
                {
                    street_unique_flags.push_back(false);
                }
                // Select the matching name as the first name found in partial name
                // This name may also not be unique. We record all intersections of streets with the same name
                std::string selected_name = Street_StreetInfo.at(partials[0]).name;
                // Keep track of which street names are selected to find intersections
                streets_selected.push_back(selected_name);
                // Record all intersections of all streets with the same name into a same vector (all_inter_same_name)
                auto range = StreetName_lower_StreetIdx.equal_range(lower_no_space(selected_name));
                std::vector<IntersectionIdx> all_inter_same_name;
                for (auto it = range.first; it != range.second; it++)
                {
                    all_inter_same_name.insert(all_inter_same_name.end(),
                                               Street_StreetInfo.at(it->second).all_intersections.begin(),
                                               Street_StreetInfo.at(it->second).all_intersections.end());
                }
                // Sort all_inter_same_name and add this vector to intersections_selected
                sort(all_inter_same_name.begin(), all_inter_same_name.end());
                intersections_selected.push_back(all_inter_same_name);
            }
        }

        // All streets entered have been found through partial street name, with intersections recorded
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
        if (start_search_bar) 
        {
            search_1_forced_change = true;
            gtk_entry_set_text(GTK_ENTRY(SearchBar), new_search_bar_content.c_str());
        } else
        {
            search_2_forced_change = true;
            gtk_entry_set_text(GTK_ENTRY(SearchBarDestination), new_search_bar_content.c_str());
        }

        // Now find union of all vectors of intersections - find intersections between selected streets
        auto first_vect = intersections_selected[0];
        for (auto curr_vect : intersections_selected)
        {
            std::vector<IntersectionIdx> intersection_vect;
            std::set_intersection(first_vect.begin(), first_vect.end(),
                                  curr_vect.begin(), curr_vect.end(),
                                  std::back_inserter(intersection_vect));
            first_vect = intersection_vect;
        }

        // c.i. No intersections found between the streets
        if (first_vect.size() == 0)
        {
            if (start_search_bar) 
            {
                to_be_converted = "Starting point: No intersections found between entered streets";
            } else
            {
                to_be_converted = "Destination: No intersections found between entered streets";
            }
            headline = "Error";
        }
        // c.ii. One intersection found between streets
        else if (first_vect.size() == 1)
        {
            // Note
            if (start_search_bar) 
            {
                to_be_converted = "Starting point: Unique intersection found\n";
                // Save the position of the (first) intersection found as starting point
                start_point = Intersection_IntersectionInfo[first_vect[0]].position_xy;
                start_point_id = first_vect[0];
                // Highlight the starting point on the map
                pin_display_start.push_back(start_point);
            } else
            {
                to_be_converted = "Destination: Unique intersection found\n";
                // Save the position of the (first) intersection found as starting point
                destination_point = Intersection_IntersectionInfo[first_vect[0]].position_xy;
                destination_point_id = first_vect[0];
                // Highlight the starting point on the map
                pin_display_dest.push_back(destination_point);
            }
        }
        // c.iii. More than 1 intersections found between streets
        // TODO: User will be prompted to choose a specific intersection when direction mode is turned ON
        // M3: Choose the first intersection found
        else
        {
            // Note
            to_be_converted = "Multiple intersections found. First intersection chosen.\n";
            if (start_search_bar) 
            {
                // Save the position of the (first) intersection found as starting point
                start_point = Intersection_IntersectionInfo[first_vect[0]].position_xy;
                start_point_id = first_vect[0];
                // Highlight the starting point on the map
                pin_display_start.push_back(start_point);
            } else
            {
                // Save the position of the (first) intersection found as starting point
                destination_point = Intersection_IntersectionInfo[first_vect[0]].position_xy;
                destination_point_id = first_vect[0];
                // Highlight the starting point on the map
                pin_display_dest.push_back(destination_point);
            }
        }

        // For all situations in c (user entered street names with '&'),
        // add warning if any street name entered isn't unique 
        auto flag_it = std::find(street_unique_flags.begin(), street_unique_flags.end(), false);
        if (flag_it != street_unique_flags.end())
        {
            to_be_converted += "\n\nWarning: ";
            if (start_search_bar) 
            {
                to_be_converted += "Starting point ";
            } else
            {
                to_be_converted += "Destination point ";
            }
            to_be_converted += "token "  
                                + std::to_string(std::distance(street_unique_flags.begin(), flag_it) + 1)
                                + " does not uniquely define a street";
        }
    }

    // Show note message any
    if (to_be_converted != "")
    {
        if (headline == "Error")
        {
            application->create_popup_message("Error", to_be_converted.c_str());
            application->refresh_drawing();
            return false;
        } else
        {
            application->create_popup_message("Note", to_be_converted.c_str());
            application->refresh_drawing();
            return true;
        }
    }

    application->refresh_drawing();
    return true;
}