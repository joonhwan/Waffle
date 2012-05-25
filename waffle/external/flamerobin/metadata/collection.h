/*
  Copyright (c) 2004-2012 The FlameRobin Development Team

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


  $Id: collection.h 2200 2012-01-20 08:31:01Z mghie $

*/

#ifndef FR_COLLECTION_H
#define FR_COLLECTION_H
//-----------------------------------------------------------------------------
#include <algorithm>
#include <iterator>
#include <vector>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "metadata/database.h"

namespace fr {

//-----------------------------------------------------------------------------
class MetadataCollectionBase : public MetadataItem
{
private:
    DatabaseWeakPtr databaseM;
public:
    MetadataCollectionBase(NodeType type, DatabasePtr database,
            const QString& name)
        : MetadataItem(type, database.get(), name), databaseM(database)
    {
    }

    virtual DatabasePtr getDatabase() const
    {
        return DatabasePtr(databaseM);
    }

    virtual bool isSystem() const { return false; }
};
//-----------------------------------------------------------------------------
template <class T>
class MetadataCollection : public MetadataCollectionBase
{
public:
    typedef typename boost::shared_ptr<T> ItemType;
    typedef typename std::vector<ItemType> CollectionType;
    typedef typename CollectionType::iterator iterator;
    typedef typename CollectionType::const_iterator const_iterator;
private:
    CollectionType itemsM;

    iterator getPosition(const QString& name)
    {
        Identifier id(name);
        for (iterator it = itemsM.begin(); it != itemsM.end(); ++it)
        {
            if ((*it)->getIdentifier().equals(id))
                return it;
        }
        return itemsM.end();
    }

    // helper structs for find_if()
    struct FindByAddress
    {
        MetadataItem* itemM;

        FindByAddress(MetadataItem* item) : itemM(item) {}
        bool operator()(const ItemType& item) { return item.get() == itemM; }
    };

    struct InsertionPosByName
    {
        QString nameM;

        InsertionPosByName(const QString& name) : nameM(name) {}
        bool operator()(const ItemType item)
        {
            return item->getName_() > nameM;
        }
    };

public:
    MetadataCollection<T>(NodeType type, DatabasePtr database,
            const QString& name)
        : MetadataCollectionBase(type, database, name)
    {
    }

    // inserts new item into list at correct position to preserve alphabetical
    // order of item names, and returns pointer to it
    ItemType insert(const QString& name)
    {
        iterator pos = std::find_if(itemsM.begin(), itemsM.end(),
            InsertionPosByName(name));
        ItemType item(new T(getDatabase(), name));
        for (unsigned int i = getLockCount(); i > 0; i--)
            item->lockSubject();
        itemsM.insert(pos, item);
        notifyObservers();
        return item;
    }

    // removes item from list
    void remove(MetadataItem* item)
    {
        if (!item)
            return;
        iterator pos = std::find_if(itemsM.begin(), itemsM.end(),
            FindByAddress(item));
        if (pos != itemsM.end())
        {
            itemsM.erase(pos);
            notifyObservers();
        }
    }

    void setItems(QStringList names)
    {
        CollectionType newItems;
        for (int i = 0; i < names.size(); ++i)
        {
            QString itemName(names[i]);
            iterator oldPos = getPosition(itemName);
            if (oldPos == itemsM.end())
            {
                ItemType item(new T(getDatabase(), names[i]));
                newItems.push_back(item);
                for (unsigned int j = getLockCount(); j > 0; j--)
                    item->lockSubject();
            }
            else
                newItems.push_back(*oldPos);
        }

        if (itemsM != newItems)
        {
            itemsM = newItems;
            notifyObservers();
        }
        setChildrenLoaded(true);
    }

    inline iterator begin()
    {
        if (!childrenLoaded())
            return itemsM.end();
        return itemsM.begin();
    };

    inline iterator end()
    {
        return itemsM.end();
    };

    inline bool empty() const
    {
        if (!childrenLoaded())
            return true;
        return itemsM.empty();
    };

    inline void clear()
    {
        if (itemsM.size())
        {
            itemsM.clear();
            notifyObservers();
        }
    };

    ItemType findByName(const QString& name)
    {
        iterator it = getPosition(name);
        return (it != itemsM.end()) ? (*it) : ItemType();
    };

    // returns vector of all subnodes
    virtual bool getChildren(std::vector<MetadataItem *>& temp)
    {
        if (!childrenLoaded())
            return false;
        std::transform(itemsM.begin(), itemsM.end(),
            std::back_inserter(temp), boost::mem_fn(&ItemType::get));
        return !itemsM.empty();
    }

    virtual size_t getChildrenCount() const
    {
        if (!childrenLoaded())
            return 0;
        return itemsM.size();
    }

protected:
    virtual void lockChildren()
    {
        for (iterator it = itemsM.begin(); it != itemsM.end(); ++it)
            (*it)->lockSubject();
    }

    virtual void unlockChildren()
    {
        for (iterator it = itemsM.begin(); it != itemsM.end(); ++it)
            (*it)->unlockSubject();
    }
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif
