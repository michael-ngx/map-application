#include "m3.h"
#include "m4.h"
#include "globals.h"
#include <queue>
#include <list>
#include <unordered_set>
#include <cfloat>
#include <random>
#include <chrono>
#include <stdlib.h>
#include <time.h>

/*******************************************************************************************************************************
 * GLOBAL VARIABLES & FUNCTION DECLARATIONS
 ********************************************************************************************************************************/
// The same intersection may appear multiple times in the pickUp
// or dropOff list. The same intersection can also appear as both a
// pickUp location and a dropOff location (for multiple packages, or one package).
//        
// If you have two pickUps to make at an intersection, traversing the
// intersection once is sufficient to pick up both packages. Additionally, 
// one traversal of an intersection is sufficient to drop off all the 
// (already picked up) packages that need to be dropped off at that intersection.
//
// NOTE: It sometimes better to drop off some of the packages, and come back later to drop off the rest
//
// Depots will never appear as pickUp or dropOff locations for deliveries.

//Time limit for doing permutation
#define TIME_LIMIT 45 //m4: 50 seconds time limit

struct DeliveryPoint
{
    // Deliveries to be picked up
    std::unordered_set<int> deliveries_to_pick;
    // Deliveries that will be dropOff
    std::unordered_set<int> deliveries_to_drop;
    // Flag to check if current DeliveryPoint is both pickUp & dropoff for some package
    int same_pickUp_dropOff = 0;
};

// Single start - Multidestination Djikstra algorithm
// Record the fastest travel time between 2 intersections, turn penalty included
// Store result directly to the row of 2D Matrix
bool multiDestinationDjakstra (
        const IntersectionIdx start_id,
        const std::unordered_set<IntersectionIdx> &delivery_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const float turn_penalty,
        bool from_depot);

// Given a std::list of found path, test if the path is legal
// The check traverses through the whole path and return based on the deliveries
// pickedUp and delivered
std::pair<bool, float> checkPathLegal(
        const std::vector<IntersectionIdx> &test_path,
        const std::unordered_map<IntersectionIdx, 
              std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>>> &Matrix,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
        const std::unordered_set<IntersectionIdx> &pickUp_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        const int &num_deliveries);

// Given a std::list of found path, perform 2-opt on the list and test if the path is legal
// The 2-opt cut the list in random order and reverse one of the sub-path. 
// The function utilize checkPathLegal, and update the best_path list if best_time is lowered and the path is legal
void greedyPath2Opt(std::list<IntersectionIdx> &test_path,
                    float &best_time,
                    const std::chrono::high_resolution_clock::time_point start_time,
                    const std::unordered_map<IntersectionIdx, 
                          std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>>> &Matrix,
                    const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
                    const std::unordered_set<IntersectionIdx> &pickUp_set,
                    const std::unordered_set<IntersectionIdx> &depot_set);

// Get the closest next legal travel point based on current_point
// "Smart" if: path exist && (never visited & have something to pickUp || have at least 1 package to dropOff)
// If no "smart" point is found, the closest legal travel point is chosen
std::pair<IntersectionIdx, float> getNextLegalDeliveryPoint (
        const std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
        const std::unordered_map<IntersectionIdx, bool> &current_picked_map,
        const std::unordered_set<IntersectionIdx> &pickUp_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        const std::unordered_set<int> &carrying_ids);

// Get multiple (including closest) next legal travel point from current_point
// "Smart" if: path exist && (never visited & have something to pickUp || have at least 1 package to dropOff)
// If at least 1 "smart" point is found, only go to "smart" points
// If no "smart" point is found, some (including closest) legal travel point is chosen
std::vector<std::pair<IntersectionIdx, float>> getNextLegalDeliveryPoint_Multi (
        const std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
        const std::unordered_map<IntersectionIdx, bool> &current_picked_map,
        const std::unordered_set<IntersectionIdx> &pickUp_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        const std::unordered_set<int> &carrying_ids);

// Check if set1 contains at least 1 element in set2
bool check_set_intersection (const std::unordered_set<int> &set1,
                             const std::unordered_set<int> &set2);

/*******************************************************************************************************************************
 * TRAVELLING COURIER
 ********************************************************************************************************************************/
