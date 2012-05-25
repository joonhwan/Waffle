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


  $Id: SqlTokenizer.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include <algorithm>

#include "config/Config.h"
#include "sql/SqlTokenizer.h"
#include <QChar>
#include <QString>

namespace fr {

//-----------------------------------------------------------------------------
// SqlTokenizerConfigCache: class to cache user preference for keyword case
class SqlTokenizerConfigCache : public ConfigCache
{
private:
    bool sqlKeywordsUpperCaseM;
protected:
    virtual void loadFromConfig()
    {
        sqlKeywordsUpperCaseM = config().get(QString::fromLatin1("SQLKeywordsUpperCase"),
            false);
    }
public:
    SqlTokenizerConfigCache() : ConfigCache(config()) {};

    static SqlTokenizerConfigCache& get()
    {
        static SqlTokenizerConfigCache stcc;
        return stcc;
    }

    bool getSqlKeywordsUpperCase()
    {
        ensureCacheValid();
        return sqlKeywordsUpperCaseM;
    }
};
//-----------------------------------------------------------------------------
SqlTokenizer::SqlTokenizer()
    : termM(QString::fromLatin1(";"))
{
    init();
}
//-----------------------------------------------------------------------------
SqlTokenizer::SqlTokenizer(const QString& statement)
    : sqlM(statement), termM(QString::fromLatin1(";"))
{
    init();
}
//-----------------------------------------------------------------------------
/*static*/
QString SqlTokenizer::getKeyword(SqlTokenType token)
{
    return getKeyword(token,
        SqlTokenizerConfigCache::get().getSqlKeywordsUpperCase());
}
//-----------------------------------------------------------------------------
/*static*/
QString SqlTokenizer::getKeyword(SqlTokenType token, bool upperCase)
{
    if (upperCase)
    {
        static TokenToKeywordMap keywordsUpperCase;
        if (keywordsUpperCase.empty())
        {
            const KeywordToTokenMap& keywordsMap = getKeywordToTokenMap();
            for (KeywordToTokenMap::const_iterator it = keywordsMap.begin();
                it != keywordsMap.end(); ++it)
            {
                keywordsUpperCase.insert(TokenToKeywordEntry((*it).second,
                    (*it).first.toUpper()));
            }
        }
        TokenToKeywordMap::const_iterator pos = keywordsUpperCase.find(token);
        if (pos != keywordsUpperCase.end())
            return (*pos).second;
    }
    else
    {
        static TokenToKeywordMap keywordsLowerCase;
        if (keywordsLowerCase.empty())
        {
            const KeywordToTokenMap& keywordsMap = getKeywordToTokenMap();
            for (KeywordToTokenMap::const_iterator it = keywordsMap.begin();
                it != keywordsMap.end(); ++it)
            {
                keywordsLowerCase.insert(TokenToKeywordEntry((*it).second,
                    (*it).first.toLower()));
            }
        }
        TokenToKeywordMap::const_iterator pos = keywordsLowerCase.find(token);
        if (pos != keywordsLowerCase.end())
            return (*pos).second;
    }
    return QString();
}
//-----------------------------------------------------------------------------
/*static*/
const SqlTokenizer::KeywordToTokenMap& SqlTokenizer::getKeywordToTokenMap()
{
    static KeywordToTokenMap keywords;
    if (keywords.empty())
    {
        static const struct { QString name; SqlTokenType type; } entries[] =
        {
            #include "keywordtokens.txt"
            // this element makes for simpler code: all lines in the file can
            // end with a comma, and it is used as the stop marker
            { QString::fromLatin1(""), tkUNKNOWN }
        };
        for (int i = 0; entries[i].type != tkUNKNOWN; i++)
        {
            keywords.insert(KeywordToTokenEntry(
                QString(entries[i].name).toUpper(), entries[i].type));
        }
    }
    return keywords;
}
//-----------------------------------------------------------------------------
/*static*/
QStringList SqlTokenizer::getKeywords(KeywordCase kwc)
{
    const KeywordToTokenMap& keywordsMap = getKeywordToTokenMap();
    QStringList keywords;
    keywords.reserve((int)keywordsMap.size());

    bool upperCase = (kwc == kwUpperCase) || (kwc == kwDefaultCase
        && config().get(QString::fromLatin1("SQLKeywordsUpperCase"), false));
    for (KeywordToTokenMap::const_iterator it = keywordsMap.begin();
        it != keywordsMap.end(); ++it)
    {
        if (upperCase)
            keywords.push_back(((*it).first).toUpper());
        else
            keywords.push_back(((*it).first).toLower());
    }
    keywords.sort();
    return keywords;
}
//-----------------------------------------------------------------------------
/*static*/
QString SqlTokenizer::getKeywordsString(KeywordCase kwc)
{
    QStringList keywordsArray(getKeywords(kwc));
    QString keywords;
    for (int i = 0; i < keywordsArray.size(); ++i)
    {
        if (i)
            keywords += QString::fromLatin1(" ");
        keywords += keywordsArray[i];
    }
    return keywords;
}
//-----------------------------------------------------------------------------
/*static*/
SqlTokenType SqlTokenizer::getKeywordTokenType(const QString& word)
{
    if (word.isEmpty())
        return tkIDENTIFIER;

    const KeywordToTokenMap& keywords = getKeywordToTokenMap();
    KeywordToTokenMap::const_iterator pos = keywords.find(word.toUpper());
    if (pos != keywords.end())
        return (*pos).second;
    return tkIDENTIFIER;
}
//-----------------------------------------------------------------------------
/*static*/
bool SqlTokenizer::isReservedWord(const QString& word)
{
    if (word.isEmpty())
        return false;

    const KeywordToTokenMap& keywords = getKeywordToTokenMap();
    KeywordToTokenMap::const_iterator pos = keywords.find(word);
    return pos != keywords.end();
}
//-----------------------------------------------------------------------------
SqlTokenType SqlTokenizer::getCurrentToken()
{
    return sqlTokenTypeM;
}
//-----------------------------------------------------------------------------
QString SqlTokenizer::getCurrentTokenString()
{
    if (sqlTokenStartM && sqlTokenEndM && sqlTokenEndM > sqlTokenStartM)
        return QString::fromRawData(sqlTokenStartM, sqlTokenEndM-sqlTokenEndM+1);
    return QString();
}
//-----------------------------------------------------------------------------
bool SqlTokenizer::isKeywordToken()
{
    return sqlTokenTypeM > tk_KEYWORDS_START_HERE;
}
//-----------------------------------------------------------------------------
void SqlTokenizer::init()
{
    sqlTokenEndM = sqlTokenStartM = sqlM.constData();
    nextToken();
}
//-----------------------------------------------------------------------------
int SqlTokenizer::getCurrentTokenPosition()
{
    return (sqlTokenStartM - sqlM.constData());
}
//-----------------------------------------------------------------------------
// same as nextToken, but skips whitespace, comments and optionally parenthesis
bool SqlTokenizer::jumpToken(bool skipParenthesis)
{
    int parencount = 0;
    while (true)
    {
        if (!nextToken())
            return false;
        SqlTokenType stt = getCurrentToken();
        if (stt == tkPARENOPEN && skipParenthesis)
            parencount++;
        if (stt != tkWHITESPACE && stt != tkCOMMENT && parencount == 0)
            break;
        if (stt == tkPARENCLOSE && skipParenthesis)
            parencount--;
        if (parencount < 0)
            parencount = 0;
    }
    return true;
}
//-----------------------------------------------------------------------------
bool SqlTokenizer::nextToken()
{
    sqlTokenStartM = sqlTokenEndM;
    if (sqlTokenEndM == 0 || *sqlTokenEndM == 0)
    {
        sqlTokenTypeM = tkEOF;
        return false;
    }
    // use QChar* member to scan
    QChar c = *sqlTokenEndM;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
        keywordIdentifierToken();
    else if (c == '"')
        quotedIdentifierToken();
    else if (c == '\'')
        stringToken();
    else if (c == '(')
        symbolToken(tkPARENOPEN);
    else if (c == ')')
        symbolToken(tkPARENCLOSE);
    else if (c == '=')
        symbolToken(tkEQUALS);
    else if (c == ',')
        symbolToken(tkCOMMA);
    else if (c == '/' && *(sqlTokenEndM + 1) == '*')
        multilineCommentToken();
    else if (c == '-' && *(sqlTokenEndM + 1) == '-')
        singleLineCommentToken();
    else if (c.isSpace())
        whitespaceToken();
    else
        defaultToken();
    return true;
}
//-----------------------------------------------------------------------------
void SqlTokenizer::setStatement(const QString& statement)
{
    sqlM = statement;
    init();
}
//-----------------------------------------------------------------------------
void SqlTokenizer::defaultToken()
{
    if (QString(sqlTokenStartM) == termM)
    {
        sqlTokenTypeM = tkTERM;
        sqlTokenEndM = sqlTokenStartM + termM.length();
        return;
    }

    // this is needed for new terminator string
    while (true)
    {
        // increase the size until we hit either whitespace, terminator, EOF
        // or some of significant characters
        sqlTokenEndM++;
        if (*sqlTokenEndM == 0
            || *sqlTokenEndM == ',' || *sqlTokenEndM == '='
            || *sqlTokenEndM == '(' || *sqlTokenEndM == ')'
            || *sqlTokenEndM == '+' || *sqlTokenEndM == '-'
            || *sqlTokenEndM == '/' || *sqlTokenEndM == '*'
			|| sqlTokenEndM->isSpace()
			|| (termM != QString(sqlTokenEndM)) == 0 )
        {
            break;
        }
    }
    sqlTokenTypeM = tkUNKNOWN;
}
//-----------------------------------------------------------------------------
void SqlTokenizer::keywordIdentifierToken()
{
    sqlTokenTypeM = tkIDENTIFIER;
    // identifier must start with letter 'a'..'z', and may continue with
    // those same letters, numbers and the characters '_' and '$'
    QChar c = *sqlTokenEndM;
    Q_ASSERT((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
    do
    {
        sqlTokenEndM++;
        c = *sqlTokenEndM;
    }
    while (c != 0 && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
        || (c >= '0' && c <= '9') || c == '_' || c == '$'));

    // check whether it's a keyword, and not an identifier
    QString checkKW(sqlTokenStartM, sqlTokenEndM - sqlTokenStartM + 1);
    SqlTokenType keywordType = getKeywordTokenType(checkKW);
    if (keywordType != tkIDENTIFIER)
        sqlTokenTypeM = keywordType;
}
//-----------------------------------------------------------------------------
void SqlTokenizer::multilineCommentToken()
{
    sqlTokenTypeM = tkCOMMENT;
    Q_ASSERT(*sqlTokenEndM == '/' && *(sqlTokenEndM + 1) == '*');
    sqlTokenEndM += 2;
    // scan until end of comment, or until "\0" found
    while (*sqlTokenEndM != 0)
    {
        if (*sqlTokenEndM == '*' && *(sqlTokenEndM + 1) == '/')
        {
            sqlTokenEndM += 2;
            break;
        }
        sqlTokenEndM++;
    }
}
//-----------------------------------------------------------------------------
void SqlTokenizer::quotedIdentifierToken()
{
    sqlTokenTypeM = tkIDENTIFIER;
    Q_ASSERT(*sqlTokenEndM == '"');
    sqlTokenEndM++;
    // scan until after next double quote char, or until "\0" found
    while (*sqlTokenEndM != 0)
    {
        if (*sqlTokenEndM == '"')
        {
            sqlTokenEndM++;
            // scan over escaped quotes
            if (*sqlTokenEndM != '"')
                break;
        }
        sqlTokenEndM++;
    }
}
//-----------------------------------------------------------------------------
void SqlTokenizer::singleLineCommentToken()
{
    sqlTokenTypeM = tkCOMMENT;
    Q_ASSERT(*sqlTokenEndM == '-' && *(sqlTokenEndM + 1) == '-');
    sqlTokenEndM += 2;
    // scan until end of line, or until "\0" found
    while (*sqlTokenEndM != 0)
    {
        if (*sqlTokenEndM == '\n')
        {
            sqlTokenEndM++;
            if (*sqlTokenEndM == '\r')
                sqlTokenEndM++;
            break;
        }
        sqlTokenEndM++;
    }
}
//-----------------------------------------------------------------------------
void SqlTokenizer::stringToken()
{
    sqlTokenTypeM = tkSTRING;
    Q_ASSERT(*sqlTokenEndM == '\'');
    sqlTokenEndM++;
    // scan until after next quote char, or until "\0" found
    while (*sqlTokenEndM != 0)
    {
        if (*sqlTokenEndM == '\'')
        {
            sqlTokenEndM++;
            // scan over escaped quotes
            if (*sqlTokenEndM != '\'')
                break;
        }
        sqlTokenEndM++;
    }
}
//-----------------------------------------------------------------------------
inline void SqlTokenizer::symbolToken(SqlTokenType type)
{
    sqlTokenTypeM = type;
    sqlTokenEndM++;
}
//-----------------------------------------------------------------------------
void SqlTokenizer::whitespaceToken()
{
    sqlTokenTypeM = tkWHITESPACE;
    Q_ASSERT(sqlTokenEndM->isSpace());
    sqlTokenEndM++;
    // scan until non-whitespace, or until "\0" found
    while (*sqlTokenEndM != 0 && sqlTokenEndM->isSpace())
        sqlTokenEndM++;
}

} // namespace fr

//-----------------------------------------------------------------------------
