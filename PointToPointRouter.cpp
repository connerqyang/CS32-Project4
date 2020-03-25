#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <queue>
#include <set>
using namespace std;

class PointToPointRouterImpl
{
  public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
  private:
    const StreetMap* m_streetMap;
    ExpandableHashMap<GeoCoord, GeoCoord>* m_path;  // Contains associations from GeoCoord -> GeoCoord that make up the route
    
        // Finds the optimal route from start to end in StreetSegments and store it in route
    bool findOptimalRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
        
      // Auxiliary function for findOptimalRoute
    void explore(set<GeoCoord>& explored, queue<GeoCoord>& toDo, const StreetSegment& seg) const;
    
      // Recreates the route history segment by segment, adds these segments to route
    void recreateRouteHistory(list<StreetSegment>& route, const GeoCoord& start, const GeoCoord& end, double& totalDistanceTravelled) const;
    
      // Quicksort the vector of StreetSegments in order of increasing distance from the end GeoCoord
    void sortSegments(vector<StreetSegment>& adjacentSegs, int first, int last, const GeoCoord& end) const;
    
      // Auxiliary function for paritioning the vector for the QuickSort function sortSegments
    int partition(vector<StreetSegment>& adjacentSegs, int low, int high, const GeoCoord& end) const;
    
      // Auxiliary function for swapping two StreetSegement's in a vector
    void swapSegInVector(vector<StreetSegment>& adjacentSegs, int index1, int index2) const;
};

  // PRECONDITION: sm points to a fully-constructed StreetMap object containing loaded street map data
PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_streetMap = sm;
    m_path = new ExpandableHashMap<GeoCoord, GeoCoord>;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
    delete m_path;
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
      // Check if the start or end GeoCoord's are valid / within the mapping data
    vector<StreetSegment> startSegments;
    vector<StreetSegment> endSegments;
    if (! (m_streetMap->getSegmentsThatStartWith(start, startSegments) && m_streetMap->getSegmentsThatStartWith(end, endSegments)))
        return BAD_COORD;
    
      // If the start and ending GeoCoord's are the exact same
    if (start == end)
    {
          // Clear the route parameter. Do not add anything to route as there is no route needed
        while (!route.empty())
        {
            list<StreetSegment>::iterator itr = route.begin();
            itr = route.erase(itr);
        }
        totalDistanceTravelled = 0;     // The distance travelled for this case is clearly 0
        return DELIVERY_SUCCESS;        // A path was found (no path needed)
    }
    
      // Determine the optimal route
    if (findOptimalRoute(start, end, route, totalDistanceTravelled))
        return DELIVERY_SUCCESS;
    else
        return NO_ROUTE;
}

  // Return true if a route is found. Otherwise, return false.
  // PRECONDITION: GeoCoord's start and end are valid
bool PointToPointRouterImpl::findOptimalRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    totalDistanceTravelled = 0;             // Reset total distance travelled
    queue<GeoCoord> toDo;
    vector<StreetSegment> adjacentSegs;     // Contains adjacent StreetSegments to GeoCoord curr (in the while loop)
    set<GeoCoord> explored;                 // Contains GeoCoord's that have been explored already
    
      // Process starting GeoCoord start
    toDo.push(start);
    explored.insert(start);

    while ( ! toDo.empty() )
    {
        GeoCoord curr = toDo.front();
        toDo.pop();

          // If we have reached the end, return true
        if (curr == end)
        {
              // Clear the route parameter before re-creating it
            while (!route.empty())
            {
                list<StreetSegment>::iterator itr = route.begin();
                itr = route.erase(itr);
            }
            totalDistanceTravelled = 0;     // Reset the total distance travelled
            recreateRouteHistory(route, start, end, totalDistanceTravelled);   // Creates the path and puts it into route
            return true;
        }
        
          // Call explore on all adjacent StreetSegments to GeoCoord curr
        m_streetMap->getSegmentsThatStartWith(curr, adjacentSegs);      // Erases contents of adjacentSegs vector
        
          // Sort the vector of adjancent StreetSegments to optimize the total route distance
        sortSegments(adjacentSegs, 0, adjacentSegs.size() - 1, end);
        
        for (vector<StreetSegment>::iterator itr = adjacentSegs.begin(); itr != adjacentSegs.end(); itr++)
        {
            GeoCoord start = itr->start;
            GeoCoord end = itr->end;
              // If GeoCoord start has already been explored, do nothing.
            set<GeoCoord>::iterator it = explored.find(end);
            if (it == explored.end())      // If we enter this for loop, then gc was not found in explored
            {
                  // Otherwise, push it onto queue of GeoCoord's toDo
                toDo.push(end);
                explored.insert(end);        // Mark GeoCoord gc as explored
                m_path->associate(end, start);
            }
        }
    }
    
    return false;       // No route found
}

