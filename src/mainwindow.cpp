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
#include "mainwindow.h"
#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QPainter>
#include <QColorDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QTextEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGraphicsView>
#include "ocrengine.h"

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent) {
    setWindowTitle("JajaBoard v2.0 | 教学白板");
    QWidget* cw = new QWidget(); 
    setCentralWidget(cw);
    QHBoxLayout* lay = new QHBoxLayout(cw);
    QVBoxLayout* left = new QVBoxLayout(); 
    QVBoxLayout* right = new QVBoxLayout();
    lay->addLayout(left, 1); 
    lay->addLayout(right, 8);
    
    pageList = new QListWidget(); 
    left->addWidget(pageList);
    connect(pageList, &QListWidget::itemClicked, this, &MainWindow::slotPageSelect);
    
    recorder = new ScreenRecorder(this);
    
    auto mk = [](const QString& t){ return new QPushButton(t); };
    QPushButton* btnPen = mk("画笔"); QPushButton* btnHigh = mk("荧光笔");
    QPushButton* btnErase = mk("橡皮擦"); QPushButton* btnLine = mk("直线");
    QPushButton* btnRect = mk("矩形"); QPushButton* btnCircle = mk("圆形");
    QPushButton* btnRuler = mk("直尺"); QPushButton* btnProt = mk("量角器");
    QPushButton* btnLaser = mk("激光笔"); QPushButton* btnSelect = mk("选择");
    QPushButton* btnColor = mk("颜色…"); QPushButton* btnClear = mk("清空笔迹");
    QPushButton* btnUndo = mk("撤销"); QPushButton* btnRedo = mk("重做");
    QPushButton* btnImport = mk("导入PPT"); QPushButton* btnNew = mk("新建空白页");
    QPushButton* btnDel = mk("删除当前页"); QPushButton* btnSave = mk("保存 .jboard");
    QPushButton* btnLoad = mk("打开 .jboard"); QPushButton* btnPdf = mk("导出PDF");
    QPushButton* btnDual = mk("双屏模式"); QPushButton* btnRecStart = mk("开始录屏");
    QPushButton* btnRecStop = mk("停止录屏"); QPushButton* btnImg = mk("插入图片");
    QPushButton* btnOcr = mk("手写识别");

    // ============ 图标加载 ============
    setIconSize(QSize(32, 32)); // 统一图标大小
    
    btnPen->setIcon(QIcon(":/assets/icons/pen.png"));
    btnErase->setIcon(QIcon(":/assets/icons/eraser.png"));
    btnLine->setIcon(QIcon(":/assets/icons/line.png"));
    btnRect->setIcon(QIcon(":/assets/icons/rect.png"));
    btnCircle->setIcon(QIcon(":/assets/icons/circle.png"));
    btnUndo->setIcon(QIcon(":/assets/icons/undo.png"));
    btnRedo->setIcon(QIcon(":/assets/icons/redo.png"));
    btnColor->setIcon(QIcon(":/assets/icons/color.png"));
    btnClear->setIcon(QIcon(":/assets/icons/clear.png"));
    btnSave->setIcon(QIcon(":/assets/icons/save.png"));
    btnLoad->setIcon(QIcon(":/assets/icons/load.png"));
    btnImg->setIcon(QIcon(":/assets/icons/image.png"));

    left->addWidget(btnPen); left->addWidget(btnHigh); left->addWidget(btnErase);
    left->addWidget(btnLine); left->addWidget(btnRect); left->addWidget(btnCircle);
    left->addWidget(btnRuler); left->addWidget(btnProt); left->addWidget(btnLaser);
    left->addWidget(btnSelect); left->addWidget(btnColor);
    left->addWidget(btnClear); left->addWidget(btnUndo); left->addWidget(btnRedo);
    left->addWidget(btnImg); left->addWidget(btnOcr);
    left->addWidget(btnImport); left->addWidget(btnNew); left->addWidget(btnDel);
    left->addWidget(btnSave); left->addWidget(btnLoad); left->addWidget(btnPdf);
    left->addWidget(btnDual);
    left->addWidget(btnRecStart); left->addWidget(btnRecStop);

    canvas = new Canvas(); 
    right->addWidget(canvas);
    pptImporter = new PptImporter(this);

    connect(btnPen, &QPushButton::clicked, this, &MainWindow::slotPen);
    connect(btnHigh, &QPushButton::clicked, this, &MainWindow::slotHigh);
    connect(btnErase, &QPushButton::clicked, this, &MainWindow::slotEraser);
    connect(btnLine, &QPushButton::clicked, this, &MainWindow::slotLine);
    connect(btnRect, &QPushButton::clicked, this, &MainWindow::slotRect);
    connect(btnCircle, &QPushButton::clicked, this, &MainWindow::slotCircle);
    connect(btnRuler, &QPushButton::clicked, this, &MainWindow::slotRuler);
    connect(btnProt, &QPushButton::clicked, this, &MainWindow::slotProtractor);
    connect(btnLaser, &QPushButton::clicked, this, &MainWindow::slotLaser);
    connect(btnSelect, &QPushButton::clicked, this, &MainWindow::slotSelect);
    connect(btnColor, &QPushButton::clicked, this, &MainWindow::slotColor);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::slotClear);
    connect(btnUndo, &QPushButton::clicked, this, &MainWindow::slotUndo);
    connect(btnRedo, &QPushButton::clicked, this, &MainWindow::slotRedo);
    connect(btnImport, &QPushButton::clicked, this, &MainWindow::slotImportPpt);
    connect(btnNew, &QPushButton::clicked, this, &MainWindow::slotNewPage);
    
    connect(btnDel, &QPushButton::clicked, this, [this]{
        if (project.pages.size() <= 1) return;
        project.pages.removeAt(currentPage);
        if (currentPage >= project.pages.size()) currentPage = project.pages.size() - 1;
        refreshPages(); setPageIndex(currentPage);
    });
    
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::slotSaveJboard);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::slotLoadJboard);
    connect(btnPdf, &QPushButton::clicked, this, &MainWindow::slotExportPdf);
    connect(btnDual, &QPushButton::clicked, this, &MainWindow::slotDualScreen);
    connect(btnRecStart, &QPushButton::clicked, this, &MainWindow::slotStartRec);
    connect(btnRecStop, &QPushButton::clicked, this, &MainWindow::slotStopRec);
    connect(btnImg, &QPushButton::clicked, this, &MainWindow::slotInsertImage);
    connect(btnOcr, &QPushButton::clicked, this, &MainWindow::slotOcr);
    connect(canvas, &Canvas::pageChanged, this, &MainWindow::onPageChanged);
    
    project.pages << PageData{QString(), QByteArray()};
    refreshPages(); setPageIndex(0);
}

