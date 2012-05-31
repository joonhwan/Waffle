#include "WSqlFirebirdTableBuilder.h"
#include <QDebug>
#include <QTextStream>

WSqlFirebirdTableBuilder::WSqlFirebirdTableBuilder(const WSqlTableDefinition& definition,
												   QSqlDatabase db)
	: m_db(db)
	, m_tableDefinition(definition)
{
	fr::ServerPtr server(new fr::Server());
	server->setName_(QLatin1String("TempName"));
	server->setHostname(db.hostName());

	fr::RootPtr root(new fr::Root);
	root->addServer(server);

	m_frDb.reset(new fr::Database());
	m_frDb->setServer(server);
	m_frDb->setUsername(db.userName());
	m_frDb->setRawPassword(db.password());
	m_frDb->setPath(db.databaseName());
	m_frDb->connect(db.password());

	if (!m_frDb->isConnected()) {
		m_frDb.reset();
	}
}

//virtual
WSqlFirebirdTableBuilder::~WSqlFirebirdTableBuilder()
{
}

bool WSqlFirebirdTableBuilder::tryGetUpdateQueryString(QStringList& updateQuery)
{
	bool okToUpdate = false;

	m_lastError.clear();

	do
	{
		// check table exists.
		fr::TablePtr table;
		fr::TablesPtr tables = m_frDb->getTables();
		for(fr::Tables::iterator it = tables->begin();
			it != tables->end();
			++it) {

			QString tableName = (*it)->getName_().toUpper();
			if (tableName == m_tableDefinition.tableName().toUpper()) {
				table = *it;
				break;
			}
		}
		if (!table) {
			// 테이블이 아예 없다면,
			// - foreign key로 참조하는 table이 존재하고,
			// - foreign key가 참조하는 table의 key가 존재하며,
			// - foreign key가 참조하는 table의 key의 data type이 'bigint' 이며,
			// - foreign key가 참조하는 table의 key가 primary key
			// 의 조건이 맞는 경우에 한해 update query를 table 생성 query로
			// 반환한다.
			bool foreignKeyProblem = false;
			foreach(const WSqlFieldDefinition& fd, m_tableDefinition.fields()) {
				if (fd.isForeignKey()) {
					foreignKeyProblem = true;

					QString foreignTableName = fd.property("foreignTable").toString().toUpper();
					QString foreignTableKeyName = fd.property("foreignTableKey").toString().toLower();

					fr::TablePtr foreignTable;
					for(fr::Tables::iterator it = tables->begin();
						it != tables->end();
						++it) {

						QString tableName = (*it)->getName_().toUpper();
						if (tableName == foreignTableName) {
							foreignTable = *it;
							break;
						}
					}
					if (!foreignTable) {
						m_lastError +=
							QObject::tr("foreign key[%1]'s foreign table[%2] does not exist!\n")
							.arg(fd.fieldName())
							.arg(foreignTableName);
						break;
					}

					WSqlFieldDefinitionList foreignTableDefinitionList = getFieldDefinition(foreignTable);
					if (0==foreignTableDefinitionList.size()) {
						m_lastError +=
							QObject::tr("foreign key[%1]'s foreign table does not "
										"have WSqlFirebirdTableBuilder compliant structure.\n")
							.arg(fd.fieldName());
						break;
					}

					const WSqlFieldDefinition* pfdForeignTable = 0;
					foreach(const WSqlFieldDefinition& fdForeignTable, foreignTableDefinitionList) {
						if (fdForeignTable.fieldName().toLower() == foreignTableKeyName) {
							pfdForeignTable = &fdForeignTable;
							break;
						}
					}
					if (!pfdForeignTable) {
						m_lastError +=
							QObject::tr("foreign key[%1]'s foreign table key[%2] does not exist "
										"in the table[%3]!")
							.arg(fd.fieldName())
							.arg(foreignTableKeyName)
							.arg(foreignTableName);
						break;
					}
					if (pfdForeignTable->dataType().toLower() != "bigint") {
						m_lastError +=
							QObject::tr("foreign key[%1]'s foreign table key[%2] is not 'bigint' type!")
							.arg(fd.fieldName())
							.arg(foreignTableKeyName);
						break;
					}
					if (!pfdForeignTable->isPrimaryKey()) {
						m_lastError +=
							QObject::tr("foreign key[%1]'s foreign table key[%2] is not primary key!")
							.arg(fd.fieldName())
							.arg(foreignTableKeyName);
						break;
					}

					foreignKeyProblem = false;
				}
			}
			if (!foreignKeyProblem) {
				updateQuery = createQueryString();
				okToUpdate = true;
			}
			break;
		}

		WSqlFieldDefinitionList fieldsInDb = getFieldDefinition(table);
		if (!fieldsInDb.size()) {
			m_lastError +=
				QObject::tr("Exsiting table[%1]'s structure does not match "
							"the one that WSqlFirebirdTableBuilder generates.\n")
				.arg(table->getName_());
			break;
		}

		bool fieldOk = false;
		// DB에 정의된 테이블이 이미 있는 경우, 새로운 Field의 추가만 지원하도록 한다.
		// 기타의 경우에 대한 처리는 너무 복잡하다.
		foreach(const WSqlFieldDefinition& fdInDb, fieldsInDb) {
			fieldOk = false;

			QString fieldName = fdInDb.fieldName().toLower();
			const WSqlFieldDefinition* pfd = 0;
			foreach(const WSqlFieldDefinition& fd, m_tableDefinition.fields()) {
				if (fd.fieldName().toLower() == fieldName) {
					pfd = &fd;
					break;
				}
			}
			// - DB에는 있지만, 정의상 누락된 경우는 Table Update할 수 없다.
			bool fieldRemovedFromDefinition = (pfd==0);
			if (fieldRemovedFromDefinition) {
				m_lastError +=
					QObject::tr("field[%1] has been removed from definition!. "
								"Only new field can be added.\n")
					.arg(fieldName);
				break;
			}

			Q_ASSERT(pfd);

			// - DB 에 있는 Field와 유형이 다른 경우는 Table Update할 수 없다.
			if (pfd->dataType().toLower() != fdInDb.dataType().toLower()) {
				m_lastError +=
					QObject::tr("field[%1]'s data type is different!.\n")
					.arg(fieldName);
				break;
			}
			if (pfd->attribute() != fdInDb.attribute()
				|| pfd->properties() != fdInDb.properties()) {
				m_lastError +=
					QObject::tr("field[%1]'s attribute(PrimaryKey, ForeignKey, Not Null, ...) is different!.\n")
					.arg(fieldName);
				break;
			}
			if (pfd->hasDefaultValue() &&
				fdInDb.hasDefaultValue() &&
				pfd->defaultValue() != fdInDb.defaultValue()) {
				m_lastError +=
					QObject::tr("field[%1]'s default value settings is different!\n")
					.arg(fieldName);
				break;
			}

			fieldOk = true;
		}
		if (!fieldOk) {
			break;
		}

		// 여기 까지 오면, 기존에 없는 Field의 추가 준비가 된 상태.

		// DB에는 없는 새로운 Field의 경우, Primary Key 가 아닌 경우에 한해서 처리가 가능하다.
		WSqlFieldDefinitionList newFieldList;
		foreach(const WSqlFieldDefinition& fd, m_tableDefinition.fields()) {
			fieldOk = false;

			QString fieldName = fd.fieldName().toLower();

			const WSqlFieldDefinition* pfdInDb = 0;
			foreach(const WSqlFieldDefinition& fdInDb, fieldsInDb) {
				if (fdInDb.fieldName().toLower() == fieldName) {
					pfdInDb = &fdInDb;
					break;
				}
			}

			bool fieldNotFound = (pfdInDb==0);
			if (fieldNotFound) {
				if (fd.isPrimaryKey()) {
					m_lastError +=
						QObject::tr("missing field[%1] is primary key!")
						.arg(fieldName);
					break;
				}

				newFieldList << fd;
			}
			fieldOk = true;
		};
		if (!fieldOk) {
			break;
		}

		updateQuery = getTableSql(newFieldList, true);

		// ensure field order.
		int colIndex = 1;  // 1-base
		foreach(const WSqlFieldDefinition& fd, m_tableDefinition.fields()) {
			fr::Identifier fieldName(fd.fieldName().toLower());
			updateQuery <<
				QString::fromLatin1("ALTER TABLE %1 ALTER %2 POSITION %3")
				.arg(m_tableDefinition.tableName().toUpper())
				.arg(fieldName.getQuoted())
				.arg(colIndex++);
		}

		okToUpdate = true;

	} while (0);

	return okToUpdate;
}

