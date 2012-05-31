#pragma once

#include "db/WSqlFieldDefinition.h"

class WSqlRelationalTableDefinition
{
public:
    WSqlRelationalTableDefinition();
    virtual ~WSqlRelationalTableDefinition();

	WSqlRelationalTableDefinition& addField(WSqlFieldDefinition& fd);
	WSqlRelationalTableDefinition& addRelation(WSqlFieldDefinition& source,
											   WSqlFieldDefinition& target);
	QString query() const;
	int size() const {
		return m_fieldDefinitionList.size();
	}
	WSqlFieldDefinition operator[](int index) const {
		return m_fieldDefinitionList[index];
	}
protected:
	mutable bool m_dirty;
	mutable QString m_queryCached;
	WSqlFieldDefinitionList m_fieldDefinitionList;
	struct _Relation {
		WSqlFieldDefinition source;
		WSqlFieldDefinition target;
	};
	QList<_Relation> m_relations;
};
