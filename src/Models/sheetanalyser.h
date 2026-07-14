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
                           int processTime = 0);
    bool processSheets();

private:
    QStringList m_sheetPathList;
    QString m_outputPath;
    SheetResult m_sheetRes;
    int m_processTime = 0;
    void runMachAndCheck();
};

#endif // SHEETANALYSER_H