MainWindow::~MainWindow(){ if (recorder && recorder->isRecording()) recorder->stopRecord(); }

void MainWindow::refreshPages(){
    suppressPageListChange = true; pageList->clear();
    for (int i = 0; i < project.pages.size(); ++i) {
        const auto &pd = project.pages[i];
        QString name = pd.bgPath.isEmpty() ? QString("空白页 %1").arg(i+1) : QFileInfo(pd.bgPath).fileName();
        pageList->addItem(QString("[%1] %2").arg(i+1).arg(name));
    }
    if (currentPage >= 0 && currentPage < pageList->count()) pageList->setCurrentRow(currentPage);
    suppressPageListChange = false;
}

void MainWindow::setPageIndex(int idx){
    if (idx < 0 || idx >= project.pages.size()) return;
    if (currentPage >= 0 && currentPage < project.pages.size()) project.pages[currentPage].canvasState = canvas->serializePage();
    currentPage = idx; project.currentPage = idx; PageData pd = project.pages[idx];
    if (pd.bgPath.isEmpty()) canvas->setBackgroundPix(QPixmap()); else canvas->setBackgroundPix(QPixmap(pd.bgPath));
    if (pd.canvasState.isEmpty()) canvas->clearDrawLayer(); else canvas->deserializePage(pd.canvasState);
    suppressPageListChange = true; pageList->setCurrentRow(idx); suppressPageListChange = false;
}

