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


#include "projectfile.h"
#include <QFile>
#include <QDataStream>
bool ProjectFile::save(const QString &path, const Project &proj) {
    QFile f(path); if (!f.open(QIODevice::WriteOnly)) return false;
    QDataStream ds(&f); ds.setVersion(QDataStream::Qt_6_0);
    ds << QByteArray(magic()) << proj.title << proj.slideSourcePaths << int(proj.pages.size());
    for (const auto &p : proj.pages) ds << p.bgPath << p.canvasState;
    ds << proj.currentPage; return true;
}
bool ProjectFile::load(const QString &path, Project &proj) {
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) return false;
    QDataStream ds(&f); ds.setVersion(QDataStream::Qt_6_0);
    QByteArray magic; ds >> magic; if (magic != QByteArray(ProjectFile::magic())) return false;
    ds >> proj.title >> proj.slideSourcePaths;
    int n = 0; ds >> n; proj.pages.clear();
    for (int i = 0; i < n; ++i) { PageData pd; ds >> pd.bgPath >> pd.canvasState; proj.pages << pd; }
    ds >> proj.currentPage; return true;
}
