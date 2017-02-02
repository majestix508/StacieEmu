#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <stdint.h>

// Stacie functions
#define TTC_OP_TRANSMIT     0x07
#define TTC_OP_RECEIVE      0x1E
#define TTC_OP_OPMODE       0x2A
#define TTC_OP_GETTELEMETRY 0x2D

// Stacie return codes
#define TTC_ACTION_ACK   0x00
#define TTC_ACTION_EXEC  0x4B
#define TTC_ACTION_NACK  0x7F

//GetTelemetry (OBC from TTC) RecordIds
#define GT_TRX1_TMP  0x01
#define GT_TRX2_TMP  0x02
//#define GT_RST_COUNT 0x03
#define GT_TEMP      0x04
#define GT_STATUS    0x05
#define GT_RSSI      0x06

// Stacie Transmit packet sizes
#define TTC_TRANSMIT_PAYLOAD_DATA_SIZE 43 //43 bytes of plain data
#define TTC_TRANSMIT_PAYLOAD_FULL_SIZE 46 //46 bytes of payload including pid, source and destination
#define TTC_TRANSMIT_FULL_PACKET_SIZE  49 //46 bytes of full payload, commandId, actionId, crc8


// Beacon PID - Downlink format PID -  CALLSIGN(6) - 39 Byte fixed Payload.
#define TTC_PID_DATATYPE_BEACON1         0x53
#define TTC_PID_DATATYPE_BEACON2         0x56

// PID for Downlink format PID - size/len -  PackNr - 43 byte data
#define TTC_PID_DATATYPE_CUSTOM         0x50	// If this is used, the cointent must be further Identifiaby by Id or something...
#define TTC_PID_DATATYPE_SCIENCEDATA    0x51
#define TTC_PID_DATATYPE_HOUSEKEEPING   0x52
#define TTC_PID_DATATYPE_IMAGES         0x54
#define TTC_PID_DATATYPE_ADCS_DOWNLINK  0x55	// ADCS Downlink Data (3 packages of internal values in binary format)

// PID for Uplink format PID - size/len -  PackNr - 43 byte data
#define TTC_REC_PID_SCIENCE_SCRIPT	0x11 	//Pid for receive science-scripts
#define TTC_REC_PID_SINGLE_CMD		0x12 	//Pid for single command
#define TTC_REC_PID_ADCS_UPLINK		0x13 	//Pid for adcs uplink data (used to send GPS fixes and sync the RTC on board)
#define TTC_REC_PID_CMD_SCRIPT		0x14 	//Pid for command scripts
#define TTC_REC_PID_LOGGER_RAWDDL	0x15 	//Pid for triggering Raw data Downlink. The package contains page pointer to Flash


//#define TTC_TELEMETRY_POSITION			 0
//#define TTC_TELEMETRY_TIME               1

#define TTC_TELEMETRY_TRX1_TEMP	         1
#define TTC_TELEMETRY_TRX2_TEMP			 2
//#define TTC_TELEMETRY_RST_COUNT          3
#define TTC_TELEMETRY_MODE              4
//#define TTC_TELEMETRY_STATUS             5
#define TTC_TELEMETRY_RSSI               6


QT_BEGIN_NAMESPACE

enum Uart { TTC1, TTC2, GPS};

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
    void selectCustomFile();
    void sendCustomCommand();
    void sendGPSCommand();
    void ClearFileButton();
    void clearConsoles();
    void ClearTTC1();
    void ClearTTC2();
    void ClearGPS();
    void uploadScript();
    void saveTTC1toFile();
    void saveTTC2toFile();
    void saveGPStoFile();

    void autorespondToCommands(QByteArray data, Uart name);

    void WriteImageToFile(QByteArray data);

    int check_and_parse_buffer(QByteArray arr);

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


    QByteArray filecontent;

    QByteArray ttc1buffer;
    QByteArray ttc2buffer;

    bool ttc1_inprogress;
    bool ttc2_inprogress;
};

#endif // MAINWINDOW_H
