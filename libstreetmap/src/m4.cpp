#include "m3.h"
#include "m4.h"
#include "globals.h"
#include <queue>
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
    // Check if current Point has been visited
    // "picked" only means that all deliveries_to_pick are picked.
    // We may still need to come back for dropping
    // (if deliveries_to_drop is not empty)
    bool picked = false;
    // Flag to check if current DeliveryPoint is both pickUp & dropoff for some package
    int same_pickUp_dropOff = 0;
};

// Single start - Multidestination Djikstra algorithm
// Record the fastest travel time between 2 intersections, turn penalty included
// Store result directly to the row of 2D Matrix
void multiDestinationDjakstra (
        const IntersectionIdx start_id,
        const std::unordered_set<IntersectionIdx> &delivery_set,
        const std::unordered_set<IntersectionIdx> &depot_set,
        std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const float turn_penalty,
        bool from_depot);

// Get the closest next legal travel point based on current_point
// Legal if: path exist && (never visited & have something to pickUp || have at least 1 package to dropOff)
IntersectionIdx getNextLegalDeliveryPoint (
        const IntersectionIdx current_id,
        const std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
        const std::unordered_set<IntersectionIdx> &pickUp_set,
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
    // Unordered map of all interested delivery intersections 
    // Interesting intersections can be pickUp or dropOff (of one or more deliveries), 
    // or both pickUp and dropOff (of one or more deliveries)
    std::unordered_map<IntersectionIdx, DeliveryPoint> delivery_map;
    // All interested points
    std::unordered_set<IntersectionIdx> delivery_set;
    std::vector<IntersectionIdx> delivery_vect;
    // All pickUp points (for begin point)
    std::unordered_set<IntersectionIdx> pickUp_set;
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
            // Delivery map
            DeliveryPoint point;
            point.deliveries_to_pick.insert(i);
            delivery_map.insert(std::make_pair(deliveries[i].pickUp, point));
            // Vector for multithread
            delivery_vect.push_back(deliveries[i].pickUp);
        } else
        {
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

    /***************************************************************
     * 1. Pre-compute travel time between any 2 interested intersections
     * (Multi-destination Djikstra)
     ***************************************************************/
    // a. Any delivery location -> All delivery (except for itself) + depots 
    #pragma omp parallel for
    for (IntersectionIdx delivery : delivery_vect)
    {
        std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> Matrix_row;
        multiDestinationDjakstra(delivery,
                                 delivery_set,
                                 depot_set,
                                 Matrix_row,
                                 turn_penalty,
                                 false);
        #pragma omp critical
        Matrix.insert(std::make_pair(delivery, std::move(Matrix_row)));
    }
    // b. Any depot location -> All pick-ups
    #pragma omp parallel for
    for (IntersectionIdx depot : depots)
    {
        std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> Matrix_row;
        multiDestinationDjakstra(depot,
                                 pickUp_set,
                                 depot_set,
                                 Matrix_row,
                                 turn_penalty,
                                 true);
        #pragma omp critical
        Matrix.insert(std::make_pair(depot, std::move(Matrix_row)));
    }

    /***************************************************************
     * 2. Greedy Algorithm
     ***************************************************************/
    // Number of deliveries left
    int num_deliveries = deliveries.size();
    // Resulting vector
    std::vector<CourierSubPath> result;
    // Stores the current legal travel path
    std::vector<IntersectionIdx> current_path;
    // Delivery ids currently carrying
    std::unordered_set<int> carrying_ids;    

    // Start at a depot that's closest to any **pick-up point**
    IntersectionIdx begin_depot;
    IntersectionIdx start_delivery_point;
    float min_begin_time = FLT_MAX;
    IntersectionIdx second_begin_depot;
    IntersectionIdx second_start_delivery_point;
    float second_min_begin_time = FLT_MAX;
    bool init = false;

    for (IntersectionIdx depot : depots)
    {
        for (auto pickUp_it = pickUp_set.begin(); 
            pickUp_it != pickUp_set.end(); 
            ++pickUp_it)
        {
            // No path from depot to pickUp point
            if (Matrix.at(depot).find(*pickUp_it) == Matrix.at(depot).end())
            {
                continue;
            }
            if (!init)
            {   // Runs here until at least one depot -> pickUp point is initialized
                if (Matrix.at(depot).at(*pickUp_it).first < min_begin_time)
                {
                    min_begin_time = Matrix.at(depot).at(*pickUp_it).first;
                    begin_depot = depot;
                    start_delivery_point = *pickUp_it;
                    init = true;
                }
            } else
            {
                if (Matrix.at(depot).at(*pickUp_it).first < min_begin_time)
                {
                    second_min_begin_time = min_begin_time;
                    second_begin_depot = begin_depot;
                    second_start_delivery_point = start_delivery_point;
                    min_begin_time = Matrix.at(depot).at(*pickUp_it).first;
                    begin_depot = depot;
                    start_delivery_point = *pickUp_it;
                } else if (Matrix.at(depot).at(*pickUp_it).first != min_begin_time
                        && Matrix.at(depot).at(*pickUp_it).first < second_min_begin_time)
                {
                    second_min_begin_time = Matrix.at(depot).at(*pickUp_it).first;
                    second_begin_depot = depot;
                    second_start_delivery_point = *pickUp_it;
                }
            }
        }
    }
    // Error path (no path from any depot to any pickUp point)
    if (!init)
    {
        return result;
    }
    // Save current path
    current_path.push_back(begin_depot);
    current_path.push_back(start_delivery_point);

    // Pick up at the first delivery point
    carrying_ids.insert(delivery_map.at(start_delivery_point).deliveries_to_pick.begin(),
                        delivery_map.at(start_delivery_point).deliveries_to_pick.end());
    delivery_map.at(start_delivery_point).picked = true;
    // Must do for every first time we visit a point: check for same_pickUp_dropOff packages
    // If true, these package(s) are automatically pickedUp and droppedOff
    if (delivery_map.at(start_delivery_point).same_pickUp_dropOff)
    {
        num_deliveries -= delivery_map.at(start_delivery_point).same_pickUp_dropOff;
    }

    // Main while loop for travelling
    IntersectionIdx current_point = start_delivery_point;
    while (num_deliveries)
    {
        // Determine next legal delivery point
        IntersectionIdx next_point = getNextLegalDeliveryPoint(current_point,
                                                               Matrix.at(current_point),
                                                               delivery_map,
                                                               pickUp_set,
                                                               carrying_ids);
        // No legal points are found --> error delivery path
        if (next_point == -1)
        {
            return result;
        }

        // If never visited the point --> pickUp packages
        if (!delivery_map.at(next_point).picked)
        {
            carrying_ids.insert(delivery_map.at(next_point).deliveries_to_pick.begin(),
                                delivery_map.at(next_point).deliveries_to_pick.end());
            delivery_map.at(next_point).picked = true;
            // Check for same_pickUp_dropOff
            if (delivery_map.at(next_point).same_pickUp_dropOff)
            {
                num_deliveries -= delivery_map.at(next_point).same_pickUp_dropOff;
            }
        }

        // dropOff packages if any
        for (auto it = delivery_map.at(next_point).deliveries_to_drop.begin(); 
            it != delivery_map.at(next_point).deliveries_to_drop.end(); )
        {
            if (carrying_ids.find(*it) != carrying_ids.end())
            {
                carrying_ids.erase(*it);
                // Update iterator to next element if a delete happens
                it = delivery_map.at(next_point).deliveries_to_drop.erase(it);
                num_deliveries--;
            } else
            {
                ++it; // Increment iterator normally
            }
        }

        // Record next point
        current_path.push_back(next_point);

        // Proceed
        current_point = next_point;
    }

    /***************************************************************
     * Recover the result
     ***************************************************************/
    // Find closest depot to the last point
    IntersectionIdx end_depot = -1;
    float min_end = FLT_MAX;
    for (IntersectionIdx depot : depots)
    {
        if (Matrix.at(current_point).find(depot) != Matrix.at(current_point).end()
            && Matrix.at(current_point).at(depot).first < min_end)
        {
            min_end = Matrix.at(current_point).at(depot).first;
            end_depot = depot;
        }
    }
    // Cannot reach any depots from the last point - should not happen
    // Since the last point should always be able to traverse back to the beginning depot
    if (end_depot == -1)
    {
        return result;
    }
    current_path.push_back(end_depot);

    // Generate result path
    IntersectionIdx point_1 = current_path[0];
    IntersectionIdx point_2;
    for (int i = 1; i < current_path.size(); i++)
    {
        point_2 = current_path[i];
        CourierSubPath subPath = {point_1, point_2, Matrix.at(point_1).at(point_2).second};
        result.push_back(subPath);
        point_1 = point_2;
    }
    return result;
}

