#pragma once

#include "db/WSqlHeader.h"
#include <QStringList>
#include <iostream>

// 일반적으로는 QSqlField 의 목록을 다루는 것이 더 완전함.
// WSqlTableFieldNameList 는 편이용.
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