QStringList WSqlFirebirdTableBuilder::createQueryString()
{
	return getTableSql(m_tableDefinition.fields(), false);
}

void WSqlFirebirdTableBuilder::test()
{
	if (!m_frDb) {
		qDebug() << "no connected database!";
		return;
	}

	fr::TablesPtr tables = m_frDb->getTables();
	for(fr::Tables::const_iterator it = tables->begin();
		it != tables->end();
		++it) {

		const fr::TablePtr& table = *it;

		qDebug() << "Table [" << table->getName_() << "]";

		table->ensurePropertiesLoaded();
		if (table->propertiesLoaded()) {
			qDebug() << "  owner:" << table->getOwner();
		}
		table->ensureChildrenLoaded();
		if (table->childrenLoaded()) {
			for (fr::ColumnPtrs::const_iterator ppCol = table->begin();
				 ppCol != table->end();
				 ++ppCol) {

				const fr::ColumnPtr& col = *ppCol;

				QString fieldName = col->getName_();
				QString dataType = QLatin1String("unknown");
				QString charset = QLatin1String("");
				QString collate = col->getCollation();
				bool notNull = col->hasNotNullConstraint();
				QString defaultValue = col->getDefault();

				if (fr::DomainPtr domain = col->getDomain()) {
					if (domain->isSystem()) {
						dataType = domain->getDatatypeAsString();
						charset = domain->getCharset();
						if (!charset.isEmpty()) {
							if (m_frDb && m_frDb->getDatabaseCharset() == charset) {
								charset = "";
							}
							if (m_frDb && m_frDb->isDefaultCollation(charset, collate)) {
								collate = "";
							}
						}
					} else {
						dataType = domain->getName_();
					}
				} else {
					Q_ASSERT(FALSE);
				}
				// QString description = col->getDescription();

				qDebug() << "  "
						 << "Name:" << fieldName << ", "
						 << "Type:" << dataType << ", "
						 << "Not Null?:" << notNull << ", "
						 << "Default:" << defaultValue << ", "
						 << "Charset:" << charset;
			}
		}
		fr::TriggersPtr triggers = m_frDb->getTriggers();
		qDebug() << "  Trigger [" << triggers->getChildrenCount() << "]";

		for (fr::Triggers::const_iterator ppTrigger = triggers->begin();
			 ppTrigger != triggers->end();
			 ++ppTrigger) {
			const fr::TriggerPtr& trigger = *ppTrigger;

			// (QString& object, bool& active, int& position,
			// QString& type, bool& isDatabaseTrigger)
			QString name = trigger->getName_();
			QString object;
			bool active = false;
			int position = 0;
			QString type;
			bool isDatabaseTrigger = false;
			QString source = trigger->getSource();
			trigger->getTriggerInfo(object, active, position, type, isDatabaseTrigger);
			qDebug() << "Name:" << name;
			qDebug() << "  " << "object: " << object;
			qDebug() << "  " << "active? : " << active;
			qDebug() << "  " << "position: " << position;
			qDebug() << "  " << "type: " << type;
			qDebug() << "  " << "isDbTrigger?: " << isDatabaseTrigger;
			qDebug() << "  " << source;
			qDebug() << " ----------------------------- ";
		}
	}
}

