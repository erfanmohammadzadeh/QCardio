#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <QObject>
#include <QSettings>
#include "Models/uiconfigs.h"
class CSettings : public QObject
{
    Q_OBJECT
public:
    explicit CSettings(QObject *parent = nullptr);
    void saveSetting(const UIConfigs& uiConfig);
    void loadSetting(UIConfigs *uiConfig);

    QSettings m_setting;

signals:
};

#endif // CSETTINGS_H
