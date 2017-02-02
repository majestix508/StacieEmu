#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QFileDialog>
#include <QTextStream>
#include <QTime>
#include <QStringRef>

#include <iostream>

using namespace std;

//helper function for a sleep without blocking the threads!
void delay(int sec)
{
    QTime dieTime= QTime::currentTime().addSecs(sec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

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

    filecontent.clear();
    ui->cFileLbl->clear();
    ui->clearFileBtn->hide();

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
    connect(ui->cSend, &QPushButton::clicked,this, &MainWindow::sendCustomCommand);


    connect(ui->cFileBtn, &QPushButton::clicked,this, &MainWindow::selectCustomFile);
    connect(ui->clearFileBtn, &QPushButton::clicked,this, &MainWindow::ClearFileButton);
    connect(ui->gpsSendButton, &QPushButton::clicked,this, &MainWindow::sendGPSCommand);

    connect(ui->uploadscriptButton, &QPushButton::clicked,this, &MainWindow::uploadScript);

    ttc1buffer.clear();
    ttc2buffer.clear();

    ttc1_inprogress=false;
    ttc2_inprogress=false;
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

    ttc1buffer.clear();
    ttc2buffer.clear();

    ttc1_inprogress=false;
    ttc2_inprogress=false;
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
    connect(ui->actionClear, &QAction::triggered, this, &MainWindow::clearConsoles);

    connect(ui->actionClear_TTC1, &QAction::triggered, this, &MainWindow::ClearTTC1);
    connect(ui->actionClear_TTC2, &QAction::triggered, this, &MainWindow::ClearTTC2);
    connect(ui->actionClear_GPS, &QAction::triggered, this, &MainWindow::ClearGPS);

    connect(ui->actionSave_TTC1, &QAction::triggered, this, &MainWindow::saveTTC1toFile);
    connect(ui->actionSave_TTC2, &QAction::triggered, this, &MainWindow::saveTTC2toFile);
    connect(ui->actionSave_GPS, &QAction::triggered, this, &MainWindow::saveGPStoFile);

}

void MainWindow::clearConsoles(){
    ui->ttc1Console->clear();
    ui->ttc2Console->clear();
    ui->gpsConsole->clear();
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

QString MainWindow::WriteDescription(const QByteArray &data, bool toOBC)
{
    QString cmdId;
    QString actionId;

    if (data.size() <2){
        cmdId = "Invalid";
        actionId = "Sequence";
    }
    else {
        if (data[0] == (char)TTC_OP_TRANSMIT){
            cmdId = "TRANSMIT";
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
    while (ttc1_inprogress){
        delay(1);
    }

    ttc1_inprogress=true;

    QString infoLine = WriteDescription(data,true);
    ui->ttc1Console->putString(infoLine,Qt::green);
    ui->ttc1Console->putData(data,Qt::green);
    ui->ttc1Console->putString("\n",Qt::green);

    serialttc1->write(data);

    ttc1_inprogress=false;
}

int MainWindow::check_and_parse_buffer(QByteArray arr)
{
    //check if stacie-package is complete!

    char cmdid=(char)0x00; //0x00 = not available!!!
    char action=(char)0xFF; // 0xFF = not available!!!

    bool cmd_action=false;

    for(int i=0;i<arr.size();i++){

        if (i==0){
            if (arr[i] == (char)TTC_OP_TRANSMIT){
                cmdid = (char)TTC_OP_TRANSMIT;
            }
            else if (arr[i] == (char)TTC_OP_RECEIVE) {
                cmdid = (char)TTC_OP_RECEIVE;
            }
            else if (arr[i] == (char)TTC_OP_OPMODE) {
                cmdid = (char)TTC_OP_OPMODE;
            }
            else if (arr[i] == (char)TTC_OP_GETTELEMETRY){
                cmdid = (char)TTC_OP_GETTELEMETRY;
            }
            else {
                return 0;
            }
        }
        else if (i==1){
            if (arr[i] == (char)TTC_ACTION_ACK){
                action = (char)TTC_ACTION_ACK;
            }
            else if (arr[i] == (char)TTC_ACTION_EXEC){
                action = (char)TTC_ACTION_EXEC;
            }
            else if (arr[i] == (char)TTC_ACTION_NACK){
                action = (char)TTC_ACTION_NACK;
            }
            else {
                return 0;
            }
            cmd_action=true;
        }

        if (cmd_action){
            break;
        }
    }

    if (cmdid == (char)TTC_OP_TRANSMIT && (action == (char)TTC_ACTION_ACK || action == (char)TTC_ACTION_NACK) ){
        return 2; //command complete
    }
    else if (cmdid == (char)TTC_OP_RECEIVE && (action == (char)TTC_ACTION_ACK || action == (char)TTC_ACTION_NACK)){
        return 2; //command complete
    }
    else if ( (cmdid == (char)TTC_OP_TRANSMIT || cmdid == (char)TTC_OP_RECEIVE) && action == (char)TTC_ACTION_EXEC){
        //check if package is 49byte long!
        if (arr.size() >=49){
            return 49;
        }
        else {
            if (arr.size()>=5){
                if (arr[4] == (char)0xFF){
                    int size = arr[3];
                    cout << "lastpkg size:" << size << endl;
                    return size+3;
                }
            }
            return 0;
        }
    }
    else if ( cmdid == (char)TTC_OP_GETTELEMETRY && action == (char)TTC_ACTION_EXEC){ //obc asks for telemetry
        if (arr.size() >=4){
            return 4;
        }
        else {
            return 0;
        }
    }
    else if (cmdid == (char)TTC_OP_OPMODE && action == (char)TTC_ACTION_EXEC) { //obc wants mode-change
        if (arr.size()>=8){
            return 8;
        }
        else {
            return 0;
        }
    }
    else if (cmdid == (char)TTC_OP_OPMODE && (action == (char)TTC_ACTION_ACK || action == (char)TTC_ACTION_NACK) ){ //obc says ok or not ok
        return 2;
    }

    return 0;
}

void MainWindow::readDataTTC1()
{
    while (ttc1_inprogress){
        delay(1);
    }

    ttc1_inprogress=true;

    ttc1buffer.append(serialttc1->readAll());

    int cmdsize = check_and_parse_buffer(ttc1buffer);

    if (cmdsize){
        QByteArray pkg_data;
        for(int i=0;i<cmdsize;i++){
            pkg_data.append(ttc1buffer[i]);
        }
        ttc1buffer.remove(0,cmdsize);

        QString  infoLine = WriteDescription(pkg_data,false);
        ui->ttc1Console->putString(QString(infoLine),Qt::red);
        ui->ttc1Console->putData(pkg_data,Qt::red);
        ui->ttc1Console->putString("\n",Qt::red);
        ttc1_inprogress=false;
        autorespondToCommands(pkg_data,TTC1);
    }
    ttc1_inprogress=false;
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
    while (ttc2_inprogress){
        delay(1);
    }

    ttc2_inprogress=true;

    QString infoLine = WriteDescription(data,true);
    ui->ttc2Console->putString(infoLine,Qt::green);
    ui->ttc2Console->putData(data,Qt::green);
    ui->ttc2Console->putString("\n",Qt::green);

    serialttc2->write(data);

    ttc2_inprogress=false;
}

void MainWindow::readDataTTC2()
{
    while (ttc2_inprogress){
        delay(1);
    }

    ttc2_inprogress=true;
    ttc2buffer.append(serialttc2->readAll());

    int cmdsize = check_and_parse_buffer(ttc2buffer);

    if (cmdsize){
        QByteArray pkg_data;
        for(int i=0;i<cmdsize;i++){
            pkg_data.append(ttc2buffer[i]);
        }
        ttc2buffer.remove(0,cmdsize);

        QString  infoLine = WriteDescription(pkg_data,false);
        ui->ttc2Console->putString(QString(infoLine),Qt::red);
        ui->ttc2Console->putData(pkg_data,Qt::red);
        ui->ttc2Console->putString("\n",Qt::red);
        ttc2_inprogress=false;
        autorespondToCommands(pkg_data,TTC2);
    }
    ttc2_inprogress=false;
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

    ui->gpsConsole->putString("Command: ",Qt::white);
    ui->gpsConsole->putData(data,Qt::white);
    ui->gpsConsole->putString("\n",Qt::white);
}

void MainWindow::readDataGPS()
{
    QByteArray data = serialgps->readAll();
    WriteImageToFile(data);
    ui->gpsConsole->putString(QString(data),Qt::yellow);
}

void MainWindow::WriteImageToFile(QByteArray data)
{
    //OBC Output: OBC: Image <size 3byte> <hex data with space seperated>\n

    QString myString(data);

    //Check if string starts with "OBC: Image ";
    QString subString=myString.mid(0,11);
    if (subString != "OBC: Image "){
        return;
    }

    //Get size
    QString sizestr = myString.mid(11,3);
    int size = sizestr.toInt();

    if (!size)
        return;

    char l_arr[512];
    int pos = 15; //first hex string

    for(int i=0; i < size; i++){

        QString hexstr = myString.mid(pos,2);
        bool ok;
        int hexint = hexstr.toInt(&ok,16);
        l_arr[i] = (char) hexint;

        pos+=3; //offset to next hex-string
    }


    QString filename="D:\\test.tif";
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)){
        file.write((const char*)l_arr,size);
    }
    file.close();
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

    //TODO remove status_test
    if (selectedText == "GPS Show Loggerinfo") {
        data = "$C,10003,0,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show last wod"){
        data = "$C,10006,0,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Erase Flash"){
        data = "$C,10007,0,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script Info"){
        data = "$C,10004,0,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script Slot1"){
        data = "$C,10004,1,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script Slot2"){
        data = "$C,10004,1,1*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script Slot3"){
        data = "$C,10004,1,2*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script Slot4"){
        data = "$C,10004,1,3*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script Slot5"){
        data = "$C,10004,1,4*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script Slot6"){
        data = "$C,10004,1,5*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script Slot7"){
        data = "$C,10004,1,6*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script SM Slot1"){
        data = "$C,10004,1,7*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script SM Slot2"){
        data = "$C,10004,1,8*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script SM Slot3"){
        data = "$C,10004,1,9*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script SM Slot4"){
        data = "$C,10004,1,10*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "GPS Show Script SM Slot5"){
        data = "$C,10004,1,11*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "Start Transmit"){
        data = "$C,10011,0,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "Stop Transmit"){
        data = "$C,10012,0,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "Start Image"){
        data = "$C,10010,0,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }
    else if (selectedText == "Show 1st Image"){
        data = "$C,10013,0,0*\r";
        if (serialgps->isOpen()){
            writeDataGPS(data);
        }
    }

//    if (serialttc1->isOpen()){
//        writeDataTTC1(data);
//    }

//    if (serialttc2->isOpen()){
//        writeDataTTC2(data);
//    }

    return;
}

void MainWindow::uploadScript() {

    QString path = QFileDialog::getOpenFileName(this,tr("File"));
    if ( path.isNull() == false )
    {
        QByteArray content;

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return;
        content = file.readAll();
        file.close();

        //Get Script Slot
        QString myslot = ui->slotBox->currentText();
        int selected_slot = myslot.toInt();
        //Stacie Packetaufbau:
        //<cmd><action><pid><destination><packageNum><raw-data><crc8>
        //RECEIVE,EXEC,0x00(ignored),0x01(sciencescript),(1-x letztes 0xFF),{1. Packet hat slot am Anfang!}daten,CRC8 1byte
        //new syntax!!!
        //RECEIVE,EXEC,0x11,0x01(sciencescript),(1-x letztes 0xFF),{1. Packet hat slot am Anfang!}daten,CRC8 1byte
        // pid = type of upload, destination=length of data in packet,packnum=1.n last 0xff

        QByteArray package;
        uint8_t i = 1;
        int index=0;
        bool first_run=true;
        int rest = (content.size()+1) % 43; //+1 = scriptslot!
        int packages = (content.size()+1) / 43; //+1 = scriptslot!

        int pkg_count = packages;
        if (rest>0)
            pkg_count++;

        while(true){

            package.clear();
            package.append((char)0x1E); //RECEIVE
            package.append((char)0x4B); //EXEC
            package.append((char)0x11); //PID == destination science script 0x11
            package.append((char)pkg_count); // number of packages (if this is not the last package!)

            if (i == packages && rest==0){
                //last full package
                package[3] = (char)rest;
                package.append((char)0xFF);
            }
            else {
                package.append((char)i);
            }

            int payload=43;

            if (first_run){
                package.append((char)selected_slot); //Script Slot
                first_run=false;
                payload=42;
            }

            for(int x=0; x<payload;x++){
                package.append(content[index+x]);
            }

            //Calc CRC
            const char* mychar = package.data();
            mychar+=2; //so the pointer points to pid
            uint8_t crc = CRC8((uint8_t *)mychar,46);
            package.append((char)crc);

            //Send to TTC1 only!
            if (serialttc1->isOpen()){
                writeDataTTC1(package);
                delay(3);
            }
            //ui->ttc1Console->putString("\npackage:\n");
            //ui->ttc1Console->putData(package);

            index+=payload;

            if (i == packages){
                break;
            }

            i++; //pkg_count
        }

        //Now add the last package if there is one
        if (rest>0){
            package.clear();
            package.append((char)0x1E); //RECEIVE
            package.append((char)0x4B); //EXEC
            package.append((char)0x11); //PID
            package.append((char)rest); //destination -> length of the data!
            package.append((char)0xFF); //last Package

            for(int y =0; y<43;y++){
                if (y<rest){
                    package.append(content[index+y]);
                }
                else { //packet auffüllen
                    package.append((char)0x55);
                }
            }

            //Calc CRC
            const char* mychar = package.data();
            mychar+=2; //so the pointer points to pid
            uint8_t crc = CRC8((uint8_t *)mychar,46);
            package.append((char)crc);

            //Send to TTC1 only!
            if (serialttc1->isOpen()){
                writeDataTTC1(package);
            }
            //ui->ttc1Console->putString("\npackage:\n");
            //ui->ttc1Console->putData(package);

        }

    }
}

void MainWindow::sendCustomCommand() {
    QByteArray data;

    QString Command = ui->cmdBox->currentText();
    QString Action = ui->actionBox->currentText();

    if (Command == "TRANSMIT"){
        data.append((char)TTC_OP_TRANSMIT);
    }
    else if (Command == "RECEIVE") {
        data.append((char)TTC_OP_RECEIVE);
    }
    else if (Command == "OPMODE"){
        data.append((char)TTC_OP_OPMODE);
    }
    else if (Command == "GETTELEMETRY"){
        data.append((char)TTC_OP_GETTELEMETRY);
    }

    if (Action == "ACK"){
        data.append((char)TTC_ACTION_ACK);
    }
    else if (Action == "EXEC"){
        data.append((char)TTC_ACTION_EXEC);
    }
    else if (Action == "NACK"){
        data.append((char)TTC_ACTION_NACK);
    }

    if (filecontent.size()){
        data.append(filecontent);

        //Calc CRC
        const char* mychar = filecontent.data();
        uint8_t crc = CRC8((uint8_t *)mychar,filecontent.size());
        data.append((char)crc);
    }

    if (serialttc1->isOpen()){
        writeDataTTC1(data);
    }

    if (serialttc2->isOpen()){
        writeDataTTC2(data);
    }

}

void MainWindow::sendGPSCommand() {
    QString Command = ui->gpsInputText->text();
    QByteArray data;
    data.append(Command);
    data.append('\r');
    writeDataGPS(data);

    ui->gpsInputText->clear();
}

void MainWindow::selectCustomFile() {

    QString path = QFileDialog::getOpenFileName(this,tr("File"));
    if ( path.isNull() == false )
    {
        filecontent.clear();
        ui->cFileLbl->clear();

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return;
        filecontent = file.readAll();
        file.close();
        //TODO - check filesize not bigger than 49 bytes!!! Stacie packet size!!

        //Now set the label to the path!
        if (path.size() > 20){
            QString small = tr("...")+path.right(20);
            ui->cFileLbl->setText(small);
        }
        else {
            ui->cFileLbl->setText(path);
        }

        ui->clearFileBtn->show();
        //For test - output content of file in ttc1 console!
        //ui->ttc1Console->putString(QString(filecontent));
    }
}

void MainWindow::saveTTC1toFile () {
    QString path = QFileDialog::getSaveFileName(this,"File");
    if (path.isNull() == false){
        QString content = ui->ttc1Console->toPlainText();
        QFile file (path);
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
            stream << content;
            file.close();
        }
    }
}

void MainWindow::saveTTC2toFile () {
    QString path = QFileDialog::getSaveFileName(this,"File");
    if (path.isNull() == false){
        QString content = ui->ttc2Console->toPlainText();
        QFile file (path);
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
            stream << content;
            file.close();
        }
    }
}

void MainWindow::saveGPStoFile () {
    QString path = QFileDialog::getSaveFileName(this,"File");
    if (path.isNull() == false){
        QString content = ui->gpsConsole->toPlainText();
        QFile file (path);
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
            stream << content;
            file.close();
        }
    }
}

