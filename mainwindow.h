#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <stdint.h>

// Stacie functions
#define TTC_OP_TRANSMIT     0x07
#define TTC_OP_GETSTATUS    0x19
#define TTC_OP_RECEIVE      0x1E
#define TTC_OP_OPMODE       0x2A
#define TTC_OP_GETTELEMETRY 0x2D

// Stacie return codes
#define TTC_ACTION_ACK   0x00
#define TTC_ACTION_EXEC  0x4B
#define TTC_ACTION_NACK  0x7F

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void about();

    void writeDataTTC1(const QByteArray &data);
    void readDataTTC1();
    void handleErrorTTC1(QSerialPort::SerialPortError error);

    void writeDataTTC2(const QByteArray &data);
    void readDataTTC2();
    void handleErrorTTC2(QSerialPort::SerialPortError error);

    void writeDataGPS(const QByteArray &data);
    void readDataGPS();
    void handleErrorGPS(QSerialPort::SerialPortError error);

    void sendQuickCommand();

private:

    void initActionsConnections();

    QString WriteDescription(const QByteArray &data, bool toOBC);

    void showStatusMessage(const QString &message);

    uint8_t CRC8(uint8_t* str, size_t length);
    void c_CRC8(char data, uint8_t *checksum);

    Ui::MainWindow *ui;
    QLabel *status;
    SettingsDialog *settings;

    QSerialPort *serialttc1;
    QSerialPort *serialttc2;
    QSerialPort *serialgps;
};

#endif // MAINWINDOW_H
