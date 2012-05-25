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


  $Id: MetadataTemplateCmdHandler.cpp 2200 2012-01-20 08:31:01Z mghie $

*/
#include "core/ProcessableObject.h"
#include "core/StringUtils.h"
#include "core/TemplateProcessor.h"
#include "metadata/CreateDDLVisitor.h"
#include "metadata/column.h"
#include "metadata/database.h"
#include "metadata/exception.h"
#include "metadata/function.h"
#include "metadata/generator.h"
#include "metadata/parameter.h"
#include "metadata/privilege.h"
#include "metadata/procedure.h"
#include "metadata/relation.h"
#include "metadata/role.h"
#include "metadata/server.h"
#include "metadata/table.h"
#include "metadata/User.h"
#include "metadata/view.h"
#include "flamerobin-port-helper.h"

namespace fr {

//-----------------------------------------------------------------------------
class MetadataTemplateCmdHandler: public TemplateCmdHandler
{
private:
    static const MetadataTemplateCmdHandler handlerInstance; // singleton; registers itself on creation.
public:
    MetadataTemplateCmdHandler() {};
    virtual void handleTemplateCmd(TemplateProcessor *tp,
        const QString& cmdName, const TemplateCmdParams& cmdParams,
        ProcessableObject* object, QString& processedText);
};
//-----------------------------------------------------------------------------
const MetadataTemplateCmdHandler MetadataTemplateCmdHandler::handlerInstance;
//-----------------------------------------------------------------------------
void MetadataTemplateCmdHandler::handleTemplateCmd(TemplateProcessor *tp,
    const QString& cmdName, const TemplateCmdParams& cmdParams,
    ProcessableObject* object, QString& processedText)
{
    struct Local
    {
        // Implements the body of all {%foreach%} loops.
        static void foreachIteration(bool& firstItem,
            TemplateProcessor* tp, QString& processedText,
            QString separator, QString text, ProcessableObject* object)
        {
            QString newText;
            tp->internalProcessTemplateText(newText, text, object);
            if ((!firstItem) && (!newText.isEmpty()))
                processedText += tp->escapeChars(separator);
            if (!newText.isEmpty())
                firstItem = false;
            processedText += newText;
        }
    };

    MetadataItem* metadataItem = dynamic_cast<MetadataItem*>(object);

    // {%parent:text%}
    // Switches the current object to the object's parent and
    // processes the specified text. If the object has no parent,
    // expands to a blank string.
    if ((cmdName == QString::fromLatin1("parent")) && metadataItem && metadataItem->getParent())
        tp->internalProcessTemplateText(processedText, cmdParams.all(),
            metadataItem->getParent());

    // {%database:text%}
    // Switches the current object to the object's database, if any, and
    // processes the specified text. If the object has no database,
    // expands to a blank string.
    if ((cmdName == QString::fromLatin1("database")) && metadataItem && metadataItem->getDatabase())
        tp->internalProcessTemplateText(processedText, cmdParams.all(),
            metadataItem->getDatabase().get());

    // {%object_handle%}
    // Expands to the current object's unique numeric handle.
    // Used to call FR's commands through URIs.
    else if ((cmdName == QString::fromLatin1("object_handle")) && metadataItem)
        processedText += QString::fromLatin1("%1").arg(metadataItem->getHandle());

    // {%object_name%}
    // Expands to the current object's (non quoted) name.
    else if ((cmdName == QString::fromLatin1("object_name")) && metadataItem)
        processedText += metadataItem->getName_();

    // {%object_quoted_name%}
    // Expands to the current object's quoted name.
    else if ((cmdName == QString::fromLatin1("object_quoted_name")) && metadataItem)
        processedText += metadataItem->getQuotedName();

    // {%object_path%}
    // Expands to the current object's full path in the DBH.
    else if ((cmdName == QString::fromLatin1("object_path")) && metadataItem)
        processedText += metadataItem->getItemPath();

    // {%object_type%}
    // Expands to the current object's type name.
    else if ((cmdName == QString::fromLatin1("object_type")) && metadataItem)
        processedText += metadataItem->getTypeName();

    // {%is_system%}
    // Expands to "true" if the current object is a system object,
    // and to "false" otherwise.
    else if ((cmdName == QString::fromLatin1("is_system")) && metadataItem)
        processedText += getBooleanAsString(metadataItem->isSystem());

    // {%foreach:<collection>:<separator>:<text>%}
    // Repeats <text> once for each item in <collection>, pasting a <separator>
    // before each item except the first.
    else if ((cmdName == QString::fromLatin1("foreach")) && (cmdParams.size() >= 3))
    {
        QString sep;
        tp->internalProcessTemplateText(sep, cmdParams[1], object);
        
        // {%foreach:column:<separator>:<text>%}
        // If the current object is a relation, processes <text> for each column.
        if (cmdParams[0] == QString::fromLatin1("column"))
        {
            Relation* r = dynamic_cast<Relation*>(object);
            if (!r)
                return;
            r->ensureChildrenLoaded();
            bool firstItem = true;
            for (ColumnPtrs::iterator it = r->begin(); it != r->end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(2), (*it).get());
            }
        }

        // {%foreach:foreign_key:<separator>:<text>%}
        // If the current object is a table, processes <text> for each foreign key.
        else if (cmdParams[0] == QString::fromLatin1("foreign_key"))
        {
            Table* t = dynamic_cast<Table*>(object);
            if (!t)
                return;
            std::vector<ForeignKey>* fks = t->getForeignKeys();
            if (!fks)
                return;
            bool firstItem = true;
            for (std::vector<ForeignKey>::iterator it = fks->begin(); it != fks->end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(2), &(*it));
            }
        }

