#include "ddldialog.h"
#include "ui_ddldialog.h"

DDLDialog::DDLDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DDLDialog)
{
    ui->setupUi(this);

    connect(ui->okButton, &QPushButton::clicked,this, &DDLDialog::apply);
    connect(ui->cancelButton, &QPushButton::clicked,this, &DDLDialog::hide);
}

DDLDialog::~DDLDialog()
{
    delete ui;
}

void DDLDialog::show()
{
    QDialog::show();
}

void DDLDialog::apply()
{
    this->content = ui->plainTextEdit->toPlainText();
    hide();
}

QString DDLDialog::getContent() const
{
    return this->content;
}
