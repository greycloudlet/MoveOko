#include "moveoko.h"

moveoko::moveoko(QWidget *parent) :
    QMainWindow(parent)
{
    connect(this, SIGNAL(signal_have_cordinates()), SLOT(slot_connect_w_mouse()));
    setFocusPolicy(Qt::StrongFocus);
}

void moveoko::slot_get_k(QSize sizeWindow){
    QRect sizeDesktop = QApplication::desktop()->screenGeometry();

    kx = sizeDesktop.width()/sizeWindow.width();
    ky = sizeDesktop.height()/sizeWindow.width();
}

void moveoko::slot_get_cordinates(QPoint newCord){
    cordinate.setX(newCord.x()*kx);
    cordinate.setY(newCord.y()*ky);
    emit signal_have_cordinates();
}

void moveoko::slot_connect_w_mouse(){
    if (work) QCursor::setPos(cordinate);
}

void moveoko::slot_get_state(bool state){
    work = state;
}

moveoko::~moveoko()
{

}