        // {%foreach:check_constraint:<separator>:<text>%}
        // If the current object is a table, processes <text> for each check constraint.
        else if (cmdParams[0] == QString::fromLatin1("check_constraint"))
        {
            Table* t = dynamic_cast<Table*>(object);
            if (!t)
                return;
            std::vector<CheckConstraint>* c = t->getCheckConstraints();
            if (!c)
                return;
            bool firstItem = true;
            for (std::vector<CheckConstraint>::iterator it = c->begin(); it != c->end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(2), &(*it));
            }
        }

        // {%foreach:unique_constraint:<separator>:<text>%}
        // If the current object is a table, processes <text> for each unique constraint.
        else if (cmdParams[0] == QString::fromLatin1("unique_constraint"))
        {
            Table* t = dynamic_cast<Table*>(object);
            if (!t)
                return;
            std::vector<UniqueConstraint>* c = t->getUniqueConstraints();
            if (!c)
                return;
            bool firstItem = true;
            for (std::vector<UniqueConstraint>::iterator it = c->begin(); it != c->end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(2), &(*it));
            }
        }

        // {%foreach:index:<separator>:<text>%}
        // If the current object is a table, processes <text> for each index.
        else if (cmdParams[0] == QString::fromLatin1("index"))
        {
            Table* t = dynamic_cast<Table*>(object);
            if (!t)
                return;
            std::vector<Index>* ix = t->getIndices();
            if (!ix)
                return;
            bool firstItem = true;
            for (std::vector<Index>::iterator it = ix->begin(); it != ix->end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(2), &(*it));
            }
        }

        // {%foreach:trigger:<separator>:<before|after>:<text>%}
        // If the current object is a relation, processes <text> for
        // each "before" or "after" trigger. If the current object is
        // a database, processes <text> for all database triggers and the
        // third param is ignored.
        else if ((cmdParams[0] == QString::fromLatin1("trigger")) && (cmdParams.size() >= 4))
        {
            std::vector<Trigger*> triggers;
            
            Relation* r = dynamic_cast<Relation*>(object);
            if (r)
            {
                if (cmdParams[2] == QString::fromLatin1("after"))
                    r->getTriggers(triggers, Trigger::afterTrigger);
                else if (cmdParams[2] == QString::fromLatin1("before"))
                    r->getTriggers(triggers, Trigger::beforeTrigger);
            }
            else
            {
                Database* d = dynamic_cast<Database*>(object);
                if (d)
                    d->getDatabaseTriggers(triggers);
            }

            bool firstItem = true;
            for (std::vector<Trigger*>::iterator it = triggers.begin();
                it != triggers.end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(3), *it);
            }
        }

        // {%foreach:privilegeitem:<separator>:<type>:<text>%}
        // If the current object is a privilege, processes <text> for each privilege item
        // of the specified <type> (SELECT, INSERT, UPDATE, DELETE, EXECUTE, REFERENCES,
        // MEMBER OF).
        else if ((cmdParams[0] == QString::fromLatin1("privilegeitem")) && (cmdParams.size() >= 4))
        {
            Privilege* p = dynamic_cast<Privilege*>(object);
            if (!p)
                return;
            PrivilegeItems list;
            p->getPrivilegeItems(cmdParams[2], list);
            bool firstItem = true;
            for (PrivilegeItems::iterator it = list.begin(); it != list.end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(3), &(*it));
            }
        }

        // {%foreach:privilege:<separator>:<text>%}
        // If the current object is a relation, procedure or role,
        // processes <text> for each privilege.
        else if (cmdParams[0] == QString::fromLatin1("privilege"))
        {
            Relation* rel = dynamic_cast<Relation*>(object);
            Procedure* proc = dynamic_cast<Procedure*>(object);
            Role* role = dynamic_cast<Role*>(object);
            std::vector<Privilege>* p = 0;
            if (rel)
                p = rel->getPrivileges();
            if (proc)
                p = proc->getPrivileges();
            if (role)
                p = role->getPrivileges();
            if (!p)
                return;
            bool firstItem = true;
            for (std::vector<Privilege>::iterator it = p->begin(); it != p->end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(2), &(*it));
            }
        }

        // {%foreach:depends_on:<separator>:<text>%}
        // Lists all objects on which the current object depends.
        // {%foreach:dependent:<separator>:<text>%}
        // Lists all objects that depend on the current object.
        else if (cmdParams[0] == QString::fromLatin1("depends_on") || cmdParams[0] == QString::fromLatin1("dependent"))
        {
            if (!metadataItem)
                return;

            std::vector<Dependency> deps;
            metadataItem->getDependencies(deps, cmdParams[0] == QString::fromLatin1("depends_on"));
            bool firstItem = true;
            for (std::vector<Dependency>::iterator it = deps.begin(); it != deps.end(); ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(2), &(*it));
            }
        }

        // {%foreach:parameter:<separator>:<input|output>:<text>%}
        // If the current object is a procedure, processes <text> for
        // each "input" or "output" parameter.
        else if ((cmdParams[0] == QString::fromLatin1("parameter")) && (cmdParams.size() >= 4))
        {
            Procedure* p = dynamic_cast<Procedure*>(object);
            if (!p)
                return;

            SubjectLocker locker(p);
            p->ensureChildrenLoaded();
            bool isOut = (cmdParams[2] == QString::fromLatin1("output"));
            bool firstItem = true;
            for (ParameterPtrs::iterator it = p->begin(); it != p->end(); ++it)
            {
                if ((*it)->isOutputParameter() == isOut)
                {
                    Local::foreachIteration(firstItem, tp, processedText, sep,
                        cmdParams.from(3), (*it).get());
                }
            }
        }

        // {%foreach:user:<separator>:<text>%}
        // If the current object is a server, processes
        // the specified text once for each defined user,
        // switching each time the current object to the nth user.
        else if (cmdParams[0] == QString::fromLatin1("user"))
        {
            Server* s = dynamic_cast<Server*>(object);
            if (!s)
                return;

            ProgressIndicator* pi = tp->getProgressIndicator();
            if (pi)
            {
                pi->initProgress(_("Connecting to Server..."));
                pi->doShow();
            }

            UserPtrs users = s->getUsers(pi);
            if (users.empty())
                return;

            bool firstItem = true;
            for (UserPtrs::iterator it = users.begin(); it != users.end();
                ++it)
            {
                Local::foreachIteration(firstItem, tp, processedText, sep,
                    cmdParams.from(2), (*it).get());
            }
        }
        // add more collections here.
        else
            return;
    }

    // {%owner_name%}
    // If the current object is a procedure, relation or role
    // expands to the owner's name.
    else if (cmdName == QString::fromLatin1("owner_name"))
    {
        QString name;
        if (Relation* r = dynamic_cast<Relation*>(object))
            name = r->getOwner();
        else if (Procedure* p = dynamic_cast<Procedure*>(object))
            name = p->getOwner();
        else if (Role* r = dynamic_cast<Role*>(object))
            name = r->getOwner();
        if (!name.isEmpty())
            processedText += tp->escapeChars(name);
    }

    // {%object_description%}
    // Expands to the current object's description, if available.
    // Otherwise expands to an empty string.
    else if ((cmdName == QString::fromLatin1("object_description")) && metadataItem)
    {
        QString desc;
        if (metadataItem->getDescription(desc))
        {
            if (desc.isEmpty())
                desc = _("No description");
            processedText += tp->escapeChars(desc);
        }
    }

    // {%dependencyinfo:property%}
    // If the current object is a dependency, expands to the requested
    // property of the dependency object.
    else if ((cmdName == QString::fromLatin1("dependencyinfo")) && (cmdParams.size() >= 1)
        && (cmdParams[0] == QString::fromLatin1("fields")))
    {
        Dependency* d = dynamic_cast<Dependency*>(object);
        if (!d)
            return;

        processedText += d->getFields();
    }

    // {%primary_key:<text>%}
    // If the current object is a table, processes <text> after switching
    // the current object to the table's primary key.
    else if (cmdName == QString::fromLatin1("primary_key"))
    {
        Table* t = dynamic_cast<Table*>(object);
        if (!t)
            return;

        PrimaryKeyConstraint* pk = t->getPrimaryKey();
        if (!pk)
            return;

        tp->internalProcessTemplateText(processedText, cmdParams.all(), pk);
    }

    // {%no_pk_or_unique%}
    // Expands to true if the current object is a table without a primary
    // key or at least one unique constraint, false otherwise.
    else if (cmdName == QString::fromLatin1("no_pk_or_unique"))
    {
        Table* t = dynamic_cast<Table*>(object);
        if (!t)
            return;

        if (!t->getPrimaryKey() && t->getUniqueConstraints()->size() == 0)
            processedText += getBooleanAsString(true);
        else
            processedText += getBooleanAsString(false);
    }

    // {%checkconstraintinfo:<property>%}
    // If the current object is a check constraint, expands to the constraint's
    // requested property.
    else if ((cmdName == QString::fromLatin1("checkconstraintinfo")) && (cmdParams.size() >= 1))
    {
        CheckConstraint* c = dynamic_cast<CheckConstraint*>(object);
        if (!c)
            return;

        if (cmdParams[0] == QString::fromLatin1("source"))
            processedText += tp->escapeChars(c->getSource(), false);
    }

    // {%constraintinfo:columns%} (uses default separator and suffix)
    // {%constraintinfo:columns:separator%} (uses default suffix)
    // {%constraintinfo:columns:separator:suffix%}
    // If the current object is a column constraint (primary key, unique
    // constraint, foreign key, check constraint), expands to
    // a delimited list of constraint columns.
    else if ((cmdName == QString::fromLatin1("constraintinfo")) && (cmdParams.size() >= 1)
        && (cmdParams[0] == QString::fromLatin1("columns")))
    {
        ColumnConstraint* c = dynamic_cast<ColumnConstraint*>(object);
        if (!c)
            return;

        if (cmdParams.size() == 1)
            processedText += c->getColumnList();
        else if (cmdParams.size() == 2)
            processedText += c->getColumnList(cmdParams[1]);
        else if (cmdParams.size() >= 3)
            processedText += c->getColumnList(cmdParams[1], cmdParams.from(2));
    }

    // {%fkinfo:<property>%}
    // If the current object is a foreign key, expands to the foreign key's
    // requested property.
    else if ((cmdName == QString::fromLatin1("fkinfo")) && (cmdParams.size() >= 1))
    {
        ForeignKey* fk = dynamic_cast<ForeignKey*>(object);
        if (!fk)
            return;

        if (cmdParams[0] == QString::fromLatin1("ref_columns"))
            processedText += tp->escapeChars(fk->getReferencedColumnList());
        else if (cmdParams[0] == QString::fromLatin1("ref_table"))
            processedText += tp->escapeChars(fk->getReferencedTable());
        else if (cmdParams[0] == QString::fromLatin1("update_action"))
            processedText += tp->escapeChars(fk->getUpdateAction());
        else if (cmdParams[0] == QString::fromLatin1("delete_action"))
            processedText += tp->escapeChars(fk->getDeleteAction());
    }

    // {%columninfo:<property>%}
    // If the current object is a column, expands to the column's
    // requested property.
    else if ((cmdName == QString::fromLatin1("columninfo")) && (cmdParams.size() >= 1))
    {
        ColumnBase* cb = dynamic_cast<ColumnBase*>(object);
        if (!cb)
            return;

        if (cmdParams[0] == QString::fromLatin1("datatype"))
            processedText += tp->escapeChars(cb->getDatatype());
        else if (cmdParams[0] == QString::fromLatin1("is_nullable"))
        {
            processedText += tp->escapeChars(getBooleanAsString(
                cb->isNullable()));
        }
        else if (cmdParams[0] == QString::fromLatin1("null_option"))
        {
            if (!cb->isNullable())
                processedText += tp->escapeChars(QString::fromLatin1("not null"));
        }
        else if (cmdParams[0] == QString::fromLatin1("default_expression"))
        {
            QString def(cb->getDefault());
            // def.Trim(false);
			def = helper::stripString(def, helper::leading);
            if (def.toUpper().startsWith(QString::fromLatin1("DEFAULT")))
            {
                def.remove(0, 7);
                // def.Trim(false);
				def = helper::stripString(def, helper::leading);
            }
            processedText += tp->escapeChars(def, false);
        }
    }

    // {%viewinfo:<property>%}
    // If the current object is a view, expands to the view's
    // requested property.
    else if ((cmdName == QString::fromLatin1("viewinfo")) && (cmdParams.size() >= 1))
    {
        View* v = dynamic_cast<View*>(object);
        if (!v)
            return;

        if (cmdParams[0] == QString::fromLatin1("source"))
            processedText += tp->escapeChars(v->getSource(), false);
    }

    // {%procedureinfo:<property>%}
    // If the current object is a procedure, expands to the procedure's
    // requested property.
    else if ((cmdName == QString::fromLatin1("procedureinfo")) && (cmdParams.size() >= 1))
    {
        Procedure* p = dynamic_cast<Procedure*>(object);
        if (!p)
            return;

        if (cmdParams[0] == QString::fromLatin1("source"))
            processedText += tp->escapeChars(p->getSource(), false);
        // {%procedureinfo:paramcount[:input|output]%}
        // Expands to the number of input or output parameters.
        // If no argument is specified, expands to the total number of
        // parameters.
        else if (cmdParams[0] == QString::fromLatin1("paramcount"))
        {
            size_t paramCount(0);
            p->ensureChildrenLoaded();
            if (cmdParams.size() >= 2)
            {
                bool isOut = (cmdParams[1] == QString::fromLatin1("output"));
                for (ParameterPtrs::iterator it = p->begin(); it != p->end(); ++it)
                {
                    if ((*it)->isOutputParameter() == isOut)
                        paramCount++;
                }
            }
            else
                paramCount = p->getParamCount();

            processedText += tp->escapeChars(
					QString::fromLatin1("%1").arg(paramCount),
					false);
        }
    }

    // {%triggerinfo:<property>%}
    // If the current object is a trigger, expands to the trigger's
    // requested property.
    else if ((cmdName == QString::fromLatin1("triggerinfo")) && (cmdParams.size() >= 1))
    {
        Trigger* t = dynamic_cast<Trigger*>(object);
        if (!t)
            return;

        if (cmdParams[0] == QString::fromLatin1("source"))
            processedText += tp->escapeChars(t->getSource(), false);
        else
        {
            QString object, type;
            bool isActive, isDBTrigger;
            int position;
            t->getTriggerInfo(object, isActive, position, type, isDBTrigger);
            if (cmdParams[0] == QString::fromLatin1("name"))
                processedText += tp->escapeChars(object);
            else if (cmdParams[0] == QString::fromLatin1("is_active"))
                processedText += tp->escapeChars(getBooleanAsString(isActive));
            else if (cmdParams[0] == QString::fromLatin1("position"))
                processedText += position;
            else if (cmdParams[0] == QString::fromLatin1("type"))
                processedText += tp->escapeChars(type);
            else if (cmdParams[0] == QString::fromLatin1("is_db_trigger"))
                processedText += tp->escapeChars(getBooleanAsString(isDBTrigger));
        }
    }

    // {%generatorinfo:<property>%}
    // If the current object is a generator, expands to the generator's
    // requested property.
    else if ((cmdName == QString::fromLatin1("generatorinfo")) && (cmdParams.size() >= 1))
    {
        Generator* g = dynamic_cast<Generator*>(object);
        if (!g)
            return;

        if (cmdParams[0] == QString::fromLatin1("value"))
            processedText += g->getValue();
    }

    // {%exceptioninfo:<property>%}
    // If the current object is an exception, expands to the exception's
    // requested property.
    else if ((cmdName == QString::fromLatin1("exceptioninfo")) && (cmdParams.size() >= 1))
    {
        Exception* e = dynamic_cast<Exception*>(object);
        if (!e)
            return;

        if (cmdParams[0] == QString::fromLatin1("number"))
            processedText += e->getNumber();
        else if (cmdParams[0] == QString::fromLatin1("message"))
            processedText += e->getMessage();
    }

    // {%functioninfo:<property>%}
    // If the current object is a function, expands to the function's
    // requested property.
    else if (cmdName == QString::fromLatin1("functioninfo") && !cmdParams.isEmpty())
    {
        Function* f = dynamic_cast<Function*>(object);
        if (!f)
            return;

        if (cmdParams[0] == QString::fromLatin1("library"))
            processedText += tp->escapeChars(f->getLibraryName());
        else if (cmdParams[0] == QString::fromLatin1("entry_point"))
            processedText += tp->escapeChars(f->getEntryPoint());
        else if (cmdParams[0] == QString::fromLatin1("definition"))
            processedText += tp->escapeChars(f->getDefinition(), false);
    }

    // {%indexinfo:<property>%}
    // If the current object is an index, expands to the index's
    // requested property.
    else if (cmdName == QString::fromLatin1("indexinfo") && !cmdParams.isEmpty())
    {
        Index* i = dynamic_cast<Index*>(object);
        if (!i)
            return;

        if (cmdParams[0] == QString::fromLatin1("type"))
            processedText += (i->getIndexType() == Index::itAscending ? QString::fromLatin1("ASC") : QString::fromLatin1("DESC"));
        else if (cmdParams[0] == QString::fromLatin1("stats"))
            processedText += QString::fromLatin1("%1").arg(i->getStatistics(), 0, 'f', 6);
        else if (cmdParams[0] == QString::fromLatin1("fields"))
            processedText += i->getFieldsAsString();
        else if (cmdParams[0] == QString::fromLatin1("is_active"))
            processedText += getBooleanAsString(i->isActive());
        else if (cmdParams[0] == QString::fromLatin1("is_unique"))
            processedText += getBooleanAsString(i->isUnique());
        else if (cmdParams[0] == QString::fromLatin1("is_ascending"))
            processedText += getBooleanAsString(i->getIndexType() == Index::itAscending);
    }

    // {%object_ddl%}
    // Expands to the current object's DDL definition.
    else if ((cmdName == QString::fromLatin1("object_ddl")) && metadataItem)
    {
        ProgressIndicator* pi = tp->getProgressIndicator();
        if (pi)
        {
            pi->setProgressLevelCount(2);
            pi->initProgress(_("Extracting DDL Definitions"));
            pi->doShow();
        }
        CreateDDLVisitor cdv(pi);
        metadataItem->acceptVisitor(&cdv);
        processedText += tp->escapeChars(cdv.getSql(), false);
    }

    // {%dbinfo:<property>%}
    // If the current object is a database, expands to the database's
    // requested property.
    else if (cmdName == QString::fromLatin1("dbinfo") && !cmdParams.isEmpty())
    {
        Database* db = dynamic_cast<Database*>(object);
        if (!db)
            return;

        if (cmdParams[0] == QString::fromLatin1("connection_string"))
            processedText += db->getConnectionString();
        else if (cmdParams[0] == QString::fromLatin1("ods_version"))
        {
            processedText += QString::number(db->getInfo().getODS());
            if (db->getInfo().getODSMinor())
            {
                processedText += QString::fromLatin1(".");
                processedText += QString::number(db->getInfo().getODSMinor());
            }
        }
        else if (cmdParams[0] == QString::fromLatin1("page_size"))
            processedText += QString::number(db->getInfo().getPageSize());
        else if (cmdParams[0] == QString::fromLatin1("pages"))
            processedText += QString::number(db->getInfo().getPages());
        else if (cmdParams[0] == QString::fromLatin1("size"))
        {
            int64_t size = db->getInfo().getSizeInBytes();
            const double kilo = 1024;
            const double mega = kilo * kilo;
            const double giga = kilo * mega;
            if (size >= giga)
                // processedText += QString::Format(QString::fromLatin1("%0.2fGB"), size / giga);
				processedText += QString::fromLatin1("%1GB").arg(size / giga, 0, 'f', 2);
            else if (size >= mega)
                // processedText += QString::Format(QString::fromLatin1("%0.2fMB"), size / mega);
				processedText += QString::fromLatin1("%1MB").arg(size / mega, 0, 'f', 2);
            else
                // processedText += QString::Format(QString::fromLatin1("%0.2fkB"), size / kilo);
				processedText += QString::fromLatin1("%1kB").arg(size / kilo, 0, 'f', 2);
        }
        else if (cmdParams[0] == QString::fromLatin1("page_buffers"))
            processedText += QString::number(db->getInfo().getBuffers());
        else if (cmdParams[0] == QString::fromLatin1("reserve_space"))
            processedText += getBooleanAsString(db->getInfo().getReserve());
        else if (cmdParams[0] == QString::fromLatin1("is_read_only"))
            processedText += getBooleanAsString(db->getInfo().getReadOnly());
        else if (cmdParams[0] == QString::fromLatin1("sql_dialect"))
            processedText += QString::number(db->getInfo().getDialect());
        else if (cmdParams[0] == QString::fromLatin1("default_charset"))
            processedText += db->getDatabaseCharset();
        else if (cmdParams[0] == QString::fromLatin1("sweep_interval"))
            processedText += QString::number(db->getInfo().getSweep());
        else if (cmdParams[0] == QString::fromLatin1("forced_writes"))
            processedText += getBooleanAsString(db->getInfo().getForcedWrites());
        else if (cmdParams[0] == QString::fromLatin1("oldest_transaction"))
            processedText += QString::number(db->getInfo().getOldestTransaction());
        else if (cmdParams[0] == QString::fromLatin1("oldest_active_transaction"))
            processedText += QString::number(db->getInfo().getOldestActiveTransaction());
        else if (cmdParams[0] == QString::fromLatin1("oldest_snapshot"))
            processedText += QString::number(db->getInfo().getOldestSnapshot());
        else if (cmdParams[0] == QString::fromLatin1("next_transaction"))
            processedText += QString::number(db->getInfo().getNextTransaction());
        else if (cmdParams[0] == QString::fromLatin1("connected_users"))
        {
            QStringList users;
            db->getConnectedUsers(users);
            processedText += wxArrayToString(users, QString::fromLatin1(","));
        }
    }

    // {%privilegeinfo:<property>%}
    // If the current object is a privilege, expands to the privilege's
    // requested property.
    else if (cmdName == QString::fromLatin1("privilegeinfo") && (cmdParams.size() > 0)) 
    {
        Privilege* p = dynamic_cast<Privilege*>(object);
        if (!p)
            return;
            
        if (cmdParams[0] == QString::fromLatin1("grantee_name"))
            processedText += tp->escapeChars(p->getGrantee());
    }

    // {%privilegeitemcount:<type>%}
    // If the current object is a privilege, expands to the count of
    // privilege items of the requested <type>.
    else if ((cmdName == QString::fromLatin1("privilegeitemcount")) && (cmdParams.size() >= 1))
    {
        Privilege* p = dynamic_cast<Privilege*>(object);
        if (!p)
            return;

        PrivilegeItems list;
        p->getPrivilegeItems(cmdParams[0], list);
        processedText += QString::number(list.size());
    }
    
    // {%privilegeiteminfo:<property>%}
    // If the current object is a privilege item, expands to the privilege
    // item's requested property.
    else if ((cmdName == QString::fromLatin1("privilegeiteminfo")) && (cmdParams.size() >= 1))
    {
        PrivilegeItem* pi = dynamic_cast<PrivilegeItem*>(object);
        if (!pi)
            return;
        
        if (cmdParams[0] == QString::fromLatin1("grantor"))
            processedText += pi->grantor;
        else if (cmdParams[0] == QString::fromLatin1("grant_option"))
            processedText += getBooleanAsString(pi->grantOption);
        else if (cmdParams[0] == QString::fromLatin1("columns"))
        {
            for (std::vector<QString>::iterator it = pi->columns.begin();
                it != pi->columns.end(); ++it)
            {
                if (it != pi->columns.begin())
                    processedText += QString::fromLatin1(",");
                processedText += (*it);
            }
        }
    }

    // {%userinfo:property%}
    // If the current object  is a user, expands to various user properties.
    else if (cmdName == QString::fromLatin1("userinfo") && !cmdParams.isEmpty())
    {
        User* u = dynamic_cast<User*>(object);
        if (!u)
            return;

        if (cmdParams[0] == QString::fromLatin1("username"))
            processedText += tp->escapeChars(u->getUsername());
        else if (cmdParams[0] == QString::fromLatin1("first_name"))
            processedText += tp->escapeChars(u->getFirstName());
        else if (cmdParams[0] == QString::fromLatin1("middle_name"))
            processedText += tp->escapeChars(u->getMiddleName());
        else if (cmdParams[0] == QString::fromLatin1("last_name"))
            processedText += tp->escapeChars(u->getLastName());
        else if (cmdParams[0] == QString::fromLatin1("unix_user"))
            processedText += QString::number(u->getUserId());
        else if (cmdParams[0] == QString::fromLatin1("unix_group"))
            processedText += QString::number(u->getGroupId());
    }
}
//-----------------------------------------------------------------------------

} // namespace fr
