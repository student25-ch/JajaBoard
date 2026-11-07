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
#include "canvas.h"
#include <QFile>
#include <QDataStream>
#include <QPainter>
#include <QEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QProcess>
#include <QTemporaryDir>
#include <QCursor>
#include <cmath>
#include <QScrollBar>

// ============ 修复: 手写克隆函数 ============
static QGraphicsItem* cloneItem(QGraphicsItem *it) {
    QGraphicsItem *copy = nullptr;
    if (auto *p = qgraphicsitem_cast<QGraphicsPathItem*>(it)) {
        copy = new QGraphicsPathItem(p->path());
        static_cast<QGraphicsPathItem*>(copy)->setPen(p->pen());
    } else if (auto *l = qgraphicsitem_cast<QGraphicsLineItem*>(it)) {
        copy = new QGraphicsLineItem(l->line());
        static_cast<QGraphicsLineItem*>(copy)->setPen(l->pen());
    } else if (auto *r = qgraphicsitem_cast<QGraphicsRectItem*>(it)) {
        copy = new QGraphicsRectItem(r->rect());
        static_cast<QGraphicsRectItem*>(copy)->setPen(r->pen());
    } else if (auto *e = qgraphicsitem_cast<QGraphicsEllipseItem*>(it)) {
        copy = new QGraphicsEllipseItem(e->rect());
        static_cast<QGraphicsEllipseItem*>(copy)->setPen(e->pen());
    } else if (auto *pm = qgraphicsitem_cast<QGraphicsPixmapItem*>(it)) {
        copy = new QGraphicsPixmapItem(pm->pixmap());
    }
    if (copy) {
        copy->setPos(it->pos());
        copy->setTransform(it->transform());
    }
    return copy;
}

Canvas::Canvas(QWidget *parent)
    : QGraphicsView(parent), mode(PEN), currentItem(nullptr),
      mouseDown(false), undoPtr(0), rulerActive(false),
      protractorActive(false), laserActive(false), projReadOnly(false),
      zoomFactor(1.0), spacePressed(false), panning(false),
      draggingItem(nullptr)
{
    setScene(new QGraphicsScene(this));
    drawGroup = new QGraphicsItemGroup();
    bgItem = new QGraphicsPixmapItem();
    scene()->addItem(bgItem);
    scene()->addItem(drawGroup);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setBackgroundBrush(Qt::white);
    setAcceptDrops(true);
    pen.setColor(Qt::black);
    pen.setWidth(4);
    scene()->setSceneRect(0, 0, 1280, 720);

    laserTimer = new QTimer(this);
    laserTimer->setInterval(30);
    connect(laserTimer, &QTimer::timeout, this, [this]{
        if (laserPoints.size() > 0) {
            if (laserPoints.size() > 15) laserPoints.removeFirst();
            viewport()->update();
        }
    });
}

void Canvas::setTool(ToolMode m){
    mode = m;
    rulerActive = (m == RULER);
    protractorActive = (m == PROTRACTOR);
    laserActive = (m == LASER);
    if (m != SELECT) {
        for (auto *it : selectedItems) it->setSelected(false);
        selectedItems.clear();
    }
    viewport()->update();
}
void Canvas::setPenColor(QColor c){ pen.setColor(c); }
void Canvas::setPenWidth(int w){ pen.setWidth(w); }
void Canvas::setProjectionMode(bool ro){ projReadOnly = ro; }

void Canvas::clearDrawItems(){
    const auto items = drawGroup->childItems();
    for (auto *it : items) { drawGroup->removeFromGroup(it); delete it; }
    selectedItems.clear();
}

void Canvas::clearDrawLayer(){
    clearDrawItems();
    undoStack.clear();
    undoPtr = 0;
    emit pageChanged();
}

void Canvas::setBackgroundPix(QPixmap px){
    bgItem->setPixmap(px);
    if (px.isNull()) scene()->setSceneRect(0, 0, 1280, 720);
    else scene()->setSceneRect(px.rect());
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    zoomFactor = 1.0;
    emit pageChanged();
}

QPixmap Canvas::backgroundPix() const { return bgItem->pixmap(); }

void Canvas::snapshotToUndo(){
    QList<QGraphicsItem*> snap;
    for (auto *it : drawGroup->childItems()) {
        // 修复: 用 cloneItem 替换 it->clone()
        QGraphicsItem *copy = cloneItem(it);
        if (copy) snap << copy;
    }
    while (undoStack.size() > undoPtr) {
        qDeleteAll(undoStack.last());
        undoStack.removeLast();
    }
    undoStack << snap;
    undoPtr = undoStack.size();
    emit pageChanged();
}

