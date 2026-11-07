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


#ifndef PROJECTFILE_H
#define PROJECTFILE_H
#include <QString>
#include <QStringList>
#include <QByteArray>
struct PageData { QString bgPath; QByteArray canvasState; };
struct Project { QString title; QStringList slideSourcePaths; QList<PageData> pages; int currentPage = 0; };
class ProjectFile {
public:
    static bool save(const QString &path, const Project &proj);
    static bool load(const QString &path, Project &proj);
    static const char* magic() { return "JAJABOARDv2"; }
};
#endif
