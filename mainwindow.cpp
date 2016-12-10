#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new SettingsDialog;

    serialttc1 = new QSerialPort(this);
    serialttc2 = new QSerialPort(this);
    serialgps = new QSerialPort(this);

    status = new QLabel;
    ui->statusBar->addWidget(status);

    initActionsConnections();

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionExit->setEnabled(true);
    ui->actionSettings->setEnabled(true);

    connect(serialttc1, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleErrorTTC1);
    connect(serialttc1, &QSerialPort::readyRead, this, &MainWindow::readDataTTC1);
    connect(ui->ttc1Console, &Console::getData, this, &MainWindow::writeDataTTC1);

    connect(serialttc2, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleErrorTTC2);
    connect(serialttc2, &QSerialPort::readyRead, this, &MainWindow::readDataTTC2);
    connect(ui->ttc2Console, &Console::getData, this, &MainWindow::writeDataTTC2);

    connect(serialgps, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleErrorGPS);
    connect(serialgps, &QSerialPort::readyRead, this, &MainWindow::readDataGPS);
    connect(ui->gpsConsole, &Console::getData, this, &MainWindow::writeDataGPS);

    connect(ui->quickButton, &QPushButton::clicked,this, &MainWindow::sendQuickCommand);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete settings;
}

void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();

    bool gps_enabled(false);
    bool ttc1_enabled(false);
    bool ttc2_enabled(false);

    if (p.gps_name != "" && p.gps_name != "Select Port") {
        serialgps->setPortName(p.gps_name);
        serialgps->setBaudRate(QSerialPort::Baud9600);
        serialgps->setDataBits(QSerialPort::Data8);
        serialgps->setParity(QSerialPort::NoParity);
        serialgps->setStopBits(QSerialPort::OneStop);
        serialgps->setFlowControl(QSerialPort::NoFlowControl);
        if (serialgps->open(QIODevice::ReadWrite)) {
            gps_enabled=true;
            ui->gpsConsole->setEnabled(true);
            ui->gpsConsole->setLocalEchoEnabled(false);
        }
        else {
            QMessageBox::critical(this, tr("Error"), serialgps->errorString());
            showStatusMessage(tr("Open error of gps serial"));
            return;
        }
    }

    if (p.ttc1_name != "" && p.ttc1_name != "Select Port") {
        serialttc1->setPortName(p.ttc1_name);
        serialttc1->setBaudRate(QSerialPort::Baud19200);
        serialttc1->setDataBits(QSerialPort::Data8);
        serialttc1->setParity(QSerialPort::NoParity);
        serialttc1->setStopBits(QSerialPort::OneStop);
        serialttc1->setFlowControl(QSerialPort::NoFlowControl);
        if (serialttc1->open(QIODevice::ReadWrite)) {
            ttc1_enabled=true;
            ui->ttc1Console->setEnabled(true);
            ui->ttc1Console->setLocalEchoEnabled(false);
        }
        else {
            QMessageBox::critical(this, tr("Error"), serialttc1->errorString());
            showStatusMessage(tr("Open error of ttc1 serial"));
            return;
        }
    }

    if (p.ttc2_name != "" && p.ttc2_name != "Select Port") {
        serialttc2->setPortName(p.ttc2_name);
        serialttc2->setBaudRate(QSerialPort::Baud19200);
        serialttc2->setDataBits(QSerialPort::Data8);
        serialttc2->setParity(QSerialPort::NoParity);
        serialttc2->setStopBits(QSerialPort::OneStop);
        serialttc2->setFlowControl(QSerialPort::NoFlowControl);
        if (serialttc2->open(QIODevice::ReadWrite)) {
            ttc2_enabled=true;
            ui->ttc2Console->setEnabled(true);
            ui->ttc2Console->setLocalEchoEnabled(false);
        }
        else {
            QMessageBox::critical(this, tr("Error"), serialttc2->errorString());
            showStatusMessage(tr("Open error of ttc2 serial"));
            return;
        }
    }

    if (gps_enabled==true || ttc1_enabled==true || ttc2_enabled==true){
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionSettings->setEnabled(false);
    }
    else {
        QMessageBox::critical(this, tr("No vaild port configured"), tr("Please configure a valid port"));
        showStatusMessage(tr("not connected"));
        return;
    }

    QString myStatusMessage=tr("Connected to:");

    if (gps_enabled){
        myStatusMessage += "GPS: " + p.gps_name + " baud 9600 ";
    }

    if (ttc1_enabled){
        myStatusMessage += "TTC1: " + p.ttc1_name + " baud 19200 ";
    }

    if (ttc2_enabled){
        myStatusMessage += "TTC2: " + p.ttc2_name + " baud 19200";
    }

    showStatusMessage(myStatusMessage);
}