WSqlFieldDefinitionList WSqlFirebirdTableBuilder::getFieldDefinition(fr::TablePtr& t)
{
	bool done = false;
	WSqlFieldDefinitionList fdList;

	do
	{
		t->ensureChildrenLoaded();
		t->ensurePropertiesLoaded();

		fr::DatabasePtr db = t->getDatabase();
		db->ensureChildrenLoaded();
		db->ensurePropertiesLoaded();

		bool error = false;

		// generate column list
		for (fr::ColumnPtrs::iterator it = t->begin(); it!=t->end(); ++it) {
			// error if any 'break'.
			error = true;

			fr::Column& c = *(*it).get();
			QString fieldName = c.getName_();

			// computed source cannot be a field definition(it is too complicated to convert!?)
			QString computed = c.getComputedSource();
			if (!computed.isEmpty()) {
				m_lastError +=
					QObject::tr("column[%1] has computed source(%2).\n")
					.arg(fieldName)
					.arg(computed);
				break;
			}

			QString dataType;
			QString charset;
			QString collate = c.getCollation();
			if (fr::DomainPtr d = c.getDomain())
			{
				if (d->isSystem())
				{
					dataType = d->getDatatypeAsString();
					QString _charset = d->getCharset();
					if (!_charset.isEmpty())
					{
						if (!db || db->getDatabaseCharset() != _charset) {
							charset = _charset;
						}
						if (db && db->isDefaultCollation(_charset, collate)) {
							collate.clear();    // don't show default collations
						}
					}
				} else {
					m_lastError +=
						QObject::tr("column[%1] has domain type(%2).\n")
						.arg(fieldName)
						.arg(d->getName_());
					break;
				}
			} else {
				m_lastError +=
					QObject::tr("column[%1] has special source(%2).\n")
					.arg(fieldName)
					.arg(c.getSource());
				break;
			}

			if (!collate.isEmpty()) {
				m_lastError +=
					QObject::tr("column[%1] has collate data(%2).\n")
					.arg(fieldName)
					.arg(collate);
				break;
			}
			if (!charset.isEmpty()) {
				m_lastError +=
					QObject::tr("column[%1] has a non default charset(%2).\n")
					.arg(fieldName)
					.arg(charset);
				break;
			}

			int attribute = 0;
			QString defaultValue = c.getDefault();
			if (c.hasDefault()) {
				attribute |= Wf::DbHasDefault;
				if (!defaultValue.isEmpty()) {
					QString trimmed = defaultValue.trimmed();
					defaultValue = trimmed;

					// remove quotes if any
					// TODO IMPROVEME
					if (trimmed.startsWith(QLatin1Char('\"'))
						|| trimmed.startsWith(QLatin1Char('\''))) {
						if (trimmed.length() > 2) {
							defaultValue = trimmed.mid(1, trimmed.length()-2);
						}
					}
				}
			}
			if (c.hasNotNullConstraint()) {
				attribute |= Wf::DbNotNull;
			}

			WSqlFieldDefinition fd(fieldName, dataType, attribute, defaultValue);
			fdList << fd;

			error = false;
		}
		if (error) {
			break;
		}

		// get primary key info
		fr::PrimaryKeyConstraint* pk = t->getPrimaryKey();
		if (pk) {
			if (pk->isSystem()) {
				m_lastError +=
					QObject::tr("system primary key constraint was detected!");
				break;
			}
			if (pk->size()!=1) {
				m_lastError +=
					QObject::tr("primary key comprising multiple column was detected");
				break;
			}
			QString pkColName = *(pk->begin());
			QString pkName = pk->getName_();
			if (pkName != normalPrimaryKeyConstraintName(t->getName_())) {
				m_lastError +=
					QObject::tr("primary key constraint's name is not expected value!");
				break;
			}

			// if (t->getIndices()->size() > 0) {
			// 	log << "primary key with indices was detected!";
			// 	break;
			// }

			QString generatorName = normalGeneratorNameOf(t->getName_(),
														  pkColName);
			fr::GeneratorPtr generator;
			fr::GeneratorsPtr generators = db->getGenerators();
			for (fr::Generators::iterator it = generators->begin();
				 it != generators->end();
				 ++it) {
				fr::GeneratorPtr g = *it;
				if (g->getName_().toUpper() == generatorName) {
					generator = g;
					break;
				}
			}
			if (!generator) {
				m_lastError +=
					QObject::tr("no expected generator [%1] for column(%2) was detected!\n")
					.arg(generatorName)
					.arg(pkName);
				break;
			}

			QString triggerName = normalTriggerNameForGenerator(t->getName_(),
																pkColName);
			fr::TriggerPtr trigger;
			fr::TriggersPtr triggers = db->getTriggers();
			for (fr::Triggers::iterator it = triggers->begin();
				it != triggers->end();
				++it) {
				fr::TriggerPtr t = *it;
				if (t->getName_().toUpper() == triggerName) {
					trigger = t;
					break;
				}
			}
			if (!trigger) {
				m_lastError +=
					QObject::tr("no expected trigger [%1] for column(%2) was detected.\n")
					.arg(triggerName)
					.arg(pkName);
				break;
			}

			QString triggerObject, triggerType;
			bool isTriggerActive = false;
			bool isDbTrigger = false;
			int triggerPosition = 0;
			trigger->getTriggerInfo(triggerObject,
									isTriggerActive,
									triggerPosition,
									triggerType,
									isDbTrigger);

			QString triggerRelation = trigger->getTriggerRelation().toUpper();
			if (isDbTrigger ||
				triggerRelation != t->getName_().toUpper()) {
				m_lastError +=
					QObject::tr("trigger[%1]'s table [%2] was not the expected one(%3).\n")
					.arg(triggerName)
					.arg(triggerRelation)
					.arg(t->getName_().toUpper());
				break;
			}
			if (!isTriggerActive) {
				m_lastError +=
					QObject::tr("trigger[%1] is not active.\n")
					.arg(triggerName);
				break;
			}
			if (triggerPosition!=0) {
				m_lastError +=
					QObject::tr("trigger[%1]'s position(%2) was not 0(zero).\n")
					.arg(triggerName)
					.arg(triggerPosition);
				break;
			}

			QString source = trigger->getSource();
			// TODO introduce more intelligent check for the trigger sql statement.
			if (-1 == source.indexOf(generatorName)) {
				m_lastError +=
					QObject::tr("trigger[%1]'s sql statement does not contain "
								"expected generator name (%2). "
								"see the sql :\n%3\n")
					.arg(triggerName)
					.arg(generatorName)
					.arg(source);
				break;
			}
			if (-1 == source.indexOf(QString::fromLatin1("NEW.%2").arg(pkColName),
									 0, Qt::CaseInsensitive)
				&& -1 == source.indexOf(QString::fromLatin1("NEW.\"%2\"").arg(pkColName),
										0, Qt::CaseInsensitive)) {
				m_lastError +=
					QObject::tr("trigger[%1]'s sql statement does not contain "
								"expected field name (%2). "
								"see the sql :\n%3\n")
					.arg(triggerName)
					.arg(pkName)
					.arg(source);
				break;
			}

			// now we can think this pk is `normal' one...
			bool touched = false;
			for (WSqlFieldDefinitionList::iterator it = fdList.begin();
				 it != fdList.end();
				 ++ it) {
				WSqlFieldDefinition& fd = *it;
				if (fd.fieldName().toLower() == pkColName.toLower()) {
					touched  = true;
					fd.addAttribute(Wf::DbPrimaryKey | Wf::DbAutoIncrement);
					break;
				}
			}
			Q_ASSERT(touched);
		}

		std::vector<fr::ForeignKey>* fkList = t->getForeignKeys();
		foreach(const fr::ForeignKey& fk, *fkList) {
			error = true;		// error if any 'break'

			QString fkName = fk.getName_();
			QString foreignTable = fk.getReferencedTable();
			if (fk.size()!=1) {
				m_lastError +=
					QObject::tr("foreign key constraint[%1] has more then one source columns!\n")
					.arg(fkName);
				break;
			}
			QString sourceFieldName = *(fk.begin());

			const std::vector<QString>& foreignTableKeyList = fk.getReferencedColumns();
			if (foreignTableKeyList.size() != 1) {
				m_lastError +=
					QObject::tr("foreign key[%1] has more then one foreign table's columns!")
					.arg(sourceFieldName);
				break;
			}
			QString foreignTableKey = *(foreignTableKeyList.begin());

			// std::vector<fr::Index>* indices = t->getIndices();
			// if (indices->size() > 0) {
			// 	log << "foreign key[" << sourceFieldName << "] has indices!";
			// 	break;
			// }

			QString normalAction = QString::fromLatin1("ON UPDATE CASCADE ON DELETE CASCADE");
			QString action;
			QString upd = fk.getUpdateAction();
			if (!upd.isEmpty() && upd != QString::fromLatin1("RESTRICT"))
				action += QString::fromLatin1("ON UPDATE ") + upd;
			QString del = fk.getDeleteAction();
			if (!del.isEmpty() && del != QString::fromLatin1("RESTRICT"))
				action += QString::fromLatin1(" ON DELETE ") + del;
			if (action != normalAction) {
				m_lastError +=
					QObject::tr("foreign key[%1]'s action does not have normal one. "
								"detected action:[%2], expected action:[%3]")
					.arg(sourceFieldName)
					.arg(action)
					.arg(normalAction);
				break;
			}

			WSqlFieldDefinition* pfd = 0;
			for(WSqlFieldDefinitionList::iterator it = fdList.begin();
				it != fdList.end();
				++it) {
				WSqlFieldDefinition& fd = *it;
				if (fd.fieldName().toLower() == sourceFieldName.toLower()) {
					pfd = &fd;
					break;
				}
			}
			if (!pfd) {
				m_lastError +=
					QObject::tr("foreign key[%1]'s is not defined normally!")
					.arg(sourceFieldName);
				break;
			}
			if (pfd->dataType().toLower() != "bigint") {
				m_lastError +=
					QObject::tr("foreign key[%1]'s type must be \"bigint\"!")
					.arg(sourceFieldName);
				break;
			}

			pfd->setDataType("bigint");
			pfd->setAttribute(Wf::DbNotNull | Wf::DbForeignKey);
			pfd->setProperty("foreignTable", foreignTable);
			pfd->setProperty("foreignTableKey", foreignTableKey);
			error = false;
		}
		if (error) {
			break;
		}

		done = true;
	} while (0);

	if (!done) {
		// when error, no items should be returned.
		fdList.clear();
	}

	return fdList;
}

