#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include "RectangleGraphicsItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

enum pic_states {
    UPLOADED,
    CHECKED,
    UNCHECKED,
    DOWNLOADED
};

class MainWidget;
class Widget;
class Cloud_Widget;

class TransportWidgetProxy: public QGraphicsProxyWidget {

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        return QGraphicsItem::mouseMoveEvent(event);
    }


    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        return QGraphicsItem::mousePressEvent(event);
    }


    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
    {
        return QGraphicsItem::mouseReleaseEvent(event);
    }

    void resizeEvent(QGraphicsSceneResizeEvent *event) override
    {
        QGraphicsProxyWidget::resizeEvent(event);

    }


};

class TransparentWidget : public QWidget {
public:
    TransparentWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground); // 设置透明背景
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制白色边框
        painter.setPen(Qt::white);
        painter.drawRect(rect().adjusted(0, 0, -1, -1)); // 调整矩形大小以避免边界被裁剪

        // 绘制内容
        painter.setPen(Qt::red);
        painter.drawText(rect(), Qt::AlignCenter, "Transparent Widget");
    }
};

class SceneWidget : public QWidget {
    Q_OBJECT
public:
    SceneWidget(const QPixmap& pic_map, QWidget *parent = nullptr);
    ~SceneWidget() = default;
    void transformPic(const qreal angle);
    QRect saveImg();

private:
    //QPainter* painter;
    QGraphicsScene *m_scene;
    RectangleGraphicsItem* r_item;
    QGraphicsView *view;
    TransportWidgetProxy *proxyWidget;
    QGraphicsPixmapItem *backgroundItem;
    QPointF leftTopPointOrigin;
    QPair<float, float> picWHOrigin;
    QPair<float, float> picWHNow;
    float picScaleRoatio = 1;
    QPixmap pic_map;
    QPixmap modifiedPixmap;
    qreal last_angle = 0;
};


class ImageEditor : public QWidget {
    Q_OBJECT
public:
    ImageEditor(const QPixmap& pic_map, QWidget *parent = nullptr);
    ~ImageEditor() override = default;

private slots:
    void saveImage();

protected:
    //void paintEvent(QPaintEvent *event) override;

private:
    SceneWidget *imageLabel = nullptr;
    RectangleGraphicsItem* image_rect = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    QHBoxLayout *headLayout = nullptr;
    QSlider* slider = nullptr;
    QImage* img = nullptr;
    QPushButton* saveButton = nullptr;

};

class FileUploader : public QObject
{
    Q_OBJECT

public:
    explicit FileUploader(QObject *parent = nullptr) : QObject(parent) {}

    void uploadFile(const QString &filePath, const QUrl &uploadUrl, Cloud_Widget* f_widget);

private:
    Cloud_Widget* f_cloud_widget = nullptr;
};


class Widget : public QWidget
{
    Q_OBJECT

public:
    QVector<QString> checked_pic;
    Widget(MainWidget* father_widget, QWidget *parent = nullptr);
    ~Widget();
    void init_widgets_pic(const QString& dir_path, const QString& uploadUrl);
    QVector<QPair<QString, QPixmap>> get_album_pic(const QString& pic_dir_path);

public slots:
    void upload_pic();

private:
    Ui::Widget *ui;  
    QMap<QString, pic_states> picName_states_map;
    QMap<QString, QString> picName_path_map;
    QMap<QString, FileUploader*> picName_uploader;
    QMap<QString, QPair<int, int>> picName_pos;
    MainWidget* fatherMainWidget = nullptr;
    int now_index = 0;
    const int MAX_NUM_EACH_COL = 5;
    QGridLayout* grid_layout = nullptr;
    QString uploadUrl = "";
    friend class ClickableLabel;
};

class ImageDownloader : public QObject
{
    Q_OBJECT

public:
    QString f_name = "1.png";

    ImageDownloader(QObject *parent = nullptr) : QObject(parent)
    {
        manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, &ImageDownloader::onDownloadFinished);
    }

    ImageDownloader(const QString& s, QObject *parent = nullptr) : QObject(parent), f_name(s)
    {
        manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, &ImageDownloader::onDownloadFinished);
    }


    void downloadImage(const QUrl &imageUrl)
    {
        QNetworkRequest request(imageUrl);
        manager->get(request);
    }

signals:
    void imageDataReady(const QPixmap &pixmap, const QString& f_n);

private slots:
    void onDownloadFinished(QNetworkReply *reply)
    {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imageData = reply->readAll();
            QPixmap pixmap;
            pixmap.loadFromData(imageData);
            emit imageDataReady(pixmap, f_name);
        } else {
            qDebug() << "Error downloading image:" << reply->errorString();
        }

        reply->deleteLater();
    }

private:
    QNetworkAccessManager *manager;
};

