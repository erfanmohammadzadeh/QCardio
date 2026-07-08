#ifndef CANALYSER_H
#define CANALYSER_H

#include <QObject>
#include <Models/sheetanalyser.h>
class CAnalyser : public QObject
{
    Q_OBJECT
public:
    explicit CAnalyser(QObject *parent = nullptr, AnalyseCfg cfg = AnalyseCfg());
    void analyse();

private:
    SheetAnalyser *m_sheetAnalyser = nullptr;
    AnalyseCfg m_analyseCfg;
signals:
};

#endif // CANALYSER_H