/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/
void multiDestinationDjakstra (
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
                break;
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
    // It is assumed here that if some interested nodes cannot be reached by start_id,
    // the corresponding travel time in the Matrix has been initialized to 0
    
}

// Get the closest next legal travel point from current_point
IntersectionIdx getNextLegalDeliveryPoint (
        const IntersectionIdx current_id,
        const std::unordered_map<IntersectionIdx, std::pair<float, std::vector<StreetSegmentIdx>>> &Matrix_row,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &delivery_map,
        const std::unordered_set<IntersectionIdx> &pickUp_set,
        const std::unordered_set<int> &carrying_ids)
{
    IntersectionIdx next_point = -1;
    float min_time = FLT_MAX;
    // Check if there are any legal points
    bool init = false;

    for (auto pair : delivery_map)
    {
        IntersectionIdx point_id = pair.first;
        DeliveryPoint next_point_info = pair.second;
        if (point_id == current_id)
        {
            continue;
        }
        // Legal if: path exist && (never visited & have something to pickUp || have at least 1 package to dropOff)
        if (!init)
        {   // Goes here until found at least 1 legal next point (can travel to)
            if (Matrix_row.find(point_id) != Matrix_row.end() && Matrix_row.at(point_id).first != 0)
            {
                next_point = point_id;
                init = true;
            }
        } else
        {   // Improve by looking for more useful next_points
            if (
                Matrix_row.find(point_id) != Matrix_row.end() && Matrix_row.at(point_id).first < min_time
                &&
                ((!next_point_info.picked && pickUp_set.find(point_id) != pickUp_set.end())
                || check_set_intersection(carrying_ids, next_point_info.deliveries_to_drop))
               )
            {
                next_point = point_id;
                min_time = Matrix_row.at(point_id).first;
            }
        }
    }

    // If there are no next_point, -1 is returned
    return next_point;
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