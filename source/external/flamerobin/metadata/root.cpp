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


  $Id: root.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include <algorithm>

#include <boost/function.hpp>

#include "config/Config.h"
#include "core/StringUtils.h"
#include "metadata/database.h"
#include "metadata/MetadataItemVisitor.h"
#include "metadata/root.h"
#include "metadata/server.h"
#include <QDomNode>
#include <QXmlStreamWriter>
#include <QFileInfo>
#include <QDir>

namespace fr {

//-----------------------------------------------------------------------------
Root::Root()
    : MetadataItem(ntRoot, 0, _("Home"))
{
    setChildrenLoaded(true);
}
//-----------------------------------------------------------------------------
void Root::disconnectAllDatabases()
{
    for (ServerPtrs::iterator its = serversM.begin();
        its != serversM.end(); ++its)
    {
        DatabasePtrs databases((*its)->getDatabases());
        std::for_each(databases.begin(), databases.end(),
            boost::mem_fn(&Database::disconnect));
    }
}
//-----------------------------------------------------------------------------
Root::~Root()
{
}
//-----------------------------------------------------------------------------
//! loads fr_databases.conf file and:
//! creates server nodes, fills their properties
//! creates database nodes for server nodes, fills their properties
//! returns: false if file cannot be loaded, true otherwise
//
bool Root::load()
{
	bool docIsOk = false;
    QDomDocument doc;
    QFileInfo fileName = getFileName();
    if (fileName.exists())
    {
		QFile file(fileName.absoluteFilePath());
        if (doc.setContent(&file)) {
			docIsOk = true;
		}
		file.close();
    }
    if (!docIsOk)
        return false;

    QDomElement xmlr = doc.documentElement();
    if (xmlr.tagName() != QString::fromLatin1("root"))
        return false;

    SubjectLocker locker(this);
    for (QDomElement xmln = xmlr.firstChildElement();
		 !xmln.isNull();
		 xmln = xmln.nextSiblingElement())
    {
        if (xmln.tagName() == QString::fromLatin1("server"))
            parseServer(xmln);
        if (xmln.tagName() == QString::fromLatin1("nextId"))
        {
			bool isOk = false;
            QString value = xmln.toElement().text();
            unsigned long l = value.toULong(&isOk);
            // nextIdM may have been written already (database id)
            if (!value.isEmpty() && isOk)
            {
                if (Database::getUIDGeneratorValue() < l)
                    Database::setUIDGeneratorValue(l);
            }
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
bool Root::parseDatabase(ServerPtr server, QDomNode& xmlnParent)
{
    Q_ASSERT(server);
    Q_ASSERT(!xmlnParent.isNull());

    DatabasePtr database = server->addDatabase();
    SubjectLocker locker(database.get());

    for (QDomElement xmln = xmlnParent.firstChildElement();
		 !xmln.isNull();
		 xmln = xmln.nextSiblingElement())
    {
		QString value = xmln.text();
        if (xmln.tagName() == QString::fromLatin1("name"))
            database->setName_(value);
        else if (xmln.tagName() == QString::fromLatin1("path"))
            database->setPath(value);
        else if (xmln.tagName() == QString::fromLatin1("charset"))
            database->setConnectionCharset(value);
        else if (xmln.tagName() == QString::fromLatin1("username"))
            database->setUsername(value);
        else if (xmln.tagName() == QString::fromLatin1("password"))
            database->setRawPassword(value);
        else if (xmln.tagName() == QString::fromLatin1("encrypted") && value == QString::fromLatin1("1"))
            database->getAuthenticationMode().setStoreEncryptedPassword();
        else if (xmln.tagName() == QString::fromLatin1("authentication"))
            database->getAuthenticationMode().setConfigValue(value);
        else if (xmln.tagName() == QString::fromLatin1("role"))
            database->setRole(value);
        else if (xmln.tagName() == QString::fromLatin1("id"))
        {
			bool isOk = false;
            unsigned long id = value.toULong(&isOk);
            if (isOk)
                database->setId(id);
        }
    }

    // make sure the database has an Id before Root::save() is called,
    // otherwise a new Id will be generated then, but the generator value
    // will not be stored because it's at the beginning of the file.
    database->getId();
    return true;
}
//-----------------------------------------------------------------------------
bool Root::parseServer(QDomNode& xmlnServer)
{
    Q_ASSERT(!xmlnServer.isNull());

    ServerPtr server = addServer();
    SubjectLocker locker(server.get());

    for (QDomElement xmln = xmln.firstChildElement();
		 !xmln.isNull();
		 xmln = xmln.nextSiblingElement())
    {
	    QString value = xmln.text();
        if (xmln.tagName() == QString::fromLatin1("name"))
            server->setName_(value);
        else if (xmln.tagName() == QString::fromLatin1("host"))
            server->setHostname(value);
        else if (xmln.tagName() == QString::fromLatin1("port"))
            server->setPort(value);
        else if (xmln.tagName() == QString::fromLatin1("database"))
        {
            if (!parseDatabase(server, xmln))
                return false;
        }
    }
    // backward compatibility with FR < 0.3.0
    if (server->getName_().isEmpty())
        server->setName_(server->getConnectionString());
    server->setChildrenLoaded(true);
    return true;
}
//-----------------------------------------------------------------------------
ServerPtr Root::addServer()
{
    ServerPtr server(new Server());
    addServer(server);
    return server;
}
//-----------------------------------------------------------------------------
void Root::addServer(ServerPtr server)
{
    if (server)
    {
        serversM.push_back(server);
        server->setParent(this);
        notifyObservers();
    }
}
//-----------------------------------------------------------------------------
void Root::removeServer(ServerPtr server)
{
    if (unregLocalDatabasesM == server)
        unregLocalDatabasesM.reset();

    ServerPtrs::iterator it = std::remove(serversM.begin(), serversM.end(),
        server);
    if (it != serversM.end())
    {
        serversM.erase(it, serversM.end());
        notifyObservers();
    }
}
//-----------------------------------------------------------------------------
void Root::addUnregisteredDatabase(DatabasePtr database)
{
    // on-demand creation of parent node for unregistered databases
    if (!unregLocalDatabasesM)
    {
        ServerPtr server(new Server());
        serversM.push_back(server);
        server->setName_(_("Unregistered local databases"));
        server->setHostname(QString::fromLatin1("localhost"));
        server->setParent(this);

        unregLocalDatabasesM = server;
        notifyObservers();
    }

    unregLocalDatabasesM->addDatabase(database);
}
//-----------------------------------------------------------------------------
// helper for Root::save()
void rsAddChildNode(QXmlStreamWriter& xmlWriter, const QString nodeName,
    const QString nodeContent)
{
    if (!nodeContent.isEmpty())
    {
		xmlWriter.writeStartElement(nodeName);
		xmlWriter.writeCharacters(nodeContent);
		xmlWriter.writeEndElement();
    }
}
//-----------------------------------------------------------------------------
// browses the server nodes, and their database nodes
// saves everything to fr_databases.conf file
// returns: false if file cannot be opened for writing, true otherwise
//
bool Root::save()
{
    // create directory if it doesn't exist yet.
	QString fileFullPath = QFileInfo(getFileName()).absoluteFilePath();
	{
		QDir _file = fileFullPath;
		if (!_file.exists())
			_file.mkdir(_file.path());
	}
	QFile file(fileFullPath);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		return false;
	}

	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();

// #if !wxUSE_UNICODE
//     doc.SetFileEncoding(getHtmlCharset());
// #endif
	xmlWriter.writeStartElement(QString::fromLatin1("root"));
	{
		// QDomNode rn = new QDomNode(wxXML_ELEMENT_NODE, QString::fromLatin1("root"));
		// doc.SetRoot(rn);
		// rsAddChildNode(rn, QString::fromLatin1("nextId"),
		//     QString::Format(QString::fromLatin1("%d"), Database::getUIDGeneratorValue()));
		rsAddChildNode(xmlWriter, QString::fromLatin1("nextId"),
					   QString::fromLatin1("%1").arg(Database::getUIDGeneratorValue()));

		for (ServerPtrs::iterator its = serversM.begin();
			 its != serversM.end(); ++its)
		{
			// do not save the dummy server node for databases that were opened
			// either via command line switch or via drag and drop
			if ((*its) == unregLocalDatabasesM)
				continue;

			// QDomNode xmlWriter = new QDomNode(wxXML_ELEMENT_NODE, QString::fromLatin1("server"));
			// rn.AddChild(xmlWriter);
			xmlWriter.writeStartElement(QString::fromLatin1("server"));
			{
				rsAddChildNode(xmlWriter, QString::fromLatin1("name"), (*its)->getName_());
				rsAddChildNode(xmlWriter, QString::fromLatin1("host"), (*its)->getHostname());
				rsAddChildNode(xmlWriter, QString::fromLatin1("port"), (*its)->getPort());

				DatabasePtrs databases((*its)->getDatabases());
				for (DatabasePtrs::iterator itdb = databases.begin();
					 itdb != databases.end(); ++itdb)
				{
					(*itdb)->resetCredentials();    // clean up eventual extra credentials

					// QDomNode xmlWriter = new QDomNode(wxXML_ELEMENT_NODE, QString::fromLatin1("database"));
					// xmlWriter.AddChild(xmlWriter);
					xmlWriter.writeStartElement(QString::fromLatin1("database"));
					{
						rsAddChildNode(xmlWriter, QString::fromLatin1("id"), (*itdb)->getId());
						rsAddChildNode(xmlWriter, QString::fromLatin1("name"), (*itdb)->getName_());
						rsAddChildNode(xmlWriter, QString::fromLatin1("path"), (*itdb)->getPath());
						rsAddChildNode(xmlWriter, QString::fromLatin1("charset"), (*itdb)->getConnectionCharset());
						rsAddChildNode(xmlWriter, QString::fromLatin1("username"), (*itdb)->getUsername());
						rsAddChildNode(xmlWriter, QString::fromLatin1("password"), (*itdb)->getRawPassword());
						rsAddChildNode(xmlWriter, QString::fromLatin1("role"), (*itdb)->getRole());
						rsAddChildNode(xmlWriter, QString::fromLatin1("authentication"),
									   (*itdb)->getAuthenticationMode().getConfigValue());
					}
					xmlWriter.writeEndElement();
				}
			}
			xmlWriter.writeEndElement();
		}
	}
	xmlWriter.writeEndElement();

	xmlWriter.writeEndDocument();
	file.close();
	if (file.error()) {
		return false;
	}

    // return doc.Save(getFileName());
	return true;
}
//-----------------------------------------------------------------------------
ServerPtrs Root::getServers() const
{
    return serversM;
}
//-----------------------------------------------------------------------------
bool Root::getChildren(std::vector<MetadataItem *>& temp)
{
    if (serversM.empty())
        return false;
    std::transform(serversM.begin(), serversM.end(), std::back_inserter(temp),
        boost::mem_fn(&ServerPtr::get));
    return !serversM.empty();
}
//-----------------------------------------------------------------------------
void Root::lockChildren()
{
    std::for_each(serversM.begin(), serversM.end(),
        boost::mem_fn(&Server::lockSubject));
}
//-----------------------------------------------------------------------------
void Root::unlockChildren()
{
    std::for_each(serversM.begin(), serversM.end(),
        boost::mem_fn(&Server::unlockSubject));
}
//-----------------------------------------------------------------------------
const QString Root::getItemPath() const
{
    // Root is root, don't make the path strings any longer than needed.
    return QString::fromLatin1("");
}
//-----------------------------------------------------------------------------
QString Root::getFileName()
{
    if (fileNameM.isEmpty())
        fileNameM = config().getDBHFileName();
    return fileNameM;
}
//-----------------------------------------------------------------------------
void Root::acceptVisitor(MetadataItemVisitor* visitor)
{
    visitor->visitRoot(*this);
}
//-----------------------------------------------------------------------------
const QString Root::getTypeName() const
{
    return QString::fromLatin1("ROOT");
}
//-----------------------------------------------------------------------------

} // namespace fr
