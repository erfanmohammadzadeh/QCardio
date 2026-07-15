#ifndef SHEETANALYSER_H
#define SHEETANALYSER_H

#include <QObject>
#include <Models/csv.h>

class SheetAnalyser : public QObject
{
    Q_OBJECT
public:
    explicit SheetAnalyser(QObject *parent = nullptr,
                           QStringList sheetPathList = {},
                           QString outputPath = "",
                           QString processFileName = 0);
    bool processSheets();

    FileProcessResult fileProcessRes() const;

private:
    QStringList m_sheetPathList;
    QString m_outputPath;
    SheetResult m_sheetRes;
    QString m_processFileName = 0;
    FileProcessResult m_fileProcessRes;
    bool CompareSampleIdxAndType();
    bool saveResult();
    bool loadCSVData();
};

#endif // SHEETANALYSER_H
