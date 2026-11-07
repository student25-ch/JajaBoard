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


#include "screencap.h"
#include <QStandardPaths>
ScreenRecorder::ScreenRecorder(QObject *p):QObject(p),ffmpeg(nullptr),rec(false){}
ScreenRecorder::~ScreenRecorder(){ if (rec) stopRecord(); }
bool ScreenRecorder::startRecord(const QString &outPath) {
    if (rec) return false;
    ffmpeg = new QProcess(this);
    ffmpeg->start("ffmpeg", {"-y","-f","gdigrab","-framerate","15","-i","desktop","-c:v","libx264","-pix_fmt","yuv420p",outPath});
    rec = ffmpeg->waitForStarted(3000);
    if (!rec) { delete ffmpeg; ffmpeg = nullptr; }
    return rec;
}
void ScreenRecorder::stopRecord(){
    if (!rec || !ffmpeg) return;
    ffmpeg->write("q"); ffmpeg->closeWriteChannel();
    if (!ffmpeg->waitForFinished(8000)) ffmpeg->kill();
    rec = false; ffmpeg->deleteLater(); ffmpeg = nullptr;
}
