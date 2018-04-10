#include "camera.h"
#include "ui_camera.h"

//overload operators for QPoint
bool operator>(const QPoint point1, const QPoint point2)
{
    return ((point1.x()>point2.x()) && (point1.y()>point2.y()));
}

QPoint operator+(const QPoint point, const QSize size)
{
    return QPoint(point.x()+size.width(), point.y()+size.height());
}


//constructor
camera::camera(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::camera)
{
    //set ui start configuration
    ui->setupUi(this);
    connect(this, SIGNAL(signal_create_window(QSize)), SLOT(slot_create_window()));
    connect(this, SIGNAL(signal_get_object(Mat)), SLOT(slot_get_object(Mat)));
    connect(this, SIGNAL(signal_sent_shift()), SLOT(slot_sent_shift()));
    this->show();
    ui->butSaveScr->setEnabled(false);
    ui->butStop->setEnabled(false);    
    setMouseTracking(true);
    centralWidget()->setMouseTracking(true);
    ui->LabelForPic->setMouseTracking(true);

    cordinatesShift.setX(0);
    cordinatesShift.setY(0);

    //initialize camera
    capture = VideoCapture(0);
    if (!capture.isOpened()) {
           qDebug()<<"Camera not opened!";
           this->close();
       }

    frame = 0;

    namedWindow("Canny");
    namedWindow("1");
    //namedWindow("2");

    //create timer
    camera::initTimer();

    }


//get frame and set in label
void camera::slot_show_camera()
{
    // получаем кадр
        capture >> frame;

        //mirror format
        flip(frame, frame, 1);
        conv_frame = frame.clone();

        if (first)
        {
            sizeWindow = QSize(frame.size().width, frame.size().height);
            emit signal_create_window(sizeWindow);
            pMOG2 = createBackgroundSubtractorMOG2();
            first = false;
        }

        Mat tmp_frame;

        //set smooth and harshness
        bilateralFilter(conv_frame, tmp_frame, 5, 10, 10);
        absdiff(frame, tmp_frame, tmp_frame);
        add(frame, tmp_frame, conv_frame);

        //substract back
        /*pMOG2->apply(conv_frame, mask_MOG2, 0.01);
        //imshow("1", mask_MOG2);
        sub_back = Scalar(255, 0, 0);
        conv_frame.copyTo(sub_back, mask_MOG2);
        //sub_back = camera::subsBack(conv_frame.clone());
        imshow("2", sub_back);
        sub_back.copyTo(conv_frame);*/
        cvtColor(conv_frame, conv_frame, CV_BGR2HSV);
        //search and tracking
        if (!search) conv_frame = addContours(conv_frame);
            else conv_frame = findObject(conv_frame);

        //convert frame to nice look
        cvtColor(conv_frame, tmp_frame, CV_HSV2RGB);

        //show on pixmap
        QImage* tmp_image = new QImage((const uchar*)(tmp_frame.data),
                                  tmp_frame.size().width,
                                  tmp_frame.size().height,
                                  static_cast<int>(tmp_frame.step),
                                  QImage::Format_RGB888);
        ui->LabelForPic->setPixmap(QPixmap::fromImage(*tmp_image));
        delete tmp_image;

        this->update();
        return;
}

//create window for showing
void camera::slot_create_window()
{
    this->setFixedSize(sizeWindow+QSize( 100 + sizeWindow.width(), 350));
    ui->LabelForPic->move(10,10);
    ui->LabelForPic->resize(sizeWindow);
    ui->LabelForPic_2->move( 10+sizeWindow.width(), 10 );
    ui->LabelForPic_2->resize(sizeWindow);
    int _tmp = this->width()/3 - ui->butStart->width()/2;
    ui->butStart->move(_tmp-_tmp/2, sizeWindow.height() + 140);
    ui->butStop->setEnabled(true);
    ui->butStop->move(_tmp*2 - _tmp/2, sizeWindow.height() + 140);
    ui->butSaveScr->setEnabled(true);
    ui->butSaveScr->move(_tmp*2 + _tmp/2, sizeWindow.height() + 140);

    vbox1 = new QVBoxLayout;
    vbox2 = new QVBoxLayout;
    slidersGroup = new QGroupBox(tr("Canny configuration"), this);
    QString set_name[6] = {"Min 1", "Min 2", "Min 3", "Max 1", "Max 2", "Max 3"};
    for (int i=0; i<6; i++){
        QSlider* slider = new QSlider(Qt::Horizontal);
        QLabel* label = new QLabel(set_name[i]);
        slider->setObjectName(QString::number(i));
        slider->setMinimum(0);
        slider->setMaximum(255);
        sliders.push_back(slider);
        labels.push_back(label);
        connect(sliders.back(), SIGNAL(valueChanged(int)), this, SLOT(setValueSlid(int)));

        if (i<3){
            slider->setValue(40);
            vbox1->addWidget(labels.back());
            vbox1->addWidget(sliders.back());
        }
        else{
            slider->setValue(200);
            vbox2->addWidget(labels.back());
            vbox2->addWidget(sliders.back());
        }
    }
    vbox1->addStretch(1);
    vbox2->addStretch(1);
    slidersGroup->setLayout(vbox1);
    slidersGroup->resize(250, 130);
    slidersGroup->move( 30, sizeWindow.height() + 10 );
    slidersGroup->setAlignment(Qt::AlignHCenter);
    slidersGroup->show();

    return;
}

