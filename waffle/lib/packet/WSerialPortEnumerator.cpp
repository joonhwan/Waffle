#include "WSerialPortEnumerator.h"

#include <QString>
#include <QtCore/qt_windows.h>
#include <tchar.h>

QStringList WSerialPortEnumerator::getPorts()
{
	QStringList ports;

	//What will be the return value from this function (assume the worst)
	HKEY hSERIALCOMM = 0;
	do {
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						 _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
						 0,
						 KEY_QUERY_VALUE,
						 &hSERIALCOMM) != ERROR_SUCCESS) {
			hSERIALCOMM = 0;
			break;
		}

		//Get the max value name and max value lengths
		DWORD dwMaxValueNameLen;
		DWORD dwMaxValueLen;
		DWORD dwQueryInfo = RegQueryInfoKey(hSERIALCOMM,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,
											&dwMaxValueNameLen,
											&dwMaxValueLen,
											NULL,
											NULL);
		if (dwQueryInfo != ERROR_SUCCESS) {
			break;
		}

		DWORD dwMaxValueNameSizeInChars = dwMaxValueNameLen + 1; //Include space for the NULL terminator
		DWORD dwMaxValueNameSizeInBytes = dwMaxValueNameSizeInChars * sizeof(TCHAR);
		DWORD dwMaxValueDataSizeInChars = dwMaxValueLen/sizeof(TCHAR) + 1; //Include space for the NULL terminator
		DWORD dwMaxValueDataSizeInBytes = dwMaxValueDataSizeInChars * sizeof(TCHAR);

		//Allocate some space for the value name and value data
		QByteArray szValueName(dwMaxValueNameSizeInBytes, 0);
		QByteArray byValue(dwMaxValueDataSizeInBytes, 0);

		if (szValueName.size() < (int)dwMaxValueNameSizeInChars
			|| byValue.size() < (int)dwMaxValueDataSizeInBytes) {
			break;
		}

		//Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
		DWORD dwIndex = 0;
		DWORD dwType;
		DWORD dwValueNameSize = dwMaxValueNameSizeInChars;
		DWORD dwDataSize = dwMaxValueDataSizeInBytes;
		LONG nEnum = RegEnumValue(hSERIALCOMM,
								  dwIndex,
								  szValueName.data(),
								  &dwValueNameSize,
								  NULL,
								  &dwType,
								  (LPBYTE)byValue.data(),
								  &dwDataSize);
		while (nEnum == ERROR_SUCCESS)
		{
			//If the value is of the correct type, then add it to the array
			if (dwType == REG_SZ)
			{
				QString portName = QString::fromLatin1(byValue.constData());
				ports << portName;
			}

			//Prepare for the next time around
			dwValueNameSize = dwMaxValueNameSizeInChars;
			dwDataSize = dwMaxValueDataSizeInBytes;
			szValueName.fill(0);
			byValue.fill(0);
			++dwIndex;
			nEnum = RegEnumValue(hSERIALCOMM,
								 dwIndex,
								 szValueName.data(),
								 &dwValueNameSize,
								 NULL,
								 &dwType,
								 (LPBYTE)byValue.data(),
								 &dwDataSize);
		}
	} while (0);

	//Close the registry key now that we are finished with it
	if (hSERIALCOMM) {
		RegCloseKey(hSERIALCOMM);
	}

	return ports;
}
