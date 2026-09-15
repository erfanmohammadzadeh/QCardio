#include "logservice.h"

#include <QDateTime>
#include <QDebug>

LogService::LogService(QObject *parent, QTextEdit *logfiled)
    : QObject{parent}
    , m_logFiled(logfiled)
{
}

void LogService::sltAppendLog(const QString &logText)
{
    m_logCounter++;
    if (m_logCounter > m_maxLogCount) {
        m_logFiled->clear();
        m_logCounter = 0;
    }
    QString dateTime = QDateTime::currentDateTime().toString("hh:mm");
#if SHOW_LOG_TERMINAL
    qDebug() << logText;
#else
    m_logFiled->append(dateTime + ": " + logText);
#endif
}
