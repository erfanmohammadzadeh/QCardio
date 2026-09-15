#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include <QObject>
#include <QSettings>
#include "Models/uiconfigs.h"

class SettingsService : public QObject
{
    Q_OBJECT
public:
    explicit SettingsService(QObject *parent = nullptr);
    void saveSetting(const UIConfigs& uiConfig);
    void loadSetting(UIConfigs *uiConfig);

    QSettings m_setting;
};

#endif // SETTINGSSERVICE_H