QString WSqlFirebirdTableBuilder::normalPrimaryKeyConstraintName(const QString& tableName) const
{
	return QLatin1String("PK_") + tableName.toUpper();
}

QString WSqlFirebirdTableBuilder::normalForeignKeyConstraintName(const QString& fieldName) const
{
	return QLatin1String("FK_") + fieldName.toUpper();
}

QString WSqlFirebirdTableBuilder::normalGeneratorNameOf(const QString& tableName,
														const QString& fieldName) const
{
	return QString::fromLatin1("GEN_%1_OF_%2")
		.arg(fieldName.toUpper())
		.arg(tableName.toUpper());
}

QString WSqlFirebirdTableBuilder::normalTriggerNameForGenerator(const QString& tableName,
																const QString& fieldName) const
{
	return QString::fromLatin1("TRIG_%1_OF_%2_BI")
		.arg(fieldName.toUpper())
		.arg(tableName.toUpper());
}

bool WSqlFirebirdTableBuilder::valueNeedQuoteFor(const QString& dataType) const
{
	bool needQuote = false;
	QString ldt = dataType.toLower().trimmed();
	if (ldt.startsWith("varchar")
		|| ldt.startsWith("char")
		|| ldt.startsWith("date")
		|| ldt.startsWith("time")) {
		needQuote = true;
	}
	return needQuote;
}

