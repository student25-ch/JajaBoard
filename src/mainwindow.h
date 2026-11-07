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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QListWidget>
#include <QPointer>
#include <QGraphicsView>
#include "canvas.h"
#include "pptimporter.h"
#include "screencap.h"
#include "projectfile.h"
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private slots:
    void slotPen(); void slotHigh(); void slotEraser();
    void slotLine(); void slotRect(); void slotCircle();
    void slotRuler(); void slotProtractor(); void slotLaser();
    void slotSelect(); void slotColor();
    void slotClear(); void slotUndo(); void slotRedo();
    void slotImportPpt(); void slotNewPage();
    void slotPageSelect(QListWidgetItem*);
    void slotSaveJboard(); void slotLoadJboard(); void slotExportPdf();
    void slotDualScreen(); void slotStartRec(); void slotStopRec();
    void slotInsertImage(); void slotOcr();
    void onPageChanged();
private:
    void refreshPages();
    void setPageIndex(int idx);
    Canvas* canvas;
    QListWidget* pageList;
    PptImporter* pptImporter;
    ScreenRecorder* recorder;
    Project project;
    int currentPage = 0;
    QPointer<QWidget> projWin;
    QPointer<QGraphicsView> projView;
    bool suppressPageListChange = false;
};
#endif
