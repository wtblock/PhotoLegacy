/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>
#include <memory>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CSmartArray<TYPE>
//
// A lightweight, memory‑safe dynamic array that stores objects of TYPE
// using shared_ptr<TYPE>. Provides:
//
//   • Automatic memory management (no raw pointers)
//   • Bounds‑checked access via valid()
//   • Append, insert, remove, resize operations
//   • Property-style access (Items, Count)
//   • Safe get/set semantics
//
// This class is the array counterpart to CKeyedCollection:
//   - CKeyedCollection manages keyed objects (map)
//   - CSmartArray manages ordered objects (vector)
//
// Used throughout PhotoLegacy for collections of metadata objects,
// image entries, UI items, and other ordered lists.
/////////////////////////////////////////////////////////////////////////////
template<class TYPE> class CSmartArray
{
protected:
	/////////////////////////////////////////////////////////////////////////////
	// m_arrItems
	//
	// Internal storage: vector<shared_ptr<TYPE>>.
	// Each element is heap‑allocated and reference‑counted.
	/////////////////////////////////////////////////////////////////////////////
	vector<shared_ptr<TYPE>> m_arrItems;

public:
	/////////////////////////////////////////////////////////////////////////////
	// Items
	//
	// Returns a reference to the underlying vector. Allows iteration:
	//
	//     for (auto& p : myArray.Items)
	//         ...
	//
	/////////////////////////////////////////////////////////////////////////////
	inline vector<shared_ptr<TYPE>>& GetItems()
	{
		return m_arrItems;
	}
	__declspec(property(get = GetItems))
		vector<shared_ptr<TYPE>> Items;

	/////////////////////////////////////////////////////////////////////////////
	// Count
	//
	// Returns the number of elements in the array.
	/////////////////////////////////////////////////////////////////////////////
	inline long GetCount()
	{
		return (long)m_arrItems.size();
	}
	__declspec(property(get = GetCount))
		long Count;

public:
	/////////////////////////////////////////////////////////////////////////////
	// valid
	//
	// Returns true if the index is within bounds (0 <= index < Count).
	// Used by get(), set(), and remove() to ensure safe access.
	/////////////////////////////////////////////////////////////////////////////
	inline bool valid(long lIndex)
	{
		return (0 <= lIndex && lIndex < Count);
	}

	/////////////////////////////////////////////////////////////////////////////
	// clear
	//
	// Removes all elements from the array. Because values are stored in
	// shared_ptr, clearing the vector automatically frees all objects.
	/////////////////////////////////////////////////////////////////////////////
	inline void clear()
	{
		m_arrItems.clear();
	}

	/////////////////////////////////////////////////////////////////////////////
	// resize
	//
	// Resizes the array to nSize. New elements are default‑constructed
	// shared_ptr<TYPE>() (i.e., null pointers).
	/////////////////////////////////////////////////////////////////////////////
	inline void resize(long nSize)
	{
		m_arrItems.resize(nSize);
	}

	/////////////////////////////////////////////////////////////////////////////
	// add
	//
	// Appends a new default‑constructed TYPE object to the array and returns
	// its index. Equivalent to push_back(make_shared<TYPE>()).
	/////////////////////////////////////////////////////////////////////////////
	inline long add()
	{
		m_arrItems.push_back(std::make_shared<TYPE>());
		return Count - 1;
	}

	/////////////////////////////////////////////////////////////////////////////
	// remove
	//
	// Removes the element at the given index, shifting all later elements
	// left by one. Returns true if the element was removed.
	/////////////////////////////////////////////////////////////////////////////
	inline bool remove(long lIndex)
	{
		if (!valid(lIndex))
			return false;

		auto pos = m_arrItems.begin() + lIndex;
		m_arrItems.erase(pos);
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////
	// get
	//
	// Returns the shared_ptr<TYPE> at the given index, or nullptr if the
	// index is out of range.
	/////////////////////////////////////////////////////////////////////////////
	inline shared_ptr<TYPE> get(long lIndex)
	{
		if (valid(lIndex))
			return m_arrItems[lIndex];
		return nullptr;
	}

	/////////////////////////////////////////////////////////////////////////////
	// set
	//
	// Assigns the given shared_ptr<TYPE> to the specified index.
	// Automatically resizes the array if the index is beyond the current size.
	/////////////////////////////////////////////////////////////////////////////
	inline void set(long lIndex, shared_ptr<TYPE> pItem)
	{
		if (lIndex >= Count)
			resize(lIndex + 1);

		if (valid(lIndex))
			m_arrItems[lIndex] = pItem;
	}

	/////////////////////////////////////////////////////////////////////////////
	// append(shared_ptr<TYPE>)
	//
	// Appends an existing shared_ptr<TYPE> to the array.
	/////////////////////////////////////////////////////////////////////////////
	inline void append(shared_ptr<TYPE> pItem)
	{
		m_arrItems.push_back(pItem);
	}

	/////////////////////////////////////////////////////////////////////////////
	// append(const TYPE&)
	//
	// Appends a new element by value. The value is copied into a new
	// shared_ptr<TYPE>.
	/////////////////////////////////////////////////////////////////////////////
	inline long append(const TYPE& value)
	{
		m_arrItems.push_back(std::make_shared<TYPE>(value));
		return Count - 1;
	}

	/////////////////////////////////////////////////////////////////////////////
	// append(TYPE&&)
	//
	// Appends a new element by move. The value is moved into a new
	// shared_ptr<TYPE> without copying.
	/////////////////////////////////////////////////////////////////////////////
	inline long append(TYPE&& value)
	{
		m_arrItems.push_back(std::make_shared<TYPE>(std::move(value)));
		return Count - 1;
	}

public:
	/////////////////////////////////////////////////////////////////////////////
	// Constructor / Destructor
	//
	// Constructor initializes an empty array.
	// Destructor clears the array, releasing all shared_ptr-managed objects.
	/////////////////////////////////////////////////////////////////////////////
	CSmartArray() {}
	virtual ~CSmartArray()
	{
		clear();
	}
};
