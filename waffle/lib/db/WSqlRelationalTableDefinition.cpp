#include "WSqlRelationalTableDefinition.h"
#include <QTextStream>
#include <QStringList>

WSqlRelationalTableDefinition::WSqlRelationalTableDefinition()
	: m_dirty(false)
{
}

//virtual
WSqlRelationalTableDefinition::~WSqlRelationalTableDefinition()
{
}

WSqlRelationalTableDefinition&
WSqlRelationalTableDefinition::addField(WSqlFieldDefinition& fd)
{
	m_dirty = true;
	m_fieldDefinitionList << fd;
	return *this;
}

WSqlRelationalTableDefinition&
WSqlRelationalTableDefinition::addRelation(WSqlFieldDefinition& source,
										   WSqlFieldDefinition& target)
{
	m_dirty = true;

	_Relation r;
	r.source = source;
	r.target = target;
	m_relations << r;

	return *this;
}

QString WSqlRelationalTableDefinition::query() const
{
	if (m_dirty) {
		m_queryCached.clear();

		const QString LF = QString::fromLatin1("\n");
		const QString INDENT = QString::fromLatin1("    ");

		QTextStream q(&m_queryCached);

		// list up tables
		QStringList tables;
		foreach(const WSqlFieldDefinition& fd, m_fieldDefinitionList) {
			QString tn = fd.tableName().toUpper();
			if (!tables.contains(tn)) {
				tables << tn;
			}
		}

		q << "SELECT * FROM (" << LF;
		q << INDENT << "SELECT" << LF;

		int index = 0;
		foreach(const WSqlFieldDefinition& fd, m_fieldDefinitionList) {
			if (index) {
				q << "," << LF;
			}
			q << INDENT << INDENT
			  << QString::fromLatin1("%1.\"%2\" as \"%1.%2\"")
				.arg(fd.tableName())
				.arg(fd.fieldName());
			++index;
		}
		q << LF;
		q << INDENT << "FROM" << LF;

		index = 0;
		foreach(const QString& tn, tables) {
			if (index) {
				q << "," << LF;
			}
			q << INDENT << INDENT << tn;
			++index;
		}
		q << LF;
		q << INDENT << "WHERE" << LF;

		index = 0;
		foreach(const _Relation& r, m_relations) {
			if (index) {
				q << LF;
				q << INDENT << INDENT << "AND" << LF;
			}
			q << INDENT << INDENT
			  << QString::fromLatin1("%1.\"%2\" = %3.\"%4\"")
				.arg(r.source.tableName())
				.arg(r.source.fieldName())
				.arg(r.target.tableName())
				.arg(r.target.fieldName());
			++index;
		}

		q << LF;
		q << ")" << LF;

		m_dirty = false;
	}
	return m_queryCached;
}
