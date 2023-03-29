/*
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated
 * documentation files (the "Software") in course work at the University
 * of Toronto, or for personal use. Other uses are prohibited, in
 * particular the distribution of the Software either publicly or to third
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "globals.h"
#include <queue>
#include <cmath>
#include <algorithm>

// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0.  The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path.  If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double computePathTravelTime (const std::vector<StreetSegmentIdx>& path, 
                             const double turn_penalty)
{
    double travelTime = 0;
    if(path.size() != 0)
    {
        for(int i = 0; i < path.size(); i++)
        {
            if(i < path.size() - 1
                && Segment_SegmentDetailedInfo[path[i + 1]].streetID != Segment_SegmentDetailedInfo[path[i]].streetID)
            {
                travelTime += turn_penalty;
            }
            travelTime += Segment_SegmentDetailedInfo[path[i]].travel_time;
        }
        return travelTime;
    } else
    {
        return 0;
    }
}


// Returns a path (route) between the start intersection (intersect_id.first)
// and the destination intersection (intersect_id.second), if one exists. 
// This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds).  If no path exists, this routine
// returns an empty (size == 0) vector.  If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the destination intersection.
std::vector<StreetSegmentIdx> findPathBetweenIntersections (
                  const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids,
                  const double turn_penalty)
{
    std::vector<StreetSegmentIdx> result;
    IntersectionIdx start_id = intersect_ids.first;
    IntersectionIdx dest_id = intersect_ids.second;
    // Create the starting node
    double h_start = findDistanceBetweenTwoPoints(Intersection_IntersectionInfo[start_id].position_xy, 
                                                  Intersection_IntersectionInfo[dest_id].position_xy) / MAX_SPEED_LIMIT;
    Node startNode = {start_id, 0, h_start, -1, -1};
    
    // Create the priority queue and the record_node hash table
    std::priority_queue<Node> pq;
    std::unordered_map<IntersectionIdx, Node> record_node;
    // Keep track of visited nodes
    std::unordered_map<IntersectionIdx, Node> visited;

    // Add the starting node to the priority queue (FIFO) and record_node hash table
    pq.push(startNode);
    record_node[start_id] = startNode;

    // A* algorithm
    while (!pq.empty())
    {
        // The current node to explore is the top node in the queue
        // a.k.a node with smallest f-value
        Node current = pq.top();
        pq.pop();

        // If current node is destination node
        if (current.id == dest_id)
        {
            // Reconstruct the path from the goal node to the start node
            while (current.parent != -1)
            {
                result.insert(result.begin(), current.parent_segment);
                current = record_node[current.parent];
            }
            break;
        }

        // If current node is visited --> Skip to next node in priority queue
        if (visited.find(current.id) != visited.end())
        {
            continue;
        }
        // Mark current node as visited
        auto it = record_node.find(current.id);
        visited.insert(std::make_pair(it->first, it->second));

        // Generate neighboring nodes and explore
        std::vector<IntersectionIdx> neighbors = findAdjacentIntersections(current.id);
        // If current node has no neighbors --> skip
        if (neighbors.empty())
        {
            continue;
        }

        // Get all segments around current node
        std::vector<StreetSegmentIdx> streetSegments = findStreetSegmentsOfIntersection(current.id);
        // Explore all neighbors
        for (IntersectionIdx neighbor : neighbors)
        {
            // If neighbor node is visited --> Skip to next neighbor
            if (visited.find(neighbor) != visited.end())
            {
                continue;
            }
            // Get the street segments connecting the current node and the neighbor node
            // There may be multiple segments connecting 2 adjacent nodes
            // NOTE: segments may belong to different streets
            std::vector<StreetSegmentIdx> connectingStreetSegments;
            for (StreetSegmentIdx streetSegment : streetSegments)
            {
                IntersectionIdx from = Segment_SegmentDetailedInfo[streetSegment].from;
                IntersectionIdx to = Segment_SegmentDetailedInfo[streetSegment].to;
                if (from == current.id && to == neighbor)
                {
                    connectingStreetSegments.push_back(streetSegment);
                } else if (!Segment_SegmentDetailedInfo[streetSegment].oneWay && from == neighbor && to == current.id)
                {
                    connectingStreetSegments.push_back(streetSegment);
                }
            }

            // g-value for neighbor = min g-value from the selected min segment
            // Initialize to first streetSegment in connectingStreetSegments
            double g = current.g + Segment_SegmentDetailedInfo[connectingStreetSegments[0]].travel_time;
            if ((current.parent_segment != -1) && 
                (Segment_SegmentDetailedInfo[current.parent_segment].streetName != Segment_SegmentDetailedInfo[connectingStreetSegments[0]].streetName))
            {
                g += turn_penalty;
            }
            // Calculate the h-value of the neighbor node (fixed for each node)
            double h = findDistanceBetweenTwoPoints(Intersection_IntersectionInfo[neighbor].position_xy,
                                                    Intersection_IntersectionInfo[dest_id].position_xy) / MAX_SPEED_LIMIT;
            // min_segment for neighbor = segment that leads to min g-value
            // Initialize to first streetSegment in connectingStreetSegments
            StreetSegmentIdx min_segment = connectingStreetSegments[0];
            
            
            for (StreetSegmentIdx streetSegment : connectingStreetSegments)
            {
                // Calculate the g-value of the neighbor node, based on different street segments
                // Record the segment with shortest travel time
                double g_temp = current.g + Segment_SegmentDetailedInfo[streetSegment].travel_time;
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

            // If the neighbor node has been recorded before, set g-value, parent, and parent_segment
            // to the path with least travel time
            if (record_node.find(neighbor) != record_node.end())
            {
                if (g < record_node[neighbor].g)
                {
                    record_node[neighbor].g = g;
                    record_node[neighbor].parent = current.id;
                    record_node[neighbor].parent_segment = min_segment;
                    // Change the previous neighbor node (that has larger g-value) inside the pq to the neighbor node with new value
                    std::priority_queue<Node> pq2;
                    while (!pq.empty())
                    {
                        Node temp = pq.top();
                        pq.pop();
                        if (temp.id == neighbor)
                        {
                            continue;
                        }
                        pq2.push(temp);
                    }
                    pq2.push(record_node[neighbor]);
                    pq = pq2;
                }
            } else
            {
                Node neighborNode = {neighbor, g, h, current.id, min_segment};
                record_node[neighbor] = neighborNode;
                pq.push(neighborNode);
            }
        }
    }
    
    return result;
}