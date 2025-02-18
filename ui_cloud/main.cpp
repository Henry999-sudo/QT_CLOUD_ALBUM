#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    Widget w;
//    const QString path("E:\\cloud_album_qt_ui\\ui\\ui_cloud\\pic");
//    w.init_widgets_pic(path);
//    w.show();
//    Cloud_Widget w;
//    w.init_cloud_widgets_pic("http://localhost:8080/get_pic_path_dict", "http://localhost:8080");
//    w.show();
//    QPixmap pixmap("E:\\cloud_album_qt_ui\\ui\\ui_cloud\\pic\\user02.png");
    MainWidget w;
    w.init_widgets("E:\\cloud_album_qt_ui\\ui\\ui_cloud\\pic", "http://localhost:8080/upload", "http://localhost:8080/get_pic_path_dict", "http://localhost:8080");
    w.show();
//    ImageEditor imageediter(pixmap);
//    imageediter.resize(1000, 1000);
//    imageediter.show();
//    SceneWidget sp (pixmap);
//    ///sp.resize(1000, 1000);
//    sp.show();

    return a.exec();
}