// This routine takes in a vector of D deliveries (pickUp, dropOff
// intersection pairs), another vector of N intersections that
// are legal start and end points for the path (depots), and a turn 
// penalty in seconds (see m3.h for details on turn penalties).
//
// You can assume that D is always at least one and N is always at least one
// (i.e. both input vectors are non-empty).
//  
// If no valid route to make *all* the deliveries exists, this routine must
// return an empty (size == 0) vector.
std::vector<CourierSubPath> travelingCourier(
                            const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const float turn_penalty)
{
    // Resulting vector
    std::vector<CourierSubPath> result;
    // Unordered map of all interested delivery intersections 
    // Interesting intersections can be pickUp or dropOff (of one or more deliveries), 
    // or both pickUp and dropOff (of one or more deliveries)
    std::unordered_map<IntersectionIdx, DeliveryPoint> delivery_map;
    // All interested points
    std::unordered_set<IntersectionIdx> delivery_set;
    std::vector<IntersectionIdx> delivery_vect;
    // All pickUp points (for begin point)
    std::unordered_set<IntersectionIdx> pickUp_set;
    std::vector<IntersectionIdx> pickUp_vect;
    // All depots
    std::unordered_set<IntersectionIdx> depot_set;
    // The start time of the program
//    auto start_time = std::chrono::high_resolution_clock::now();

    // Fill in data
    for (int i = 0; i < deliveries.size(); i++)
    {
        // Edge case: A DeliveryInf may have its pickUp and dropOff at the same intersection
        // If so, turn on same_pickUp_dropOff flag. The intersection must still be visited
        // Do not add delivery to deliveries_to_pick or deliveries_to_drop
        if (deliveries[i].pickUp == deliveries[i].dropOff)
        {
            // Look up sets
            delivery_set.insert(deliveries[i].pickUp);
            pickUp_set.insert(deliveries[i].pickUp);
            pickUp_vect.push_back(deliveries[i].pickUp);
            // Delivery map (notice that we do not record the package)
            if (delivery_map.find(deliveries[i].pickUp) == delivery_map.end())
            {
                DeliveryPoint point;
                point.same_pickUp_dropOff = 1;
                delivery_map.insert(std::make_pair(deliveries[i].pickUp, point));
                // Vector for multithread
                delivery_vect.push_back(deliveries[i].pickUp);
            } else
            {
                delivery_map.at(deliveries[i].pickUp).same_pickUp_dropOff++;
            }
            continue;
        }

        // Record pick up
        if (delivery_map.find(deliveries[i].pickUp) == delivery_map.end())
        {
            // Look up sets
            delivery_set.insert(deliveries[i].pickUp);
            pickUp_set.insert(deliveries[i].pickUp);
            pickUp_vect.push_back(deliveries[i].pickUp);
            // Delivery map
            DeliveryPoint point;
            point.deliveries_to_pick.insert(i);
            delivery_map.insert(std::make_pair(deliveries[i].pickUp, point));
            // Vector for multithread
            delivery_vect.push_back(deliveries[i].pickUp);
        } else
        {
            if (pickUp_set.find(deliveries[i].pickUp) == pickUp_set.end())
            {
                pickUp_set.insert(deliveries[i].pickUp);
                pickUp_vect.push_back(deliveries[i].pickUp);
            }
            delivery_map.at(deliveries[i].pickUp).deliveries_to_pick.insert(i);
        }
        
        // Record drop off
        if (delivery_map.find(deliveries[i].dropOff) == delivery_map.end())
        {
            // Look up sets
            delivery_set.insert(deliveries[i].dropOff);
            // Delivery map
            DeliveryPoint point;
            point.deliveries_to_drop.insert(i);
            delivery_map.insert(std::make_pair(deliveries[i].dropOff, point));
            // Vector for multithread
            delivery_vect.push_back(deliveries[i].dropOff);
        } else
        {
            delivery_map.at(deliveries[i].dropOff).deliveries_to_drop.insert(i);
        }

    }
    // Look up set depot_set
    for (auto depot : depots)
    {
        depot_set.insert(depot);
    }
    
    // 2D Matrix of travel time between any 2 intersections (except for itself)
    // Call to Matrix.at(From).at(To) returns a pair:
    // - pair.first: Fastest travel time From -> To
    // - pair.second: Fastest travel route From -> To
    // CAUTION: There will be an error if there's no path From->To
    std::unordered_map<IntersectionIdx, 
    std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>>> Matrix;

    /*********************************************************************************************
     * 1. Pre-compute travel time between any 2 interested intersections
     * (Multi-destination Djikstra)
     *********************************************************************************************/
    // a. Any delivery location -> All delivery (except for itself) + depots 
    int count_error = 0;
    #pragma omp parallel for
    for (IntersectionIdx delivery : delivery_vect)
    {
        std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> Matrix_row;
        bool check = multiDestinationDjakstra(delivery,
                                              delivery_set,
                                              depot_set,
                                              Matrix_row,
                                              turn_penalty,
                                              false);
        if (check == false)
        {
            count_error++;
        }
        #pragma omp critical
        Matrix.insert(std::make_pair(delivery, std::move(Matrix_row)));
    }
    // If there's an interested point that can reach no interested point
    if (count_error)
    {
        return result;
    }
    // b. Any depot location -> All pick-ups
    #pragma omp parallel for
    for (IntersectionIdx depot : depots)
    {
        std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> Matrix_row;
        bool check = multiDestinationDjakstra(depot,
                                              pickUp_set,
                                              depot_set,
                                              Matrix_row,
                                              turn_penalty,
                                              true);
        if (check == false)
        {
            count_error++;
        }
        #pragma omp critical
        Matrix.insert(std::make_pair(depot, std::move(Matrix_row)));
    }
    // If there's an interested point that can reach no interested point
    if (count_error)
    {
        return result;
    }

    /*********************************************************************************************
     * 2. Greedy Algorithm
     *********************************************************************************************/
    // Current best travel path (global)
    std::list<IntersectionIdx> best_path;
    // Current best travel path time (global)
    float best_time = FLT_MAX;

    #pragma omp parallel for
    // Check different starting points
    for (auto pickUp_start : pickUp_vect)
    {        
        // Picked up map for initial path
        std::unordered_map<IntersectionIdx, bool> picked_map;
        for (auto delivery : pickUp_set)
        {
            picked_map.insert(std::make_pair(delivery, false));
        }
        picked_map.at(pickUp_start) = true;
        // **************************************** IMPORTANT *****************************************//
        // Normally, next point in greedy algorithm is chosen by the closest legal point               //
        // We instead choose some second points, perform greedy for them, then take the best path      //
        // The closest legal point to the first point must be included                                 //
        // ********************************************************************************************//
        auto second_point_options = getNextLegalDeliveryPoint_Multi(Matrix.at(pickUp_start),
                                                                    delivery_map,
                                                                    picked_map,
                                                                    pickUp_set,
                                                                    depot_set,
                                                                    delivery_map.at(pickUp_start).deliveries_to_pick);
        // (FROM CURRENT START POINT) Current best travel path
        std::list<IntersectionIdx> best_path_local;
        // (FROM CURRENT START POINT) Current best travel path time (different second points)
        float best_time_local = FLT_MAX;

        // Assume that at least 1 smart/dumb point is found
        for (auto second_point_pair : second_point_options)
        {
            // Number of deliveries left
            int num_deliveries = deliveries.size();
            // Stores the current legal travel path
            std::list<IntersectionIdx> current_path;
            // Save first point
            current_path.push_back(pickUp_start);
            // Total travel time of the current path
            float total_time = 0;
            // Current picked map for current path
            std::unordered_map<IntersectionIdx, bool> current_picked_map(picked_map);
            
            // Delivery ids currently carrying
            std::unordered_set<int> carrying_ids;
            // Pick up at the first delivery point
            carrying_ids.insert(delivery_map.at(pickUp_start).deliveries_to_pick.begin(),
                                delivery_map.at(pickUp_start).deliveries_to_pick.end());
            // Must do for every first time we visit a point: check for same_pickUp_dropOff packages
            // If true, these package(s) are automatically pickedUp and droppedOff
            if (delivery_map.at(pickUp_start).same_pickUp_dropOff)
            {
                num_deliveries -= delivery_map.at(pickUp_start).same_pickUp_dropOff;
            }
           
            
            // Main while loop for traveling
            bool init = true;   // Actions are done on second point in first while loop
            bool run = true;    // Error checking
            IntersectionIdx current_point = pickUp_start;
            
            while (num_deliveries)
            {
                IntersectionIdx next_point = second_point_pair.first;
                float time = second_point_pair.second;
                // Determine next legal delivery point
                // Will always go inside after first while loop
                if (!init)
                {
                    std::tie(next_point, time) = getNextLegalDeliveryPoint(Matrix.at(current_point),
                                                                    delivery_map,
                                                                    current_picked_map,
                                                                    pickUp_set,
                                                                    depot_set,
                                                                    carrying_ids);
                    // No legal points are found --> error delivery path
                    if (next_point == -1)
                    {
                        run = false;
                        break;
                    }
                }
                init = false;
                
                // If never visited the point --> pickUp packages
                if (pickUp_set.find(next_point) != pickUp_set.end() && !current_picked_map.at(next_point))
                {
                    carrying_ids.insert(delivery_map.at(next_point).deliveries_to_pick.begin(),
                                        delivery_map.at(next_point).deliveries_to_pick.end());
                    current_picked_map.at(next_point) = true;
                    // Check for same_pickUp_dropOff
                    if (delivery_map.at(next_point).same_pickUp_dropOff)
                    {
                        num_deliveries -= delivery_map.at(next_point).same_pickUp_dropOff;
                    }
                }

                // dropOff packages if any
                for (auto it = delivery_map.at(next_point).deliveries_to_drop.begin(); 
                    it != delivery_map.at(next_point).deliveries_to_drop.end(); it++)
                {
                    if (carrying_ids.find(*it) != carrying_ids.end())
                    {
                        carrying_ids.erase(*it);
                        num_deliveries--;
                    }
                }

                // Record next point
                current_path.push_back(next_point);
                // Add total time
                total_time += time;

                // Proceed
                current_point = next_point;
            }

            if (run == false || current_path.empty())
            {
                continue;
            }
            
            // Find closest depot to end point
            IntersectionIdx chosen_end_depot = -1;
            float min_end_time = FLT_MAX;
            
            // Check path from last delivery point to depot
            for (auto depot : depots)
            {   
                if (Matrix.at(current_path.back()).find(depot) != Matrix.at(current_path.back()).end())
                {
                    if (Matrix.at(current_path.back()).at(depot).first < min_end_time)
                    {
                        min_end_time = Matrix.at(current_path.back()).at(depot).first;
                        chosen_end_depot = depot;
                    }
                }
            }
            
            // Error path (no path from any depot to pickUp point or last point to any depot)
            if (chosen_end_depot == -1)
            {
                continue;
            }
            
            total_time += min_end_time;
            current_path.push_back(chosen_end_depot);
            
            // Local check (for each second point)
            if (total_time != 0 && total_time < best_time_local)
            {
                best_time_local = total_time;
                best_path_local = std::move(current_path);
            }
        } // End of each second point

        if (best_path_local.empty())
        {
            continue;
        }
        
        // Add the beginning depot
        IntersectionIdx chosen_start_depot = -1;
        float min_begin_time = FLT_MAX;
        
        // Check path from depot to first pickUp point
        for (auto depot : depots)
        {
            if (Matrix.at(depot).find(pickUp_start) != Matrix.at(depot).end())
            {
                if (Matrix.at(depot).at(pickUp_start).first < min_begin_time)
                {
                    min_begin_time = Matrix.at(depot).at(pickUp_start).first;
                    chosen_start_depot = depot;
                }
            }
        }
        if (chosen_start_depot == -1)
        {
            continue;
        }
        
        // Travel time of first courier sub-path
        best_time_local += min_begin_time;
        best_path_local.push_front(chosen_start_depot);
        

        #pragma omp critical
        if (best_time_local < best_time)
        {
            best_time = best_time_local;
            best_path = std::move(best_path_local);
        }
    } // End of each first point

    /***************************************************************
     * Run 2-opt funciton
     ***************************************************************/
    // greedyPath2Opt(best_path, best_time, start_time, Matrix, delivery_map, pickUp_set, depot_set);

    /***********************************************************************************************
     * Randomly select an element (non-depot)
     * Try to fit the element somewhere else in the path for legality
     ***********************************************************************************************/
    // Copy best_path into a vector for ease of swapping
    std::vector<IntersectionIdx> best_path_vect;
    std::copy(best_path.begin(), best_path.end(), std::back_inserter(best_path_vect));

    // Generate random seed
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, best_path_vect.size() - 2);
    
    int limit = 30000;
    if (CURRENT_MAP_PATH == "/cad2/ece297s/public/maps/toronto_canada.streets.bin" )
    {
        limit = 5000;
    } else if (CURRENT_MAP_PATH == "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin")
    {
        limit = 3000;
    } else if (CURRENT_MAP_PATH == "/cad2/ece297s/public/maps/iceland.streets.bin")
    {
        limit = 5000;
    } else if (CURRENT_MAP_PATH == "/cad2/ece297s/public/maps/tokyo_japan.streets.bin")
    {
        limit = 100;
    }

    #pragma omp parallel for
    for (int i = 0; i < limit; i++)
    {
        // Copy initial path
        std::vector<IntersectionIdx> best_path_vect_copy(best_path_vect);        
            
        // Select a random element
        int index_rand = dist(gen);
        IntersectionIdx rand_element = best_path_vect_copy.at(index_rand);

        // Traverse through the path to find a valid position
        for (int index = 0; index < best_path_vect_copy.size(); index++)
        {
            // Swapping elements
            best_path_vect_copy.at(index_rand) = best_path_vect_copy.at(index);
            best_path_vect_copy.at(index) = rand_element;
            // Check if the new path is legal
            auto [legal, check_time] = checkPathLegal(best_path_vect_copy, Matrix, delivery_map, pickUp_set, depot_set, deliveries.size());

            if (legal && check_time < best_time) 
            {
                best_time = check_time;
                #pragma omp critical
                best_path_vect = std::move(best_path_vect_copy);
                break;
            } else
            {
                // Undo the move and try the next position
                best_path_vect_copy.at(index) = best_path_vect_copy.at(index_rand);
                best_path_vect_copy.at(index_rand) = rand_element;
            }
        }
    }
    
    /***************************************************************
     * Generate result path
     ***************************************************************/
    for (auto i = 0; i < best_path_vect.size() - 1; ++i)
    {
        CourierSubPath subPath = {best_path_vect[i],
                                  best_path_vect[i + 1],
                                  Matrix.at(best_path_vect[i]).at(best_path_vect[i + 1]).second};
        result.push_back(subPath);
    }

    // auto [final_legal, final_time] = checkPathLegal(best_path_vect, Matrix, delivery_map, pickUp_set, depot_set, deliveries.size()); 
    // std::cout << "QOR " << final_time << std::endl;

    return result;
}

