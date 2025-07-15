#include "mainwindow.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QRadioButton>
#include <QSpinBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTimer>
#include <QIcon>
#include <QFileInfo>
#include <QDebug>
#include <QDoubleSpinBox>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("GratingMagic - 光栅卡制作工具 (v1.4.0)");
    resize(900, 680);
    setMinimumSize(850, 650);

    setupUI();
    setupStyles();
    setupConnections();
    updateButtonStates();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // --- 中央控件 ---
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // --- 左侧预览区 ---
    scrollArea = new QScrollArea(centralWidget);
    scrollArea->setGeometry(10, 10, 560, 660);
    scrollArea->setWidgetResizable(true);
    scrollArea->setObjectName("scrollArea");

    previewLabel = new QLabel("预览仅供参考，请导入图像以开始...", scrollArea);
    previewLabel->setObjectName("previewLabel");
    previewLabel->setAlignment(Qt::AlignCenter);
    scrollArea->setWidget(previewLabel);

    // --- 右侧控制面板 ---
    int rightPanelX = 580;
    int rightPanelWidth = 310;

    // --- 控制面板: 图像管理 ---
    importButton = new QPushButton(" 导入图像...", centralWidget);
    importButton->setGeometry(rightPanelX, 10, rightPanelWidth, 35);

    imageListWidget = new QListWidget(centralWidget);
    imageListWidget->setGeometry(rightPanelX, 55, rightPanelWidth, 250);
    imageListWidget->setIconSize(QSize(64, 64));
    imageListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    moveUpButton = new QPushButton("上移", centralWidget);
    moveUpButton->setGeometry(rightPanelX, 315, (rightPanelWidth / 3) - 5, 30);

    moveDownButton = new QPushButton("下移", centralWidget);
    moveDownButton->setGeometry(rightPanelX + (rightPanelWidth / 3), 315, (rightPanelWidth / 3) - 5, 30);

    deleteButton = new QPushButton("删除", centralWidget);
    deleteButton->setGeometry(rightPanelX + 2 * (rightPanelWidth / 3), 315, (rightPanelWidth / 3), 30);

    // --- 控制面板: 合成参数设置 ---
    QGroupBox* settingsGroup = new QGroupBox("合成参数设置", centralWidget);
    settingsGroup->setGeometry(rightPanelX, 360, rightPanelWidth, 90);

    verticalRadio = new QRadioButton("纵向", settingsGroup);
    horizontalRadio = new QRadioButton("横向", settingsGroup);
    verticalRadio->setChecked(true);

    sliceWidthSpinBox = new QSpinBox(settingsGroup);
    sliceWidthSpinBox->setRange(1, 200);
    sliceWidthSpinBox->setValue(4);

    QFormLayout* settingsLayout = new QFormLayout(settingsGroup);
    QHBoxLayout* radioLayout = new QHBoxLayout();
    radioLayout->addWidget(verticalRadio);
    radioLayout->addWidget(horizontalRadio);
    settingsLayout->addRow("切分方向:", radioLayout);
    settingsLayout->addRow("切片宽度(像素):", sliceWidthSpinBox);

    // --- 控制面板: 打印参数设置 ---
    QGroupBox* printSettingsGroup = new QGroupBox("打印参数设置", centralWidget);
    printSettingsGroup->setGeometry(rightPanelX, 460, rightPanelWidth, 120);

    desiredPrintSizeSpinBox = new QDoubleSpinBox(printSettingsGroup);
    desiredPrintSizeSpinBox->setDecimals(2);
    desiredPrintSizeSpinBox->setSuffix(" 厘米");
    desiredPrintSizeSpinBox->setRange(0.0, 999.0); // 0 表示不启用

    actualLpiSpinBox = new QDoubleSpinBox(printSettingsGroup);
    actualLpiSpinBox->setDecimals(2);
    actualLpiSpinBox->setRange(10.0, 1000.0);
    actualLpiSpinBox->setValue(70.00);

    calibratedLpiSpinBox = new QDoubleSpinBox(printSettingsGroup);
    calibratedLpiSpinBox->setDecimals(2);
    calibratedLpiSpinBox->setRange(10.0, 1000.0);
    calibratedLpiSpinBox->setValue(70.00);

    QFormLayout* printSettingsLayout = new QFormLayout(printSettingsGroup);
    printSettingsLayout->addRow("打印尺寸(宽度):", desiredPrintSizeSpinBox);
    printSettingsLayout->addRow("光栅板实际LPI:", actualLpiSpinBox);
    printSettingsLayout->addRow("打印机校准LPI:", calibratedLpiSpinBox);

    // --- 控制面板: 生成操作 ---
    saveButton = new QPushButton(" 生成并保存图像...", centralWidget);
    saveButton->setObjectName("saveButton");
    saveButton->setGeometry(rightPanelX, 630, rightPanelWidth, 40);
}

