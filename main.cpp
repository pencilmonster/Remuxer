#include "dialog.h"
#include <QApplication>
#include "ffmpeg.h"

int main(int argc, char *argv[])
{
    avcodec_register_all();
    av_register_all();
    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
