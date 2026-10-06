#ifndef COMPARESETTINGSDIALOG_H
#define COMPARESETTINGSDIALOG_H

#include "Models/beattestprofile.h"

#include <QDialog>

class QCheckBox;
class QPushButton;

class CompareSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CompareSettingsDialog(QWidget *parent = nullptr);

    void setProfile(const BeatTestProfile &profile);
    BeatTestProfile profile() const;

private:
    void bindGroup(QCheckBox *group, const QList<QCheckBox *> &children);
    void refreshOkButton();

    QCheckBox *m_beats = nullptr;
    QCheckBox *m_qrs = nullptr;
    QCheckBox *m_shutdown = nullptr;
    QCheckBox *m_vRuns = nullptr;
    QCheckBox *m_sRuns = nullptr;

    QCheckBox *m_groupNormal = nullptr;
    QCheckBox *m_bundle = nullptr;
    QCheckBox *m_supra = nullptr;

    QCheckBox *m_groupPvc = nullptr;
    QCheckBox *m_fusion = nullptr;
    QCheckBox *m_escape = nullptr;
    QCheckBox *m_interpolated = nullptr;

    QCheckBox *m_unknown = nullptr;
    QPushButton *m_ok = nullptr;
};

#endif // COMPARESETTINGSDIALOG_H