void camera::setValueSlid(int val)
{
    QString sender = QObject::sender()->objectName();
    if (sender == "1") par[0] = val;
    else if (sender == "2") par[1] = val;
        else par[3] = val;
    return;
}

Mat camera::addContours(Mat image)
{
    Mat out_frame = image.clone();
    Mat tmp_frame = image.clone();

    //Filter HSV
    /*int mink = 0.2;
    int maxk = 1.8;
    inRange( tmp_frame, Scalar(color_cursor[0]*mink, color_cursor[1]*mink, color_cursor[2]*mink),
                        Scalar(color_cursor[0]*maxk, color_cursor[1]*maxk, color_cursor[2]*maxk), tmp_frame);
*/
    Mat _tmp = tmp_frame.clone();
    cvtColor(tmp_frame, _tmp, CV_HSV2BGR);
    cvtColor(_tmp, _tmp, CV_BGR2GRAY);
    Canny(tmp_frame, tmp_frame, minTrash, maxTrash, rangeFil);

    //cvtColor(tmp_frame, _tmp, CV_HSV2RGB);
    //show on pixmap
    QImage* tmp_image = new QImage((const uchar*)(_tmp.data),
                              _tmp.size().width,
                              _tmp.size().height,
                              static_cast<int>(_tmp.step),
                              QImage::Format_Grayscale8);
    ui->LabelForPic_2->setPixmap(QPixmap::fromImage(*tmp_image));
    delete tmp_image;
   //Range
    //cvtColor(image, image, CV_BGR2HLS);
    //inRange(image,Scalar(0, ), Scalar(),image);

    //imshow("Canny", tmp_frame);

    exist = false;

    std::vector< std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    //find contours
    findContours(tmp_frame, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    //draw rect contours
    for (size_t idx = 0; idx < contours.size(); idx++) {
        if (hierarchy[idx][2]>0)
        {
            Rect Rec = boundingRect( contours[idx] );
            if ((Rec.width > 5) &&
                 inRect( mouseCursor, QPoint(Rec.x, Rec.y), QPoint( Rec.x + Rec.width, Rec.y + Rec.height)))
            {
                exist = true;
                objectRect = Rec;
                //drawContours(obj_mask, contours, idx, Scalar(255), CV_FILLED);
                //imshow("2",obj_mask);
                rectangle( out_frame, Point( Rec.x, Rec.y ), Point( Rec.x + Rec.width, Rec.y + Rec.height ), Scalar(255,0,0), 2 );
                targetsHead.setX(Rec.width/2 + Rec.x);
                targetsHead.setY(Rec.y);
                break;
            }
        }
    }
    return out_frame;
}

void camera::slot_get_object(Mat image)
{
    Mat tmp;
    image.copyTo(tmp);
    object = tmp(objectRect);
    imshow("1", object);    
    cvtColor(object, object, CV_BGR2HSV);
    obj_SURF_elem = camera::SURF_find_elem(object); //detect and compute of object
    search = true;
    //imwrite("obj.png", crop);
}

Mat camera::findObject(Mat image)
{
    Mat out_frame = image.clone();

    Mat res;
    int res_cols = image.size().width-object.size().width+1;
    int res_rows = image.size().height-object.size().height+1;
    res.create( res_cols, res_rows, CV_32FC1);

    imshow("object", object);
    imshow("2", image);
    matchTemplate(image, object, res, CV_TM_SQDIFF);

    Point minloc;
    double minval;
    minMaxLoc(res, &minval, NULL, &minloc, NULL);
    normalize( res, res, 0, 1, NORM_MINMAX, -1); //normalize for good look

    // select an area of rect
     showRect = Rect(Point(minloc.x, minloc.y), Point(minloc.x+object.size().width-1, minloc.y+object.size().height-1));
     rectangle( out_frame, showRect, Scalar(255,0,0), 1, 8);
     targetsHead.setX(object.size().width/2 + minloc.x);
     targetsHead.setY(minloc.y);
     emit signal_sent_shift();

    return out_frame;
}

Mat camera::SURF_find_obj( Mat image )
{
    Mat out_frame = image.clone();
    SURF_elem scene = camera::SURF_find_elem(out_frame); //detect and compute of scene

    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( obj_SURF_elem.descriptor, scene.descriptor, matches );

    double max_dist = 0; double min_dist = 100;

      //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < obj_SURF_elem.descriptor.rows; i++ )
          { double dist = matches[i].distance;
            if( dist < min_dist ) min_dist = dist;
            if( dist > max_dist ) max_dist = dist;
          }

    qDebug()<<min_dist;
    qDebug()<<max_dist;

     /*DescriptorMatcher

      -- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
      -- or a small arbitary value ( 0.02 ) in the event that min_dist is very
      -- small)
      -- PS.- radiusMatch can also be used here.*/
    std::vector< DMatch > good_matches;

    for( int i = 0; i < obj_SURF_elem.descriptor.rows; i++ )
    {
        if( matches[i].distance <= max(2*min_dist, 0.02) )
        { good_matches.push_back( matches[i]); }
    }

    //-- Draw only "good" matches
    Mat img_matches = image.clone();

    drawMatches( object, obj_SURF_elem.keypoint, out_frame, scene.keypoint,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    //-- Show detected matches
    /*imshow( "Good Matches", img_matches );
*/
    //drawKeypoints(out_frame, img_matches, Scalar::all(-1));
    /*drawMatches( img_1, keypoints_1, img_2, keypoints_2,
                     good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                     vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );*/

    for( int i = 0; i < (int)good_matches.size(); i++ )
    {
        qDebug()<< "-- Good Match" + QString::number(i) + " Keypoint 1: " + QString::number(good_matches[i].queryIdx) + "  -- Keypoint 2: " + QString::number(good_matches[i].trainIdx );
    }

    // find object on scene
        std::vector< Point2f > obj_point;
        std::vector< Point2f > scene_point;

        for( size_t i = 0; i < good_matches.size(); i++ ) {
            // Отбираем ключевые точки из хороших совпадений
            obj_point.push_back( obj_SURF_elem.keypoint[ good_matches[ i ].queryIdx ].pt );
            scene_point.push_back( scene.keypoint[ good_matches[ i ].trainIdx ].pt );
        }

        if (!obj_point.empty() && !scene_point.empty())
        {
            Mat H = findHomography( obj_point, scene_point, CV_RANSAC);
            if (!H.empty()){
                // Занесем в вектор углы искомого объекта
                std::vector< Point2f > obj_corners( 4 );
                obj_corners[ 0 ] = Point( 0, 0 );
                obj_corners[ 1 ] = Point( object.cols, 0);
                obj_corners[ 2 ] = Point( object.cols, object.rows );
                obj_corners[ 3 ] = Point( 0, object.rows );

                std::vector< Point2f > scene_corners( 4 );
                perspectiveTransform( obj_corners, scene_corners, H );

                // Нарисуем линии между углами (отображение искоромого объекта на сцене)
                line( img_matches, scene_corners[ 0 ] + Point2f( object.cols, 0 ), scene_corners[ 1 ] + Point2f( object.cols, 0 ), Scalar( 0, 255, 0 ), 4 );
                line( img_matches, scene_corners[ 1 ] + Point2f( object.cols, 0 ), scene_corners[ 2 ] + Point2f( object.cols, 0 ), Scalar( 0, 255, 0 ), 4 );
                line( img_matches, scene_corners[ 2 ] + Point2f( object.cols, 0 ), scene_corners[ 3 ] + Point2f( object.cols, 0 ), Scalar( 0, 255, 0 ), 4 );
                line( img_matches, scene_corners[ 3 ] + Point2f( object.cols, 0 ), scene_corners[ 0 ] + Point2f( object.cols, 0 ), Scalar( 0, 255, 0 ), 4 );
            }
        }
    return img_matches;
}

bool camera::have_keypoint( Mat frag_image ){

    bool have_kpoint = false;
    Mat copy_frame = frag_image.clone();
    SURF_elem frag_scene = camera::SURF_find_elem(copy_frame); //detect and compute of scene

    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( obj_SURF_elem.descriptor, frag_scene.descriptor, matches );

    double max_dist = 0; double min_dist = 100;

      //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < obj_SURF_elem.descriptor.rows; i++ )
          { double dist = matches[i].distance;
            if( dist < min_dist ) min_dist = dist;
            if( dist > max_dist ) max_dist = dist;
          }

    qDebug()<<min_dist;
    qDebug()<<max_dist;

    std::vector< DMatch > good_matches;

    for( int i = 0; i < obj_SURF_elem.descriptor.rows; i++ )
    {
        if( matches[i].distance <= max(2*min_dist, 0.02) )
        { good_matches.push_back( matches[i]); }
    }

    return have_kpoint;
}

//detect and compute keypoint and descriptor of input image
camera::SURF_elem camera::SURF_find_elem(Mat image)
{
    Mat tmp_frame = image.clone();
    double minHessian = 300.0;

    SURF_elem item;

    Ptr<SIFT> sift = SIFT::create();
    sift->detectAndCompute(tmp_frame, Mat(), item.keypoint, item.descriptor);

    return item;
}

void camera::slot_sent_shift()
{
    if ((cordinatesShift.x()!=targetsHead.x()))
        if (((cordinatesShift.x()-targetsHead.x())<sizeWindow.width()/4) &
                ((cordinatesShift.y()-targetsHead.y())<sizeWindow.height()/4)){
            cordinatesShift = targetsHead;
            emit signal_sent_ready(cordinatesShift);
        }
}

bool camera::inRect(QPoint point, QPoint pointZero, QPoint pointEnd)
{
    if ((point > pointZero) && (pointEnd > point))
        return true;
    else return false;
}

//control mouse event
void camera::mouseMoveEvent(QMouseEvent *event)
{
    mouseCursor = event->pos()-ui->LabelForPic->pos();
    if (inRect(event->pos(), QPoint(10, 10), QPoint(10+sizeWindow.width(), 10+sizeWindow.height()))){;
            color_cursor = conv_frame.at<Vec3f>(mouseCursor.x(), mouseCursor.y());
    }
    QMainWindow::mouseMoveEvent(event);
}

void camera::mousePressEvent(QMouseEvent *event)
{
    if ((inRect(event->pos(), ui->LabelForPic->pos(), ui->LabelForPic->pos()+ui->LabelForPic->size())) & (!search) & exist)
    {
        //qDebug()<< event->pos()-ui->LabelForPic->pos();
        if (event->button() == Qt::LeftButton){
            qDebug() << "Tergets Head";
            qDebug() << targetsHead;
            emit signal_get_object(frame);
        }
    }
    QMainWindow::mousePressEvent(event);
}

void camera::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_F2) state = false;
    if (event->key()==Qt::Key_F1) state = true;
    qDebug() << "Boom!";
    emit signal_sent_state(state);
    QMainWindow::keyPressEvent(event);
}

