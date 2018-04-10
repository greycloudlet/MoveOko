#include <camera.h>
#include <QApplication>
#include <opencv2/opencv.hpp>
//#include <QObject>
#include <moveoko.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    camera cam;
    moveoko *wind = new moveoko(0);
    QObject::connect(&cam, SIGNAL(signal_create_window(QSize)), wind, SLOT(slot_get_k(QSize)));
    QObject::connect(&cam, SIGNAL(signal_sent_ready(QPoint)), wind, SLOT(slot_get_cordinates(QPoint)));
    QObject::connect(&cam, SIGNAL(signal_sent_state(bool)), wind, SLOT(slot_get_state(bool)));
    return a.exec();
}
