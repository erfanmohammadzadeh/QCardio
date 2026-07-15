#ifndef CANALYSER_H
#define CANALYSER_H

#include <QObject>
#include <Models/sheetanalyser.h>
class CAnalyser : public QObject
{
    Q_OBJECT
public:
    explicit CAnalyser(QObject *parent = nullptr, AnalyseCfg cfg = AnalyseCfg());
    bool analyse();

private:
    AnalyseCfg m_analyseCfg;
    bool saveProcessFileResult(const QVector<FileProcessResult>& fileProcessRes);
signals:
};

#endif // CANALYSER_H
