#include "RectangleGraphicsItem.h"

PointGraphicsItem::PointGraphicsItem(QGraphicsItem* parent)
    :QGraphicsItem(parent)
{
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
}

PointGraphicsItem::~PointGraphicsItem() = default;

QRectF PointGraphicsItem::boundingRect() const {
    return {pos().x() - Width/2.0, pos().y()-Height/2.0, Width, Height};
}

void PointGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

}

RectangleGraphicsItem::RectangleGraphicsItem(QPointF lt_p, qreal width_x, qreal height_y, const QPointF& center, QGraphicsItem* boundaryItem, QGraphicsItem* parent)
    :QGraphicsItem(parent),
    m_left_top_item(new PointGraphicsItem),
    m_left_bottom_item(new PointGraphicsItem),
    m_right_top_item(new PointGraphicsItem),
    m_right_bottom_item(new PointGraphicsItem),
    m_center_item(new PointGraphicsItem),
    m_left_top_item_boundary(new QPointF),
    m_left_bottom_item_boundary(new QPointF),
    m_right_top_item_boundary(new QPointF),
    m_right_bottom_item_boundary(new QPointF),
    m_center_item_boundary(new QPointF),
    m_left_line_boundary(new QLineF),
    m_top_line_boundary(new QLineF),
    m_bottom_line_boundary(new QLineF),
    m_right_line_boundary(new QLineF),
    boundaryItem(boundaryItem),
    m_drag_type(DragType::Release)
{
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    //pos_boundartRect = QRect(lt_p.x(), lt_p.y(), width_x, height_y);
    m_line_color = Qt::white;

    m_left_top_item->setPos({lt_p.x(), lt_p.y()});
    m_left_bottom_item->setPos({lt_p.x(), lt_p.y()+height_y});
    m_right_top_item->setPos({lt_p.x()+width_x, lt_p.y()});
    m_right_bottom_item->setPos({lt_p.x()+width_x, lt_p.y()+height_y});
    m_center_item->setPos({lt_p.x()+width_x/2.0, lt_p.y()+height_y/2.0});

    m_left_top_item_boundary->setX(lt_p.x());
    m_left_top_item_boundary->setY(lt_p.y());
    m_left_bottom_item_boundary->setX(lt_p.x());
    m_left_bottom_item_boundary->setY(lt_p.y()+height_y);
    m_right_top_item_boundary->setX(lt_p.x()+width_x);
    m_right_top_item_boundary->setY(lt_p.y());
    m_right_bottom_item_boundary->setX(lt_p.x()+width_x);
    m_right_bottom_item_boundary->setY(lt_p.y()+height_y);
    m_center_item_boundary->setX(center.x());
    m_center_item_boundary->setY(center.y());

    m_left_line_boundary->setPoints(*m_left_top_item_boundary, *m_left_bottom_item_boundary);
    m_top_line_boundary->setPoints(*m_left_top_item_boundary, *m_right_top_item_boundary);
    m_bottom_line_boundary->setPoints(*m_left_bottom_item_boundary, *m_right_bottom_item_boundary);
    m_right_line_boundary->setPoints(*m_right_bottom_item_boundary, *m_right_top_item_boundary);

    qDebug() << "left top,     x:" << m_left_top_item->pos().x() << " y:" << m_left_top_item->pos().y();
        qDebug() << "left bottom,  x:" << m_left_bottom_item->pos().x() << " y:" << m_left_bottom_item->pos().y();
        qDebug() << "right top,    x:" << m_right_top_item->pos().x() << " y:" << m_right_top_item->pos().y();
        qDebug() << "right bottom, x:" << m_right_bottom_item->pos().x() << " y:" << m_right_bottom_item->pos().y();
        qDebug() << "--------------------------------------------------------------------------------------------";

}

QRectF RectangleGraphicsItem::boundingRect() const {
    const QPointF point(20.0, 20.0);
    const QPointF startPoint(m_left_top_item->pos());
    const QPointF endPoint(m_right_bottom_item->pos() + point);
    return {startPoint, endPoint};
}

