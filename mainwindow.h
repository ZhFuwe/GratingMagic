#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QImage>

// 前向声明
class QLabel;
class QPushButton;
class QListWidget;
class QRadioButton;
class QSpinBox;
class QScrollArea;
class QDoubleSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化主窗口。
     * @param parent 父控件指针。
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~MainWindow();

private slots:
    // === UI交互槽函数 ===

    /**
     * @brief 响应“导入图像”按钮点击，打开文件对话框并加载图像。
     */
    void importImages();

    /**
     * @brief 响应“删除”按钮点击，删除列表中选中的图像，并智能选择下一项。
     */
    void deleteSelectedImage();

    /**
     * @brief 响应“上移”按钮点击，将选中图像在序列中上移，并保持选中。
     */
    void moveImageUp();

    /**
     * @brief 响应“下移”按钮点击，将选中图像在序列中下移，并保持选中。
     */
    void moveImageDown();

    /**
     * @brief 响应“生成并保存”按钮点击，执行最终的合成与保存流程。
     * 包含逆运算逻辑：如果未指定物理尺寸，会计算并推荐尺寸。
     */
    void saveFinalImage();

    /**
     * @brief 当列表选中项变化时，更新按钮（上移/下移/删除）的可用状态。
     */
    void updateButtonStates();

    /**
     * @brief 调度一次预览更新，使用QTimer避免因参数连续变化导致的过于频繁的刷新。
     */
    void schedulePreviewUpdate();

    /**
     * @brief 当用户完成“期望打印尺寸”输入后触发，执行计算和缩放确认流程。
     */
    void onPrintSizeEditingFinished();

    /**
     * @brief 显示帮助。
     */
    void showHelpDialog();

private:
    // === 数据模型 ===

    /// @brief 存储用户导入的原始图像的文件路径。这是所有数据的“源头”。
    QList<QString> imagePaths;
    /// @brief 缓存从磁盘加载的、未经任何修改的原始QImage对象。所有缩放都以此为基础。
    QList<QImage> originalImages;
    /// @brief 存储当前用于预览和最终生成的工作图像。它可能是原始图像的拷贝，也可能是缩放后的版本。
    QList<QImage> processedImages;

    // === UI控件成员变量 ===
    QScrollArea* scrollArea;
    QLabel* previewLabel;
    QPushButton* importButton;
    QPushButton* helpButton;
    QListWidget* imageListWidget;
    QPushButton* moveUpButton;
    QPushButton* moveDownButton;
    QPushButton* deleteButton;
    QRadioButton* verticalRadio;
    QRadioButton* horizontalRadio;
    QSpinBox* sliceWidthSpinBox;
    QDoubleSpinBox* desiredPrintSizeSpinBox;
    QDoubleSpinBox* actualLpiSpinBox;
    QDoubleSpinBox* calibratedLpiSpinBox;
    QPushButton* saveButton;

    // === 内部辅助函数 ===

    /**
     * @brief 在构造函数中调用，负责创建所有UI控件并手动设置其位置和大小。
     */
    void setupUI();

    /**
     * @brief 在构造函数中调用，负责加载和应用QSS样式表。
     */
    void setupStyles();

    /**
     * @brief 在构造函数中调用，负责连接所有控件的信号到对应的槽函数。
     */
    void setupConnections();

    /**
     * @brief 根据 imagePaths 列表的内容，刷新UI中的图像列表控件。
     */
    void updateImageList();

    /**
     * @brief 根据 processedImages 列表的内容，生成缩略图并更新左侧的预览区。
     */
    void updateAndShowPreview();

    /**
     * @brief 【核心算法】根据给定的图像序列和参数，交织生成最终的光栅图像。
     * @param images 输入的图像列表。
     * @param isVertical 是否为纵向切分。
     * @param sliceWidth 每个切片的像素宽度。
     * @return 返回合成后的光栅图像。
     */
    QImage generateLenticularImage(const QList<QImage>& images, bool isVertical, int sliceWidth);

    /**
     * @brief 【核心计算】根据物理参数计算目标像素尺寸。
     * @param physical_size_cm 期望的物理尺寸（厘米）。
     * @return 计算出的目标像素宽度。
     */
    QSize calculateTargetPixels(double physical_width_cm);

    /**
     * @brief 【核心计算】根据像素尺寸反向计算物理尺寸。
     * @return 计算出的推荐物理尺寸（厘米）。
     */
    QSizeF calculatePhysicalSize();

    /**
     * @brief 【核心操作】对图像执行缩放。此操作总是基于originalImages。
     * @param targetWidth 目标像素尺寸。
     */
    void applyScaling(const QSize& targetSize);

    /**
     * @brief 【核心操作】撤销缩放，将工作图像恢复为原始图像。
     */
    void revertScaling();


};
#endif // MAINWINDOW_H
