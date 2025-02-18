#include "widget.h"
#include "ui_widget.h"


Widget::Widget(MainWidget* father_widget, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , fatherMainWidget(father_widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::init_widgets_pic(const QString& dir_path, const QString& uploadUrl) {
    QVector<QPair<QString, QPixmap>> r(get_album_pic(dir_path));
    QGridLayout* pic_layout = new QGridLayout();
    this->uploadUrl = uploadUrl;
    int row = 0;
    int col = 0;
    for (auto& pair_r: r) {
        QString f_name = pair_r.first;
        QPixmap pixmap = pair_r.second;

        QVBoxLayout* p_n = new QVBoxLayout();
//        QLabel* pic_ = new QLabel();
//        pic_->setFixedSize(200, 200);
//        pixmap.scaled(pic_->size(), Qt::KeepAspectRatio);
//        pic_->setPixmap(pixmap);
//        pic_->setScaledContents(true);
        ClickableLabel* pic_ = new ClickableLabel(pixmap, f_name, this);
        picName_pos[f_name] = QPair<int, int>(row, col);
        ++now_index;
        p_n->addWidget(pic_);
        p_n->addWidget(new QLabel(f_name));
        QPushButton* eButton = pic_->getEditorButton();
        p_n->addWidget(eButton);
        connect(eButton, &QPushButton::clicked, this->fatherMainWidget, [this, pic_](){
            this->fatherMainWidget->changeImgEditor(*(pic_->pixmap()));
        });
        QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        p_n->addSpacerItem(spacer);
        pic_layout->addLayout(p_n, row, col++);
        if (col >= MAX_NUM_EACH_COL) {
            col = 0;
            ++row;
        }
    }

    if (row == 0 && col < MAX_NUM_EACH_COL) {
        QWidget *spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        pic_layout->addWidget(spacer, 0, col++);
    }
//    if (now_index < MAX_NUM_EACH_COL) {
//        int left_col = MAX_NUM_EACH_COL - now_index;
//        while (left_col > 0) {
//            pic_layout->addWidget(new QLabel(""));
//            --left_col;
//        }
//    }
    pic_layout->setSpacing(10);

    QHBoxLayout* button_layout = new QHBoxLayout();
    QPushButton* upload_button = new QPushButton();
    upload_button->setText("批量上传");
    QObject::connect(upload_button, &QPushButton::clicked, this, &Widget::upload_pic);
    QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    button_layout->addItem(spacer);
    button_layout->addWidget(upload_button);

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->addLayout(button_layout);
    main_layout->addLayout(pic_layout);
    this->grid_layout = pic_layout;

    this->setLayout(main_layout);
}

void Widget::upload_pic() {
    int needUpdateNum = 0;
    for (auto it = picName_states_map.constBegin(); it != picName_states_map.constEnd(); it++) {
        if (it.value() == CHECKED) {
            //上传
            ++needUpdateNum;
        }
        //qDebug() << it.key() << " " << it.value() << " " << picName_path_map[it.key()] <<"\n";
    }
    this->fatherMainWidget->getCloudWidget()->setLoaderNum(needUpdateNum);
    qDebug() << "the loader num is " << needUpdateNum << "\n";
    for (auto it = picName_states_map.constBegin(); it != picName_states_map.constEnd(); it++) {
        if (it.value() == CHECKED) {
            //上传
            picName_uploader[it.key()] = new FileUploader(this);
            picName_uploader[it.key()]->uploadFile(picName_path_map[it.key()], this->uploadUrl, this->fatherMainWidget->getCloudWidget());
            picName_states_map[it.key()] = UPLOADED;
        }
        qDebug() << it.key() << " " << it.value() << " " << picName_path_map[it.key()] <<"\n";
    }

}

QVector<QPair<QString, QPixmap>> Widget::get_album_pic(const QString& pic_dir_path) {
    QDir dir(pic_dir_path);
    QVector<QPair<QString, QPixmap>> r;

    if (!dir.exists()) {
        qDebug() << pic_dir_path << " Folder not exist.\n" << QDir::currentPath();
        return (r);
    }

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList list_files = dir.entryInfoList();

    for (const auto& file: list_files) {
        if (file.isFile()) {
            QPixmap pixmap(file.absoluteFilePath());
            if (!pixmap.isNull()) {
                this->picName_path_map[file.fileName()] = file.absoluteFilePath();
                this->picName_states_map[file.fileName()] = UNCHECKED;
                r.push_back(qMakePair(file.fileName(), pixmap));
            } else {
                qDebug() << "ERROR When get file\n";
            }
        } else {
            qDebug() << "ERROR When get file\n";
        }
    }

    return (r);
}

Cloud_Widget::Cloud_Widget(MainWidget* fatherMainWidget, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , fatherMainWidget(fatherMainWidget)
{
    ui->setupUi(this);
}

Cloud_Widget::~Cloud_Widget()
{
    delete ui;
}

void Cloud_Widget::download_pic() {
    qDebug() << "begin download \n";
    for (auto it = picName_states_map.begin(); it != picName_states_map.end(); it++) {
        if (it.value() == CHECKED) {
            QString f_name = it.key();
            picName_Downloader[f_name] = new ImageDownloader(f_name, this);
            picName_states_map[f_name] = DOWNLOADED;
            QObject::connect(picName_Downloader[it.key()], &ImageDownloader::imageDataReady, [](const QPixmap &pixmap, const QString& f_name) {
                // 处理下载并转换后的 QPixmap 对象
                //qDebug() << "Finis load image." << *f_name << "from " << picName_path_map[*f_name] << "\n";
                //ImageDownloader *senderObject = qobject_cast<ImageDownloader*>(QObject::sender());
                qDebug() << "Finis load image.";
                if (!pixmap.isNull()) {
                    // 显示图像
                    pixmap.save("E:\\cloud_album_qt_ui\\ui\\ui_cloud\\download\\" + f_name);
                } else {
                    qDebug() << "Failed to load image.";
                }
                //delete f_name;
            });

            QUrl imageUrl(picName_path_map[it.key()]);
            picName_Downloader[it.key()]->downloadImage(imageUrl);
            qDebug() << "begin download from" << imageUrl << "\n";
        }

        qDebug() << it.key() << " " << it.value() << "\n";
    }

}

void Cloud_Widget::init_cloud_widgets_pic(const QString& get_pic_map_url, const QString& server_root_url) {
    this->pic_map_url = get_pic_map_url;
    this->server_root_url = server_root_url;
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QUrl url(get_pic_map_url);
    QNetworkRequest request(url);
    qDebug() << "begin get sonmething from " << url << "\n";
    QNetworkReply *reply = manager->get(request);

    QObject::connect(reply, &QNetworkReply::finished, [reply, this, server_root_url]() {
        qDebug() << "get something from server\n";
        if (reply == nullptr) {
            qDebug() << "reply null\n";
            return;
        }

        if (reply->error() == QNetworkReply::NoError) {
                // 读取并处理响应内容
            QByteArray responseData = reply->readAll();
                // 在这里处理响应内容，例如将其转换为图片等
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            //qDebug() << "get something from server2\n";
            // 检查 JSON 数据是否有效
            if (!jsonDoc.isNull()) {
                //qDebug() << "get something from server3\n";
                if (jsonDoc.isObject()) {
                    // 将 JSON 数据转换为 QJsonObject
                    QJsonObject jsonObj = jsonDoc.object();
                    qDebug() << "begin get pic from server\n";
                    this->display_pic_widgets(jsonObj, server_root_url);
                } else {
                    qDebug() << "JSON document is not an object";
                }
            } else {
                qDebug() << "Failed to parse JSON document";
            }
        } else {
            qDebug() << "Error:" << reply->errorString();
        }

            // 释放资源
        reply->deleteLater();
    });
    qDebug() << "finish init cloud weight\n";
}


void Cloud_Widget::display_pic_widgets(const QJsonObject& jsonObj, const QString& root_url) {
    // 从 QJsonObject 中读取具体的键值对
    QGridLayout* pic_layout;
    if (this->grid_layout == nullptr) {
        pic_layout = new QGridLayout();
    } else {
        pic_layout = this->grid_layout;
    }


    int row = this->now_index / MAX_NUM_EACH_COL;
    int col = this->now_index % MAX_NUM_EACH_COL;
    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
        QString f_name = it.key();
        QJsonValue value = it.value();

        // 检查值的类型并进行相应处理
        if (value.isString()) {
            if (this->picName_states_map.count(f_name)) {
                continue;
            }
            QString strValue = value.toString();
            QString pic_url = root_url + strValue;
            this->picName_states_map[f_name] = UNCHECKED;
            this->picName_path_map[f_name] = pic_url;

            QVBoxLayout* p_n = new QVBoxLayout();
            ClickableLabel* pic_ = new ClickableLabel(pic_url, f_name, this);
            picName_pos[f_name] = QPair<int, int>(row, col);
            ++now_index;
            p_n->addWidget(pic_);
            p_n->addWidget(new QLabel(f_name));
            QPushButton* eButton = pic_->getEditorButton();
            p_n->addWidget(eButton);
            connect(eButton, &QPushButton::clicked, this->fatherMainWidget, [this, pic_](){
                this->fatherMainWidget->changeImgEditor(*(pic_->pixmap()));
            });
            QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            p_n->addSpacerItem(spacer);
            pic_layout->addLayout(p_n, row, col++);
            if (col >= MAX_NUM_EACH_COL) {
                col = 0;
                ++row;
            }

            qDebug() << "Key:" << f_name << ", Value:" << strValue;
        }
    }

    if (row == 0 && col < MAX_NUM_EACH_COL) {
        QWidget *spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        pic_layout->addWidget(spacer, 0, col++);
    }

    if (this->grid_layout == nullptr) {
        pic_layout->setSpacing(10);

        QHBoxLayout* button_layout = new QHBoxLayout();
        QPushButton* download_button = new QPushButton();
        download_button->setText("批量下载");
        QObject::connect(download_button, &QPushButton::clicked, this, &Cloud_Widget::download_pic);
        QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        button_layout->addItem(spacer);
        button_layout->addWidget(download_button);

        QVBoxLayout* main_layout = new QVBoxLayout();
        main_layout->addLayout(button_layout);
        main_layout->addLayout(pic_layout);
        this->grid_layout = pic_layout;
        this->setLayout(main_layout);
    }

}

void Cloud_Widget::reload_widgets(const QString &get_pic_map_url, const QString &server_root_url) {
    init_cloud_widgets_pic(get_pic_map_url, server_root_url);
}

void Cloud_Widget::updateWidgets() {
    qDebug() << "begin update widgets\n";
    this->reload_widgets(this->pic_map_url, this->server_root_url);
}

void FileUploader::uploadFile(const QString &filePath, const QUrl &uploadUrl, Cloud_Widget *f_widget) {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkRequest request(uploadUrl);


    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file for upload";
        return;
    }
    // 生成一个随机的 boundary 参数
    QString boundary = "boundaryString"; // 你可以使用随机字符串生成方法来生成

    // 设置正确的 Content-Type 头信息，包括边界参数
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);

    // 创建 multipart 数据，包含文件数据和边界参数
    QByteArray data;
    data.append("--" + boundary + "\r\n");
    data.append("Content-Disposition: form-data; name=\"file\"; filename=\"" + file.fileName() + "\"\r\n");
    data.append("Content-Type: application/octet-stream\r\n\r\n");
    data.append(file.readAll());
    data.append("\r\n--" + boundary + "--\r\n");

    QNetworkReply *reply = manager->post(request, data);
    file.close();
    qDebug() << "begin upload file" << filePath << "to" << uploadUrl << "\n";

    connect(reply, &QNetworkReply::finished, this, [reply, f_widget]() {
        if (reply->error() == QNetworkReply::NoError) {
            if (f_widget->loaderCompete()) {
                qDebug() << "begin update cloud widgets\n";
                f_widget->updateWidgets();
            }
            qDebug() << "File uploaded successfully!";
        } else {
            qDebug() << "Failed to upload file:" << reply->errorString();
        }

        reply->deleteLater();
    });
}