void Canvas::undo(){
    if (undoPtr <= 0) return;
    undoPtr--;
    clearDrawItems();
    if (undoPtr > 0)
        for (auto *it : undoStack[undoPtr - 1]) {
            QGraphicsItem *copy = cloneItem(it);
            if (copy) drawGroup->addToGroup(copy);
        }
    emit pageChanged();
}

void Canvas::redo(){
    if (undoPtr >= undoStack.size()) return;
    undoPtr++;
    clearDrawItems();
    for (auto *it : undoStack[undoPtr - 1]) {
        QGraphicsItem *copy = cloneItem(it);
        if (copy) drawGroup->addToGroup(copy);
    }
    emit pageChanged();
}

static void writeItem(QDataStream &ds, QGraphicsItem *it) {
    int t = it->type();
    ds << t << it->pos() << it->transform();
    if (auto *p = qgraphicsitem_cast<QGraphicsPathItem*>(it)) ds << p->path() << p->pen();
    else if (auto *l = qgraphicsitem_cast<QGraphicsLineItem*>(it)) ds << l->line() << l->pen();
    else if (auto *r = qgraphicsitem_cast<QGraphicsRectItem*>(it)) ds << r->rect() << r->pen();
    else if (auto *e = qgraphicsitem_cast<QGraphicsEllipseItem*>(it)) ds << e->rect() << e->pen();
    else if (auto *p = qgraphicsitem_cast<QGraphicsPixmapItem*>(it)) ds << p->pixmap();
}

static QGraphicsItem* readItem(QDataStream &ds) {
    int t; QPointF pos; QTransform tr;
    ds >> t >> pos >> tr;
    QGraphicsItem *it = nullptr;
    if (t == QGraphicsPathItem::Type) {
        QPainterPath pp; QPen pn; ds >> pp >> pn;
        auto *p = new QGraphicsPathItem(pp); p->setPen(pn); it = p;
    } else if (t == QGraphicsLineItem::Type) {
        QLineF ln; QPen pn; ds >> ln >> pn;
        auto *l = new QGraphicsLineItem(ln); l->setPen(pn); it = l;
    } else if (t == QGraphicsRectItem::Type) {
        QRectF r; QPen pn; ds >> r >> pn;
        auto *rr = new QGraphicsRectItem(r); rr->setPen(pn); it = rr;
    } else if (t == QGraphicsEllipseItem::Type) {
        QRectF r; QPen pn; ds >> r >> pn;
        auto *e = new QGraphicsEllipseItem(r); e->setPen(pn); it = e;
    } else if (t == QGraphicsPixmapItem::Type) {
        QPixmap px; ds >> px;
        auto *pm = new QGraphicsPixmapItem(px); it = pm;
    }
    if (it) { it->setPos(pos); it->setTransform(tr); }
    return it;
}

QByteArray Canvas::serializePage() const {
    QByteArray out;
    QDataStream ds(&out, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_6_0);
    ds << bgItem->pixmap();
    ds << int(undoStack.size()) << undoPtr;
    for (const auto &snap : undoStack) {
        ds << int(snap.size());
        for (auto *it : snap) writeItem(ds, it);
    }
    return out;
}

void Canvas::deserializePage(const QByteArray &data) {
    QDataStream ds(data);
    ds.setVersion(QDataStream::Qt_6_0);
    QPixmap bg; ds >> bg;
    bgItem->setPixmap(bg);
    scene()->setSceneRect(bg.isNull() ? QRectF(0,0,1280,720) : bg.rect());
    clearDrawItems();
    for (auto &s : undoStack) qDeleteAll(s);
    undoStack.clear();
    int gcount = 0; ds >> gcount >> undoPtr;
    for (int g = 0; g < gcount; ++g) {
        int n = 0; ds >> n;
        QList<QGraphicsItem*> snap;
        for (int i = 0; i < n; ++i) { auto *it = readItem(ds); if (it) snap << it; }
        undoStack << snap;
    }
    if (undoPtr > 0)
        for (auto *it : undoStack[undoPtr - 1]) {
            QGraphicsItem *copy = cloneItem(it);
            if (copy) drawGroup->addToGroup(copy);
        }
    emit pageChanged();
}