QStringList WSqlFirebirdTableBuilder::getTableSql(const WSqlFieldDefinitionList& fieldDefinitionList, bool useAlterStatement) const
{
	QStringList queryList;

	if (fieldDefinitionList.empty()) {
		return queryList;
	}

	fr::Identifier tableName(m_tableDefinition.tableName().toUpper());

	QString INDENT = QString::fromLatin1("    ");
	// CREATE TABLE table_name
	// (
	//     column_name {< datatype> | COMPUTED BY (< expr>) | domain}
	//         [DEFAULT { literal | NULL | USER}] [NOT NULL]
	//     ...
	//     CONSTRAINT constraint_name
	//         PRIMARY KEY (column_list),
	//         UNIQUE      (column_list),
	//         FOREIGN KEY (column_list) REFERENCES other_table (column_list),
	//         CHECK       (condition),
	//     ...
	// );
	//
	// or....
	//
	// ALTER TABLE table_name ADD
	//         column_name {< datatype> | COMPUTED BY (< expr>) | domain}
	//         [DEFAULT { literal | NULL | USER}] [NOT NULL]
	if (useAlterStatement) {
		foreach(const WSqlFieldDefinition& fd, fieldDefinitionList) {
			fr::Identifier id(fd.fieldName());

			QString query;
			QTextStream s(&query);
			s << "ALTER TABLE " << tableName.getQuoted()
			  << " ADD " << id.getQuoted() << " " << fd.dataType().toLower();
			if (fd.hasDefaultValue()) {
				s << " DEFAULT ";
				if (fd.defaultValue().isEmpty()) {
					s << "NULL";
				} else {
					if (valueNeedQuoteFor(fd.dataType())) {
						s << "'" << fd.defaultValue() << "'";
					} else {
						s << fd.defaultValue();
					}
				}
			}
			if (fd.isNotNull()) {
				s << " NOT NULL";
			}
			s.flush();

			queryList << query;
		}
		// "ALTER TABLE  ..." sql should not contain primary key constraint.?!
		foreach(const WSqlFieldDefinition& fd, fieldDefinitionList) {
			Q_ASSERT(!fd.isPrimaryKey());
		}
	} else {
		QString query;
		QTextStream s(&query);

		s << "CREATE TABLE " << tableName.getQuoted() << "\n";
		s << "(" << "\n";
		int index = 0;
		foreach(const WSqlFieldDefinition& fd, fieldDefinitionList) {
			fr::Identifier id(fd.fieldName());
			if (index) {
				s << "," << "\n";
			}
			s << id.getQuoted() << " " << fd.dataType().toLower();
			if (fd.hasDefaultValue()) {
				s << " DEFAULT ";
				if (fd.defaultValue().isEmpty()) {
					s << "NULL";
				} else {
					if (valueNeedQuoteFor(fd.dataType())) {
						s << "'" << fd.defaultValue() << "'";
					} else {
						s << fd.defaultValue();
					}
				}
			}
			if (fd.isNotNull()) {
				s << " NOT NULL";
			}
			++index;
		}
		foreach(const WSqlFieldDefinition& fd, fieldDefinitionList) {
			if (fd.isPrimaryKey()) {
				if (index) {
					s << "," << "\n";
				}
			}
			s << INDENT
			  << "CONSTRAINT " << normalPrimaryKeyConstraintName(m_tableDefinition.tableName())
			  << " PRIMARY KEY (" << fr::Identifier(fd.fieldName()).getQuoted() << ")";
			// only-one primary key sql is generated.
			break;
		}
		s << "\n" << ")";
		s.flush();
		queryList << query;
	}

	foreach(const WSqlFieldDefinition& fd, fieldDefinitionList) {
		// ALTER TABLE {TABLENAME} ADD CONSTRAINT {FOREIGN_KEY_CONSTRAINT_NAME}
		// 	FOREIGN KEY ( {field} ) REFERENCES {foreignTable} ( {foreignField} )
		// 	ON UPDATE CASCADE ON DELETE CASCADE;
		if (fd.isForeignKey()) {
			fr::Identifier fieldName(fd.fieldName().toLower());
			fr::Identifier foreignTableName(fd.property("foreignTable").toString().toUpper());
			fr::Identifier foreignTableKeyName(fd.property("foreignTableKey").toString().toLower());

			QString query;
			QTextStream s(&query);
			s << "ALTER TABLE " << tableName.getQuoted()
			  << " ADD CONSTRAINT " << normalForeignKeyConstraintName(fd.fieldName()) << "\n"
			  << " FOREIGN KEY (" << fieldName.getQuoted() << ")"
			  << " REFERENCES " << foreignTableName.getQuoted()
			  << " (" << foreignTableKeyName.getQuoted() << ")"
			  << " ON UPDATE CASCADE ON DELETE CASCADE";
			s.flush();
			queryList << query;
		}
	};

	foreach(const WSqlFieldDefinition& fd, fieldDefinitionList) {
		if (fd.isAutoIncrement()) {
			fr::Identifier fieldName(fd.fieldName().toLower());
			QString generatorName = normalGeneratorNameOf(m_tableDefinition.tableName(),
														  fd.fieldName());
			// _BI --> _Before_Insert
			QString triggerName = normalTriggerNameForGenerator(m_tableDefinition.tableName(), fd.fieldName());
			queryList += QString::fromLatin1("CREATE GENERATOR %1").arg(generatorName);

			QString query;
			QTextStream s(&query);
			s << "CREATE TRIGGER " << triggerName << " FOR " << m_tableDefinition.tableName() << " ACTIVE" << "\n"
			  << "BEFORE INSERT POSITION 0" << "\n"
			  << "AS BEGIN" << "\n"
			  << "IF (NEW." << fieldName.getQuoted() << " IS NULL) THEN" << "\n"
			  << "NEW." << fieldName.getQuoted() << " = GEN_ID(\"" << generatorName << "\", 1);" << "\n"
			  << "END";
			s.flush();
			queryList << query;
		}
	}

	return queryList;
}