void MainWindow::onPageChanged(){
    if (currentPage >= 0 && currentPage < project.pages.size()) project.pages[currentPage].canvasState = canvas->serializePage();
    if (projView) projView->viewport()->update();
}

void MainWindow::slotPageSelect(QListWidgetItem* it){
    if (suppressPageListChange) return; int idx = pageList->row(it); if (idx == currentPage) return; setPageIndex(idx);
}

void MainWindow::slotNewPage(){ project.pages << PageData{QString(), QByteArray()}; refreshPages(); setPageIndex(project.pages.size() - 1); }
void MainWindow::slotPen(){canvas->setTool(PEN);canvas->setPenColor(Qt::black);canvas->setPenWidth(4);}
void MainWindow::slotHigh(){canvas->setTool(HIGHLIGHT);canvas->setPenColor(QColor(255,255,0,80));canvas->setPenWidth(14);}
void MainWindow::slotEraser(){canvas->setTool(ERASER);}
void MainWindow::slotLine(){canvas->setTool(LINE);canvas->setPenColor(Qt::black);canvas->setPenWidth(3);}
void MainWindow::slotRect(){canvas->setTool(RECT);canvas->setPenColor(Qt::black);canvas->setPenWidth(3);}
void MainWindow::slotCircle(){canvas->setTool(CIRCLE);canvas->setPenColor(Qt::black);canvas->setPenWidth(3);}
void MainWindow::slotRuler(){canvas->setTool(RULER);}
void MainWindow::slotProtractor(){canvas->setTool(PROTRACTOR);}
void MainWindow::slotLaser(){canvas->setTool(LASER);}
void MainWindow::slotSelect(){canvas->setTool(SELECT);}
void MainWindow::slotClear(){canvas->clearDrawLayer(); onPageChanged();}
void MainWindow::slotUndo(){canvas->undo(); onPageChanged();}
void MainWindow::slotRedo(){canvas->redo(); onPageChanged();}

void MainWindow::slotColor(){ QColor c = QColorDialog::getColor(Qt::black, this, "选择颜色", QColorDialog::ShowAlphaChannel); if (c.isValid()) canvas->setPenColor(c); }

void MainWindow::slotInsertImage(){
    QString f = QFileDialog::getOpenFileName(this, "选择图片", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (f.isEmpty()) return; QPixmap px(f);
    if (px.isNull()) { QMessageBox::warning(this, "失败", "无法读取图片"); return; }
    // 这里修复了之前的 scene() 报错
    canvas->insertImageAt(px, canvas->scene()->sceneRect().center()); onPageChanged();
}

void MainWindow::slotOcr(){
    OcrEngine ocr; if (!ocr.available()) { QMessageBox::information(this, "OCR", "未检测到 tesseract。"); return; }
    QImage img = canvas->grabCanvasImage(); if (img.isNull()) { QMessageBox::information(this, "OCR", "画布为空"); return; }
    QString text = ocr.recognize(img);
    QDialog dlg(this); dlg.setWindowTitle("OCR 识别结果"); QVBoxLayout* l = new QVBoxLayout(&dlg);
    QTextEdit* te = new QTextEdit(&dlg); te->setPlainText(text.isEmpty() ? "（未识别到文本）" : text); l->addWidget(te);
    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg); l->addWidget(bb);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept); dlg.resize(600, 400); dlg.exec();
}