/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/
bool multiDestinationDjakstra (
        const IntersectionIdx start_id,
        const std::unordered_set<IntersectionIdx> &delivery_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const float turn_penalty,
        bool from_depot)
{
    // Create the priority queue and the record_node hash table
    std::priority_queue<NodeMulti> pq;
    std::unordered_map<IntersectionIdx, NodeMulti> record_node;
    // Keep track of visited node ids
    std::unordered_set<IntersectionIdx> visited;

    // Create the starting node
    // Add the starting node to the priority queue (FIFO) and record_node hash table
    NodeMulti startNode = {start_id, 0, -1, -1};
    pq.push(startNode);
    record_node.insert(std::make_pair(start_id, startNode));

    // Keep running Djakstra algorithm until explored all nodes in the map
    // or all interested nodes can be reached by start_id
    while (!pq.empty())
    {
        // The current node to explore is the top node in the queue
        // a.k.a node with smallest f-value
        NodeMulti current = pq.top();
        pq.pop();

        // If current node is visited --> Skip to next node in priority queue
        if (visited.find(current.id) != visited.end())
        {
            continue;
        }
        // Mark current node as visited
        visited.insert(current.id);

        // If current node is one of the interested nodes 
        if  (
                (!from_depot && current.id != start_id &&
                (delivery_set.find(current.id) != delivery_set.end()
                || depot_set.find(current.id) != depot_set.end()))

                ||

                (from_depot && current.id != start_id &&
                delivery_set.find(current.id) != delivery_set.end())
            )
        {
            IntersectionIdx point_reached = current.id;
            float time = current.g;
            std::vector<StreetSegmentIdx> path;
            // Reconstruct the path from start_id and current.id
            while (current.parent != -1)
            {
                path.insert(path.begin(), current.parent_segment);
                current = record_node[current.parent];
            }
            // Record travel time to current row in Matrix
            Matrix_row.insert(std::make_pair(point_reached, std::make_pair(time, path)));

            // Break early if found path to all interested nodes
            // NOT from_depot : delivery points - 1 (itself) + depots
            // from_depot : pickUp points
            if ((!from_depot && Matrix_row.size() == delivery_set.size() + depot_set.size() - 1)
                || 
                (from_depot && Matrix_row.size() == delivery_set.size()))
            {
                return true;
            }
        }

        // Intersection Info for current node (to get neighbors and edges)
        IntersectionInfo intersection_info = Intersection_IntersectionInfo[current.id];
        
        // If current node has no neighbors --> skip
        if (intersection_info.neighbors_and_segments.empty())
        {
            continue;
        }

        // Explore all neighbors
        for (auto pair : intersection_info.neighbors_and_segments)
        {
            // If neighbor node is visited --> Skip to next neighbor
            if (visited.find(pair.first) != visited.end())
            {
                continue;
            }

            // Get the street segments connecting the current node and the neighbor node
            // There may be multiple segments connecting 2 adjacent nodes
            // Segments may belong to different streets --> Must consider turn penalties
            std::vector<StreetSegmentIdx> connectingStreetSegments = pair.second;

            // g-value for neighbor = min g-value (travel time) of a connecting segment
            float g = current.g + Segment_SegmentDetailedInfo[connectingStreetSegments[0]].travel_time;
            if ((current.parent_segment != -1) && 
                (Segment_SegmentDetailedInfo[current.parent_segment].streetName != Segment_SegmentDetailedInfo[connectingStreetSegments[0]].streetName))
            {
                g += turn_penalty;
            }
            // min_segment for neighbor = segment that leads to min g-value
            StreetSegmentIdx min_segment = connectingStreetSegments[0];
            
            if (connectingStreetSegments.size() != 1)
            {
                for (StreetSegmentIdx streetSegment : connectingStreetSegments)
                {
                    // Calculate the g-value of the neighbor node, based on different street segments
                    // Record the segment with shortest travel time
                    float g_temp = current.g + Segment_SegmentDetailedInfo[streetSegment].travel_time;
                    // Add turn_penalty if choosing the current streetsegment leads to a new street
                    if ((current.parent_segment != -1) && 
                        (Segment_SegmentDetailedInfo[current.parent_segment].streetName != Segment_SegmentDetailedInfo[streetSegment].streetName))
                    {
                        g_temp += turn_penalty;
                    }
                    // Update attirbutes for the neighbor node 
                    if (g_temp < g)
                    {
                        g = g_temp;
                        min_segment = streetSegment;
                    }
                }
            }

            // If the neighbor node has been recorded before, set g-value, parent, and parent_segment
            // to the path with least travel time
            if (record_node.find(pair.first) != record_node.end())
            {
                if (g < record_node[pair.first].g)
                {
                    record_node[pair.first].g = g;
                    record_node[pair.first].parent = current.id;
                    record_node[pair.first].parent_segment = min_segment;
                    pq.push(record_node[pair.first]);
                }
            } else
            {
                NodeMulti neighborNode = {pair.first, g, current.id, min_segment};
                record_node[pair.first] = neighborNode;
                pq.push(neighborNode);
            }
        }
    }
    // No interested nodes can be reached by start_id, the Matrix has 0 size
    if (Matrix_row.size() == 0)
    {
        return false;
    } 
    // Some, but not all interested nodes can be reached by start_id
    else
    {
        return true;
    }
}

