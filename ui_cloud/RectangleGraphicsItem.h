#ifndef POINTGRAPHICSITEM_H
#define POINTGRAPHICSITEM_H
//base on https://www.cnblogs.com/greaton/p/16350449.html
#include <QGraphicsItem>
#include <QPointF>
#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QGraphicsScene>
#include <cmath>

class PointGraphicsItem: public QObject, public QGraphicsItem {

public:
    PointGraphicsItem(QGraphicsItem* parent = Q_NULLPTR);
    ~PointGraphicsItem() override;

    QRectF boundingRect() const override;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

private:
    QColor m_paint_color;
    const double Width = 10.0;
    const double Height = 10.0;
};



class RectangleGraphicsItem : public QObject, public QGraphicsItem
{

public:
    enum class DragType {
        Release,
        LeftTop,
        RightTop,
        LeftBottom,
        RightBottom,
        Center
    };

    RectangleGraphicsItem (QPointF lt_p, qreal width, qreal height, const QPointF& center, QGraphicsItem* boundaryItem, QGraphicsItem* parent=nullptr);
    ~RectangleGraphicsItem() override = default;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    inline QPointF getIntersectPos(QPointF posA, QPointF posB, QPointF posC, QPointF posD);//返回AB与CD交点，无交点返回（0,0）
    void transformBoundaryRect(QTransform* transform);
    QPointF rotatePoint(const QPointF& center, qreal angleRadians, const QPointF& point);
    bool isPointOnLine(const QPointF& A, const QPointF& B, const QPointF& P, qreal tolerance = 1e-3);
    bool touchBoundaryLine(const QPointF& P);
    //返回此刻的矩形框
    QRectF imgPartRect(){return QRectF(m_left_top_item->x(), m_left_top_item->y(), m_right_top_item->x()-m_left_top_item->x(), m_left_bottom_item->y()-m_left_top_item->y());};

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
private:
    QSharedPointer<PointGraphicsItem> m_left_top_item;
    QSharedPointer<PointGraphicsItem> m_left_bottom_item;
    QSharedPointer<PointGraphicsItem> m_right_top_item;
    QSharedPointer<PointGraphicsItem> m_right_bottom_item;
    QSharedPointer<PointGraphicsItem> m_center_item;
    //QRectF pos_boundartRect;
    QSharedPointer<QPointF> m_left_top_item_boundary;
    QSharedPointer<QPointF> m_left_bottom_item_boundary;
    QSharedPointer<QPointF> m_right_top_item_boundary;
    QSharedPointer<QPointF> m_right_bottom_item_boundary;
    QSharedPointer<QPointF> m_center_item_boundary;

    QSharedPointer<QLineF> m_left_line_boundary;
    QSharedPointer<QLineF> m_top_line_boundary;
    QSharedPointer<QLineF> m_bottom_line_boundary;
    QSharedPointer<QLineF> m_right_line_boundary;

    QGraphicsItem* boundaryItem;
    QPointF m_startPos;
    DragType m_drag_type;
    QColor m_line_color;

};



#endif // POINTGRAPHICSITEM_H
