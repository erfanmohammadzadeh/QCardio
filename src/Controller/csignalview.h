#ifndef CSIGNALVIEW_H
#define CSIGNALVIEW_H

#include <QObject>
#include <Models/global_qcardio.h>
#include <Views/signalviewwidget.h>

class CSignalView : public QWidget
{
    Q_OBJECT
public:
    explicit CSignalView(QWidget *parent = nullptr);
    ~CSignalView();

    SignalViewWidget *signalWidget() const;
    void setData(const MIT_BIH_ECGData &data);

private:
    SignalViewWidget *m_signalViewWidget = nullptr;
};

#endif // CSIGNALVIEW_H
