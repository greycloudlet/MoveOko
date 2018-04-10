#ifndef MOVEOKO_H
#define MOVEOKO_H

#include <QMainWindow>
#include <camera.h>
#include <QDesktopWidget>
#include <QApplication>
#include <QKeyEvent>

class moveoko : public QMainWindow
{
    Q_OBJECT

public:
    explicit moveoko(QWidget *parent);
    ~moveoko();
    QPoint cordinate;

private:
    double kx;
    double ky;
    bool work = true;

signals:
    void signal_have_cordinates();

public slots:
    void slot_get_k(QSize size);
    void slot_get_cordinates(QPoint newCord);
    void slot_get_state(bool state);

private slots:
    void slot_connect_w_mouse();
};


#endif // MOVEOKO_H