//QVector<QPair<QString, QPixmap>> Cloud_Widget::get_album_pic(const QString& dir_url) {
//    return QVector<QPair<QString, QPixmap>>();
//}

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

MainWidget::~MainWidget() {
    delete ui;
}

void MainWidget::init_widgets(const QString& dir_path, const QString& uploadUrl, const QString& get_pic_map_url, const QString& server_root_url) {
    localWidget = new Widget(this);
    localWidget->init_widgets_pic(dir_path, uploadUrl);
    //localWidget->fatherMainWidget = this;

    cloudWidget = new Cloud_Widget(this);
    cloudWidget->init_cloud_widgets_pic(get_pic_map_url, server_root_url);

    stackedWidget = new QStackedWidget();
    stackedWidget->addWidget(localWidget);
    stackedWidget->addWidget(cloudWidget);
    //stackedWidget->addWidget(imageEditor);
    stackedWidget->setCurrentWidget(localWidget);

    switchToLoacl = new QPushButton("本地相册");
    switchToCloud = new QPushButton("云相册");
    lastPressButton = switchToLoacl;
    switchToCloud->installEventFilter(this);
    switchToLoacl->installEventFilter(this);

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(switchToLoacl);
    buttonLayout->addWidget(switchToCloud);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(stackedWidget);

    setLayout(mainLayout);
}