void MainWindow::slotImportPpt(){
    QString f = QFileDialog::getOpenFileName(this, "选择PPT", "", "PowerPoint (*.ppt *.pptx)"); if (f.isEmpty()) return;
    QString out = QFileInfo(f).absolutePath() + "/jajaboard_slides"; QStringList sl;
    if (!pptImporter->importPpt(f, out, sl)){ QMessageBox::warning(this, "导入失败", "LibreOffice / pdftoppm 转换失败。"); return; }
    for (int i = 0; i < sl.size(); ++i){ PageData pd; pd.bgPath = sl[i]; project.pages << pd; project.slideSourcePaths << sl[i]; }
    refreshPages(); setPageIndex(project.pages.size() - sl.size());
}

void MainWindow::slotSaveJboard(){
    if (currentPage >= 0 && currentPage < project.pages.size()) project.pages[currentPage].canvasState = canvas->serializePage();
    QString f = QFileDialog::getSaveFileName(this, "保存课件", "untitled.jboard", "JajaBoard (*.jboard)"); if (f.isEmpty()) return;
    project.currentPage = currentPage;
    if (!ProjectFile::save(f, project)) QMessageBox::warning(this, "保存失败", "无法写入文件"); else QMessageBox::information(this, "已保存", f);
}

void MainWindow::slotLoadJboard(){
    QString f = QFileDialog::getOpenFileName(this, "打开课件", "", "JajaBoard (*.jboard)"); if (f.isEmpty()) return;
    Project p; if (!ProjectFile::load(f, p)){ QMessageBox::warning(this, "打开失败", "文件格式不正确"); return; }
    project = p; currentPage = project.currentPage; refreshPages(); setPageIndex(currentPage);
}

void MainWindow::slotExportPdf(){
    if (currentPage >= 0 && currentPage < project.pages.size()) project.pages[currentPage].canvasState = canvas->serializePage();
    QString f = QFileDialog::getSaveFileName(this, "导出PDF", "board.pdf", "PDF (*.pdf)"); if (f.isEmpty()) return;
    QPrinter prt(QPrinter::HighResolution); prt.setOutputFormat(QPrinter::PdfFormat); prt.setOutputFileName(f);
    QPainter painter(&prt); bool first = true;
    int savedIdx = currentPage;
    for (int i = 0; i < project.pages.size(); ++i) {
        if (!first) prt.newPage(); first = false;
        PageData pd = project.pages[i];
        if (pd.bgPath.isEmpty()) canvas->setBackgroundPix(QPixmap()); else canvas->setBackgroundPix(QPixmap(pd.bgPath));
        if (pd.canvasState.isEmpty()) canvas->clearDrawLayer(); else canvas->deserializePage(pd.canvasState);
        canvas->renderPageTo(&painter, prt.pageRect(QPrinter::DevicePixel));
    }
    currentPage = savedIdx; setPageIndex(currentPage); QMessageBox::information(this, "导出完成", f);
}

void MainWindow::slotDualScreen(){
    if (projWin) { projWin->raise(); projWin->activateWindow(); return; }
    QWidget* w = new QWidget(nullptr, Qt::Window); w->setWindowTitle("投影窗口（同步）");
    QHBoxLayout* pl = new QHBoxLayout(w); pl->setContentsMargins(0,0,0,0);
    QGraphicsView* pv = new QGraphicsView(canvas->sharedScene()); pv->setRenderHint(QPainter::Antialiasing); pv->setInteractive(false);
    pl->addWidget(pv); w->resize(1280, 720); w->showFullScreen(); projWin = w; projView = pv;
    connect(w, &QObject::destroyed, this, [this]{ projWin = nullptr; projView = nullptr; });
}

void MainWindow::slotStartRec(){
    QString f = QFileDialog::getSaveFileName(this, "保存录屏", "rec.mp4", "MP4 (*.mp4)"); if (f.isEmpty()) return;
    if (recorder->startRecord(f)) QMessageBox::information(this, "录屏", "开始录制"); else QMessageBox::warning(this, "录屏失败", "需安装 ffmpeg");
}

void MainWindow::slotStopRec(){ recorder->stopRecord(); QMessageBox::information(this, "录屏", "录制已停止"); }