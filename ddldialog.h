#ifndef DDLDIALOG_H
#define DDLDIALOG_H

#include <QDialog>

namespace Ui {
class DDLDialog;
}

class DDLDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DDLDialog(QWidget *parent = 0);
    ~DDLDialog();

    void show();
    void apply();
    QString getContent() const;

private:
    Ui::DDLDialog *ui;

    QString content;
};

#endif // DDLDIALOG_H