// 修复: 用 QPainter 渲染代替 toImage
QImage Canvas::grabCanvasImage(){
    QRectF r = scene()->sceneRect();
    if (r.isEmpty()) return QImage();
    QImage img(r.size().toSize(), QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    scene()->render(&p, QRectF(img.rect()), r);
    return img;
}

void Canvas::renderPageTo(QPainter *painter, const QRectF &target){
    scene()->render(painter, target, scene()->sceneRect());
}

void Canvas::insertImageAt(const QPixmap &pix, const QPointF &scenePos){
    if (pix.isNull()) return;
    QPixmap p = pix;
    if (p.width() > 600 || p.height() > 600)
        p = p.scaled(600, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    auto *item = new QGraphicsPixmapItem(p);
    item->setPos(scenePos - QPointF(p.width()/2.0, p.height()/2.0));
    item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    drawGroup->addToGroup(item);
    snapshotToUndo();
    emit pageChanged();
}

QString Canvas::recognizeHandwriting(const QList<QGraphicsPathItem*> &paths) const {
    return QString();
}

void Canvas::mousePressEvent(QMouseEvent *e){
    if (projReadOnly) { QGraphicsView::mousePressEvent(e); return; }
    if ((spacePressed && e->button() == Qt::LeftButton) || e->button() == Qt::MiddleButton){
        panning = true;
        panStart = e->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    if (e->button() != Qt::LeftButton) { QGraphicsView::mousePressEvent(e); return; }
    mouseDown = true;
    startPt = mapToScene(e->pos());
    mouseNow = startPt;

    if (mode == SELECT) {
        auto hit = scene()->items(mapToScene(e->pos()));
        QGraphicsItem* found = nullptr;
        for (auto *it : hit) {
            if (drawGroup->childItems().contains(it)) { found = it; break; }
        }
        for (auto *it : selectedItems) it->setSelected(false);
        selectedItems.clear();
        if (found) {
            found->setSelected(true);
            selectedItems << found;
            draggingItem = found;
            dragOffset = found->pos() - mapToScene(e->pos());
            setCursor(Qt::SizeAllCursor);
        } else { draggingItem = nullptr; }
        viewport()->update();
        return;
    }

    if (mode == LASER) {
        laserPoints.clear(); laserPoints << startPt;
        if (!laserTimer->isActive()) laserTimer->start();
        return;
    }

    if (mode == PEN || mode == HIGHLIGHT){
        auto *pi = new QGraphicsPathItem();
        QPen p = pen;
        if (mode == HIGHLIGHT) p.setCapStyle(Qt::RoundCap);
        pi->setPen(p);
        QPainterPath path; path.moveTo(startPt);
        pi->setPath(path);
        currentItem = pi;
        drawGroup->addToGroup(pi);
    } else if (mode == LINE){
        auto *li = new QGraphicsLineItem(QLineF(startPt, startPt));
        li->setPen(pen); currentItem = li; drawGroup->addToGroup(li);
    } else if (mode == RECT){
        auto *ri = new QGraphicsRectItem(QRectF(startPt, startPt));
        ri->setPen(pen); currentItem = ri; drawGroup->addToGroup(ri);
    } else if (mode == CIRCLE){
        auto *ei = new QGraphicsEllipseItem(QRectF(startPt, startPt));
        ei->setPen(pen); currentItem = ei; drawGroup->addToGroup(ei);
    } else if (mode == ERASER){
        auto hit = scene()->items(mapToScene(e->pos()));
        for (auto *i : hit) {
            if (drawGroup->childItems().contains(i)) {
                drawGroup->removeFromGroup(i); delete i;
            }
        }
        snapshotToUndo();
    }
    viewport()->update();
}

void Canvas::mouseMoveEvent(QMouseEvent *e){
    if (panning) {
        QPoint delta = e->pos() - panStart;
        panStart = e->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        return;
    }
    mouseNow = mapToScene(e->pos());

    if (mode == SELECT && draggingItem && (e->buttons() & Qt::LeftButton)) {
        draggingItem->setPos(mapToScene(e->pos()) + dragOffset);
        viewport()->update(); return;
    }
    if (mode == LASER && mouseDown) {
        laserPoints << mouseNow;
        if (laserPoints.size() > 60) laserPoints.removeFirst();
        if (!laserTimer->isActive()) laserTimer->start();
        viewport()->update(); return;
    }
    if (!mouseDown || !currentItem) { viewport()->update(); return; }
    if (mode == PEN || mode == HIGHLIGHT){
        auto *pi = static_cast<QGraphicsPathItem*>(currentItem);
        QPainterPath p = pi->path(); p.lineTo(mouseNow); pi->setPath(p);
    } else if (mode == LINE){
        static_cast<QGraphicsLineItem*>(currentItem)->setLine(QLineF(startPt, mouseNow));
    } else if (mode == RECT){
        static_cast<QGraphicsRectItem*>(currentItem)->setRect(QRectF(startPt, mouseNow).normalized());
    } else if (mode == CIRCLE){
        static_cast<QGraphicsEllipseItem*>(currentItem)->setRect(QRectF(startPt, mouseNow).normalized());
    }
    viewport()->update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *e){
    if (panning) { panning = false; setCursor(spacePressed ? Qt::OpenHandCursor : Qt::ArrowCursor); return; }
    if (mode == SELECT && draggingItem) {
        draggingItem = nullptr; setCursor(Qt::ArrowCursor);
        snapshotToUndo(); emit pageChanged(); return;
    }
    if (currentItem){ currentItem = nullptr; snapshotToUndo(); }
    mouseDown = false;
    viewport()->update();
}

void Canvas::wheelEvent(QWheelEvent *e){
    if (e->modifiers() & Qt::ControlModifier) {
        double factor = e->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
        zoomFactor *= factor;
        if (zoomFactor < 0.1) zoomFactor = 0.1;
        if (zoomFactor > 10.0) zoomFactor = 10.0;
        scale(factor, factor);
        e->accept(); return;
    }
    QGraphicsView::wheelEvent(e);
}

void Canvas::keyPressEvent(QKeyEvent *e){
    if (e->key() == Qt::Key_Space && !e->isAutoRepeat()) {
        spacePressed = true; setCursor(Qt::OpenHandCursor); return;
    }
    if (e->key() == Qt::Key_Delete && mode == SELECT) {
        for (auto *it : selectedItems) { drawGroup->removeFromGroup(it); delete it; }
        selectedItems.clear(); snapshotToUndo(); emit pageChanged(); return;
    }
    QGraphicsView::keyPressEvent(e);
}
void Canvas::keyReleaseEvent(QKeyEvent *e){
    if (e->key() == Qt::Key_Space && !e->isAutoRepeat()) {
        spacePressed = false; if (!panning) setCursor(Qt::ArrowCursor); return;
    }
    QGraphicsView::keyReleaseEvent(e);
}
void Canvas::leaveEvent(QEvent *e){ mouseDown = false; viewport()->update(); QGraphicsView::leaveEvent(e); }
void Canvas::dragEnterEvent(QDragEnterEvent *e){ if (e->mimeData()->hasImage() || e->mimeData()->hasUrls()) e->acceptProposedAction(); }
void Canvas::dropEvent(QDropEvent *e){
    if (e->mimeData()->hasImage()) {
        QImage img = qvariant_cast<QImage>(e->mimeData()->imageData());
        insertImageAt(QPixmap::fromImage(img), mapToScene(e->position().toPoint())); e->acceptProposedAction();
    } else if (e->mimeData()->hasUrls()) {
        for (const QUrl &u : e->mimeData()->urls()) {
            if (u.isLocalFile()) {
                QPixmap px(u.toLocalFile());
                if (!px.isNull()) { insertImageAt(px, mapToScene(e->position().toPoint())); e->acceptProposedAction(); return; }
            }
        }
    }
}
void Canvas::paintEvent(QPaintEvent *event){
    QGraphicsView::paintEvent(event);
    QPainter painter(viewport()); painter.setRenderHint(QPainter::Antialiasing);
    if (mode == LASER && !laserPoints.isEmpty()) {
        for (int i = 0; i < laserPoints.size(); ++i) {
            QPointF vp = mapFromScene(laserPoints[i]);
            double alpha = double(i + 1) / laserPoints.size();
            QColor c(255, 30, 30, int(255 * alpha));
            painter.setPen(Qt::NoPen); painter.setBrush(c); painter.drawEllipse(vp, 4+4*alpha, 4+4*alpha);
        }
    }
    if (!mouseDown) return;
    QPoint vpStart = mapFromScene(startPt); QPoint vpNow = mapFromScene(mouseNow);
    if (rulerActive){
        painter.setPen(QPen(Qt::darkBlue, 2)); painter.drawLine(vpStart, vpNow);
        painter.drawText(vpNow.x()+8, vpNow.y(), QString("%1 px").arg(QLineF(vpStart, vpNow).length()));
    }
    if (protractorActive){
        painter.setPen(QPen(Qt::darkRed, 2)); painter.drawEllipse(vpStart, 120, 120);
        qreal deg = QLineF(0,0,120,0).angleTo(QLineF(vpStart, vpNow));
        painter.drawLine(vpStart, vpNow); painter.drawText(vpStart.x()+130, vpStart.y(), QString("%1°").arg(deg,0,'f',1));
    }
}