bool MainWidget::eventFilter(QObject *obj, QEvent *e) {
    if ((obj == switchToLoacl || obj == switchToCloud) && e->type() == QEvent::MouseButtonPress)
    {
        if (obj != lastPressButton) {
            if (obj == switchToCloud) {
                lastPressButton = switchToCloud;
                stackedWidget->setCurrentWidget(cloudWidget);
            } else if (obj == switchToLoacl) {
                lastPressButton = switchToLoacl;
                stackedWidget->setCurrentWidget(localWidget);
            }
        }

        return true;
    }

    return false;
}

void MainWidget::updataCloudWidgets() {
    this->cloudWidget->updateWidgets();
}

void MainWidget::changeImgEditor(const QPixmap& pic_map) {
    if (imageEditor != nullptr) {
        stackedWidget->removeWidget(imageEditor);
        delete imageEditor;
    }
    imageEditor = new ImageEditor(pic_map);
    stackedWidget->addWidget(imageEditor);
    stackedWidget->setCurrentWidget(imageEditor);
    lastPressButton = nullptr;
}

SceneWidget::SceneWidget(const QPixmap& pic_map, QWidget *parent) :QWidget(parent) {
    this->resize(1000, 1000);
    QPixmap largerPic = pic_map.scaled(800, 800);
    this->pic_map = pic_map;
    this->modifiedPixmap = this->pic_map;

    picScaleRoatio = largerPic.width() / (float)pic_map.width();
    picWHNow.first = pic_map.width();
    picWHNow.second = pic_map.height();
    picWHOrigin.first = pic_map.width();
    picWHOrigin.second = pic_map.height();

    m_scene = new QGraphicsScene(this);
    qDebug() << this->height() << "is height \n";
    //m_scene->setSceneRect(0, 0, this->height()-100, this->height()-100);
    //qreal sceneHeight = this->height();


    backgroundItem = m_scene->addPixmap(largerPic);
    qDebug() << backgroundItem->boundingRect() << "bounding RECT of bcakgroundItem\n";

    leftTopPointOrigin.setX(backgroundItem->boundingRect().x());
    leftTopPointOrigin.setY(backgroundItem->boundingRect().y());

    backgroundItem->setAcceptedMouseButtons(Qt::NoButton);
//    QString filePath = QFileDialog::getSaveFileName(nullptr, "Save Image", "", "Images (*.png *.xpm *.jpg)");
//    if (!filePath.isEmpty()) {
//        //pic_map_copy.copy(0, 0, 20, 30).save(filePath);
//    }

//    QString filePath2 = QFileDialog::getSaveFileName(nullptr, "Save Image", "", "Images (*.png *.xpm *.jpg)");
//    if (!filePath2.isEmpty()) {
//        //p.copy(0+(pic_map_copy.width()-p., ).save(filePath2);
//    }

    //qDebug() << backgroundItem->boundingRect() << "bounding RECT of bcakgroundItem\n";
    this->r_item = new RectangleGraphicsItem({backgroundItem->boundingRect().x(), backgroundItem->boundingRect().y()}, backgroundItem->boundingRect().width(), backgroundItem->boundingRect().height(), backgroundItem->boundingRect().center(), backgroundItem);
    m_scene->addItem(r_item);

    view = new QGraphicsView(m_scene, this);

    //view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //view->setGeometry(0, 0, this->height()-100, this->height()-100);
//    QTransform rotationTransform;
//        rotationTransform.rotate(45); // 旋转角度为45度

//        view->setTransform(rotationTransform);
    //view->fitInView(backgroundItem, Qt::KeepAspectRatio);
    //saveImg();
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);
}