//create timer and connect them with slot get frame
void camera::initTimer()
{
    const int imagePeriod = 2000 / 25;   // ms
    imageTimer = new QTimer(this);
    imageTimer->setInterval(imagePeriod);
    connect(imageTimer, SIGNAL(timeout()), this, SLOT(slot_show_camera()));
    return;
}

//start showing camera
void camera::on_butStart_clicked()
{
    imageTimer->start(10);
}

//stop showing camera
void camera::on_butStop_clicked()
{
    imageTimer->stop();
}

//save screen
void camera::on_butSaveScr_clicked()
{
    QString filename="";
    const Mat _pic = frame;
    std::vector<int> compression_params;
        compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);
    try {
         imwrite("alpha.png", _pic, compression_params);
        }
    catch (...) {
         qDebug()<< "Error!";
         //return 1;
        }

    qDebug() << "Saved PNG file with alpha data.\n";
    /*filename = QFileDialog::getSaveFileName(this, "Save picture", QDir::currentPath(),  tr("Images (*.png *.xpm *.jpg)"), 0);
    if (!filename.isEmpty())
    {
        if (cvSaveImage(filename.toLocal8Bit().data(), _pic))
            QMessageBox::information(NULL,"Information", "Find your pic in " + filename);
            else QMessageBox::information(NULL,"Information", "Error");
    }*/
    //delete _pic;
}

//release camera
camera::~camera()
{
    delete imageTimer;
    capture.release();
}

