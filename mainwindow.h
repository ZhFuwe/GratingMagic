
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QImage>
#include <QSize>

// 前向声明
class QLabel;
class QPushButton;
class QListWidget;
class QRadioButton;
class QSpinBox;
class QScrollArea;
class QDoubleSpinBox;

enum class SizeMode {
    Automatic,
    ManualOverride
};

/**
 * @class MainWindow
 * @brief GratingMagic应用程序的主窗口类。
 *
 * 负责管理所有UI控件、用户交互、图像数据处理以及核心的光栅合成算法。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化主窗口。
     * @param parent 父控件指针，默认为nullptr。
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~MainWindow();

private slots:

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

    // 当任何参数（非打印宽度）变化时，调用此槽函数以重置
    void onCoreParametersChanged(bool showResetDialog);
    void onResetPrintSizeClicked();// 响应“重置打印尺寸”按钮点击的槽函数

    void onTestButtonClicked(); //用于测试的槽函数

    /**
     * @brief 响应“帮助”按钮点击，显示一个包含详细操作指南的对话框。
     */
    void showHelpDialog();



private:
    // === 数据模型 ===

    /// @brief 存储用户导入的原始图像的文件路径。这是所有数据的“源头”。
    QList<QString> imagePaths;

    // === UI控件成员变量 ===
    QScrollArea* scrollArea;
    QLabel* previewLabel;
    QLabel* outputPixelSizeLabel;
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
    QPushButton* resetPrintSizeButton;
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
     * @brief 根据 imagePaths 列表和当前参数，按需加载并生成缩略图，更新左侧预览区。
     */
    void updateAndShowPreview();

    /**
     * @brief 用于生成预览图的函数
     */
    QImage generateLenticularPreview(const QList<QImage>& thumbnailImages, bool isVertical, int sliceWidth);

    /**
     * @brief 根据当前参数计算对打印机的最终DPI精度要求。
     */
    double calculateRequiredDPI();

    /**
     * @brief 【核心算法】用源图像的裸数据行(sourceScanlines)填充目标大图(resultImage)的指定行。
     * @param resultImage 对最终结果图像的引用，此函数将直接在上面填充像素。
     * @param sourceScanlines 包含了所有源图像【单行】裸像素数据的列表。
     * @param y 正在处理的行在目标图像中的Y坐标。
     * @param stripHeight 此参数在此模型中固定为1。
     * @param isVertical 是否为纵向切分。
     * @param sliceWidth 每个切片的像素宽度。
     */
    void generateLenticularStrip(QImage& resultImage, const QList<QByteArray>& sourceScanlines, int y, int stripHeight, bool isVertical, int sliceWidth);


    /**
     * @brief 【核心计算】根据物理参数计算目标像素尺寸。
     * @param physical_width_cm 期望的物理尺寸（厘米）。
     * @param original_image_size 原始图像的尺寸，用于计算宽高比。
     * @return 计算出的目标像素尺寸(QSize)。
     */
    QSize calculateTargetPixels(double physical_width_cm, const QSize& original_image_size);

    /**
     * @brief 【核心计算】根据像素尺寸反向计算物理尺寸。
     * @param current_pixel_size 当前图像的像素尺寸。
     * @return 计算出的推荐物理尺寸(QSizeF)。
     */
    QSizeF calculatePhysicalSize(const QSize& current_pixel_size);

    /// @brief 记录当前尺寸模式是“自动”还是“手动覆盖”。
    SizeMode currentSizeMode = SizeMode::Automatic;

    /// @brief 只在手动模式下生效，存储用户设定的物理宽度。
    double manualPrintWidthCm = 0.0;
};
#endif // MAINWINDOW_H
