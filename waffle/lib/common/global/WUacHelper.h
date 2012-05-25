#pragma once

#include <QtGlobal>

// from : microsoft 'all-in-one' code framework sample named "CppUac"
// (url: http://1code.codeplex.com/)
namespace Wf {

bool isUacAvailable();

bool isUserInAdminGroup(quint32* errorCode=0);

bool isRunAsAdmin(quint32* errorCode=0);

bool isProcessElevated(quint32* errorCode=0);

quint32 GetProcessIntegrityLevel(quint32* errorCode=0);

};