void MainWindow::closeSerialPort()
{
    if (serialgps->isOpen())
        serialgps->close();

    if (serialttc1->isOpen())
        serialttc1->close();

    if (serialttc2->isOpen())
        serialttc2->close();

    ui->gpsConsole->setEnabled(false);
    ui->ttc1Console->setEnabled(false);
    ui->ttc2Console->setEnabled(false);


    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionSettings->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About StacieEmu"),
                       tr("The <b>StacieEmu</b> is a simple SerialPort Client "
                          "for the PEGASUS CubeSat Project "));
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionSettings, &QAction::triggered, settings, &SettingsDialog::show);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

QString MainWindow::WriteDescription(const QByteArray &data, bool toOBC)
{
    QString cmdId;
    QString actionId;
    if (data[0] == (char)TTC_OP_TRANSMIT){
        cmdId = "TRANSMIT";
    }
    else if (data[0] == (char)TTC_OP_GETSTATUS){
        cmdId = "GETSTATUS";
    }
    else if (data[0] == (char)TTC_OP_RECEIVE){
        cmdId = "RECEIVE";
    }
    else if (data[0] == (char)TTC_OP_OPMODE){
        cmdId = "OPMODE";
    }
    else if (data[0] == (char)TTC_OP_GETTELEMETRY){
        cmdId = "GETTELEMETRY";
    }

    if (data[1] == (char)TTC_ACTION_ACK){
        actionId="ACK";
    }
    else if (data[1] == (char)TTC_ACTION_EXEC){
        actionId="EXEC";
    }
    else if (data[1] == (char)TTC_ACTION_NACK){
        actionId="NACK";
    }

    QString result;

    if (toOBC){
        result= "TTC->OBC: ";
    }
    else {
        result= "OBC->TTC: ";
    }
    result += cmdId + " " + actionId + "\n";

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::writeDataTTC1(const QByteArray &data)
{
    QString infoLine = WriteDescription(data,false);
    ui->ttc1Console->putString(infoLine);
    ui->ttc1Console->putData(data);
    ui->ttc1Console->putString("\n");

    serialttc1->write(data);
}

void MainWindow::readDataTTC1()
{
    QByteArray data = serialttc1->readAll();

    QString  infoLine = WriteDescription(data,true);
    ui->ttc1Console->putString(QString(infoLine));
    ui->ttc1Console->putData(data);
    ui->ttc1Console->putString("\n");
}

void MainWindow::handleErrorTTC1(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error (TTC1)"), serialttc1->errorString());
        closeSerialPort();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::writeDataTTC2(const QByteArray &data)
{
    serialttc2->write(data);
}

void MainWindow::readDataTTC2()
{
    QByteArray data = serialttc2->readAll();
    ui->ttc2Console->putData(data);
}

void MainWindow::handleErrorTTC2(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error (TTC2)"), serialttc2->errorString());
        closeSerialPort();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::writeDataGPS(const QByteArray &data)
{
    serialgps->write(data);
}

void MainWindow::readDataGPS()
{
    QByteArray data = serialgps->readAll();
    ui->gpsConsole->putString(QString(data));
}

void MainWindow::handleErrorGPS(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error (GPS)"), serialgps->errorString());
        closeSerialPort();
    }
}

void MainWindow::sendQuickCommand() {

    QString selectedText = ui->quickBox->currentText();

    QByteArray data;

    if (selectedText == "STATUS TEST") {
        data.append(TTC_OP_GETSTATUS);
        data.append(TTC_ACTION_EXEC);
        data.append(0xFF);
        char test=0xFF;
        uint8_t crc = this->CRC8((unsigned char*)&test,1);
        data.append((char)crc);
    }
    //TODO -> other quick commands!

    if (serialttc1->isOpen()){
        writeDataTTC1(data);
    }

    if (serialttc2->isOpen()){
        writeDataTTC2(data);
    }

    return;
}

/* Update CRC8 Checksum */
void MainWindow::c_CRC8(char data, uint8_t *checksum)
{
    uint8_t i;
    *checksum ^= data;

    for (i = 0; i < 8; ++i)
    {
        *checksum = (*checksum << 1) ^ ((*checksum & 0x80) ? 0x07 : 0x00);
    }
}

/* Compute CRC8 (binary String) */
uint8_t MainWindow::CRC8(uint8_t* str, size_t length)
{
    uint8_t checksum = 0;

    for (; length--; c_CRC8(*str++, &checksum))
        ;

    return checksum;
}


