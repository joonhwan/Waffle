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


  $Id: metadataitem.h 2200 2012-01-20 08:31:01Z mghie $

*/

//-----------------------------------------------------------------------------
#ifndef FR_METADATAITEM_H
#define FR_METADATAITEM_H

#include <algorithm>
#include <vector>

#include "core/ObjectWithHandle.h"
#include "core/ProcessableObject.h"
#include "core/Subject.h"
#include "metadata/MetadataClasses.h"
#include "sql/Identifier.h"

namespace fr {

class Dependency;
class MetadataItemVisitor;
//-----------------------------------------------------------------------------
typedef enum { ntUnknown, ntRoot, ntServer, ntDatabase,
    // each item type and (if applicable) its parent type
    ntTable, ntTables, ntView, ntViews, ntProcedure, ntProcedures,
    ntTrigger, ntTriggers, ntGenerator, ntGenerators, ntFunction, ntFunctions,
    ntSysTable, ntSysTables, ntException, ntExceptions,
    ntDomain, ntDomains, ntSysDomain, ntSysDomains,
    ntRole, ntRoles, ntSysRole, ntSysRoles, ntColumn, ntParameter, ntIndex,
    ntLastType
} NodeType;
//-----------------------------------------------------------------------------
NodeType getTypeByName(const QString& name);
QString getNameOfType(NodeType type);
//-----------------------------------------------------------------------------
class MetadataItem: public Subject, public ObjectWithHandle<MetadataItem>,
    public ProcessableObject
{
private:
    MetadataItem* parentM;
    NodeType typeM;

    enum LoadState { lsNotLoaded, lsLoadPending, lsLoaded, lsNotAvailable };
    LoadState childrenLoadedM;
    LoadState descriptionLoadedM;
    LoadState propertiesLoadedM;

    QString descriptionM;
    void ensureDescriptionLoaded();

protected:
    Identifier identifierM;

    virtual void loadDescription();
    virtual void saveDescription(const QString& description);
    void saveDescription(const QString& saveStatement,
        const QString& description);

    virtual void loadProperties();
    void setPropertiesLoaded(bool loaded);

    virtual void doSetChildrenLoaded(bool loaded);
    virtual void loadChildren();
    virtual void lockChildren();
    virtual void unlockChildren();

    void resetPendingLoadData();

public:
    MetadataItem();
    MetadataItem(NodeType type, MetadataItem* parent = 0,
				 const QString& name = QString());
    virtual ~MetadataItem();

    virtual void lockSubject();
    virtual void unlockSubject();

    void getDependencies(std::vector<Dependency>& list, bool ofObject);  // load from db
    void getDependencies(std::vector<Dependency>& list, bool ofObject,
        const QString& field);  // load from db

    // returned shared ptr may be unassigned
    virtual DatabasePtr getDatabase() const;

    virtual void invalidate();

    // items description (in database)
    QString getDescription();
    bool getDescription(QString& description);
    void invalidateDescription();
    void setDescription(const QString& description);

    bool childrenLoaded() const;
    void ensureChildrenLoaded();
    void ensurePropertiesLoaded();
    void loadPendingData();
    bool propertiesLoaded() const;
    void setChildrenLoaded(bool loaded);

    virtual bool getChildren(std::vector<MetadataItem *>& temp);
    virtual size_t getChildrenCount() const { return 0; };

    // returns complete DROP SQL statement
    virtual QString getDropSqlStatement() const;

    // getters/setters
    virtual MetadataItem* getParent() const;
    void setParent(MetadataItem* parent);
    virtual QString getName_() const;
    virtual QString getQuotedName() const;
    virtual Identifier getIdentifier() const;
    virtual void setName_(const QString& name);
    virtual NodeType getType() const;
    void setType(NodeType type);

    // returns the name of the data type (f. ex. TABLE)
    virtual const QString getTypeName() const;

    // returns the item path, currently only used to store settings in config().
    // It could also be used to locate items in the DBH tree.
    virtual const QString getItemPath() const;

    // returns the id QString of the metadata item that contributes to the path. The
    // predefined implementation just returns getId().
    virtual const QString getPathId() const;
    // returns the id of the item (to be saved in config files, etc.).
    // The predefined implementation just returns getName_().
    virtual const QString getId() const;

    // returns true if the metadata item is a system (as opposed to user-defined) item.
    virtual bool isSystem() const;
    static bool hasSystemPrefix(const QString& name);

    virtual void acceptVisitor(MetadataItemVisitor* visitor);
};
//-----------------------------------------------------------------------------
//! masks the object it points to so others see it transparently
class Dependency: public MetadataItem
{
private:
    MetadataItem *objectM;
    std::vector<QString> fieldsM;

public:
    virtual MetadataItem *getParent() const;
    virtual QString getName_() const;
    virtual NodeType getType() const;
    virtual const QString getTypeName() const;
    MetadataItem *getDependentObject() const;

    Dependency(MetadataItem *object);
    QString getFields() const;
    void getFields(std::vector<QString>& fields) const;
    void addField(const QString& name);
    void setFields(const std::vector<QString>& fields);
    bool hasField(const QString& name) const;
    bool operator== (const Dependency& other) const;
    bool operator!= (const Dependency& other) const;
    virtual void acceptVisitor(MetadataItemVisitor* visitor);
};
//-----------------------------------------------------------------------------

} // namespace fr
#endif //FR_METADATAITEM_H
