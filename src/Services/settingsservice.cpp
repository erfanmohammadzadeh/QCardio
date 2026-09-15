#include "settingsservice.h"

SettingsService::SettingsService(QObject *parent)
    : QObject{parent}
    , m_setting("ErfanMohammadzade", "QCardio")
{
}

void SettingsService::saveSetting(const UIConfigs &uiConfig)
{
    m_setting.setValue("UI/DBPath", uiConfig.dataBasePath());
    m_setting.setValue("UI/DBName", uiConfig.dataBaseName());
    m_setting.setValue("UI/lineEditSignalIndex1", uiConfig.lineEditSignal1Index());
    m_setting.setValue("UI/lineEditSignalIndex2", uiConfig.lineEditSignal2Index());
    m_setting.setValue("UI/lineEditSignalIndex3", uiConfig.lineEditSignal3Index());
    m_setting.setValue("UI/signalTargetFreq", uiConfig.targetFreq());
    m_setting.setValue("UI/signalGain", uiConfig.gainSignal());
    m_setting.setValue("UI/signalOfset", uiConfig.signalOfset());
    m_setting.setValue("UI/hasExportAllReq", uiConfig.exportAll());
    m_setting.setValue("UI/hasREC7Export", uiConfig.exportRC7());
    m_setting.setValue("UI/hasRAWExport", uiConfig.exportRaw());
    m_setting.sync();
}

void SettingsService::loadSetting(UIConfigs *uiConfig)
{
    uiConfig->setDataBasePath(m_setting.value("UI/DBPath").toString());
    uiConfig->setDataBaseName(m_setting.value("UI/DBName").toString());
    uiConfig->setLineEditSignal1Index(m_setting.value("UI/lineEditSignalIndex1").toInt());
    uiConfig->setLineEditSignal2Index(m_setting.value("UI/lineEditSignalIndex2").toInt());
    uiConfig->setLineEditSignal3Index(m_setting.value("UI/lineEditSignalIndex3").toInt());
    uiConfig->setTargetFreq(m_setting.value("UI/signalTargetFreq").toInt());
    uiConfig->setGainSignal(m_setting.value("UI/signalGain").toInt());
    uiConfig->setSignalOfset(m_setting.value("UI/signalOfset").toInt());
    uiConfig->setExportAll(m_setting.value("UI/hasExportAllReq").toBool());
    uiConfig->setExportRC7(m_setting.value("UI/hasREC7Export").toBool());
    uiConfig->setExportRaw(m_setting.value("UI/hasRAWExport").toBool());
}