// Check for legality of new path
std::pair<bool, float> checkPathLegal(
        const std::vector<IntersectionIdx> &test_path,
        const std::unordered_map<IntersectionIdx, 
              std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>>> &Matrix,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
        const std::unordered_set<IntersectionIdx> &pickUp_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        const int &num_deliveries)
{
    int deliveries_left = num_deliveries;
    float time = 0;
    // If first point and last point is not a depot
    if (depot_set.find(test_path[0]) == depot_set.end()
        || depot_set.find(test_path[test_path.size() - 1]) == depot_set.end())
    {
        return std::make_pair(false, time);
    }
    // Deliveries pickedUp so far
    std::unordered_set<int> carrying_ids;
    // Traverse the path and do pickUp/dropOff
    for (auto i = 0; i < test_path.size() - 1; ++i)
    {
        // If there are no path between it and std::next(it)
        if (Matrix.at(test_path[i]).find(test_path[i + 1]) == Matrix.at(test_path[i]).end())
        {
            return std::make_pair(false, time);
        }
        
        // Pick Up if is a pickUp point
        if (pickUp_set.find(test_path[i]) != pickUp_set.end())
        {
            carrying_ids.insert(delivery_map.at(test_path[i]).deliveries_to_pick.begin(),
                                delivery_map.at(test_path[i]).deliveries_to_pick.end());
            // Check for same_pickUp_dropOff
            if (delivery_map.at(test_path[i]).same_pickUp_dropOff)
            {
                deliveries_left -= delivery_map.at(test_path[i]).same_pickUp_dropOff;
            }
        }

        // dropOff packages if any (if current point is not a depot)
        if (depot_set.find(test_path[i]) == depot_set.end())
        {
            for (auto it_drop = delivery_map.at(test_path[i]).deliveries_to_drop.begin(); 
            it_drop != delivery_map.at(test_path[i]).deliveries_to_drop.end(); it_drop++)
            {
                if (carrying_ids.find(*it_drop) != carrying_ids.end())
                {
                    carrying_ids.erase(*it_drop);
                    deliveries_left--;
                }
            }
        }

        // Keep track of current path's QoR
        time += Matrix.at(test_path[i]).at(test_path[i + 1]).first;
    }
    // Check for num_deliveries left
    if (deliveries_left != 0)
    {
        return std::make_pair(false, time);
    } else
    {
        return std::make_pair(true, time);
    }
}

