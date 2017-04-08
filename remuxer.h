#ifndef REMUXER_H
#define REMUXER_H

#include "ffmpeg.h"
#include <QThread>

class QProgressBar;
class ReMuxer : public QThread
{
    Q_OBJECT
public:
    explicit ReMuxer(QObject *parent);
    ~ReMuxer()=default;

    void setInputFile(QString filename);
    void setOutFile(QString filename);
    int remux();

protected:
    void run();

private:
    QString In_File;
    QString Out_File;
    QProgressBar *progress = nullptr;

signals:
    void sendProgressMaxValue(int);
    void sendProgress(int value);

};

#endif // REMUXER_H