void PointToPointRouterImpl::explore(set<GeoCoord>& explored, queue<GeoCoord>& toDo, const StreetSegment& seg) const
{
    GeoCoord start = seg.start;
    GeoCoord end = seg.end;
      // If GeoCoord start has already been explored, do nothing.
    set<GeoCoord>::iterator itr = explored.find(end);
    if (itr == explored.end())      // If we enter this for loop, then gc was not found in explored
    {
          // Otherwise, push it onto queue of GeoCoord's toDo
        toDo.push(end);
        explored.insert(end);        // Mark GeoCoord gc as explored
        m_path->associate(end, start);
    }
}

void PointToPointRouterImpl::recreateRouteHistory(list<StreetSegment>& route, const GeoCoord& start, const GeoCoord& end, double& totalDistanceTravelled) const
{
      // Trace our hashmap of GeoCoord's -> GeoCoord's, finding the route from start to end, BACKWARDS!
    const GeoCoord* endingG = &end;
    const GeoCoord* startingG;
    startingG = m_path->find(*endingG);
    
      // Get the street name by finding all segments that start with startingG, and finding the street with the same
      // starting and ending GeoCoord's, and extracting the streetname
    vector<StreetSegment> possibleSegs;
    string streetName;
    
      // While we have not reached the start... (we are retracing steps backwards)
    while (*endingG != const_cast<const GeoCoord&>(start))
    {
        m_streetMap->getSegmentsThatStartWith(*startingG, possibleSegs);
        for (vector<StreetSegment>::iterator itr = possibleSegs.begin(); itr != possibleSegs.end(); itr++)
        {
              // Check to see if the StreetSegment has the same ending GeoCoord (it must already have the same starting)
            if (*endingG == itr->end && *startingG == itr->start)
            {
                streetName = itr->name;
                break;
            }
        }
        
          // Checks for error- ensures that streetName was assigned and we found a matching street
        if (streetName == "")
        {
            cerr << "nani! streetName was not assigned, meaning the street was not found";
            return;
        }
        
          // Push this street segment onto our route list
        route.push_front(StreetSegment(*startingG, *endingG, streetName));
        totalDistanceTravelled += distanceEarthMiles(*startingG, *endingG);    // Add to the distance travelled for the route
          // Update starting and ending for the next iteration of the loop
        endingG = startingG;                    // New endingG is the old starting location
        startingG = m_path->find(*startingG);
    }
}

void PointToPointRouterImpl::sortSegments(vector<StreetSegment>& adjacentSegs, int first, int last, const GeoCoord& end) const
{
      // If the size is 0 or 1, there is nothing to sort!
    if (last - first <= 1)
        return;
    
      // Recursively call sortSegments on partitioned sections of adjacentSegs vector
    int pivotIndex = partition(adjacentSegs, first, last, end);
    sortSegments(adjacentSegs, first, pivotIndex - 1, end);
    sortSegments(adjacentSegs, pivotIndex + 1, last, end);
}

int PointToPointRouterImpl::partition(vector<StreetSegment>& adjacentSegs, int low, int high, const GeoCoord& end) const
{
    int pi = low;
    GeoCoord pivot = adjacentSegs[low].end;
    do
    {
        while ( low <= high && distanceEarthMiles(adjacentSegs[low].end, end) <= distanceEarthMiles(pivot, end) )
            low++;
        while ( distanceEarthMiles(adjacentSegs[high].end, end) > distanceEarthMiles(pivot, end) )
            high--;
        if (low < high)
        {
            swapSegInVector(adjacentSegs, low, high);
        }
        
    }
    while (low < high);
    
    swapSegInVector(adjacentSegs, pi, high);
    pi = high;
    return pi;
}

void PointToPointRouterImpl::swapSegInVector(vector<StreetSegment>& adjacentSegs, int index1, int index2) const
{
      // Swaps two segments in a vector
    const StreetSegment temp = adjacentSegs[index1];
    adjacentSegs[index1] = adjacentSegs[index2];
    adjacentSegs[index2] = temp;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