void MainWindow::ClearFileButton() {
    filecontent.clear();
    ui->cFileLbl->clear();
    ui->clearFileBtn->hide();
}

void MainWindow::ClearTTC1(){
    ui->ttc1Console->clear();
}

void MainWindow::ClearTTC2(){
    ui->ttc2Console->clear();
}

void MainWindow::ClearGPS(){
    ui->gpsConsole->clear();
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

void MainWindow::autorespondToCommands(QByteArray data,Uart name){

    SettingsDialog::Settings p = settings->settings();

    if (!p.telemetry_answer){
        return;
    }

    if (p.telemetry_answer){
        int index=0;
        while(index<data.size()){

            if (data[index] == (char)TTC_OP_GETTELEMETRY){
                if (data.size() >= index+4 && data[index+1] == (char)TTC_ACTION_EXEC){
                    char recordId = data[index+2];
                    char crc = data[index+3];

                    //Check Crc
                    uint8_t mycrc = CRC8((uint8_t*)&recordId,1);
                    if (crc != mycrc){
                        //TODO - send NACK?
                    }

                    QByteArray responseData;
                    responseData.append((char)TTC_OP_GETTELEMETRY);
                    responseData.append((char)TTC_ACTION_ACK);
                    //responseData.append(recordId);

                    char l_payload[4];
                    int l_size=0;

                    //we need the recordId also for CRC8!
                    l_payload[0] = recordId;

                    if (recordId == GT_TRX1_TMP){
                        l_payload[1] = (char)60;
                        l_size = 2;
                    }
                    else if (recordId == GT_TRX2_TMP){
                        l_payload[1] = (char)50;
                        l_size = 2;
                    }
                    else if (recordId == GT_TEMP){
                        l_payload[1] = (char)30;
                        l_size = 2;
                    }
                    else if (recordId == GT_RSSI){
                        l_payload[1] = (char)20;
                        l_payload[2] = (char)10;
                        l_size = 3;
                    }

                    uint8_t newcrc = CRC8((uint8_t*)l_payload,l_size);

                    responseData.append(l_payload,l_size);
                    responseData.append(newcrc);

                    if (name == TTC1){
                        writeDataTTC1(responseData);
                    }
                    else if (name == TTC2){
                        writeDataTTC2(responseData);
                    }
                    else if (name == GPS) {
                        writeDataGPS(responseData);
                    }

                    index +=4; //set to next position
                    continue;
                }
            }

            index++;
        }
    }//telemetry answer


    if (p.transmit_answer){

        if (data.size() >2){

            if (data[0] == (char)TTC_OP_TRANSMIT && data[1] == (char)TTC_ACTION_EXEC){
                QByteArray responseData;
                responseData.append((char)TTC_OP_GETTELEMETRY);
                responseData.append((char)TTC_ACTION_ACK);

                if (name == TTC1){
                    writeDataTTC1(responseData);
                }
                else if (name == TTC2){
                    writeDataTTC2(responseData);
                }
                else if (name == GPS) {
                    writeDataGPS(responseData);
                }
            }
        }

    }//transmit answer

    return;
}