// Get the closest next legal travel point from current_point
std::pair<IntersectionIdx, float> getNextLegalDeliveryPoint (
        const std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
        const std::unordered_map<IntersectionIdx, bool> &current_picked_map,
        const std::unordered_set<IntersectionIdx> &pickUp_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        const std::unordered_set<int> &carrying_ids)
{
    IntersectionIdx next_point_smart = -1;
    IntersectionIdx next_point_dumb = -1;
    float min_time_smart = FLT_MAX;
    float min_time_dumb = FLT_MAX;

    // Only looping through intersections current_id can reach to
    for (auto pair : Matrix_row)
    {
        IntersectionIdx point_id = pair.first;
        // Skip if point_id is a depot
        if (depot_set.find(point_id) != depot_set.end())
        {
            continue;
        }
        // "Smart" if: never visited & have something to pickUp || have at least 1 package to dropOff
        if (
            pair.second.first < min_time_smart
            &&
            ((pickUp_set.find(point_id) != pickUp_set.end() && !current_picked_map.at(point_id))
            || check_set_intersection(carrying_ids, delivery_map.at(point_id).deliveries_to_drop))
            )
        {
            next_point_smart = point_id;
            min_time_smart = pair.second.first;
        } else
        {
            if (pair.second.first < min_time_dumb)
            {
                next_point_dumb = point_id;
                min_time_dumb = pair.second.first;
            }
        }
    }

    // If there's a "smart" next point, go to that one
    if (min_time_smart != FLT_MAX)
    {
        return std::make_pair(next_point_smart, min_time_smart);
    } else
    {
        return std::make_pair(next_point_dumb, min_time_dumb);
    }
}