void MainWindow::setupStyles()
{
    // 从Qt资源系统加载样式表文件
    QFile styleFile(":/styles/stylesheet.qss");
    if (!styleFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning("警告: 无法打开样式表文件 ':/styles/stylesheet.qss'");
        return;
    }
    QTextStream stream(&styleFile);
    QString styleSheet = stream.readAll();
    this->setStyleSheet(styleSheet);

    // 为按钮设置图标
    importButton->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
    deleteButton->setIcon(this->style()->standardIcon(QStyle::SP_TrashIcon));
    moveUpButton->setIcon(this->style()->standardIcon(QStyle::SP_ArrowUp));
    moveDownButton->setIcon(this->style()->standardIcon(QStyle::SP_ArrowDown));
    saveButton->setIcon(this->style()->standardIcon(QStyle::SP_DialogSaveButton));
    QSize iconSize(20, 20);
    importButton->setIconSize(iconSize);
    deleteButton->setIconSize(iconSize);
    moveUpButton->setIconSize(iconSize);
    moveDownButton->setIconSize(iconSize);
    saveButton->setIconSize(iconSize);
}

void MainWindow::setupConnections()
{
    // 连接UI控件信号到对应槽函数
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importImages);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedImage);
    connect(moveUpButton, &QPushButton::clicked, this, &MainWindow::moveImageUp);
    connect(moveDownButton, &QPushButton::clicked, this, &MainWindow::moveImageDown);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveFinalImage);
    connect(imageListWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::updateButtonStates);

    // 合成参数变化时，预览更新
    connect(verticalRadio, &QRadioButton::toggled, this, &MainWindow::schedulePreviewUpdate);
    connect(horizontalRadio, &QRadioButton::toggled, this, &MainWindow::schedulePreviewUpdate);
    connect(sliceWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::schedulePreviewUpdate);

    // 打印尺寸输入完成时，触发尺寸计算和确认流程
    connect(desiredPrintSizeSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::onPrintSizeEditingFinished);
}

// ==============================================================================================================
//                                          核心工作流与状态管理函数
// ==============================================================================================================

void MainWindow::importImages()
{
    // 弹出文件对话框
    QStringList files = QFileDialog::getOpenFileNames(this, "选择源图像", "", "图像文件 (*.png *.jpg *.jpeg)");
    if (files.isEmpty()) return;

    // 重置所有图像数据和状态
    imagePaths.clear();
    originalImages.clear();
    processedImages.clear();
    imagePaths.append(files);

    // 加载图像文件到内存缓存
    for(const QString& path : imagePaths) {
        QImage img(path);
        if(!img.isNull()){
            originalImages.append(img);
        }
    }

    // 初始化工作图像列表
    processedImages = originalImages;

    // 重置UI状态
    desiredPrintSizeSpinBox->setValue(0.0);
    updateImageList();
    if (!imagePaths.isEmpty()) {
        imageListWidget->setCurrentRow(0);
    }
    schedulePreviewUpdate();
}

