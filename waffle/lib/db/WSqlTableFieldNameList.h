#pragma once

#include "db/WSqlHeader.h"
#include <QStringList>
#include <iostream>

// �Ϲ������δ� QSqlField �� ����� �ٷ�� ���� �� ������.
// WSqlTableFieldNameList �� ���̿�.
//
// example)
// std::cout << WSqlTableNameList("MyTable") << std::endl;
// qDebug() << WSqlTableNameList("YourTable");
class WSqlTableFieldNameList : public QStringList
{
public:
    WSqlTableFieldNameList(const QString& tableName, QSqlDatabase db = QSqlDatabase());
	WSqlTableFieldNameList(QSqlQuery& query);
    virtual ~WSqlTableFieldNameList();
	bool buildListFrom(QSqlQuery& query);
};

std::ostream& operator << (std::ostream& os, const WSqlTableFieldNameList& me);
