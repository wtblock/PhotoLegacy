/////////////////////////////////////////////////////////////////////////////
// Copyright © 2022 by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <map>
#include <memory>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CKeyedCollection
//
// A lightweight, memory‑safe associative container that maps keys (KEY)
// to heap‑allocated objects (TYPE) stored in shared_ptr<TYPE>.
//
// Purpose:
//   • Provides a safer alternative to raw pointer maps
//   • Automatically manages object lifetime via shared_ptr
//   • Offers simple property-style access (Count, Exists[], Items)
//   • Supports diff operations (GetDeletedItems, GetNewItems)
//   • Used throughout PhotoLegacy for metadata tables, lookup maps,
//     month-name tables, image/comment maps, etc.
//
// Design notes:
//   • Keys are stored by value (KEY)
//   • Values are stored as shared_ptr<TYPE>
//   • No copying of TYPE unless caller explicitly requests it
//   • API mirrors MFC-style naming (Exists[], Count, Items)
//   • Works with any key type supporting map<K,V> semantics
/////////////////////////////////////////////////////////////////////////////
template<class KEY, class TYPE>
class CKeyedCollection
{
public:
	/////////////////////////////////////////////////////////////////////////////
	// PAIR_KEY_PTR
	//
	// Convenience typedef for a map entry: (KEY, shared_ptr<TYPE>).
	/////////////////////////////////////////////////////////////////////////////
	typedef pair<KEY, shared_ptr<TYPE>> PAIR_KEY_PTR;

	/////////////////////////////////////////////////////////////////////////////
	// MAP_KEY_PTR
	//
	// Underlying container type: std::map<KEY, shared_ptr<TYPE>>.
	// Provides sorted key ordering and stable iteration.
	/////////////////////////////////////////////////////////////////////////////
	typedef map<KEY, shared_ptr<TYPE>> MAP_KEY_PTR;

protected:
	/////////////////////////////////////////////////////////////////////////////
	// m_mapItems
	//
	// Internal storage for all keyed items. Keys are unique; values are
	// shared_ptr<TYPE> to ensure automatic memory management.
	/////////////////////////////////////////////////////////////////////////////
	MAP_KEY_PTR m_mapItems;

public:
	/////////////////////////////////////////////////////////////////////////////
	// count / Count
	//
	// Returns the number of items stored in the collection.
	// Exposed both as a method and a property.
	/////////////////////////////////////////////////////////////////////////////
	inline int count()
	{
		return (int)m_mapItems.size();
	}
	__declspec(property(get = count))
		int Count;

	/////////////////////////////////////////////////////////////////////////////
	// clear
	//
	// Removes all items from the collection. Because values are stored in
	// shared_ptr, clearing the map automatically frees all objects.
	/////////////////////////////////////////////////////////////////////////////
	void clear()
	{
		m_mapItems.clear();
	}

	/////////////////////////////////////////////////////////////////////////////
	// exists / Exists[]
	//
	// Returns true if the given key exists in the collection.
	// Exists[key] provides property-style syntax.
	/////////////////////////////////////////////////////////////////////////////
	bool exists(KEY key)
	{
		shared_ptr<TYPE> value = find(key);
		return value != nullptr;
	}
	__declspec(property(get = exists))
		bool Exists[];

	/////////////////////////////////////////////////////////////////////////////
	// find
	//
	// Returns the shared_ptr<TYPE> associated with the key, or nullptr if
	// the key is not present. Does not throw.
	/////////////////////////////////////////////////////////////////////////////
	shared_ptr<TYPE> find(KEY key)
	{
		auto pos = m_mapItems.find(key);
		if (pos != m_mapItems.end())
			return pos->second;
		return nullptr;
	}

	/////////////////////////////////////////////////////////////////////////////
	// remove
	//
	// Removes the entry for the given key if it exists.
	// Returns true if an item was removed.
	/////////////////////////////////////////////////////////////////////////////
	bool remove(KEY key)
	{
		if (exists(key))
		{
			m_mapItems.erase(key);
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////
	// add(key, shared_ptr<TYPE>)
	//
	// Adds a new entry to the map. Returns false if the key already exists.
	// Caller supplies the shared_ptr<TYPE>.
	/////////////////////////////////////////////////////////////////////////////
	bool add(KEY key, shared_ptr<TYPE> value)
	{
		if (exists(key))
			return false;

		m_mapItems.insert(PAIR_KEY_PTR(key, value));
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////
	// add(key, const TYPE&)
	//
	// Convenience overload: constructs a shared_ptr<TYPE> automatically
	// using std::make_shared<TYPE>(value).
	/////////////////////////////////////////////////////////////////////////////
	bool add(KEY key, const TYPE& value)
	{
		return add(key, std::make_shared<TYPE>(value));
	}

	/////////////////////////////////////////////////////////////////////////////
	// Items
	//
	// Returns a reference to the underlying map. Allows iteration:
	//
	//     for (auto& kv : myCollection.Items)
	//         ...
	//
	/////////////////////////////////////////////////////////////////////////////
	inline MAP_KEY_PTR& GetItems()
	{
		return m_mapItems;
	}
	__declspec(property(get = GetItems))
		MAP_KEY_PTR Items;

public:
	/////////////////////////////////////////////////////////////////////////////
	// GetDeletedItems
	//
	// Computes the set of items present in "before" but missing in "after".
	// Useful for diffing metadata tables, index updates, etc.
	//
	// Populates "deleted" with shared_ptr copies of the missing items.
	// Returns true if any items were deleted.
	/////////////////////////////////////////////////////////////////////////////
	static bool GetDeletedItems
	(
		CKeyedCollection<KEY, TYPE>& before,
		CKeyedCollection<KEY, TYPE>& after,
		CKeyedCollection<KEY, TYPE>& deleted
	)
	{
		for (auto& node : before.Items)
		{
			const KEY key = node.first;
			if (!after.Exists[key])
				deleted.add(node.first, node.second);
		}
		return deleted.Count > 0;
	}

	/////////////////////////////////////////////////////////////////////////////
	// GetNewItems
	//
	// Computes the set of items present in "after" but missing in "before".
	// Useful for detecting newly added metadata entries.
	//
	// Populates "added" with shared_ptr copies of the new items.
	// Returns true if any items were added.
	/////////////////////////////////////////////////////////////////////////////
	static bool GetNewItems
	(
		CKeyedCollection<KEY, TYPE>& before,
		CKeyedCollection<KEY, TYPE>& after,
		CKeyedCollection<KEY, TYPE>& added
)
	{
		for (auto& node : after.Items)
		{
			const KEY key = node.first;
			if (!before.Exists[key])
				added.add(node.first, node.second);
		}
		return added.Count > 0;
	}

	/////////////////////////////////////////////////////////////////////////////
	// Constructor / Destructor
	//
	// Constructor initializes an empty map.
	// Destructor clears the map, releasing all shared_ptr-managed objects.
	/////////////////////////////////////////////////////////////////////////////
	CKeyedCollection() {}
	virtual ~CKeyedCollection()
	{
		clear();
	}
};
