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


  $Id: MetadataTemplateManager.h 2200 2012-01-20 08:31:01Z mghie $

*/

//-----------------------------------------------------------------------------
#ifndef FR_SQLTEMPLATEMANAGER_H
#define FR_SQLTEMPLATEMANAGER_H

#include <list>

#include <boost/shared_ptr.hpp>

#include "config/Config.h"
#include "metadata/metadataitem.h"

namespace fr {

//-----------------------------------------------------------------------------
//! Holds information about a single template.
class TemplateDescriptor
{
private:
    QFileInfo templateFileNameM;
    Config configM;
    MetadataItem* metadataItemM;
    void loadDescriptionFromConfigFile();
    QString menuCaptionM;
    int menuPositionM;
    QString matchesTypeM;
    QString matchesNameM;
    bool matchesWhenM;
    QString expandTemplateCommands(const QString& inputText) const;
public:
    TemplateDescriptor(const QFileInfo& templateFileName,
        MetadataItem* metadataItem);
    QString getMenuCaption() const;
    int getMenuPosition() const { return menuPositionM; }
    bool operator<(const TemplateDescriptor& right) const;
    //! returns true if the template can be run on the specified metadata item.
    bool matches(const MetadataItem* metadataItem) const;
    const QFileInfo& getTemplateFileName() const { return templateFileNameM; }
    void setTemplateFileName(const QFileInfo& value) { templateFileNameM = value; }
    QString getBaseFileName() const { return templateFileNameM.baseName(); }
};
//-----------------------------------------------------------------------------
typedef boost::shared_ptr<TemplateDescriptor> TemplateDescriptorPtr;
typedef std::list<TemplateDescriptorPtr> TemplateDescriptorList;
//-----------------------------------------------------------------------------
class MetadataTemplateManager
{
private:
    MetadataItem* metadataItemM;
    TemplateDescriptorList descriptorsM;

    void collectDescriptors();
    // Returns a pointer to the first descriptor with the specified base name, or 0.
    TemplateDescriptorPtr findDescriptor(const QString& baseFileName) const;
public:
    MetadataTemplateManager(MetadataItem* metadataItem);
    TemplateDescriptorList::const_iterator descriptorsBegin() const;
    TemplateDescriptorList::const_iterator descriptorsEnd() const;
};
//-----------------------------------------------------------------------------

} // namespace fr

#endif // FR_SQLTEMPLATEMANAGER_H
