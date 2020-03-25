#include "provided.h"
#include <vector>
#include <set>
using namespace std;

class DeliveryOptimizerImpl
{
  public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
  private:
    const StreetMap* m_streetMap;        // Pointer to a fully-constructed and loaded StreetMap object
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
    m_streetMap = sm;   // Fully-constructed and loaded StreetMap object
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
      // Reset old and new crow distances to 0
    oldCrowDistance = 0;
    newCrowDistance = 0;
    
      // Determine the old crow's distance in miles from the depot to each of the successive delivery coordinates
      // in their initial order, and then back to the depot.
    GeoCoord prev = depot;
    for (vector<DeliveryRequest>::iterator itr = deliveries.begin(); itr != deliveries.end(); itr++)
    {
          // Add distance from prev GeoCoord to current GeoCoord
        oldCrowDistance += distanceEarthMiles(prev, itr->location);
        prev = itr->location;   // Update prev GeoCoord
    }
    oldCrowDistance += distanceEarthMiles(prev, depot);     // Add distance from final delivery back to depot
    
      /* Attempt to optimize the delivery order in some way (how is up to you), placing the (possibly) re-ordered deliveries back into the deliveries vector, to reduce the overall travel distance. For instance, if you had a bunch of deliveries that were geographically close to each other, you could place those close to each other in your vector */
    /*
      //IDEA: Find distances from point A to all other points. Prioritize based on distance
    vector<DeliveryRequest> optimizedDeliveries;
    set<DeliveryRequest> alreadyPlanned;        // Holds delivery requests that have already been optimized
    prev = depot;           // Reset prev to our starting location, the depot

      // Compare each delivery by distance from the prev location
    
      // If this is the first delivery we are checking, no need to check against min
    double currentMin = distanceEarthMiles(prev, deliveries[0].location);
    DeliveryRequest* nearestDelivery = &deliveries[0];
    
    for (vector<DeliveryRequest>::iterator start = deliveries.begin(); start != deliveries.end(); start++)
    {
        for (vector<DeliveryRequest>::iterator end = deliveries.begin(); end != deliveries.end(); end++)
        {
              // If we are comparing a delivery to itself, it is meaningless, so skip
            if (start == end)
                continue;
            
            bool hasBeenPlanned = (alreadyPlanned.count(*end) == 1);
              // If this delivery is closer than the currentMin and has not been planned...
            if (distanceEarthMiles(prev, end->location) < currentMin && !hasBeenPlanned)
            {
                  // Update the currentMin and nearestDelivery
                currentMin = distanceEarthMiles(prev, end->location);
                nearestDelivery = &(*end);
            }
        }
        
        optimizedDeliveries.push_back(*nearestDelivery);        // Add the nearest delivery
        newCrowDistance += distanceEarthMiles(prev, nearestDelivery->location); // Update newCrosDistance
        alreadyPlanned.insert(*nearestDelivery);        // Insert into alreadyPlanned
        prev = nearestDelivery->location;       // Update prev GeoCoord
        
    }
    
      // Now the only movement left is from the final delivery location back to the depot
    newCrowDistance += distanceEarthMiles(prev, depot);
    
    deliveries = optimizedDeliveries;       // Update deliveries to our finalized deliveries
    
    */
      // After (optionally) re-ordering the delivery locations to optimize for travel distance,
      // compute the new crow's distance, in miles, for your newly-proposed delivery ordering
      // Determine the old crow's distance in miles from the depot to each of the successive delivery coordinates
      // in their initial order, and then back to the depot.
    prev = depot;
    for (vector<DeliveryRequest>::iterator itr = deliveries.begin(); itr != deliveries.end(); itr++)
    {
          // Add distance from prev GeoCoord to current GeoCoord
        newCrowDistance += distanceEarthMiles(prev, itr->location);
        prev = itr->location;   // Update prev GeoCoord
    }
    newCrowDistance += distanceEarthMiles(prev, depot);     // Add distance from final delivery back to depot
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
