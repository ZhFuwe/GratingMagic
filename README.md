


<br />
<p align="center">
    <img src="https://github.com/user-attachments/assets/13b18529-e6c1-456b-a4f3-9538720bea58" alt="Logo" width="156" height="156">
  </a>
  <h2 align="center" style="font-weight: 600">GratingMagic</h2>

<p align="center">
  <strong>一款易用、强大的光栅卡图像合成工具</strong>
  <br>
  轻松将您的图像序列转换为具有动态效果的、可供专业打印的光栅图像。
</p>
<p align="center">
  <img src="https://img.shields.io/badge/Platform-Windows-blue.svg" alt="Platform">
  <img src="https://img.shields.io/badge/Qt-6.x-green.svg" alt="Qt Version">
  <img src="https://img.shields.io/badge/License-Apache 2.0-blue.svg" alt="License">
</p>






## 目录

- [简介](#简介)
- [核心功能](#核心功能)
- [软件截图](#软件截图)
- [快速开始](#快速开始)
  - [系统要求](#系统要求)
  - [从源码构建](#从源码构建)
- [使用指南](#使用指南)
- [贡献](#贡献)
- [许可证](#许可证)

## 简介

**GratingMagic** 是一个为光栅画、光栅卡爱好者和专业设计师打造的桌面应用程序。它简化了从普通图像序列到可打印光栅图的繁琐转换过程，通过直观的界面精确控制交织（interleaving）过程的细节，并根据最终的物理打印参数来自动调整图像尺寸，实现所见即所得。

无论您是想制作一张有趣的3D贺卡，还是需要为商业印刷准备高精度的光栅文件，GratingMagic将是您的得力助手。

## 核心功能

✨ **图像管理** - 轻松导入多张图片并调整帧顺序。

🖼️ **实时预览** - 所有参数的调整都会立即在预览窗口中得到反馈，无需等待。

🎚️ **合成控制** - 自定义切分方向（纵向/横向）和每个切片的像素宽度。

📐 **物理尺寸感知** - 根据您设置的物理打印尺寸（厘米）和光栅板LPI值，自动计算并调整图像的像素尺寸。

🔄 **智能逆运算** - 如果您不确定打印尺寸，软件可以根据当前图像的分辨率和打印参数，逆运算推荐最佳的物理打印尺寸。

## 软件截图

<img width="1348" height="1062" alt="image" src="https://github.com/user-attachments/assets/08ade324-37cf-4bd1-b5b2-ef6a04b2dd94" />

## 快速开始

### 系统要求

- Windows 10 或更高版本 (64位)

### 从源码构建

如果您希望自行编译，请确保您的环境已配置好 **Qt 6** 和 **CMake**。

```bash
# 1. 克隆仓库
git clone https://github.com/ZhFuwe/GratingMagic.git
cd GratingMagic

# 2. 创建构建目录
mkdir build && cd build

# 3. 配置项目 (指定Qt6的安装路径)
#    请将 "C:/Qt/6.x.x/msvcxxxx_64" 替换为您自己的Qt安装路径
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvcxxxx_64"

# 4. 构建项目
cmake --build .

# 5. 构建完成后，可执行文件将位于 build/debug 或 build/release 目录下
```

## 使用说明

1.  **导入图像**：点击`导入图像...`按钮，选择2张或更多图片。
2.  **调整顺序**：在右侧列表中选中一张图片，使用`上移`或`下移`按钮来调整它在动画序列中的位置。
3.  **设置合成参数**：选择`纵向`或`横向`切分，并设置一个合适的`切片宽度`。
4.  **设置打印参数 (可选)**：
    - 如果您有明确的打印尺寸目标（如10厘米宽），请在`期望打印尺寸`中输入`10.00`，程序会提示您自动缩放图像。
    - 如果您不关心物理尺寸，请将此项保持为`0.00`。
5.  **预览**: 在左侧预览区观察实时效果。
6.  **保存**: 点击`生成并保存图像...`按钮。如果未设置打印尺寸，程序会推荐一个尺寸并请求您确认。最后，选择路径保存最终的PNG文件。

如果需要更详细的使用说明，请转至[操作指南](https://github.com/ZhFuwe/GratingMagic/blob/main/INSTRUCTIONS.md)

## 贡献

欢迎对本项目做出贡献！
如果您有任何想法、建议或发现了Bug，请随时提交一个 [Issue](https://github.com/ZhFuwe/GratingMagic/issues)。

## 许可证

本项目采用 `Apache-2.0 license`。详情请见 `LICENSE` 文件。

