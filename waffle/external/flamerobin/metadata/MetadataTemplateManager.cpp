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


  $Id: MetadataTemplateManager.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "config/Config.h"
#include "core/CodeTemplateProcessor.h"
#include "core/FRError.h"
#include "core/StringUtils.h"
#include "metadata/metadataitem.h"
#include "metadata/MetadataTemplateManager.h"
#include "flamerobin-port-helper.h"
#include <QDir>

namespace fr {

//-----------------------------------------------------------------------------
TemplateDescriptor::TemplateDescriptor(const QFileInfo& templateFileName,
    MetadataItem* metadataItem)
    : templateFileNameM(templateFileName), metadataItemM(metadataItem),
    menuPositionM(0)
{
    loadDescriptionFromConfigFile();
}
//-----------------------------------------------------------------------------
void TemplateDescriptor::loadDescriptionFromConfigFile()
{
    QFileInfo confFileName = helper::filePathWithExtension(QFileInfo(templateFileNameM),
														"info");
    // confFileName.SetExt(QString::fromLatin1("info"));
    if (confFileName.exists())
    {
        configM.setConfigFileName(confFileName);
        menuCaptionM = expandTemplateCommands(
            configM.get(QString::fromLatin1("templateInfo/menuCaption"),
            templateFileNameM.baseName()));
        menuPositionM = configM.get(QString::fromLatin1("templateInfo/menuPosition"), 0);
        matchesTypeM = expandTemplateCommands(
            configM.get(QString::fromLatin1("templateInfo/matchesType"), matchesTypeM));
        if (matchesTypeM.isEmpty())
            matchesTypeM = QString::fromLatin1(".*");
        matchesNameM = expandTemplateCommands(
            configM.get(QString::fromLatin1("templateInfo/matchesName"), matchesNameM));
        if (matchesNameM.isEmpty())
            matchesNameM = QString::fromLatin1(".*");
        QString matchesWhen = getBooleanAsString(true);
        matchesWhenM = getStringAsBoolean(expandTemplateCommands(
            configM.get(QString::fromLatin1("templateInfo/matchesWhen"), matchesWhen)));
    }
    else
    {
        menuCaptionM = templateFileNameM.baseName();
        menuPositionM = 0;
        matchesTypeM = QString::fromLatin1(".*");
        matchesNameM = QString::fromLatin1(".*");
        matchesWhenM = true;
    }
}
//-----------------------------------------------------------------------------
QString TemplateDescriptor::expandTemplateCommands(const QString& inputText) const
{
    QString result = QString();
    CodeTemplateProcessor tp(metadataItemM, 0);
    tp.processTemplateText(result, inputText, metadataItemM);
    return result;
 }
//-----------------------------------------------------------------------------
bool TemplateDescriptor::operator<(const TemplateDescriptor& right) const
{
    int p = getMenuPosition();
    int rp = right.getMenuPosition();
    if (p == rp)
        return (getMenuCaption().compare(right.getMenuCaption()) < 0);
    else
        return (p < rp);
}
//-----------------------------------------------------------------------------
QString TemplateDescriptor::getMenuCaption() const
{
    if (!menuCaptionM.isEmpty())
        return menuCaptionM;
    return templateFileNameM.baseName();
}
//-----------------------------------------------------------------------------
bool TemplateDescriptor::matches(const MetadataItem* metadataItem) const
{
    // wxRegEx typeRegEx(matchesTypeM, wxRE_ADVANCED);
    // if (!typeRegEx.IsValid())
    //     throw FRError(_("Invalid regex"));
    // if (typeRegEx.Matches(metadataItem->getTypeName()))
    // {
    //     wxRegEx nameRegEx(matchesNameM, wxRE_ADVANCED);
    //     if (!nameRegEx.IsValid())
    //         throw FRError(_("Invalid regex"));
    //     if (nameRegEx.Matches(metadataItem->getName_()))
    //         return matchesWhenM;
    // }
    // return false;
	QRegExp typeRegEx(matchesTypeM, Qt::CaseInsensitive, QRegExp::RegExp2);
	if (!typeRegEx.isValid())
		throw FRError(QObject::tr("Invalid regex"));
	if (typeRegEx.indexIn(metadataItem->getTypeName()) >= 0) {
		QRegExp nameRegEx(matchesNameM, Qt::CaseInsensitive, QRegExp::RegExp2);
		if (!nameRegEx.isValid()) {
			throw FRError(QObject::tr("Invalid regex"));
		}
		if (nameRegEx.indexIn(metadataItem->getName_())) {
			return matchesWhenM;
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
//! needed in checkDescriptorsSorted() to sort on objects instead of pointers
bool templateDescriptorPointerLT(const TemplateDescriptorPtr left,
    const TemplateDescriptorPtr right)
{
    return *left < *right;
}
//-----------------------------------------------------------------------------
MetadataTemplateManager::MetadataTemplateManager(MetadataItem* metadataItem)
    : metadataItemM(metadataItem)
{
    collectDescriptors();
}
//-----------------------------------------------------------------------------
void MetadataTemplateManager::collectDescriptors()
{
    QStringList fileNames;
    // Collect predefined and user-defined template descriptors.
    // A user-defined descriptor will supercede a predefined one with the same
    // base name.
    // A user may also remove a predefined template by overriding it with one
    // that does not match any object.
    // wxDir::GetAllFiles(config().getCodeTemplatesPath(), &fileNames,
    //     QString::fromLatin1("*.template"), wxDIR_FILES);
	QStringList templateFilter;
	templateFilter << QString::fromLatin1("*.template");
	fileNames = QDir(config().getCodeTemplatesPath())
				.entryList(templateFilter, QDir::Files);
    QString userPath = config().getUserCodeTemplatesPath();

	QDir userPathDir(userPath);
    if (userPathDir.exists()) {
		fileNames << userPathDir.entryList(templateFilter, QDir::Files);
        // wxDir::GetAllFiles(userPath, &fileNames, QString::fromLatin1("*.template"), wxDIR_FILES);
	}
    descriptorsM.clear();
    for (int i = 0; i < fileNames.size(); i++)
    {
        QFileInfo fileName(fileNames[i]);
        TemplateDescriptorPtr tdp = findDescriptor(fileName.baseName());
        if (tdp)
        {
            // Present already - override it (and re-check matching).
            tdp->setTemplateFileName(fileName);
            if (!tdp->matches(metadataItemM))
                descriptorsM.remove(tdp);
        }
        else
        {
            tdp.reset(new TemplateDescriptor(fileName, metadataItemM));
            if (tdp->matches(metadataItemM))
                descriptorsM.push_back(tdp);
        }
    }
    // Sort everything by menu position.
    descriptorsM.sort(templateDescriptorPointerLT);
}
//-----------------------------------------------------------------------------
TemplateDescriptorPtr MetadataTemplateManager::findDescriptor(
    const QString& baseFileName) const
{
    for (TemplateDescriptorList::const_iterator it = descriptorsBegin();
        it != descriptorsEnd(); ++it)
    {
        if ((*it)->getBaseFileName() == baseFileName)
            return (*it);
    }
    return TemplateDescriptorPtr();
}
//-----------------------------------------------------------------------------
TemplateDescriptorList::const_iterator MetadataTemplateManager::descriptorsBegin() const
{
    return descriptorsM.begin();
}
//-----------------------------------------------------------------------------
TemplateDescriptorList::const_iterator MetadataTemplateManager::descriptorsEnd() const
{
    return descriptorsM.end();
}
//-----------------------------------------------------------------------------

} // namespace fr