void MainWindow::onPrintSizeEditingFinished()
{
    double desiredWidth = desiredPrintSizeSpinBox->value();
    if (originalImages.isEmpty()) {
        desiredPrintSizeSpinBox->setValue(0.0);
        return;
    }
    if (desiredWidth <= 0.0) {
        revertScaling();
        schedulePreviewUpdate();
        return;
    }

    QSize targetPixelSize = calculateTargetPixels(desiredWidth);
    QSize currentPixelSize = processedImages.first().size();

    if (targetPixelSize == currentPixelSize) return;

    QString message = QString("图像将被缩放到 %1 x %2 像素以匹配打印尺寸。\n"
                              "当前工作图像尺寸为 %3 x %4 像素。\n\n"
                              "此操作不会改变源文件大小，是否继续？")
                          .arg(targetPixelSize.width()).arg(targetPixelSize.height())
                          .arg(currentPixelSize.width()).arg(currentPixelSize.height());

    if (QMessageBox::question(this, "尺寸调整", message, QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        applyScaling(targetPixelSize);
        schedulePreviewUpdate();
    } else {
        desiredPrintSizeSpinBox->setValue(0.0);
    }
}

void MainWindow::saveFinalImage()
{
    if (processedImages.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先导入至少一张图像！");
        return;
    }

    double desiredSize = desiredPrintSizeSpinBox->value();
    if (desiredSize <= 0.0) {
        QSizeF recommendedSize = calculatePhysicalSize();

        QString message = QString("您未指定期望的打印尺寸。\n\n"
                                  "根据当前图像像素和打印参数，推荐的物理尺寸为: \n"
                                  "     %1 x %2 厘米\n\n"
                                  "是否继续使用此推荐尺寸进行保存？")
                              .arg(QString::number(recommendedSize.width(), 'f', 2))
                              .arg(QString::number(recommendedSize.height(), 'f', 2));

        if (QMessageBox::question(this, "打印尺寸确认", message, QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel) {
            return;
        }
    }

    // 弹出文件保存对话框
    QString savePath = QFileDialog::getSaveFileName(this, "保存光栅图像", "", "PNG图像 (*.png)");
    if (savePath.isEmpty()) return;

    // 显示进度条并执行合成
    QProgressDialog progress("正在合成最终图像...", "取消", 0, 1, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);
    QApplication::processEvents();

    bool isVertical = verticalRadio->isChecked();
    int sliceWidth = sliceWidthSpinBox->value();

    QImage finalImage = generateLenticularImage(processedImages, isVertical, sliceWidth);

    progress.setValue(1);

    // 保存文件
    if (finalImage.save(savePath, "PNG")) {
        QMessageBox::information(this, "保存成功", QString("最终图像已成功保存至:\n%1").arg(savePath));
    } else {
        QMessageBox::critical(this, "失败", "图像保存失败！请检查路径或权限。");
    }
}

// ==============================================================================================================
//                                          列表操作函数
// ==============================================================================================================

void MainWindow::deleteSelectedImage()
{
    QList<QListWidgetItem*> selectedItems = imageListWidget->selectedItems();
    if (selectedItems.isEmpty()) return;

    // 智能计算删除后应该选中的行号
    QList<int> rowsToDelete;
    for(auto item : selectedItems) {
        rowsToDelete.append(imageListWidget->row(item));
    }
    std::sort(rowsToDelete.begin(), rowsToDelete.end());
    int selectionAnchor = rowsToDelete.first();

    // 删除所有数据模型中的对应项
    for (int i = rowsToDelete.size() - 1; i >= 0; --i) {
        imagePaths.removeAt(rowsToDelete[i]);
        originalImages.removeAt(rowsToDelete[i]);
        processedImages.removeAt(rowsToDelete[i]);
    }

    updateImageList(); // 刷新UI

    // 重新设置选中项
    int newCount = imagePaths.count();
    if (newCount > 0) {
        if (selectionAnchor >= newCount) {
            selectionAnchor = newCount - 1;
        }
        imageListWidget->setCurrentRow(selectionAnchor);
    }

    schedulePreviewUpdate();
}

void MainWindow::moveImageUp()
{
    int currentIndex = imageListWidget->currentRow();
    if (currentIndex > 0) {
        // 同步移动所有数据列表中的项
        imagePaths.swapItemsAt(currentIndex, currentIndex - 1);
        originalImages.swapItemsAt(currentIndex, currentIndex - 1);
        processedImages.swapItemsAt(currentIndex, currentIndex - 1);

        updateImageList();
        imageListWidget->setCurrentRow(currentIndex - 1); // 保持选中
        schedulePreviewUpdate();
    }
}

void MainWindow::moveImageDown()
{
    int currentIndex = imageListWidget->currentRow();
    if (currentIndex != -1 && currentIndex < imagePaths.count() - 1) {
        // 同步移动所有数据列表中的项
        imagePaths.swapItemsAt(currentIndex, currentIndex + 1);
        originalImages.swapItemsAt(currentIndex, currentIndex + 1);
        processedImages.swapItemsAt(currentIndex, currentIndex + 1);

        updateImageList();
        imageListWidget->setCurrentRow(currentIndex + 1); // 保持选中
        schedulePreviewUpdate();
    }
}

// ==============================================================================================================
//                                              辅助与计算函数
// ==============================================================================================================

QSize MainWindow::calculateTargetPixels(double physical_width_cm)
{
    if (physical_width_cm <= 0 || originalImages.isEmpty()) return QSize(0, 0);

    // 以第一张原始图像的宽高比为基准
    const QImage& firstImage = originalImages.first();
    double aspectRatio = static_cast<double>(firstImage.height()) / firstImage.width();

    // 计算目标像素宽度
    double calLpi = calibratedLpiSpinBox->value();
    int sliceWidth = sliceWidthSpinBox->value();
    int numFrames = originalImages.size();

    double total_pixels_per_lenticule = sliceWidth * numFrames;
    double required_print_dpi = total_pixels_per_lenticule * calLpi;
    double physical_size_inch = physical_width_cm / 2.54;
    double calculated_pixels_w = physical_size_inch * required_print_dpi;

    // 根据宽高比计算目标像素高度
    double calculated_pixels_h = calculated_pixels_w * aspectRatio;

    return QSize(static_cast<int>(round(calculated_pixels_w)),
                 static_cast<int>(round(calculated_pixels_h)));
}

QSizeF MainWindow::calculatePhysicalSize()
{
    if (processedImages.isEmpty()) return QSizeF(0.0, 0.0);

    // 以第一张工作图像的宽高比为基准
    const QImage& firstImage = processedImages.first();
    double aspectRatio = static_cast<double>(firstImage.height()) / firstImage.width();

    // 计算物理宽度
    double total_pixels_w = firstImage.width();
    double calLpi = calibratedLpiSpinBox->value();
    int sliceWidth = sliceWidthSpinBox->value();
    int numFrames = processedImages.size();

    double total_pixels_per_lenticule = sliceWidth * numFrames;
    double required_print_dpi = total_pixels_per_lenticule * calLpi;

    if (required_print_dpi <= 0) return QSizeF(0.0, 0.0);

    double physical_size_inch_w = total_pixels_w / required_print_dpi;
    double physical_size_cm_w = physical_size_inch_w * 2.54;

    // 根据宽高比计算物理高度
    double physical_size_cm_h = physical_size_cm_w * aspectRatio;

    return QSizeF(physical_size_cm_w, physical_size_cm_h);
}

void MainWindow::applyScaling(const QSize& targetSize)
{
    if (originalImages.isEmpty() || targetSize.isEmpty()) return;

    processedImages.clear();
    for(const QImage& img : originalImages) {
        processedImages.append(img.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    qDebug() << "应用缩放：工作图像已从原始版本缩放至" << targetSize.width() << "x" << targetSize.height();
}

void MainWindow::revertScaling()
{
    processedImages = originalImages;
    qDebug() << "撤销缩放：工作图像已恢复为原始版本。";
}


void MainWindow::updateImageList()
{
    disconnect(imageListWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::updateButtonStates);
    imageListWidget->clear();
    for (const QString& path : imagePaths) {
        QFileInfo fileInfo(path);
        QListWidgetItem* item = new QListWidgetItem(QIcon(path), fileInfo.fileName());
        imageListWidget->addItem(item);
    }
    connect(imageListWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::updateButtonStates);
    updateButtonStates();
}

void MainWindow::updateButtonStates()
{
    int count = imagePaths.count();
    int selectedCount = imageListWidget->selectedItems().count();
    int currentIndex = imageListWidget->currentRow();

    deleteButton->setEnabled(selectedCount > 0);
    moveUpButton->setEnabled(selectedCount == 1 && currentIndex > 0);
    moveDownButton->setEnabled(selectedCount == 1 && currentIndex < count - 1);
    saveButton->setEnabled(count > 0);
}

void MainWindow::schedulePreviewUpdate()
{
    QTimer::singleShot(50, this, &MainWindow::updateAndShowPreview);
}

void MainWindow::updateAndShowPreview()
{
    if (processedImages.isEmpty()) {
        previewLabel->setText("请导入图像以开始...");
        previewLabel->setPixmap(QPixmap());
        return;
    }

    const int previewSize = 400;
    QList<QImage> previewThumbnails;
    for(const QImage& img : processedImages){
        previewThumbnails.append(img.scaled(previewSize, previewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (previewThumbnails.isEmpty()) return;

    bool isVertical = verticalRadio->isChecked();
    int sliceWidth = sliceWidthSpinBox->value();
    QImage previewImage = generateLenticularImage(previewThumbnails, isVertical, sliceWidth);

    if(!previewImage.isNull()){
        previewLabel->setPixmap(QPixmap::fromImage(previewImage).scaled(
            previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
            ));
    }
}

QImage MainWindow::generateLenticularImage(const QList<QImage>& images, bool isVertical, int sliceWidth)
{
    if (images.isEmpty() || sliceWidth <= 0) return QImage();

    const int numFrames = images.size();
    const QSize targetSize = images.first().size();

    QList<QImage> sourceImages;
    for (const QImage& originalImg : images) {
        QImage convertedImg = originalImg.convertToFormat(QImage::Format_ARGB32);
        if (convertedImg.size() != targetSize) {
            sourceImages.append(convertedImg.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        } else {
            sourceImages.append(convertedImg);
        }
    }

    QImage resultImage(targetSize, QImage::Format_ARGB32);

    if (isVertical) {
        for (int y = 0; y < targetSize.height(); ++y) {
            QRgb* resultLine = reinterpret_cast<QRgb*>(resultImage.scanLine(y));
            QList<const uchar*> sourceLines;
            for(const QImage& img : sourceImages){
                sourceLines.append(img.scanLine(y));
            }

            for (int x = 0; x < targetSize.width(); ++x) {
                int sourceImageIndex = (x / sliceWidth) % numFrames;
                // 直接从源行指针复制像素数据
                memcpy(resultLine + x, sourceLines[sourceImageIndex] + x * 4, 4);
            }
        }
    } else {
        for (int y = 0; y < targetSize.height(); ++y) {
            int sourceImageIndex = (y / sliceWidth) % numFrames;
            const uchar* sourceLine = sourceImages[sourceImageIndex].scanLine(y);
            uchar* resultLine = resultImage.scanLine(y);
            // 将一整行直接复制过去
            memcpy(resultLine, sourceLine, resultImage.bytesPerLine());
        }
    }

    return resultImage;
}
