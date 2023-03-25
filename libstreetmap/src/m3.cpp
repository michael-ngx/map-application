#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "globals.h"
#include "OSMDatabaseAPI.h"
#include "string.h"
#include <cmath>
#include <algorithm>

// Function declarations
double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, 
                             const double turn_penalty);
std::vector<StreetSegmentIdx> findPathBetweenIntersections(
                  const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids,
                  const double turn_penalty);







// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0.  The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path.  If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, 
                             const double turn_penalty){
                                 
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
std::vector<StreetSegmentIdx> findPathBetweenIntersections(
                  const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids,
                  const double turn_penalty){

                  }