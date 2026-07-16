#include "clog.h"

CLog::CLog(QObject *parent, QTextEdit *logfiled)
    : QObject{parent}, m_logFiled(logfiled)
{

}

void CLog::sltAppendLog(const QString &logText)
{
    m_logCounter++;
    if(m_logCounter > 100)
    {
        m_logFiled->clear();
        m_logCounter = 0;
    }
    QString dateTime = QDateTime::currentDateTime().toString("hh:mm");
    m_logFiled->append(dateTime+": "+logText);
}

