#ifndef SIGNALVIEWCONTROLLER_H
#define SIGNALVIEWCONTROLLER_H

#include <QWidget>
#include "Models/global_qcardio.h"
#include "Views/signalviewwidget.h"

class SignalViewController : public QWidget
{
    Q_OBJECT
public:
    explicit SignalViewController(QWidget *parent = nullptr);
    ~SignalViewController();

    SignalViewWidget *signalWidget() const;
    void setData(const MIT_BIH_ECGData &data);

private:
    SignalViewWidget *m_signalViewWidget = nullptr;
};

#endif // SIGNALVIEWCONTROLLER_H
