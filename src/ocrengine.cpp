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


#include "ocrengine.h"
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QDir>
OcrEngine::OcrEngine(QObject *p):QObject(p){ tesseractPath = QStandardPaths::findExecutable("tesseract"); }
bool OcrEngine::available() const { return !tesseractPath.isEmpty(); }
QString OcrEngine::recognize(const QImage &img, const QString &lang) {
    if (tesseractPath.isEmpty() || img.isNull()) return {};
    QTemporaryFile tf(QDir::tempPath() + "/jajaocr_XXXXXX.png");
    if (!tf.open()) return {};
    QString path = tf.fileName(); tf.close();
    if (!img.save(path, "PNG")) return {};
    QProcess p; p.start(tesseractPath, {path, "stdout", "-l", lang});
    if (!p.waitForFinished(30000)) return {};
    return QString::fromUtf8(p.readAllStandardOutput()).trimmed();
}
