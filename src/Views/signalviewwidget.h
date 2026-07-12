#ifndef SIGNALVIEWWIDGET_H
#define SIGNALVIEWWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QColor>
#include <QStringList>
#define DEBUG_SIGNALVIEW false

class SignalViewWidget : public QWidget
{
    Q_OBJECT

public:
    static constexpr int MaxLeads = 3;

    explicit SignalViewWidget(QWidget *parent = nullptr);

    void setLeads(const QVector<QVector<qreal>> &leads,
                  const QStringList &names,
                  int sampleRate,
                  qreal adcPerMillivolt = 200.0);
    void clearLeads();

    void setPaperSpeed(qreal mmPerSecond);
    void setSensitivity(qreal mmPerMillivolt);
    void setVisibleDuration(qreal seconds);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int activeLeadCount() const;
    qreal totalDurationSec() const;
    qreal maxTimeOffset() const;
    QRectF plotRect() const;
    QRectF leadStripRect(int leadIndex) const;

    qreal sampleToMillivolts(qreal sample) const;
    qreal sampleIndexToTime(int index) const;
    qreal timeToX(qreal timeSec, const QRectF &plotArea) const;
    qreal millivoltsToY(qreal millivolts, qreal stripCenterY, double signalDC = 0) const;

    void drawGrid(QPainter &painter, const QRectF &plotArea);
    void drawLead(QPainter &painter, int leadIndex, const QRectF &stripRect);
    void drawLeadLabel(QPainter &painter, int leadIndex, const QRectF &stripRect);
    void drawScaleLegend(QPainter &painter, const QRectF &plotArea);
    void autoScaleAndCenter();
    void updateVisibleDuration();
    qreal pixelsPerSecond() const;


    QVector<QVector<qreal>> m_leads;
    QStringList m_leadNames;
    int m_sampleRate = 360;
    qreal m_adcPerMv = 200.0;

    qreal m_paperSpeedMmPerSec = 25.0;
    qreal m_sensitivityMmPerMv = 10.0;
    qreal m_pixelsPerMm = 3.5;
    qreal m_stripHeightMm = 80.0;

    qreal m_timeOffsetSec = 0.0;
    qreal m_visibleDurationSec = 5.0;

    bool m_isPanning = false;
    QPoint m_lastMousePos;

    static const QColor s_leadColors[MaxLeads];
};

#endif // SIGNALVIEWWIDGET_H