// Get multiple (including closest) next legal travel point from current_point
std::vector<std::pair<IntersectionIdx, float>> getNextLegalDeliveryPoint_Multi (
        const std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
        const std::unordered_map<IntersectionIdx, bool> &current_picked_map,
        const std::unordered_set<IntersectionIdx> &pickUp_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        const std::unordered_set<int> &carrying_ids)
{
    IntersectionIdx best_point_smart = -1;
    IntersectionIdx best_point_dumb = -1;
    float min_time_smart = FLT_MAX;
    float min_time_dumb = FLT_MAX;
    
    std::vector<std::pair<IntersectionIdx, float>> next_points_smart;
    std::vector<std::pair<IntersectionIdx, float>> next_points_dumb;

    // Only looping through intersections current_id can reach to
    for (auto pair : Matrix_row)
    {
        IntersectionIdx point_id = pair.first;
        // Skip if point_id is a depot
        if (depot_set.find(point_id) != depot_set.end())
        {
            continue;
        }
        // "Smart" if: never visited & have something to pickUp || have at least 1 package to dropOff
        if ((pickUp_set.find(point_id) != pickUp_set.end() && !current_picked_map.at(point_id))
            || check_set_intersection(carrying_ids, delivery_map.at(point_id).deliveries_to_drop))
        {
            if (pair.second.first < min_time_smart)
            {
                if (best_point_smart != -1)
                {
                    next_points_smart.push_back(std::make_pair(best_point_smart, min_time_smart));
                }
                best_point_smart = point_id;
                min_time_smart = pair.second.first;
            }
        } else
        {
            if (pair.second.first < min_time_dumb)
            {
                if (best_point_dumb != -1)
                {
                    next_points_dumb.push_back(std::make_pair(best_point_dumb, min_time_dumb));
                }
                best_point_dumb = point_id;
                min_time_dumb = pair.second.first;
            }
        }
    }

    // Always go to best_point_start or best_point_dumb
    next_points_smart.push_back(std::make_pair(best_point_smart, min_time_smart));
    next_points_dumb.push_back(std::make_pair(best_point_dumb, min_time_dumb));

    // If there's at least a "smart" next point, go to those ones only
    if (best_point_smart != -1)
    {
        return next_points_smart;
    } else
    {
        return next_points_dumb;
    }
}

