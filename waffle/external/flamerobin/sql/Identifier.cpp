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


  $Id: Identifier.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "config/Config.h"
#include "core/Observer.h"
#include "core/Subject.h"
#include "sql/Identifier.h"
#include "sql/SqlTokenizer.h"
#include "flamerobin-port-helper.h"

namespace fr {

//----------------------------------------------------------------------------
// IdentifierQuotes: class to cache config data for identifier quoting
class IdentifierQuotes: public ConfigCache
{
private:
    bool quoteAlwaysM;
    bool quoteCharsAreRegularM;
    bool quoteMixedCaseM;
protected:
    virtual void loadFromConfig();
public:
    IdentifierQuotes();

    static IdentifierQuotes& get();

    bool getQuoteAlways();
    bool getQuoteCharsAreRegular();
    bool getQuoteMixedCase();
};
//----------------------------------------------------------------------------
IdentifierQuotes::IdentifierQuotes()
    : ConfigCache(config()), quoteAlwaysM(false),
        quoteCharsAreRegularM(false), quoteMixedCaseM(true)
{
}
//-----------------------------------------------------------------------------
IdentifierQuotes& IdentifierQuotes::get()
{
    static IdentifierQuotes iq;
    return iq;
}
//-----------------------------------------------------------------------------
void IdentifierQuotes::loadFromConfig()
{
    quoteAlwaysM = !config().get(QString::fromLatin1("quoteOnlyWhenNeeded"), true);
    quoteCharsAreRegularM = config().get(QString::fromLatin1("quoteCharsAreRegular"), false);
    quoteMixedCaseM = config().get(QString::fromLatin1("quoteMixedCase"), false);
}
//----------------------------------------------------------------------------
bool IdentifierQuotes::getQuoteAlways()
{
    ensureCacheValid();
    return quoteAlwaysM;
}
//----------------------------------------------------------------------------
bool IdentifierQuotes::getQuoteCharsAreRegular()
{
    ensureCacheValid();
    return quoteCharsAreRegularM;
}
//----------------------------------------------------------------------------
bool IdentifierQuotes::getQuoteMixedCase()
{
    ensureCacheValid();
    return quoteMixedCaseM;
}
//----------------------------------------------------------------------------
// Identifier class
Identifier::Identifier(const QString& source)
{
    setText(source);
}
//----------------------------------------------------------------------------
Identifier::Identifier()
{
}
//----------------------------------------------------------------------------
void Identifier::setText(const QString& source)
{
    // although it may not be completely correct we right-trim everything we
    // get. This means that users can't use quoted identifiers which end with
    // a space - but who does that anyway
    textM = helper::stripString(source);
}
//----------------------------------------------------------------------------
void Identifier::setFromSql(const QString& source)
{
    // const QChar pointers to first and last characters
    const QChar* p = source.constData();
    const QChar* q = p + source.length() - 1;
    // skip leading and trailing whitespace
    while (q > p && p->isSpace())
        p++;
    while (q > p && q->isSpace())
        q--;
    if (p > q) // p is first, q is last character, so they may be equal...
    {
        textM = QString();
        return;
    }
    // strings/quoted identifier -> strip and unescape single/double quotes
    if (*q == *p && (*p == '\"' || *p == '\''))
    {
        // NOTE: first parameter must point to first char, but second parameter
        //       has to point to the char *after* the last char !!!
        textM = helper::rangedString(p+1, q); //wxString(p + 1, q);
        QString escapedChar(p);
        textM.replace(escapedChar + escapedChar, escapedChar);
        return;
    }
    // set to uppercased input parameter, no leading and trailing whitespace
    // textM = QString(p, (q + 1) - (p) + 1).toUpper();
	textM = helper::rangedString(p, q + 1).toUpper();
}
//----------------------------------------------------------------------------
bool Identifier::isQuoted(const QString &s)
{
    int p = s.length();
    return (s[0] == QChar('\"') && p > 1 && s[p - 1] == QChar('\"'));
}
//----------------------------------------------------------------------------
QString& Identifier::escape(QString& s)
{
    s.replace(QString::fromLatin1("\""), QString::fromLatin1("\"\""));
    return s;
}
//----------------------------------------------------------------------------
QString& Identifier::strip(QString& s)
{
    if (isQuoted(s))
        // s = s.SubString(1, s.length()-2);
		s = s.mid(1, s.length()-2);
    return s;
}
//----------------------------------------------------------------------------
QString& Identifier::quote(QString &s)
{
    s = QString::fromLatin1("\"") + s + QString::fromLatin1("\"");
    return s;
}
//----------------------------------------------------------------------------
bool hasBothCases(const QString& value)
{
    if (value.isEmpty())
        return false;

    bool hasLower = false;
    bool hasUpper = false;
    const QChar* p = value.constData();
    while (*p != 0)
    {
        if (*p >= 'A' && *p <= 'Z')
            hasUpper = true;
        if (*p >= 'a' && *p <= 'z')
            hasLower = true;
        if (hasUpper && hasLower)
            return true;
        p++;
    }
    return false;
}
//----------------------------------------------------------------------------
QString Identifier::userString(const QString& s)
{
    if (s.isEmpty())
        return QString();
    QString ret(s);
    if (IdentifierQuotes::get().getQuoteAlways())
    {
        if (IdentifierQuotes::get().getQuoteCharsAreRegular())
            return quote(escape(ret));
        else
            return quote(escape(strip(ret)));
    }
    else
    {
        if (isQuoted(ret))   // pass the quoted text as-it-is
            return ret;
        if (IdentifierQuotes::get().getQuoteMixedCase() && hasBothCases(ret))
            return quote(escape(ret));
        if (Identifier::needsQuoting(ret.toUpper()))    // special chars
            return quote(escape(ret));
        return ret;
    }
}
//----------------------------------------------------------------------------
bool Identifier::needsQuoting(const QString& s)
{
    if (s.isEmpty())
        return false;
    const QChar* p = s.constData();
    // first character: only 'A'..'Z' allowed, else quotes needed
    if (*p < 'A' || *p > 'Z')
        return true;
    p++;
    // after first character: 'A'..'Z', '0'..'9', '_', '$' allowed
    while (*p != 0)
    {
        bool validChar = (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')
            || *p == '_' || *p == '$';
        if (!validChar)
            return true;
        p++;
    }
    // may still need quotes if reserved word
    return SqlTokenizer::isReservedWord(s);
}
//----------------------------------------------------------------------------
bool Identifier::equals(const Identifier& rhs) const
{
    return textM == rhs.textM;
}
//----------------------------------------------------------------------------
bool Identifier::equals(const QString& rhs) const
{
    if (needsQuoting(textM))
        return (0 == rhs.compare(textM));
    else
        return (0 == rhs.compare(textM, Qt::CaseInsensitive));
}
//----------------------------------------------------------------------------
QString Identifier::get() const
{
    return textM;
}
//----------------------------------------------------------------------------
QString Identifier::getQuoted() const
{
    if (IdentifierQuotes::get().getQuoteAlways() || needsQuoting(textM))
    {
        QString retval(textM);
        return quote(escape(retval));
    }
    else
        return textM;
}
//----------------------------------------------------------------------------

} // namespace fr
