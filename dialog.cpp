#include "dialog.h"
#include "ui_dialog.h"
#include "remuxer.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    muxer(new ReMuxer(this)),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromUtf8("格式转换"));
    //Init component
    Ofmt = ".mp4";
    ui->InputEdit->setReadOnly(true);
    ui->InputEdit->setPlaceholderText(QString::fromUtf8("请设置待处理视频"));
    ui->InputButton->setWhatsThis("open input dialog");
    ui->OutputButton->setWhatsThis("open output dialog");
    ui->OutputEdit->setReadOnly(true);
    ui->OutputEdit->setPlaceholderText(QString::fromUtf8("请设置输出地址"));
    ui->OutputFormatBox->addItem(QObject::tr(".mp4"));
    ui->OutputFormatBox->addItem(QObject::tr(".mkv"));
    ui->OutputFormatBox->addItem(QObject::tr(".flv"));
    ui->OutputFormatBox->addItem(QObject::tr(".avi"));
    ui->OutputFormatBox->addItem(QObject::tr(".ts"));
    ui->progressBar->hide();

    connect(ui->InputButton,SIGNAL(clicked()),this,SLOT(slotOpenInputFile()));
    connect(ui->OutputButton,SIGNAL(clicked()),this,SLOT(slotOpenOutputfile()));
    connect(ui->OutputFormatBox,SIGNAL(activated(QString)),this,SLOT(slotSetOutFormat(QString)));
    connect(ui->startButton,SIGNAL(clicked()),this,SLOT(startTask()));
    connect(muxer,SIGNAL(sendProgressMaxValue(int)),this,SLOT(slotSetProgressMaxValue(int)));
    connect(muxer,SIGNAL(sendProgress(int)),this,SLOT(slotSetProgress(int)));
    connect(muxer,SIGNAL(finished()),this,SLOT(slotFinish()));


}

void Dialog::slotOpenInputFile()
{
    QString file = QFileDialog::getOpenFileName(this,tr("open video"),".",
                                                tr("video(*.mp4 *.mkv *.flv)"));
    if(file.isEmpty())
        return;

    ui->InputEdit->setText(file);
}

void Dialog::slotOpenOutputfile()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty())
        return;

    ui->OutputEdit->setText(dir);
}

void Dialog::slotSetOutFormat(const QString &fmt)
{
    if(fmt.isEmpty())
        return;
    Ofmt =fmt;
}

void Dialog::startTask()
{
    if(ui->InputEdit->text().isEmpty() ||
            ui->OutputEdit->text().isEmpty())
    {
        QMessageBox::warning(this,tr("warning"),
                             tr("input or output can not be null"),
                             QMessageBox::Ok);
        return;
    }

    ui->InputButton->setEnabled(false);
    ui->OutputButton->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->progressBar->show();

    QString inFile = ui->InputEdit->text();
    QString baseName = QFileInfo(inFile).baseName();
    QString file = ui->OutputEdit->text();
    file = file + baseName + "_res" + Ofmt;

    if(muxer->isRunning())
    {
        muxer->terminate();
        muxer->wait();
    }
    muxer->setInputFile(ui->InputEdit->text());
    muxer->setOutFile(file);
    muxer->start();
}

void Dialog::slotSetProgressMaxValue(int value)
{
    ui->progressBar->setMaximum(value);
}

void Dialog::slotSetProgress(int value)
{
    ui->progressBar->setValue(value);
}

void Dialog::slotFinish()
{
    ui->InputButton->setEnabled(true);
    ui->OutputButton->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->progressBar->reset();
    ui->progressBar->hide();
    QMessageBox::information(this,tr("message"),tr("finish!"),QMessageBox::Ok);
}

Dialog::~Dialog()
{
    delete ui;
}