// Check if set1 contains at least 1 element in set2
bool check_set_intersection (const std::unordered_set<int> &set1,
                             const std::unordered_set<int> &set2)
{
    for (auto it = set2.begin(); it != set2.end(); ++it)
    {
        if (set1.find(*it) != set1.end())
        {
            return true;
        }
    }
    return false;
}



//find better path by doing 2-opt
void greedyPath2Opt(std::list<IntersectionIdx> &best_path,
                    float &best_time,
                    const std::chrono::high_resolution_clock::time_point start_time,
                    const std::unordered_map<IntersectionIdx, 
                          std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>>> &Matrix,
                    const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
                    const std::unordered_set<IntersectionIdx> &pickUp_set,
                    const std::unordered_set<IntersectionIdx> &depot_set)
{
    if (best_path.size() <= 3)
    {
        return;
    }
    std::list<IntersectionIdx> test_path;
    std::list<IntersectionIdx> cut_path_front;
    std::list<IntersectionIdx> cut_path_middle;
    std::list<IntersectionIdx> cut_path_end;
    int num_deliveries = delivery_map.size();
    int best_path_size = best_path.size() - 2;
    bool timeout = false;
    auto current_time = std::chrono::high_resolution_clock::now();
    auto wall_clock = std::chrono::duration_cast<std::chrono::duration<double>> (current_time - start_time);
    if (wall_clock.count() > 0.9 * TIME_LIMIT){
        timeout = true;
    }
    
    while (!timeout)
    {
        //update and check if the remaining time is sufficient for more 2-opt
        current_time = std::chrono::high_resolution_clock::now();
        wall_clock = std::chrono::duration_cast<std::chrono::duration<double>> (current_time - start_time);
        if (wall_clock.count() > 0.9 * TIME_LIMIT){
            timeout = true;
        }
        
        //pre-process the best_path list for 2-opt
        std::srand (time(NULL));
        test_path = best_path;
        test_path.pop_front();
        test_path.pop_back();
        int first_cut = std::rand() % (best_path_size - 1) + 1;
        int second_cut = std::rand() % (best_path_size - 1) + 1;
        int temp_cut = first_cut;
        int local_best_time = 0;
        if (first_cut > second_cut)
        {
            first_cut = second_cut;
            second_cut = temp_cut;
        } else if (first_cut == second_cut)
        {
            continue;
        }
        int counter = 0;
        auto iterator = test_path.begin();
        while (counter < first_cut)
        {
            cut_path_front.push_back(*iterator);
            test_path.pop_front();
            counter++;
        }
        while (counter < second_cut)
        {
            cut_path_middle.push_back(*iterator);
            test_path.pop_front();
            counter++;
        }
        while (counter < best_path_size)
        {
            cut_path_end.push_back(*iterator);
            test_path.pop_front();
            counter++;
        }
        
        //2-opt
        //reverse the first sub path
        cut_path_front.reverse();
        counter = 0;
        iterator = cut_path_front.begin();
        while (counter < first_cut)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        iterator = cut_path_middle.begin();
        while (counter < second_cut)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        iterator = cut_path_middle.begin();
        while (counter < best_path_size)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        test_path.push_front(best_path.front());
        test_path.push_back(best_path.back());

        // Copy to vector for checking legality
        std::vector<IntersectionIdx> best_path_vect_temp_1;
        std::copy(best_path.begin(), best_path.end(), std::back_inserter(best_path_vect_temp_1));

        auto [legal_1, time_1] = checkPathLegal(best_path_vect_temp_1, Matrix, delivery_map, pickUp_set, depot_set, num_deliveries);

        if (legal_1)
        {
            for (auto it = test_path.begin(); it != std::prev(test_path.end()); ++it)
            {
                local_best_time += Matrix.at(*it).at(*std::next(it)).first;
            }
            if (local_best_time < best_time)
            {
                best_time = local_best_time;
                best_path = test_path;
            }
        }
        
        //reverse the middle sub path
        test_path.clear();
        cut_path_front.reverse();
        cut_path_middle.reverse();
        counter = 0;
        iterator = cut_path_front.begin();
        while (counter < first_cut)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        iterator = cut_path_middle.begin();
        while (counter < second_cut)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        iterator = cut_path_middle.begin();
        while (counter < best_path_size)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        test_path.push_front(best_path.front());
        test_path.push_back(best_path.back());
        // Copy to vector for checking legality
        std::vector<IntersectionIdx> best_path_vect_temp_2;
        std::copy(best_path.begin(), best_path.end(), std::back_inserter(best_path_vect_temp_2));

        auto [legal_2, time_2] = checkPathLegal(best_path_vect_temp_2, Matrix, delivery_map, pickUp_set, depot_set, num_deliveries);
        if (legal_2)
        {
            for (auto it = test_path.begin(); it != std::prev(test_path.end()); ++it)
            {
                local_best_time += Matrix.at(*it).at(*std::next(it)).first;
            }
            if (local_best_time < best_time)
            {
                best_time = local_best_time;
                best_path = test_path;
            }
        }
        
        //reverse the end sub path
        test_path.clear();
        cut_path_middle.reverse();
        cut_path_end.reverse();
        counter = 0;
        iterator = cut_path_front.begin();
        while (counter < first_cut)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        iterator = cut_path_middle.begin();
        while (counter < second_cut)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        iterator = cut_path_middle.begin();
        while (counter < best_path_size)
        {
            test_path.push_back(*iterator);
            iterator++;
            counter++;
        }
        test_path.push_front(best_path.front());
        test_path.push_back(best_path.back());
        // Copy to vector for checking legality
        std::vector<IntersectionIdx> best_path_vect_temp_3;
        std::copy(best_path.begin(), best_path.end(), std::back_inserter(best_path_vect_temp_3));

        auto [legal_3, time_3] = checkPathLegal(best_path_vect_temp_3, Matrix, delivery_map, pickUp_set, depot_set, num_deliveries);
        if (legal_3)
        {
            for (auto it = test_path.begin(); it != std::prev(test_path.end()); ++it)
            {
                local_best_time += Matrix.at(*it).at(*std::next(it)).first;
            }
            if (local_best_time < best_time)
            {
                best_time = local_best_time;
                best_path = test_path;
            }
        }
        
        //clear containers for the next loop
        test_path.clear();
        cut_path_front.clear();
        cut_path_middle.clear();
        cut_path_end.clear();
    }
    return;
}