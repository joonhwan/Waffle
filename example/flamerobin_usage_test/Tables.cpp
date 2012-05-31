#include "Tables.h"

WSqlTableDefinition locationTable;
WSqlTableDefinition departmentTable;
WSqlTableDefinition employeeTable;
WSqlRelationalTableDefinition allEmployeeData;

namespace {

class TableDefiner : public QObject
{
public:
	TableDefiner()
	{
		locationTable =
			WSqlTableDefinition("LOCATION")
			.serialPrimaryKey("id")
			.field("name", tr("Location Name"), "varchar(40)", Wf::DbNotNull, "noname")
			;

		departmentTable =
			WSqlTableDefinition("DEPARTMENT")
			.serialPrimaryKey("id")
			.field("name", tr("Department Name"), "varchar(40)", Wf::DbNotNull, "noname")
			.field("code", tr("Department Code"), "varchar(8)", Wf::DbNotNull, "NN")
			.foreignSerialKey("location_id", "LOCATION", "id")
			;

		employeeTable =
			WSqlTableDefinition("EMPLOYEE")
			.serialPrimaryKey("id")
			.foreignSerialKey("department_id", "DEPARTMENT", "id")
			.field("name", tr("Name"), "varchar(40)", Wf::DbNotNull, "noname")
			.field("age", tr("Age"), "integer", Wf::DbNotNull)
			.field("extension", tr("Extension"), "integer", Wf::DbNotNull)
			.field("email", tr("Email"), "varchar(40)", Wf::DbNotNull)
			.field("startdate", tr("Start Date"), "date", Wf::DbNotNull)
			;

		allEmployeeData =
			WSqlRelationalTableDefinition()
			// fields
			.addField(employeeTable["name"])
			.addField(departmentTable["name"])
			.addField(locationTable["name"])
			.addField(employeeTable["age"])
			.addField(employeeTable["email"])
			.addField(employeeTable["extension"])
			.addField(employeeTable["startdate"])
			// relation
			.addRelation(employeeTable["department_id"], departmentTable["id"])
			.addRelation(departmentTable["location_id"], locationTable["id"])
			;
	}
} _tableDefiner;

}
