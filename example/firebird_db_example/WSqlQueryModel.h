#pragma once

#include "db/WSqlRelationalTableDefinition.h"
#include <QSqlQueryModel>

class QTimer;

class WSqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    WSqlQueryModel(QObject* parent=0);
    virtual ~WSqlQueryModel();
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	virtual void fetchMore(const QModelIndex &parent = QModelIndex());
	void setDefinition(WSqlRelationalTableDefinition& definition);
signals:
public slots:
private slots:
	void tryFetchMoreSilently();
protected:
	QTimer* m_timer;
	WSqlRelationalTableDefinition m_definition;
};