class Cloud_Widget : public QWidget
{
    Q_OBJECT

public:
    QVector<QString> checked_pic;
    Cloud_Widget(MainWidget* fatherMainWidget, QWidget *parent = nullptr);
    ~Cloud_Widget();
    void init_cloud_widgets_pic(const QString& get_pic_map_url, const QString& server_root_url);
    void display_pic_widgets(const QJsonObject& jsonObj, const QString& root_url);
    void reload_widgets(const QString& get_pic_map_url, const QString& server_root_url);
    void updateWidgets();
    void setLoaderNum(int n) {this->now_loader_num = n >= 0? n: 0;};
    bool loaderCompete() {return this->now_loader_num == 0?false: (now_loader_num-- == 1);};
    //QVector<QPair<QString, QPixmap>> get_album_pic(const QString& dir_url);

public slots:
    void download_pic();

private:
    Ui::Widget *ui;
    QMap<QString, pic_states> picName_states_map;
    QMap<QString, QString> picName_path_map;
    QMap<QString, ImageDownloader*> picName_Downloader;
    QMap<QString, QPair<int, int>> picName_pos;
    MainWidget* fatherMainWidget = nullptr;
    int now_index = 0;
    int now_loader_num = 0;
    const int MAX_NUM_EACH_COL = 5;
    QGridLayout* grid_layout = nullptr;
    QString pic_map_url = "";
    QString server_root_url = "";
    friend class ClickableLabel;
};

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    ClickableLabel(const QString& pic_path, Widget *f_widget, QWidget *parent = nullptr) : QLabel(parent), f_widget(f_widget) {
        pic_name = QFileInfo(pic_path).fileName();
        // 加载图片
        QPixmap pixmap(pic_path);
        if (pixmap.isNull()) {
            qDebug() << "the pixmap is null!\n";
        }

        init_label(pixmap);
        editorButton = new QPushButton("编辑");
        isSetPixMap = true;
    }

    ClickableLabel(const QPixmap& pixmap, const QString& pic_name, Widget *f_widget, QWidget *parent = nullptr) : QLabel(parent), pic_name(pic_name), f_widget(f_widget) {
        if (pixmap.isNull()) {
            qDebug() << "the pixmap is null!\n";
        }

        init_label(pixmap);
        isSetPixMap = true;
        editorButton = new QPushButton("编辑");
    }

    ClickableLabel(const QString& pic_url, const QString& pic_name, Cloud_Widget *f_widget, QWidget *parent = nullptr) : QLabel(parent), pic_name(pic_name), f_widget_cloud(f_widget) {

        qDebug() << "begin to get pic from " << pic_url << "\n";
        this->editorButton = new QPushButton("编辑");
        imageDownloader = new ImageDownloader(this);
        QObject::connect(imageDownloader, &ImageDownloader::imageDataReady, [&](const QPixmap &pixmap) {
            // 处理下载并转换后的 QPixmap 对象
            if (!pixmap.isNull()) {
                // 显示图像
                this->init_label(pixmap);
                this->isSetPixMap = true;
            } else {
                qDebug() << "Failed to load image.";
            }
        });

        QUrl imageUrl(pic_url);
        imageDownloader->downloadImage(imageUrl);
    }

    void setPixMap(const QPixmap& pixmap) {
        if (pixmap.isNull()) {
            qDebug() << "the pixmap is null!\n";
        }

        init_label(pixmap);
        isSetPixMap = true;
    }

    QPushButton* getEditorButton() {
        return editorButton;
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && isSetPixMap) {
            // 检查点击位置是否在图片上
            if (this->rect().contains(event->pos())) {
                // 切换√的显示状态
                if (f_widget != nullptr && (f_widget->picName_states_map).count(pic_name)) {
                    if (!checkmark->isVisible()) {
                        (f_widget->picName_states_map)[pic_name] = CHECKED;
                    } else {
                        (f_widget->picName_states_map)[pic_name] = UNCHECKED;
                    }
                }

                if (f_widget_cloud != nullptr && (f_widget_cloud->picName_states_map).count(pic_name)) {
                    if (!checkmark->isVisible()) {
                        (f_widget_cloud->picName_states_map)[pic_name] = CHECKED;
                    } else {
                        (f_widget_cloud->picName_states_map)[pic_name] = UNCHECKED;
                    }
                }

                checkmark->setVisible(!checkmark->isVisible());
                event->accept();
            }
        }
    }

private:
    QLabel *checkmark = nullptr;
    QPushButton* editorButton = nullptr;
    QString pic_name = "";
    Widget *f_widget = nullptr;
    Cloud_Widget* f_widget_cloud = nullptr;
    ImageDownloader* imageDownloader = nullptr;
    bool isSetPixMap = false;

    inline void init_label(const QPixmap& pixmap) {
        this->setFixedSize(200, 200);
        pixmap.scaled(this->size(), Qt::KeepAspectRatio);
        this->setPixmap(pixmap);
        this->setScaledContents(true);


        checkmark = new QLabel(this);
        QPixmap checked_pix("..//ui_cloud/checked.png");
        if (checked_pix.isNull()) {
            qDebug() << "the checked_pix is null!\n";
        }
        checked_pix.scaled(30, 30, Qt::KeepAspectRatio);
        checkmark->setPixmap(checked_pix);
        checkmark->setScaledContents(true);
        checkmark->hide();
    }
};



class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
    void init_widgets(const QString& dir_path, const QString& uploadUrl, const QString& get_pic_map_url, const QString& server_root_url);
    void updataCloudWidgets();
    Cloud_Widget* getCloudWidget(){return this->cloudWidget;};
    void changeImgEditor(const QPixmap& pic_map);
protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    Ui::Widget *ui;
    QTimer* timer = nullptr;
    Widget* localWidget = nullptr;
    Cloud_Widget* cloudWidget = nullptr;
    //QPixmap* now_pic_pixmap = nullptr;
    QStackedWidget* stackedWidget = nullptr;
    QPushButton* switchToLoacl = nullptr;
    QPushButton* switchToCloud = nullptr;
    QPushButton* lastPressButton = nullptr;
    ImageEditor* imageEditor = nullptr;

};





#endif // WIDGET_H
