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


#ifndef SCREENCAP_H
#define SCREENCAP_H
#include <QObject>
#include <QProcess>
class ScreenRecorder : public QObject {
    Q_OBJECT
public:
    explicit ScreenRecorder(QObject *p = nullptr);
    ~ScreenRecorder();
    bool startRecord(const QString &outPath);
    void stopRecord();
    bool isRecording() const { return rec; }
private:
    QProcess* ffmpeg; bool rec;
};
#endif
