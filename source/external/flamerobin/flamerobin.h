#pragma once

#ifdef WIN32
#define IBPP_WINDOWS
#else
#define IBPP_LINUX
#endif

#include "metadata/database.h"
#include "metadata/collection.h"
#include "metadata/column.h"
#include "metadata/constraints.h"
#include "metadata/domain.h"
#include "metadata/exception.h"
#include "metadata/function.h"
#include "metadata/generator.h"
#include "metadata/parameter.h"
#include "metadata/privilege.h"
#include "metadata/procedure.h"
#include "metadata/relation.h"
#include "metadata/role.h"
#include "metadata/root.h"
#include "metadata/server.h"
#include "metadata/table.h"
#include "metadata/trigger.h"
#include "metadata/User.h"
#include "metadata/view.h"
