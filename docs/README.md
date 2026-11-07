# JajaBoard (v2.0)

GPLv3 | 跨平台教学白板 (Windows / Linux / macOS)

JajaBoard 是一款对标希沃白板的开源轻量级教学工具，基于 Qt6 和 C++17 编写。

## ✨ 功能特性

- **基础绘图**：画笔、荧光笔、橡皮擦、直线、矩形、圆形。
- **教学工具**：直尺（实时长度）、量角器（实时角度）、激光笔（轨迹 0.5s 淡出）。
- **高级操作**：选择/移动/删除笔迹（Ctrl+A 全选，Delete 删除）、Ctrl+滚轮缩放、空格+左键拖拽平移。
- **多媒体**：拖拽图片到画布、插入本地图片。
- **手写识别**：集成 tesseract CLI，一键提取画布文字。
- **课件管理**：多页课件（支持导入 PPT/PPTX 自动拆分页面）、自定义 `.jboard` 格式保存/加载。
- **导出与演示**：多页 PDF 完整带笔迹导出、双屏投影模式（共享场景，实时同步）。
- **扩展功能**：基于 FFmpeg 的屏幕录制（Wayland 下自动调用 wf-recorder）、WebSocket 协作（可选编译）。

## 🛠️ 依赖说明

| 功能模块 | 环境依赖 |
| --- | --- |
| 主程序 | Qt6 (Core, Gui, Widgets, PrintSupport) |
| PPT 导入 | LibreOffice + poppler-utils (`pdftoppm`) |
| 录屏 | ffmpeg (X11/Win/macOS) 或 wf-recorder (Wayland) |
| 手写识别 (OCR) | tesseract + tesseract-ocr-chi-sim |
| 实时协作 | Qt6 WebSockets |

## 🚀 编译运行

请参阅 [docs/BUILD.md](docs/BUILD.md) 获取各平台的详细编译指南。

### 快速开始
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/JajaBoard