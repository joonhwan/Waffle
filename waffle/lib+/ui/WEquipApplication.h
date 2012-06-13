#pragma once

#include <QApplication>

// Application class for any equipment sw where you should do
//
//  - splash window handling
//  - file location/extension/path data store
//  - initialize logger
//  - load application configuration file

class WEquipApplication : public QApplication
{
    Q_OBJECT
public:
    WEquipApplication(QObject* parent=0);
    virtual ~WEquipApplication();
signals:
public slots:
protected:
};
