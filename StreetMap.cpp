#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
  public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
    
  private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>>* m_hashMap;
    /*
    ExpandableHashMap<GeoCoord, vector<StreetSegment*>>* m_hashMap;
    vector<StreetSegment> m_streetSegments;
    */
      // Adds an association of GeoCoord g and StreetSegment* s to the hashmap
    void addToHashMap(const GeoCoord& g, StreetSegment s);
      // Returns true if the string parameter is a street name. Otherwise, return false
    bool isStreetName(string str);
};

StreetMapImpl::StreetMapImpl()
{
    m_hashMap = new ExpandableHashMap<GeoCoord, vector<StreetSegment>>;
}

StreetMapImpl::~StreetMapImpl()
{
    delete m_hashMap;
}

  // Load all data from map data file into the expandable hash map
bool StreetMapImpl::load(string mapFile)
{
    m_hashMap->reset();     // Reset the hashmap to load the map data file
    
      // If there is a failure to read the file, return false
    ifstream infile(mapFile);
    if (!infile)
    {
        cerr << "Error: Cannot open mapdata.txt!" << endl;
        return false;
    }
    
      // Repeatedly read lines from the file
    string line;
    string streetName;
    while(getline(infile, line))
    {
        istringstream iss(line);
        string startingLat, startingLong, endingLat, endingLong;
        
        string temp;
          // Get the steet's name (meaning there is one string and the first char is not a digit
        if (isStreetName(line))
        {
            streetName = "";
            while (iss >> temp)
            {
                if (streetName.size() != 0)
                    streetName += " ";
                streetName += temp;
            }
            continue;       // If this line is a street name, do not add it to the hashmap as a StreetSegment
        }
        
          // If the line does not have GeoCoords (meaning it is a line representing the number of segments), skip it
        if (!(iss >> startingLat >> startingLong >> endingLat >> endingLong))
            continue;   // If this line is a list of GeoCoords, do not add it to the hashmap as a StreetSegment
        
        GeoCoord s(startingLat, startingLong);
        GeoCoord e(endingLat, endingLong);
        StreetSegment segment(s, e, streetName);
        StreetSegment reverseS(e, s, streetName);
        addToHashMap(s, segment);
        addToHashMap(e, reverseS);
        /*
        StreetSegment* segment = new StreetSegment(s, e, streetName);
        addToHashMap(s, segment);
        m_streetSegments.push_back(*segment);
        StreetSegment* reverseS = new StreetSegment(e, s, streetName);
        addToHashMap(e, reverseS);
        m_streetSegments.push_back(*reverseS);
        delete segment;
        delete reverseS;
         */
    }
    
    return true;    // File automatically closed as stream goes out of scope
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    vector<StreetSegment>* v = m_hashMap->find(gc);
    
      // If there weren't any StreetSegment's found, return false immediately
    if (v == nullptr)
        return false;
    
      // Erase segs, since we found a StreetSegment if we got here
    while (!segs.empty())
    {
        vector<StreetSegment>::iterator itr = segs.begin();
        itr = segs.erase(itr);
    }
    
      // Iterate through each StreetSegment, inserting the ones that start with parameter gc
    for (vector<StreetSegment>::iterator itr = v->begin(); itr != v->end(); itr++)
    {
        segs.push_back(*itr);
    }
    
    return true;
}

void StreetMapImpl::addToHashMap(const GeoCoord& g, StreetSegment s)
{
      // If the GeoCoord is already an existing key, simply add the street segment to the vector
    if (m_hashMap->find(g))
    {
            // Get the same vector of StreetSegment pointers
        vector<StreetSegment>* v = m_hashMap->find(g);
            // Add the segment to the existing vector
        v->push_back(s);
    }
      // Else, if it is a new key, associate a new vector
    else
    {
          // Create a new vector
        vector<StreetSegment> v;
        v.push_back(s);
          // Associate it in the hashmap
        m_hashMap->associate(g, v);
    }
}

bool StreetMapImpl::isStreetName(string str)
{
      // If any character is an alphabetical character, it must be a street name
    for (int i = 0; i < str.length(); i++)
    {
        if (isalpha(str[i]))
            return true;
    }
    return false;   // If no alphabetical characters were found, it is not a street name
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
