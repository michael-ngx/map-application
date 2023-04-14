#include "m3.h"
#include "m4.h"
#include "globals.h"
#include <queue>
#include <list>
#include <unordered_set>
#include <cfloat>

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
bool checkPathLegal(const std::list<IntersectionIdx> &test_path,
                    const std::unordered_map<IntersectionIdx, 
                          std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>>> &Matrix,
                    const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
                    const std::unordered_set<IntersectionIdx> &pickUp_set,
                    const std::unordered_set<IntersectionIdx> &depot_set,
                    const int &num_deliveries);

// Get the closest next legal travel point based on current_point
// Legal if: path exist && (never visited & have something to pickUp || have at least 1 package to dropOff)
std::pair<IntersectionIdx, float> getNextLegalDeliveryPoint (
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
    // Current best travel path
    std::list<IntersectionIdx> best_path;
    // Current best travel path time between different starting points
    float best_time = FLT_MAX;

    #pragma omp parallel for
    // Check different starting points
    for (auto pickUp_start : pickUp_vect)
    {
        // Number of deliveries left
        int num_deliveries = deliveries.size();
        // Stores the current legal travel path
        std::list<IntersectionIdx> current_path;
        // Delivery ids currently carrying
        std::unordered_set<int> carrying_ids;
        // Picked up map for current path
        std::unordered_map<IntersectionIdx, bool> current_picked_map;
        for (auto delivery : pickUp_set)
        {
            current_picked_map.insert(std::make_pair(delivery, false));
        }
        // Total travel time of the current path
        float total_time = 0;

        // Save current path
        current_path.push_back(pickUp_start);

        // Pick up at the first delivery point
        carrying_ids.insert(delivery_map.at(pickUp_start).deliveries_to_pick.begin(),
                            delivery_map.at(pickUp_start).deliveries_to_pick.end());
        current_picked_map.at(pickUp_start) = true;
        // Must do for every first time we visit a point: check for same_pickUp_dropOff packages
        // If true, these package(s) are automatically pickedUp and droppedOff
        if (delivery_map.at(pickUp_start).same_pickUp_dropOff)
        {
            num_deliveries -= delivery_map.at(pickUp_start).same_pickUp_dropOff;
        }

        // Main while loop for travelling
        bool run = true;
        IntersectionIdx current_point = pickUp_start;

        while (num_deliveries)
        {
            // Determine next legal delivery point
            auto [next_point, time] = getNextLegalDeliveryPoint(Matrix.at(current_point),
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
            // Add total time
            total_time += time;
            
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

            // Proceed
            current_point = next_point;
        }

        if (run == false || current_path.empty())
        {
            continue;
        }
        
        // Find closest depot to start and end point
        IntersectionIdx chosen_start_depot = -1;
        IntersectionIdx chosen_end_depot = -1;
        float min_begin_time = FLT_MAX;
        float min_end_time = FLT_MAX;
        
        for (auto depot : depots)
        {
            // Check path from depot to first pickUp point
            if (Matrix.at(depot).find(pickUp_start) != Matrix.at(depot).end())
            {
                if (Matrix.at(depot).at(pickUp_start).first < min_begin_time)
                {
                    min_begin_time = Matrix.at(depot).at(pickUp_start).first;
                    chosen_start_depot = depot;
                }
            }

            // Check path from last delivery point to depot
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
        if (chosen_start_depot == -1 || chosen_end_depot == -1)
        {
            continue;
        }
        // Travel time of first courier subpath
        total_time += min_begin_time;
        total_time += min_end_time;
        current_path.push_front(chosen_start_depot);
        current_path.push_back(chosen_end_depot);

        #pragma omp critical
        if (total_time < best_time && total_time != 0)
        {
            best_time = total_time;
            best_path = std::move(current_path);
        }
    }

    /***************************************************************
     * Generate result path
     ***************************************************************/
    if (best_path.size() == 0)
    {
        return result;
    }

    for (auto it = best_path.begin(); it != std::prev(best_path.end()); ++it)
    {
        CourierSubPath subPath = {*it,
                                  *std::next(it),
                                  Matrix.at(*it).at(*std::next(it)).second};
        result.push_back(subPath);
    }
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
bool checkPathLegal(const std::list<IntersectionIdx> &test_path,
                    const std::unordered_map<IntersectionIdx, 
                          std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>>> &Matrix,
                    const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
                    const std::unordered_set<IntersectionIdx> &pickUp_set,
                    const std::unordered_set<IntersectionIdx> &depot_set,
                    const int &num_deliveries)
{
    int deliveries_left = num_deliveries;
    // If first point and last point is not a depot
    if (depot_set.find(*test_path.begin()) == depot_set.end()
        || depot_set.find(*std::prev(test_path.end())) == depot_set.end())
    {
        return false;
    }
    // Deliveries pickedUp so far
    std::unordered_set<int> carrying_ids;
    // Traverse the path and do pickUp/dropOff
    for (auto it = test_path.begin(); it != std::prev(test_path.end()); ++it)
    {
        // If there are no path between it and std::next(it)
        if (Matrix.at(*it).find(*std::next(it)) == Matrix.at(*it).end())
        {
            return false;
        }
        
        // Pick Up if is a pickUp point
        if (pickUp_set.find(*it) != pickUp_set.end())
        {
            carrying_ids.insert(delivery_map.at(*it).deliveries_to_pick.begin(),
                                delivery_map.at(*it).deliveries_to_pick.end());
            // Check for same_pickUp_dropOff
            if (delivery_map.at(*it).same_pickUp_dropOff)
            {
                deliveries_left -= delivery_map.at(*it).same_pickUp_dropOff;
            }
        }

        // dropOff packages if any (if current point is not a depot)
        if (depot_set.find(*it) == depot_set.end())
        {
            for (auto it_drop = delivery_map.at(*it).deliveries_to_drop.begin(); 
            it_drop != delivery_map.at(*it).deliveries_to_drop.end(); it_drop++)
            {
                if (carrying_ids.find(*it_drop) != carrying_ids.end())
                {
                    carrying_ids.erase(*it_drop);
                    deliveries_left--;
                }
            }
        }
    }
    // Check for num_deliveries left
    if (deliveries_left != 0)
    {
        return false;
    } else
    {
        return true;
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
