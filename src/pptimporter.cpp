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


#include "pptimporter.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
PptImporter::PptImporter(QObject *p):QObject(p){}
bool PptImporter::importPpt(const QString &pptPath, const QString &outDir, QStringList &outSlides) {
    QDir d(outDir); if(!d.exists() && !d.mkpath(".")) return false;
    QString soffice = "soffice";
    QString base = QFileInfo(pptPath).baseName();
    QString pdfPath = d.absoluteFilePath(base + ".pdf");
    QProcess p; p.start(soffice, {"--headless","--convert-to","pdf","--outdir",outDir,pptPath});
    if (!p.waitForFinished(180000) || !QFile::exists(pdfPath)) return false;
    QProcess ppm; ppm.start("pdftoppm", {"-png","-r","150", pdfPath, d.absoluteFilePath(base)});
    ppm.waitForFinished(180000);
    QStringList files = d.entryList(QStringList(base + "-*.png"), QDir::Files, QDir::Name);
    for (const auto &f : files) outSlides << d.absoluteFilePath(f);
    return !outSlides.isEmpty();
}
