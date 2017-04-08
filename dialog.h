#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class Dialog;
}
class ReMuxer;
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void slotOpenInputFile();
    void slotOpenOutputfile();
    void slotSetOutFormat(const QString &fmt);
    void startTask();
    void slotSetProgressMaxValue(int);
    void slotSetProgress(int);
    void slotFinish();



private:
    QString Ofmt;
    ReMuxer *muxer;
    Ui::Dialog *ui;
};

#endif // DIALOG_H
