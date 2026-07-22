#include "clog.h"

CLog::CLog(QObject *parent, QTextEdit *logfiled)
    : QObject{parent}, m_logFiled(logfiled)
{

}

void CLog::sltAppendLog(const QString &logText)
{
    m_logCounter++;
    if(m_logCounter > m_maxLogCount)
    {
        m_logFiled->clear();
        m_logCounter = 0;
    }
    QString dateTime = QDateTime::currentDateTime().toString("hh:mm");
#if SHOW_LOG_TERMINAL
    qDebug() << logText;
#else
    m_logFiled->append(dateTime+": "+logText);
#endif
}