QPointF RectangleGraphicsItem::getIntersectPos(QPointF posA, QPointF posB, QPointF posC, QPointF posD) {
    QLineF line1(posA, posB);
    QLineF line2(posC, posD);
    QPointF interPos(0,0);
    QLineF::IntersectType type = line1.intersect(line2, &interPos);
    if (type != QLineF::BoundedIntersection)
        interPos = QPointF(-10000, -10000);
    return interPos;
}

bool RectangleGraphicsItem::isPointOnLine(const QPointF& A, const QPointF& B, const QPointF& P, qreal tolerance) {
    // 计算向量AB和AP
    QPointF AB = B - A;
    QPointF AP = P - A;

    // 计算AB和AP的点积
    qreal dotProduct = AB.x() * AP.x() + AB.y() * AP.y();

    // 计算AB的模长
    qreal AB_length = std::sqrt(AB.x() * AB.x() + AB.y() * AB.y());

    // 计算AP在AB上的投影长度
    qreal projectionLength = dotProduct / AB_length;

    // 检查投影长度是否在AB的范围内
    if (projectionLength < 0 || projectionLength > AB_length) {
        return false;
    }

    // 计算点P到直线AB的垂直距离（即点到直线的距离）
    qreal perpendicularDistance = std::sqrt(AP.x() * AP.x() + AP.y() * AP.y() - projectionLength * projectionLength);

    // 检查垂直距离是否小于容差
    return perpendicularDistance < tolerance;
}

bool RectangleGraphicsItem::touchBoundaryLine(const QPointF &P) {
    bool top = getIntersectPos(*m_left_top_item_boundary, *m_right_top_item_boundary, *m_center_item_boundary, P).x() > -999;
    bool bottom = getIntersectPos(*m_left_bottom_item_boundary, *m_right_bottom_item_boundary, *m_center_item_boundary, P).x() > -999;
    bool left = getIntersectPos(*m_left_top_item_boundary, *m_left_bottom_item_boundary, *m_center_item_boundary, P).x() > -999;
    bool right = getIntersectPos(*m_right_bottom_item_boundary, *m_right_top_item_boundary, *m_center_item_boundary, P).x() > -999;

    return top || bottom || left || right;
}

void RectangleGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    qDebug() << "paint for Rec\n";
    painter->save();
    QPen pen(Qt::white); // 设置画笔颜色
    pen.setWidth(2); // 设置线条宽度为2
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);

//    const QLineF line1(m_left_top_item->boundingRect().x(), m_left_top_item->boundingRect().y(), m_right_top_item->pos().x(), m_right_top_item->pos().y());
//    painter->drawLine(line1);

//    const QLineF line2(m_left_top_item->boundingRect().x(), m_left_top_item->boundingRect().y(), m_left_bottom_item->pos().x(), m_left_bottom_item->pos().y());
//    painter->drawLine(line2);

//    const QLineF line3(m_right_top_item->boundingRect().x(), m_right_top_item->boundingRect().y(), m_right_bottom_item->pos().x(), m_right_bottom_item->pos().y());
//    painter->drawLine(line3);

