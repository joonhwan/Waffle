#include <QApplication>
#include <QDate>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QMap>
#include <QMapIterator>
#include <QVariant>
#include <iostream>
#include "flamerobin/flamerobin.h"

const char* DBPATH = "c:/db/test.fdb";

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("joonhwan");
	QCoreApplication::setOrganizationDomain("www.joonhwan.org");
	QCoreApplication::setApplicationName("firebird_db_test");

	fr::RootPtr root(new fr::Root());
	fr::ServerPtr s(new fr::Server());
	s->setName_(QLatin1String("Localhost"));
	s->setHostname(QLatin1String("localhost"));
	root->addServer(s);

	fr::DatabasePtr db(new fr::Database());
	db->setServer(s);
	db->setUsername(QString::fromLatin1("SYSDBA"));
	db->setRawPassword(QString::fromLatin1("masterkey"));
	db->setPath(QString::fromLatin1("c:/db/test.fdb"));
	db->connect(QString::fromLatin1("masterkey"));

	if (!db->isConnected()) {
		qDebug() << "unable to connect to database.";
		return -1;
	}

	fr::TablesPtr tables = db->getTables();
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

				QString colName = col->getName_();
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
							if (db && db->getDatabaseCharset() == charset) {
								charset = "";
							}
							if (db && db->isDefaultCollation(charset, collate)) {
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
						 << "Name:" << colName << ", "
						 << "Type:" << dataType << ", "
						 << "Not Null?:" << notNull << ", "
						 << "Default:" << defaultValue << ", "
						 << "Charset:" << charset;
			}
		}
		fr::TriggersPtr triggers = db->getTriggers();
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
	// return app.exec();
	return 0;
}
