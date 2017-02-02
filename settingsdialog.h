#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    struct Settings {
        QString ttc1_name;
        QString ttc2_name;
        QString gps_name;
        bool    telemetry_answer;
        bool    transmit_answer;
    };

    void apply();
    Settings settings() const;
    void show();

private:
    void fillPortsInfo();
    void showPortInfo(int idx);
    void updateSettings();

    Ui::SettingsDialog *ui;
    Settings currentSettings;
};

#endif // SETTINGSDIALOG_H