//    const QLineF line4(m_left_bottom_item->boundingRect().x(), m_left_bottom_item->boundingRect().y(), m_right_bottom_item->pos().x(), m_right_bottom_item->pos().y());
//    painter->drawLine(line4);
    painter->drawRect(m_left_top_item->pos().x(), m_left_top_item->pos().y(), m_right_top_item->pos().x()-m_left_top_item->pos().x(), m_left_bottom_item->pos().y()- m_left_top_item->pos().y());

    painter->restore();

    painter->save();
    painter->setPen(QPen(Qt::black));
    painter->setBrush(Qt::yellow);

    painter->drawRect(m_left_top_item->boundingRect());
    painter->drawRect(m_left_bottom_item->boundingRect());
    painter->drawRect(m_right_top_item->boundingRect());
    painter->drawRect(m_right_bottom_item->boundingRect());

    //画边界点
    int dotSize = 10; // 点的直径
    painter->drawEllipse(*m_left_top_item_boundary, dotSize / 2, dotSize / 2);
    painter->drawEllipse(*m_left_bottom_item_boundary, dotSize / 2, dotSize / 2);
    painter->drawEllipse(*m_right_top_item_boundary, dotSize / 2, dotSize / 2);
    painter->drawEllipse(*m_right_bottom_item_boundary, dotSize / 2, dotSize / 2);
    painter->restore();

    painter->save();
    painter->setPen(QPen(Qt::black));
    painter->setBrush(Qt::red);
    painter->drawRect(m_center_item->boundingRect());
    painter->restore();

    painter->save();
    const qreal width = m_right_bottom_item->pos().x() - m_left_bottom_item->pos().x() + 10;
    const qreal height = m_right_bottom_item->pos().y() - m_right_top_item->pos().y() + 10;

    const QPointF textPos(m_left_bottom_item->pos().x(), m_left_bottom_item->y() + 20);
    painter->drawText(textPos, QStringLiteral("宽度：%1,高度：%2").arg(width).arg(height));
    painter->restore();
}

void RectangleGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    qDebug() << "pressevent\n";
    if (event->button() == Qt::LeftButton) {
        m_startPos = event->pos();

        QString s = "";
        if (m_left_top_item->boundingRect().contains(m_startPos)) {
            setCursor(Qt::PointingHandCursor);
            m_drag_type = DragType::LeftTop;
            s = "1";
        } else if (m_left_bottom_item->boundingRect().contains(m_startPos)) {
            setCursor(Qt::PointingHandCursor);
            m_drag_type = DragType::LeftBottom;
            s = "2";
        } else if (m_right_top_item->boundingRect().contains(m_startPos)) {
            setCursor(Qt::PointingHandCursor);
            m_drag_type = DragType::RightTop;
            s = "3";
        } else if (m_right_bottom_item->boundingRect().contains(m_startPos)) {
            setCursor(Qt::PointingHandCursor);
            m_drag_type = DragType::RightBottom;
            s = "4";
        } else if (m_center_item->boundingRect().contains(m_startPos)) {
                    setCursor(Qt::PointingHandCursor);
                    m_drag_type = DragType::Center;
                    s = "5";
        }
        qDebug() << "pressevent: " << s << "\n";
    }
}

void RectangleGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (touchBoundaryLine(event->pos())) {
        qDebug() << "waring no  value able\n";
        return;
    }

    const QPointF point = (event->pos() - m_startPos);
    switch (m_drag_type) {
    case DragType::LeftTop: {
        QPointF new_p_left_top(m_left_top_item->x()+point.x(), m_left_top_item->y()+point.y());
        QPointF new_p_left_bottom(m_left_bottom_item->x()+point.x(), m_left_bottom_item->y());
        QPointF new_p_right_top(m_right_top_item->x(), m_right_top_item->y()+point.y());
        if (touchBoundaryLine(new_p_left_top) || touchBoundaryLine(new_p_left_bottom) || touchBoundaryLine(new_p_right_top)) {
            return;
        }


        m_left_top_item->moveBy(point.x(), point.y());
        m_left_bottom_item->moveBy(point.x(), 0);
        m_right_top_item->moveBy(0, point.y());
        m_center_item->moveBy(point.x() / 2.0, point.y() / 2.0);
        break;
    }
    case DragType::LeftBottom: {
        QPointF new_p_left_top(m_left_top_item->x()+point.x(), m_left_top_item->y());
        QPointF new_p_left_bottom(m_left_bottom_item->x()+point.x(), m_left_bottom_item->y()+point.y());
        QPointF new_p_right_bottom(m_right_bottom_item->x(), m_right_bottom_item->y()+point.y());
        if (touchBoundaryLine(new_p_left_top) || touchBoundaryLine(new_p_left_bottom) || touchBoundaryLine(new_p_right_bottom)) {
            return;
        }

        m_left_top_item->moveBy(point.x(), 0);
        m_left_bottom_item->moveBy(point.x(), point.y());
        m_right_bottom_item->moveBy(0, point.y());
        m_center_item->moveBy(point.x()/2.0, point.y()/2.0);
        break;
    }
    case DragType::RightTop: {
        QPointF new_p_left_top(m_left_top_item->x(), m_left_top_item->y()+point.y());
        QPointF new_p_right_top(m_right_top_item->x()+point.x(), m_right_top_item->y()+point.y());
        QPointF new_p_right_bottom(m_right_bottom_item->x()+point.x(), m_right_bottom_item->y());
        if (touchBoundaryLine(new_p_left_top) || touchBoundaryLine(new_p_right_top) || touchBoundaryLine(new_p_right_bottom)) {
            return;
        }

        m_left_top_item->moveBy(0, point.y());
        m_right_top_item->moveBy(point.x(), point.y());
        m_right_bottom_item->moveBy(point.x(), 0);
        m_center_item->moveBy(point.x()/2.0, point.y()/2.0);
        break;
    }
    case DragType::RightBottom: {
        QPointF new_p_left_bottom(m_left_bottom_item->x(), m_left_bottom_item->y()+point.y());
        QPointF new_p_right_top(m_right_top_item->x()+point.x(), m_right_top_item->y());
        QPointF new_p_right_bottom(m_right_bottom_item->x()+point.x(), m_right_bottom_item->y()+point.y());
        if (touchBoundaryLine(new_p_left_bottom) || touchBoundaryLine(new_p_right_top) || touchBoundaryLine(new_p_right_bottom)) {
            return;
        }

        m_left_bottom_item->moveBy(0, point.y());
        m_right_top_item->moveBy(point.x(), 0);
        m_right_bottom_item->moveBy(point.x(), point.y());
        m_center_item->moveBy(point.x()/2.0, point.y()/2.0);
        break;
    }
    case DragType::Center: {
        QPointF new_p_left_top(m_left_top_item->x()+point.x(), m_left_top_item->y()+point.y());
        QPointF new_p_left_bottom(m_left_bottom_item->x()+point.x(), m_left_bottom_item->y()+point.y());
        QPointF new_p_right_top(m_right_top_item->x()+point.x(), m_right_top_item->y()+point.y());
        QPointF new_p_right_bottom(m_right_bottom_item->x()+point.x(), m_right_bottom_item->y()+point.y());
        if (touchBoundaryLine(new_p_left_top) || touchBoundaryLine(new_p_left_bottom) || touchBoundaryLine(new_p_right_top) || touchBoundaryLine(new_p_right_bottom)) {
            return;
        }


        m_left_top_item->moveBy(point.x(), point.y());
        m_left_bottom_item->moveBy(point.x(), point.y());
        m_right_top_item->moveBy(point.x(), point.y());
        m_right_bottom_item->moveBy(point.x(), point.y());
        m_center_item->moveBy(point.x(), point.y());
        break;
    }
    }
    qDebug() << "left top,     x:" << m_left_top_item->pos().x() << " y:" << m_left_top_item->pos().y();
        qDebug() << "left bottom,  x:" << m_left_bottom_item->pos().x() << " y:" << m_left_bottom_item->pos().y();
        qDebug() << "right top,    x:" << m_right_top_item->pos().x() << " y:" << m_right_top_item->pos().y();
        qDebug() << "right bottom, x:" << m_right_bottom_item->pos().x() << " y:" << m_right_bottom_item->pos().y();
        qDebug() << "--------------------------------------------------------------------------------------------";
    m_startPos = event->pos();
    scene()->update();
}

void RectangleGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    qDebug() << "--------------------------------------------------------------------------------------------";
    setCursor(Qt::CrossCursor);
    m_drag_type = DragType::Release;
}

void RectangleGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    m_line_color = Qt::yellow;
    qDebug() << "hoverEnterEvent";
}

void RectangleGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    m_line_color = Qt::blue;
    qDebug() << "hoverLeaveEvent";
}



QPointF RectangleGraphicsItem::rotatePoint(const QPointF& center, qreal angleRadians, const QPointF& point) {
    qreal xTranslated = point.x() - center.x();
        qreal yTranslated = point.y() - center.y();

        // 应用旋转矩阵
        qreal xRotated = xTranslated * std::cos(angleRadians) - yTranslated * std::sin(angleRadians);
        qreal yRotated = xTranslated * std::sin(angleRadians) + yTranslated * std::cos(angleRadians);

        // 将旋转后的点平移回原来的位置
        qreal xNew = xRotated + center.x();
        qreal yNew = yRotated + center.y();

        return QPointF(xNew, yNew);
}

void RectangleGraphicsItem::transformBoundaryRect(QTransform* transform) {
    qDebug() << "transformBoundaryRect: center:" << m_center_item_boundary << "\n";
//    QPointF new_p_left_top = rotatePoint(*m_center_item_boundary, angle, *m_left_top_item_boundary);
//    QPointF new_p_left_bottom = rotatePoint(*m_center_item_boundary, angle, *m_left_bottom_item_boundary);
//    QPointF new_p_right_top = rotatePoint(*m_center_item_boundary, angle, *m_right_top_item_boundary);
//    QPointF new_p_right_bottom = rotatePoint(*m_center_item_boundary, angle, *m_right_bottom_item_boundary);
    QPointF new_p_left_top = transform->map(*m_left_top_item_boundary);
    QPointF new_p_left_bottom = transform->map(*m_left_bottom_item_boundary);
    QPointF new_p_right_top = transform->map(*m_right_top_item_boundary);
    QPointF new_p_right_bottom = transform->map(*m_right_bottom_item_boundary);
    m_left_top_item_boundary->setX(new_p_left_top.x());
    m_left_top_item_boundary->setY(new_p_left_top.y());
    m_left_bottom_item_boundary->setX(new_p_left_bottom.x());
    m_left_bottom_item_boundary->setY(new_p_left_bottom.y());
    m_right_top_item_boundary->setX(new_p_right_top.x());
    m_right_top_item_boundary->setY(new_p_right_top.y());
    m_right_bottom_item_boundary->setX(new_p_right_bottom.x());
    m_right_bottom_item_boundary->setY(new_p_right_bottom.y());

    //调整每个点以适应边界
    QVector<QSharedPointer<PointGraphicsItem>> m({m_left_top_item, m_left_bottom_item, m_right_top_item, m_right_bottom_item});
    for (auto& p: m) {
        QPointF top = getIntersectPos(*m_left_top_item_boundary, *m_right_top_item_boundary, *m_center_item_boundary, QPointF(p->x(), p->y()));
        QPointF bottom = getIntersectPos(*m_left_bottom_item_boundary, *m_right_bottom_item_boundary, *m_center_item_boundary, QPointF(p->x(), p->y()));
        QPointF left = getIntersectPos(*m_left_top_item_boundary, *m_left_bottom_item_boundary, *m_center_item_boundary, QPointF(p->x(), p->y()));
        QPointF right = getIntersectPos(*m_right_bottom_item_boundary, *m_right_top_item_boundary, *m_center_item_boundary, QPointF(p->x(), p->y()));

        if (top.x() > -999) {
            p->setX(top.x());
            p->setY(top.y());
        } else if (bottom.x() > -999) {
            p->setX(bottom.x());
            p->setY(bottom.y());
        } else if (left.x() > -999) {
            p->setX(left.x());
            p->setY(left.y());
        } else if (right.x() > -999) {
            p->setX(right.x());
            p->setY(right.y());
        }
    }
    scene()->update();
}

