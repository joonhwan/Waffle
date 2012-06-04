#include "WSqlQueryModel.h"
#include <QTimer>
#include <QDebug>
#include <QSqlQuery>

WSqlQueryModel::WSqlQueryModel(QObject* parent)
	: QSqlQueryModel(parent)
{
	m_timer = new QTimer(this);
	m_timer->setInterval(0);
	connect(m_timer, SIGNAL(timeout()), SLOT(tryFetchMoreSilently()));
}

//virtual
WSqlQueryModel::~WSqlQueryModel()
{
}

//virtual
QVariant WSqlQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        // QVariant val = d->headers.value(section).value(role);
        // if (role == Qt::DisplayRole && !val.isValid())
        //     val = d->headers.value(section).value(Qt::EditRole);
        // if (val.isValid())
        //     return val;

        // // See if it's an inserted column (iiq.column() != -1)
        // QModelIndex dItem = indexInQuery(createIndex(0, section));

        // if (role == Qt::DisplayRole && d->rec.count() > section && dItem.column() != -1)
        //     return d->rec.fieldName(section);
		if (role == Qt::DisplayRole && (section >= 0 && section < m_definition.size())) {
			return m_definition[section].displayName();
		}
    }
    return QSqlQueryModel::headerData(section, orientation, role);
}


//virtual
void WSqlQueryModel::fetchMore(const QModelIndex &parent)
{
	QSqlQueryModel::fetchMore(parent);

	if (canFetchMore(parent)) {
		m_timer->start();
	}
}

void WSqlQueryModel::setDefinition(WSqlRelationalTableDefinition& definition)
{
	m_definition = definition;

	QSqlQuery query(m_definition.query());
	setQuery(query);
}

void WSqlQueryModel::tryFetchMoreSilently()
{
	if (!canFetchMore()) {
		qDebug() << "fetch more stopped.";
		m_timer->stop();
	} else {
		fetchMore();
		qDebug() << "fetched more... count: " << this->rowCount();
	}
}
