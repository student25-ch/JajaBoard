/*
 * JajaBoard - 跨平台开源教学白板
 * Copyright (C) 2026 student25-ch
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CANVAS_H
#define CANVAS_H
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainterPath>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QImage>
#include <QTimer>

enum ToolMode { PEN, HIGHLIGHT, ERASER, LINE, RECT, CIRCLE,
                RULER, PROTRACTOR, LASER, SELECT };

class Canvas : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);
    void setTool(ToolMode m);
    void setPenColor(QColor c);
    void setPenWidth(int w);
    ToolMode tool() const { return mode; }
    void clearDrawLayer();
    void setBackgroundPix(QPixmap px);
    QPixmap backgroundPix() const;
    void undo();
    void redo();
    QByteArray serializePage() const;
    void deserializePage(const QByteArray &data);
    QImage grabCanvasImage();
    QGraphicsScene* sharedScene() const { return scene(); }
    void setProjectionMode(bool readOnly);
    void renderPageTo(QPainter *painter, const QRectF &target);
    void insertImageAt(const QPixmap &pix, const QPointF &scenePos);
    QString recognizeHandwriting(const QList<QGraphicsPathItem*> &paths) const;
signals:
    void pageChanged();
    void strokeCommitted();
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *event) override;
    void leaveEvent(QEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;
private:
    void snapshotToUndo();
    void clearDrawItems();
    ToolMode mode;
    QPen pen;
    QGraphicsItem* currentItem;
    QPointF startPt;
    QPointF mouseNow;
    bool mouseDown;
    QGraphicsItemGroup* drawGroup;
    QGraphicsPixmapItem* bgItem;
    QList<QList<QGraphicsItem*>> undoStack;
    int undoPtr;
    bool rulerActive, protractorActive, laserActive;
    bool projReadOnly;
    double zoomFactor;
    bool spacePressed;
    bool panning;
    QPoint panStart;
    QList<QGraphicsItem*> selectedItems;
    QGraphicsItem* draggingItem;
    QPointF dragOffset;
    QTimer* laserTimer;
    QList<QPointF> laserPoints;
};
#endif