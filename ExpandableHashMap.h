// ExpandableHashMap.h
#include <iostream>
using namespace std;
const int DEFAULT_NUM_BUCKETS = 8;

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }
    
      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    struct Node
    {
        KeyType key;
        ValueType value;
        Node* next = nullptr;
    };

    double m_maxLoadFactor;  // Max load factor (Max # values / Total buckets)
    int m_numBuckets;        // Number of buckets
    int m_maxNumItems;       // Maximum # of associations, dependent on max load factor. Helps to ensure the load factor is not exceeded
    int m_numItems;          // The number of associations in the hashmap
    
    Node** m_hashMap;        // Array of Node pointers
    
    void freeMemory();       // Frees all dynamically-allocated memory (Nodes in all buckets, dynamically-allocated m_hashMap array
    void expandHashMap();    // Doubles the current number of Buckets, rehashes into the new, larger hashmap
    void associateHelper(const KeyType& k, const ValueType& v);   // Inserts Node* n into Node** arr, or updates the ValueType if it already exists
    unsigned int getBucketNumber(const KeyType& key, int numBuckets) const; // Gets a bucket number using hasher function
};

  // Constructor: A newly constructed ExpandableHashMap must have 8 buckets and no associations
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
      // If load factor negative, use the default value (0.5)
    if (maximumLoadFactor < 0.0)
        m_maxLoadFactor = 0.5;
    else
        m_maxLoadFactor = maximumLoadFactor;
    
    m_numBuckets = DEFAULT_NUM_BUCKETS;   // Starts with 8 buckets by default
    
    m_hashMap = new Node*[m_numBuckets];
        // Starts with no associations
    for (int i = 0; i < m_numBuckets; i++)
        m_hashMap[i] = nullptr;

    m_maxNumItems = (int) (m_maxLoadFactor * m_numBuckets);
    m_numItems = 0;     // Starts with no items
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    freeMemory();       // Frees all dynamically-allocated memory
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    freeMemory();       // Frees all dynamically-allocated memory
                        // The maximum load factor should stay the same
    m_numBuckets = DEFAULT_NUM_BUCKETS;     // Resets the number of Buckets to 8, which is the default
    
    m_hashMap = new Node*[m_numBuckets];
        // Starts with no associations
    for (int i = 0; i < m_numBuckets; i++)
        m_hashMap[i] = nullptr;

    m_maxNumItems = (int) (m_maxLoadFactor * m_numBuckets);
    m_numItems = 0;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    cout << m_numItems << " items " << m_numBuckets << " buckets" << endl;
    return m_numItems;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    associateHelper(key, value);
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    unsigned int bucketNum = getBucketNumber(key, m_numBuckets);   // Gets the correct bucket
        // Loops through the linked list (bucket), searching for the key
    Node* cur = m_hashMap[bucketNum];
    while (cur != nullptr)
    {
        if (cur->key == key)
            return &(cur->value);   // If we find an association, return &ValueType
        cur = cur->next;
    }
    
    return nullptr;     // If we get here, there was no association with the parameter
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::freeMemory()
{
    for (int i = 0; i < m_numBuckets; i++)
    {
        Node* cur = m_hashMap[i];
            // Delete all (dynamically-allocated) nodes in each bucket (linked list)
        while (cur != nullptr)
        {
            Node* temp = cur->next;
            delete cur;
            cur = temp;
        }
    }
    
    delete [] m_hashMap;      // Delete the dynamically allocated array
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::expandHashMap()
{
    int newNumBuckets = m_numBuckets * 2;   // The new hashmap will have twice as many buckets
    m_maxNumItems = (int) (m_maxLoadFactor * newNumBuckets);
    
      // Allocate the new hashmap and initialize all buckets (head nodes) to nullptr
    Node** newHashMap;
    newHashMap = new Node*[newNumBuckets];
    for (int i = 0; i < newNumBuckets; i++)
        newHashMap[i] = nullptr;
    
    m_numItems = 0;     // We are freeing the memory of the old hashmap... size is 0
    
    for (int i = 0; i < m_numBuckets; i++)
    {
        Node* cur = m_hashMap[i];
        while (cur != nullptr)
        {
              // Essentially copy over all nodes into the new hash map to rehash
            unsigned int bucket = getBucketNumber(cur->key, newNumBuckets);
            if (newHashMap[bucket] == nullptr)
            {
                  // If the bucket is empty
                Node* n = new Node;
                n->next = nullptr;
                n->key = cur->key;
                n->value = cur->value;
                newHashMap[bucket] = n;
                m_numItems++;
            }
            else if (newHashMap[bucket]->key == cur->key)
                newHashMap[bucket]->value = cur->value;
            else
            {
                Node* p = newHashMap[bucket];
                while (p->next != nullptr)
                {
                    p = p->next;
                }
                Node* n = new Node;
                p->next = n;
                n->key = cur->key;
                n->value = cur->value;
                n->next = nullptr;
                m_numItems++;
            }
            cur = cur->next;                   // Iterate to the next node to copy it over
        }
    }
    
    freeMemory();               // Free the old hashmap
    m_numBuckets = newNumBuckets;
    m_hashMap = newHashMap;     // Reassigns the pointer of old hashmap to new hashmap
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associateHelper(const KeyType& k, const ValueType& v)
{
    ValueType* value = find(k);
      // If the association is found, simply update the value
    if (value != nullptr)
    {
        *value = v;
    }
      // If the association is not found, insert Node* n at the end of the linked list
    else
    {
            // We are preparing to insert an item; check if the hashmap needs to be expanded
        if (m_numItems + 1 > m_maxNumItems)
            expandHashMap();
        
        unsigned int bucketNum = getBucketNumber(k, m_numBuckets);
        Node* cur = m_hashMap[bucketNum];
        
            // If there are no Nodes, simply insert as the first Node
        if (cur == nullptr)
        {
            Node* n = new Node;
            n->next = nullptr;
            n->key = k;
            n->value = v;
            m_hashMap[bucketNum] = n;
        }
        else
        {
                // Loop to the final Node (or simply stay if there are no Nodes)
            while (cur->next != nullptr)
            {
                cur = cur->next;
            }
            
                // Allocate a new Node with KeyType k, ValueType v. Insert it here
            Node* n = new Node;
            cur->next = n;
            n->next = nullptr;
            n->key = k;
            n->value = v;
        }
        
        m_numItems++;   // Increment the number of associations in the hashmap
    }
}

template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getBucketNumber(const KeyType& key, int numBuckets) const
{
    unsigned int hasher(const KeyType& k);
    unsigned int bucketNum = hasher(key);
    return bucketNum % numBuckets;
}
