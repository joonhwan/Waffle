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


  $Id: TemplateProcessor.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "config/Config.h"
#include "core/StringUtils.h"
#include "frutils.h"
#include "core/FRError.h"
#include "core/ProcessableObject.h"
#include "core/ProgressIndicator.h"
#include "TemplateProcessor.h"
#include "flamerobin-port-helper.h"
#include <QDir>

namespace fr {

//-----------------------------------------------------------------------------
TemplateProcessor::TemplateProcessor(ProcessableObject* object)
    : objectM(object)
{
}
//-----------------------------------------------------------------------------
void TemplateProcessor::processCommand(const QString& cmdName,
    const TemplateCmdParams& cmdParams, ProcessableObject* object,
    QString& processedText)
{
    if (cmdName == QLatin1String("--"))
        // Comment.
        ;

    // {%template_root%}
    // Expands to the full path of the folder containing the currently
    // processed template, including the final path separator.
    // May expand to a blank string if the template being processed does
    // not come from a text file.
    else if (cmdName == QLatin1String("template_root"))
        processedText += getTemplatePath();

    // {%getvar:name%}
    // Expands to the value of the specified string variable, or a blank
    // string if the variable is not defined.
    else if (cmdName == QLatin1String("getvar") && !cmdParams.isEmpty())
        processedText += getVar(cmdParams.all());

    // {%setvar:name:value%}
    // Sets the value of the specified variable and expands to a blank string.
    // If the variable is already defined overwrites it.
    else if (cmdName == QLatin1String("setvar") && !cmdParams.isEmpty())
    {
        if (cmdParams.size() == 1)
            clearVar(cmdParams[0]);
        else
            setVar(cmdParams[0], cmdParams[1]);
    }

    // {%clearvar:name%}
    // Sets the value of the specified variable to a blank string.
    // If the variable is not defined it does nothing.
    // Expands to a blank string.
    else if (cmdName == QLatin1String("clearvar") && !cmdParams.isEmpty())
        clearVar(cmdParams.all());

    // {%clearvars%}
    // Sets all defined variables to blank strings.
    // Expands to a blank string.
    else if (cmdName == QLatin1String("clearvars"))
        clearVars();

    // {%getconf:key:default%}
    // Expands to the value of the specified local config key,
    // or a blank string if the key is not found.
    // The local config is associated to the template - this is
    // not one of FlameRobin's global config files.
    else if (cmdName == QLatin1String("getconf") && !cmdParams.isEmpty())
    {
        QString text;
        internalProcessTemplateText(text, cmdParams[0], object);
        QString defValue;
        if (cmdParams.size() > 1)
            defValue = cmdParams.from(1);
        processedText += configM.get(text, defValue);
    }

    // {%setconf:key:value%}
    // Sets the value of the specified local config key.
    // If the key is already defined overwrites it.
    // Expands to a blank string.
    else if (cmdName == QLatin1String("setconf") && !cmdParams.isEmpty())
    {
        if (cmdParams.size() == 1)
            configM.setValue(cmdParams[0], QString(QLatin1String("")));
        else
            configM.setValue(cmdParams[0], cmdParams[1]);
    }

    // {%getglobalconf:key%}
    // Expands to the value of the specified global config key,
    // or a blank string if the key is not found.
    else if (cmdName == QLatin1String("getglobalconf") && !cmdParams.isEmpty())
    {
        QString text;
        internalProcessTemplateText(text, cmdParams.all(), object);
        processedText += config().get(text, QString(QLatin1String("")));
    }

    // {%abort%}
    // Throws a silent exception, interrupting the processing.
    // Expands to a blank string.
    else if (cmdName == QLatin1String("abort"))
        throw FRAbort();

    // {%parent_window%}
    // Expands to the current window's numeric memory address.
    // Used to call FR's commands through URIs.
    else if (cmdName == QLatin1String("parent_window"))
		Q_ASSERT(FALSE);
        // processedText += QString::Format(QLatin1String("%ld"), (uintptr_t)windowM);

    // {%colon%}
    else if (cmdName == QLatin1String("colon"))
        processedText += QLatin1String(":");

    // {%if:<term>:<true output>[:<false output>]%}
    // If <term> equals true expands to <true output>, otherwise
    // expands to <false output> (or an empty string).
    else if (cmdName == QLatin1String("if") && (cmdParams.size() >= 2))
    {
        QString val;
        internalProcessTemplateText(val, cmdParams[0], object);

        QString trueText;
        internalProcessTemplateText(trueText, cmdParams[1], object);
        QString falseText;
        if (cmdParams.size() >= 3)
            internalProcessTemplateText(falseText, cmdParams.from(2), object);
        if (getStringAsBoolean(val))
            processedText += trueText;
        else
            processedText += falseText;
    }

    // {%ifeq:<left term>:<right term>:<true output>[:<false output>]%}
    // If <left term> equals <right term> expands to <true output>, otherwise
    // expands to <false output> (or an empty string).
    else if (cmdName == QLatin1String("ifeq") && (cmdParams.size() >= 3))
    {
        QString val1;
        internalProcessTemplateText(val1, cmdParams[0], object);
        QString val2;
        internalProcessTemplateText(val2, cmdParams[1], object);
        QString trueText;
        internalProcessTemplateText(trueText, cmdParams[2], object);
        QString falseText;
        if (cmdParams.size() >= 4)
            internalProcessTemplateText(falseText, cmdParams.from(3), object);
        if (val1 == val2)
            processedText += trueText;
        else
            processedText += falseText;
    }

    // {%!:<input>%} or {%not:<input>%}
    // If <input> equals "true" returns "false";
    // if <input> equals "false" returns "true";
    // otherwise returns <input>.
    // Always expands the argument before evaluating it.
    else if ((cmdName == QLatin1String("!") || (cmdName == QLatin1String("not"))) && (cmdParams.size()))
    {
        QString input;
        internalProcessTemplateText(input, cmdParams[0], object);
        if (input == getBooleanAsString(true))
            processedText += getBooleanAsString(false);
        else if (input == getBooleanAsString(false))
            processedText += getBooleanAsString(true);
        else
            processedText += input;
    }

    // {%ifcontains:<list>:<term>:<true output>[:<false output>]%}
    // If <list> contains <term> expands to <true output>, otherwise
    // expands to <false output> (or an empty string).
    // <list> is a list of comma-separated values.
    else if (cmdName == QLatin1String("ifcontains") && (cmdParams.size() >= 3))
    {
        QString listStr;
        internalProcessTemplateText(listStr, cmdParams[0], object);
        QStringList list = listStr.split(QLatin1Char(','));
        for (int i = 0; i < list.size(); i++) {
            //list[i] = list[i].Trim(true).Trim(false);
			list[i] = helper::stripString(list[i], helper::both);
		}
        QString val2;
        internalProcessTemplateText(val2, cmdParams[1], object);
        QString trueText;
        internalProcessTemplateText(trueText, cmdParams[2], object);
        QString falseText;
        if (cmdParams.size() >= 4)
            internalProcessTemplateText(falseText, cmdParams.from(3), object);
        if (list.indexOf(val2) >= 0)
            processedText += trueText;
        else
            processedText += falseText;
    }

    // {%forall:<list>:<separator>:<text>%}
    // Repeats <text> once for each string in <list>, pasting a <separator>
    // before each item except the first.
    // <list> is a list of comma-separated values.
    // Inside <text> use the placeholder %%current_value%% to
    // mean the current string in the list.
    else if (cmdName == QLatin1String("forall") && (cmdParams.size() >= 3))
    {
        QString listStr;
        internalProcessTemplateText(listStr, cmdParams[0], object);
        QStringList list = listStr.split(QLatin1Char(',')); //(wxStringTokenize(listStr, QLatin1String(",")));
        for (int i = 0; i < list.size(); i++) {
            // list[i] = list[i].Trim(true).Trim(false);
			list[i] = helper::stripString(list[i], helper::both);
		}

        QString separator;
        internalProcessTemplateText(separator, cmdParams[1], object);

        bool firstItem = true;
        for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
        {
            QString param = cmdParams.from(2);
            // Try to replace %%current_value%% both before and after expansion.
            param.replace(QLatin1String("%%current_value%%"), *(it));
            QString newText;
            internalProcessTemplateText(newText, param, object);
            newText.replace(QLatin1String("%%current_value%%"), *(it));
            if ((!firstItem) && (!newText.isEmpty()))
                processedText += escapeChars(separator);
            if (!newText.isEmpty())
                firstItem = false;
            processedText += newText;
        }
    }

    // {%countall:<list>%}
    // Expands to the number of strings in <list>.
    // <list> is a list of comma-separated values.
    else if (cmdName == QLatin1String("countall") && (cmdParams.size() >= 1))
    {
        QString listStr;
        internalProcessTemplateText(listStr, cmdParams.all(), object);
        QStringList list = listStr.split(QLatin1Char(',')); //(wxStringTokenize(listStr, QLatin1String(",")));
        processedText += QString::number(list.size());
    }

    // {%alternate:<text1>:<text2>%}
    // Alternates expanding to <text1> and <text2> at each call,
    // starting with <text1>.
    // Used to alternate table row colours, for example.
    else if (cmdName == QLatin1String("alternate") && (cmdParams.size() >= 2))
    {
        static bool first = false;
        first = !first;
        if (first)
            processedText += cmdParams[0];
        else
            processedText += cmdParams[1];
    }

    // {%mid:<text>:<from>:<for>%}
    // Extracts a substring of <for> characters from <text>
    // starting at character <from>.
    // <from> defaults to 0. <for> defaults to <text>'s length minus 1.
    else if (cmdName == QLatin1String("mid") && (cmdParams.size() >= 3))
    {
        QString text;
        internalProcessTemplateText(text, cmdParams[0], object);
        QString from;
        internalProcessTemplateText(from, cmdParams[1], object);
		bool isOk = false;
        long fromI = from.toULong(&isOk);
        if (!isOk)
            fromI = 0;
        QString for_;
        internalProcessTemplateText(for_, cmdParams.from(2), object);
        long forI = for_.toULong(&isOk);
        if (!isOk)
            forI = text.length() - 1;

        // processedText += text.SubString(fromI, fromI + forI - 1);
		processedText += helper::subString(text, fromI, fromI + forI - 1);
    }

    // {%uppercase:<text>%}
    // Converts <text> to upper case.
    else if (cmdName == QLatin1String("uppercase") && (cmdParams.size() >= 1))
    {
        QString text;
        internalProcessTemplateText(text, cmdParams.all(), object);

        processedText += text.toUpper();
    }

    // {%lowercase:<text>%}
    // Converts <text> to lower case.
    else if (cmdName == QLatin1String("lowercase") && (cmdParams.size() >= 1))
    {
        QString text;
        internalProcessTemplateText(text, cmdParams.all(), object);

        processedText += text.toLower();
    }

    // {%wrap:<text>[:<width>[:<indent>]]%}
    // Wraps <text> to lines fo maximum <width> chars, indenting all
    // resulting lines after the first one by <indent> chars.
    // <width> defaults to config item sqlEditorEdgeColumn, or 80.
    // <indent> defaults to config item sqlEditorTabSize, or 4.
    else if (cmdName == QLatin1String("wrap") && (cmdParams.size() >= 1))
    {
        QString text;
        internalProcessTemplateText(text, cmdParams[0], object);

        long widthI;
        if (cmdParams.size() >= 2)
        {
            QString width;
            internalProcessTemplateText(width, cmdParams[1], object);
			bool isOk = false;
			widthI = width.toLong(&isOk);
            if (!isOk)
                widthI = config().get(QLatin1String("sqlEditorEdgeColumn"), 80);
        }
        else
            widthI = config().get(QLatin1String("sqlEditorEdgeColumn"), 80);

        long indentI;
        if (cmdParams.size() >= 3)
        {
            QString indent;
            internalProcessTemplateText(indent, cmdParams[2], object);
			bool isOk = false;
			indentI = indent.toLong(&isOk);
            if (!isOk)
                indentI = config().get(QLatin1String("sqlEditorTabSize"), 4);
        }
        else
            indentI = config().get(QLatin1String("sqlEditorTabSize"), 4);

        processedText += wrapText(text, widthI, indentI);
    }

    // {%kw:<text>%}
    // Formats <text> as a keyword (upper or lower case)
    // according to config item SQLKeywordsUpperCase.
    else if (cmdName == QLatin1String("kw") && (cmdParams.size() >= 1))
    {
        QString text;
        internalProcessTemplateText(text, cmdParams.all(), object);

        if (config().get(QLatin1String("SQLKeywordsUpperCase"), true))
            processedText += text.toUpper();
        else
            processedText += text.toLower();
    }

    // {%tab%}
    // Expands to a number of spaces defined by config item
    // sqlEditorTabSize.
    else if (cmdName == QLatin1String("tab"))
    {
        QString tab;
        // processedText += tab.Pad(config().get(QLatin1String("sqlEditorTabSize"), 4));
		processedText += helper::padString(tab, config().get(QLatin1String("sqlEditorTabSize"), 4));
    }

    // Only if no internal commands are recognized, call external command handlers.
    else
    {
        getTemplateCmdHandlerRepository().handleTemplateCmd(this, cmdName,
           cmdParams, object, processedText);
    }
}
//-----------------------------------------------------------------------------
void TemplateProcessor::internalProcessTemplateText(QString& processedText,
    const QString& inputText, ProcessableObject* object)
{
    if (object == 0)
        object = objectM;

    // parse commands
    int pos = 0, oldpos = 0, endpos = 0;
    while (true)
    {
        pos = inputText.indexOf(QLatin1String("{%"), pos);
        if (pos == -1)
        {
            processedText += inputText.mid(oldpos);
            break;
        }

        int check, startpos = pos;
        int cnt = 1;
        while (cnt > 0)
        {
            endpos = inputText.indexOf(QLatin1String("%}"), startpos+1);
            if (endpos == -1)
                break;

            check = inputText.indexOf(QLatin1String("{%"), startpos+1);
            if (check == -1)
                startpos = endpos;
            else
            {
                startpos = (check < endpos ? check : endpos);
                if (startpos == check)
                    cnt++;
            }
            if (startpos == endpos)
                cnt--;
            startpos++;
        }

        if (cnt > 0)    // no matching closing %}
            break;

        processedText += inputText.mid(oldpos, pos - oldpos);
        QString cmd = inputText.mid(pos + 2, endpos - pos - 2); // 2 = start_marker_len = end_marker_len

        // parse command name and params.
        QString cmdName;
        TemplateCmdParams cmdParams;

        enum TemplateCmdState
        {
            inText,
            inString1,
            inString2
        };
        TemplateCmdState state = inText;
        QString buffer;
        unsigned int nestLevel = 0;
        for (int i = 0; i < cmd.length(); i++)
        {
            QChar c = cmd[i];

            if (c == QLatin1Char(':'))
            {
                if ((nestLevel == 0) && (state == inText))
                {
                    cmdParams.push_back(buffer);
                    buffer.clear();
                    continue;
                }
            }
            buffer += c;

            if ((c == QLatin1Char('{')) && (i < cmd.length() - 1) && (cmd[i + 1] == QLatin1Char('%')))
                nestLevel++;
            else if ((c == QLatin1Char('}')) && (i > 0) && (cmd[i - 1] == QLatin1Char('%')))
                nestLevel--;
            else if (c == QLatin1Char('\''))
                state == inString1 ? state = inText : state = inString1;
            else if (c == QLatin1Char('"'))
                state == inString2 ? state = inText : state = inString2;
        }
        if (buffer.length() > 0)
            cmdParams.push_back(buffer);

        if (cmdParams.size() > 0)
        {
            cmdName = cmdParams[0];
            cmdParams.removeAt(0);
            if (!cmdName.isEmpty())
                processCommand(cmdName, cmdParams, object, processedText);
        }
        oldpos = pos = endpos + 2;
    }
}
//-----------------------------------------------------------------------------
void TemplateProcessor::processTemplateFile(QString& processedText,
    const QFileInfo&  inputFileName, ProcessableObject* object,
    ProgressIndicator* progressIndicator)
{
    fileNameM = inputFileName;

    // QFileInfo infoFileName(inputFileName);
    // infoFileName.SetExt(QLatin1String("info"));
    // infoM.setConfigFileName(infoFileName);
	infoM.setConfigFileName(helper::filePathWithExtension(inputFileName, "info"));
    // put settings file in user writable directory
    QString confFileNameStr(inputFileName.absoluteFilePath());
    // confFileNameStr.replace(config().getHomePath(),
	// 						config().getUserHomePath(),
	// 						false);
	helper::replaceString(confFileNameStr, config().getHomePath(), config().getUserHomePath(),
					   helper::replaceFirstOnly);
    QFileInfo confFileName(confFileNameStr);
    // confFileName.SetExt(QLatin1String("conf"));
    // configM.setConfigFileName(confFileName);
	configM.setConfigFileName(helper::filePathWithExtension(confFileName, "conf"));
    progressIndicatorM = progressIndicator;
    internalProcessTemplateText(processedText, loadEntireFile(fileNameM),
        object);
}
//-----------------------------------------------------------------------------
void TemplateProcessor::processTemplateText(QString& processedText,
    const QString& inputText, ProcessableObject* object,
    ProgressIndicator* progressIndicator)
{
    // fileNameM.clear();
    // configM.setConfigFileName(QFileInfo(wxEmptyString));
	fileNameM = QFileInfo();
    configM.setConfigFileName(QFileInfo());
    progressIndicatorM = progressIndicator;
    internalProcessTemplateText(processedText, inputText, object);
}
//-----------------------------------------------------------------------------
void TemplateProcessor::setVar(const QString& varName,
    const QString& varValue)
{
    varsM[varName] = varValue;
}
//-----------------------------------------------------------------------------
const QString& TemplateProcessor::getVar(const QString& varName)
{
    return varsM[varName];
}
//-----------------------------------------------------------------------------
void TemplateProcessor::clearVar(const QString& varName)
{
    varsM.erase(varName);
}
//-----------------------------------------------------------------------------
void TemplateProcessor::clearVars()
{
    varsM.clear();
}
//-----------------------------------------------------------------------------
QString TemplateProcessor::getTemplatePath()
{
    // return fileNameM.GetPathWithSep();
	return fileNameM.absoluteFilePath() + QDir::separator();
}
//-----------------------------------------------------------------------------
QString TemplateCmdParams::all() const
{
    return from(0);
}
//-----------------------------------------------------------------------------
QString TemplateCmdParams::from(int start) const
{
    QString result;
    if (start < size() && start >= 0)
    {
        result = at(start);
        for (int i = start + 1; i < size(); i++)
            result += QLatin1Char(':') + at(i);
    }
    return result;
}
//-----------------------------------------------------------------------------
TemplateCmdHandlerRepository& getTemplateCmdHandlerRepository()
{
    static TemplateCmdHandlerRepository repository;
    return repository;
}
//-----------------------------------------------------------------------------
//! needed to disallow instantiation
TemplateCmdHandlerRepository::TemplateCmdHandlerRepository()
    : handlerListSortedM(false)
{
}
//-----------------------------------------------------------------------------
TemplateCmdHandlerRepository::~TemplateCmdHandlerRepository()
{
    while (!handlersM.empty())
        removeHandler(handlersM.front());
}
//-----------------------------------------------------------------------------
//! needed in checkHandlerListSorted() to sort on objects instead of pointers
bool templateCmdHandlerPointerLT(const TemplateCmdHandler* left,
    const TemplateCmdHandler* right)
{
    return *left < *right;
}
//-----------------------------------------------------------------------------
void TemplateCmdHandlerRepository::checkHandlerListSorted()
{
    if (!handlerListSortedM)
    {
        handlersM.sort(templateCmdHandlerPointerLT);
        handlerListSortedM = true;
    }
}
//-----------------------------------------------------------------------------
//! returns false if no suitable handler found
void TemplateCmdHandlerRepository::handleTemplateCmd(TemplateProcessor *tp,
    const QString& cmdName, const TemplateCmdParams& cmdParams,
    ProcessableObject* object, QString& processedText)
{
    checkHandlerListSorted();
    for (std::list<TemplateCmdHandler*>::iterator it = handlersM.begin();
        it != handlersM.end(); ++it)
    {
        (*it)->handleTemplateCmd(tp, cmdName, cmdParams, object,
            processedText);
    }
}
//-----------------------------------------------------------------------------
void TemplateCmdHandlerRepository::addHandler(TemplateCmdHandler* handler)
{
    // can't do ordered insert here, since the getPosition() function that
    // serves TemplateCmdHandler::operator< is virtual, and this function (addHandler)
    // is called in the constructor of TemplateCmdHandler.
    // The list will be sorted on demand (see checkHandlerListSorted()).
    handlersM.push_back(handler);
    handler->setRepository(this);
    handlerListSortedM = false;
}
//-----------------------------------------------------------------------------
void TemplateCmdHandlerRepository::removeHandler(TemplateCmdHandler* handler)
{
    handlersM.erase(std::find(handlersM.begin(), handlersM.end(), handler));
    handler->setRepository(0);
}
//-----------------------------------------------------------------------------
TemplateCmdHandler::TemplateCmdHandler() :
    repositoryM(0)
{
    getTemplateCmdHandlerRepository().addHandler(this);
}
//-----------------------------------------------------------------------------
void TemplateCmdHandler::setRepository(TemplateCmdHandlerRepository* const repository)
{
    repositoryM = repository;
}
//-----------------------------------------------------------------------------
//! this is currently only called on program termination
TemplateCmdHandler::~TemplateCmdHandler()
{
    if (repositoryM)
        repositoryM->removeHandler(this);
}
//-----------------------------------------------------------------------------

} // namespace fr

