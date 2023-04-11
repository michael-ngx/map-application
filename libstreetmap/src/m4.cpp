#include "m3.h"
#include "m4.h"
#include "globals.h"
#include <queue>

/*******************************************************************************************************************************
 * GLOBAL VARIABLES & FUNCTION DECLARATIONS
 ********************************************************************************************************************************/
// It is legal for the same intersection to appear multiple times in the pickUp
// or dropOff list (e.g. you might have two deliveries with a pickUp
// intersection id of #50). The same intersection can also appear as both a
// pickUp location and a dropOff location.
//        
// If you have two pickUps to make at an intersection, traversing the
// intersection once is sufficient to pick up both packages. Additionally, 
// one traversal of an intersection is sufficient to drop off all the 
// (already picked up) packages that need to be dropped off at that intersection.
//
// Depots will never appear as pickUp or dropOff locations for deliveries.
// *****
// ***** A DeliveryInf may have its pickUp and dropOff at the same intersection
// *****
struct DeliveryPoint
{
    // All deliveries that this Point will "pickUp"
    std::vector<int> pickUp_delivery_ids;
    // All deliveries that this Point will "dropOff"
    std::vector<int> dropOff_delivery_ids;
    // Check if performed pick up (will pick up all packages)
    bool picked = false;
    // Check if performed drop off (will drop off all packages)
    bool dropped = false;
};

// Single start - Multidestination Djikstra algorithm
// Record the fastest travel time between 2 intersections, turn penalty included
// Store result directly to the row of 2D Matrix
void multiDestinationDjakstra (
        const IntersectionIdx start_id,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &all_delivery_points,
        const std::unordered_map<IntersectionIdx, bool> &all_depots_map,
        std::unordered_map<IntersectionIdx, float> &Matrix_row,
        const float turn_penalty);

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
    // Vector of all interested delivery intersections 
    // Interesting intersections can be pickUp or dropOff (of one or more deliveries), 
    // or both pickUp and dropOff (of one or more deliveries)
    std::unordered_map<IntersectionIdx, DeliveryPoint> all_delivery_points;
    // Unordered map for depot fast searching
    std::unordered_map<IntersectionIdx, bool> all_depots_map;
    // Fill in data for all_delivery_points
    for (int i = 0; i < deliveries.size(); i++)
    {
        all_delivery_points[deliveries[i].pickUp].pickUp_delivery_ids.push_back(i);
        all_delivery_points[deliveries[i].dropOff].dropOff_delivery_ids.push_back(i);
    }
    // Fill in data for all_depots_map
    for (auto depot : depots)
    {
        all_depots_map[depot] = NULL;
    }
    
    // 2D Matrix of travel time between any 2 intersections
    // Row: From; Column: To. Call to Matrix[From][To] 
    // returns the fastest travel time From -> To
    std::unordered_map<IntersectionIdx, std::unordered_map<IntersectionIdx, float>> Matrix;

    // 1. Pre-compute travel time between any 2 interested intersections: multi-destination Djikstra
    // a. Any delivery location -> All delivery + depots
    // #pragma omp parallel for
    for (auto pair : all_delivery_points)
    {
        multiDestinationDjakstra(pair.first,
                                 all_delivery_points,
                                 all_depots_map,
                                 Matrix[pair.first],
                                 turn_penalty);
    }


    return result;
}

/*******************************************************************************************************************************
 * HELPER FUNCTIONS
 ********************************************************************************************************************************/
void multiDestinationDjakstra (
        const IntersectionIdx start_id,
        const std::unordered_map<IntersectionIdx, DeliveryPoint> &all_delivery_points,
        const std::unordered_map<IntersectionIdx, bool> &all_depots_map,
        std::unordered_map<IntersectionIdx, float> &Matrix_row,
        const float turn_penalty)
{
    // Create the priority queue and the record_node hash table
    std::priority_queue<NodeMulti> pq;
    std::unordered_map<IntersectionIdx, NodeMulti> record_node;
    // Keep track of visited node ids
    std::unordered_map<IntersectionIdx, bool> visited;

    // Create the starting node
    // Add the starting node to the priority queue (FIFO) and record_node hash table
    NodeMulti startNode = {start_id, 0, -1, -1};
    pq.push(startNode);
    record_node.insert(std::make_pair(start_id, startNode));

    // Keep running Djakstra algorithm until explored all nodes in the map
    // This means some interested nodes cannot be reached by start_id
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
        visited.insert(std::make_pair(current.id, NULL));

        // If current node is one of the interested nodes 
        if (all_delivery_points.find(current.id) != all_delivery_points.end()
            || all_depots_map.find(current.id) != all_depots_map.end())
        {
            // Record travel time to current row in Matrix
            Matrix_row.insert(std::make_pair(current.id, current.g));

            // Break early if found path to all interested nodes (delivery points + depots)
            if (Matrix_row.size() == all_delivery_points.size() + all_depots_map.size())
            {
                break;
            }
        }

        // Intersection Info for current node (to get neighbors and edges)
        IntersectionInfo intersection = Intersection_IntersectionInfo[current.id];
        // If current node has no neighbors --> skip
        if (intersection.neighbors_and_segments.empty())
        {
            continue;
        }

        // Explore all neighbors
        for (std::pair<IntersectionIdx, std::vector<StreetSegmentIdx>> pair : intersection.neighbors_and_segments)
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