void SceneWidget::transformPic(const qreal angle) {
    // 将旋转中心移动到图片中心
    qDebug() << "transform pic for " << angle << "\n";
    //首先恢复原图，然后再原图的基础上旋转
    QPointF sceneCenter = backgroundItem->boundingRect().center();
    QTransform transform;
    transform.translate(sceneCenter.x(), sceneCenter.y());
    transform.rotate(angle - last_angle);
    transform.translate(-sceneCenter.x(), -sceneCenter.y());
    backgroundItem->setTransform(transform, true);
    r_item->transformBoundaryRect(&transform);

    modifiedPixmap = (this->pic_map).transformed(transform);
    qDebug() << pic_map.size() << " this size of origin map \n";
    qDebug() << modifiedPixmap.size() << " this size of transformed map \n";
    picWHNow.first = modifiedPixmap.width();
    picWHNow.second = modifiedPixmap.height();
    last_angle = angle;
}

QRect SceneWidget::saveImg() {
    //获取截图框在画布中的位置
    QRectF picRectOnScene = r_item->imgPartRect();
    qDebug() << picRectOnScene << " this rect size on the scene\n";
    qDebug() << leftTopPointOrigin << " this point of origin backitem on scene\n";
    qDebug() << picScaleRoatio << " this picRect ratio\n";
    //获取截图框相对于原始图像的位置
    float x_pox = (picWHNow.first - picWHOrigin.first)/2;
    float y_pox = (picWHNow.second - picWHOrigin.second)/2;
    QRect rectOnPic((picRectOnScene.x()-leftTopPointOrigin.x())/picScaleRoatio+x_pox, (picRectOnScene.y()-leftTopPointOrigin.y())/picScaleRoatio+y_pox, picRectOnScene.width()/picScaleRoatio, picRectOnScene.height()/picScaleRoatio);
    QString filePath = QFileDialog::getSaveFileName(nullptr, "Save Image", "", "Images (*.png *.xpm *.jpg)");
    if (!filePath.isEmpty()) {
        modifiedPixmap.copy(rectOnPic).save(filePath);
    }


    qDebug() << rectOnPic << " the rect on now pic\n";
    return rectOnPic;
}

void ImageEditor::saveImage() {
    qDebug() << "save img\n";
    //imageLabel->transformPic(45);
    QRect m_r = imageLabel->saveImg();

}

ImageEditor::ImageEditor(const QPixmap& pic_map, QWidget *parent): QWidget(parent) {
    imageLabel = new SceneWidget(pic_map);
    headLayout = new QHBoxLayout();
    saveButton = new QPushButton("保存");
    headLayout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &ImageEditor::saveImage);
    mainLayout = new QVBoxLayout();
    mainLayout->addLayout(headLayout);
    mainLayout->addWidget(imageLabel);
    slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 180);
//    slider->setMinimum(0);
//    slider->setMaximum(180);
    slider->setSingleStep(10);
    slider->setPageStep(20);
    connect(slider, &QSlider::valueChanged, imageLabel, &SceneWidget::transformPic);
    mainLayout->addWidget(slider);

    this->setLayout(this->mainLayout);
}





