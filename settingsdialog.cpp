#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");
static const char selectPort[] = "Select Port";

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    fillPortsInfo();
    updateSettings();

    connect(ui->applyButton, &QPushButton::clicked,this, &SettingsDialog::apply);
    connect(ui->refreshButton, &QPushButton::clicked,this,&SettingsDialog::fillPortsInfo);
    connect(ui->infoBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::showPortInfo);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

SettingsDialog::Settings SettingsDialog::settings() const
{
    return currentSettings;
}

void SettingsDialog::apply()
{
    updateSettings();
    hide();
}

void SettingsDialog::fillPortsInfo()
{
    ui->ttc1Box->clear();
    ui->ttc2Box->clear();
    ui->gpsBox->clear();
    ui->infoBox->clear();

    ui->infoBox->addItem(tr(selectPort));
    ui->ttc1Box->addItem(tr(selectPort));
    ui->ttc2Box->addItem(tr(selectPort));
    ui->gpsBox->addItem(tr(selectPort));

    QString description;
    QString manufacturer;
    QString serialNumber;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->infoBox->addItem(list.first(), list);
        ui->ttc1Box->addItem(list.first(), list);
        ui->ttc2Box->addItem(list.first(), list);
        ui->gpsBox->addItem(list.first(), list);
    }

}

void SettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    QStringList list = ui->infoBox->itemData(idx).toStringList();
    ui->desc_text->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->man_text->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serial_text->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->location_text->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vendor_text->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->product_text->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void SettingsDialog::updateSettings()
{
    currentSettings.ttc1_name = ui->ttc1Box->currentText();
    currentSettings.ttc2_name = ui->ttc2Box->currentText();
    currentSettings.gps_name = ui->gpsBox->currentText();
    currentSettings.telemetry_answer = (ui->TelemetrycheckBox->checkState() == Qt::Checked)?true:false;
    currentSettings.transmit_answer = (ui->TransmitcheckBox->checkState() == Qt::Checked)?true:false;

}

void SettingsDialog::show()
{
    QDialog::show();
    fillPortsInfo();

    if (this->settings().ttc1_name != "" && this->settings().ttc1_name != selectPort){
        int index = ui->ttc1Box->findText(this->settings().ttc1_name);
        if ( index != -1 ) { // -1 for not found
           ui->ttc1Box->setCurrentIndex(index);
        }
    }

    if (this->settings().ttc2_name != "" && this->settings().ttc2_name != selectPort){
        int index = ui->ttc2Box->findText(this->settings().ttc2_name);
        if ( index != -1 ) { // -1 for not found
           ui->ttc2Box->setCurrentIndex(index);
        }
    }

    if (this->settings().gps_name != "" && this->settings().gps_name != selectPort){
        int index = ui->gpsBox->findText(this->settings().gps_name);
        if ( index != -1 ) { // -1 for not found
           ui->gpsBox->setCurrentIndex(index);
        }
    }

    if (this->settings().telemetry_answer == true){
        ui->TelemetrycheckBox->setCheckState(Qt::Checked);
    }
    if (this->settings().transmit_answer == true){
        ui->TransmitcheckBox->setCheckState(Qt::Checked);
    }

}
