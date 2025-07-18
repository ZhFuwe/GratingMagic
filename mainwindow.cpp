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
#include <QTextBrowser>
#include <algorithm>
#include <QDesktopServices>
#include <QUrl>
#include <QTemporaryDir>
#include <QImageWriter>
#include <QImageReader>
#include <QScopedPointer>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 初始化窗口
    setWindowTitle("GratingMagic - 光栅卡制作工具 (v2.0.0)");
    resize(900, 680);
    setMinimumSize(850, 650);

    // 构建和设置应用程序
    setupUI();
    setupStyles();
    setupConnections();
    updateButtonStates();
}

MainWindow::~MainWindow()
{}

void MainWindow::setupUI()
{
    // --- 中央控件 ---
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // --- 左侧：预览区 ---
    scrollArea = new QScrollArea(centralWidget);
    scrollArea->setGeometry(10, 10, 560, 660);
    scrollArea->setWidgetResizable(true); // 允许内部控件随滚动区缩放
    scrollArea->setObjectName("scrollArea"); // 用于QSS样式

    // 创建用于显示预览图像的标签
    previewLabel = new QLabel("预览仅供参考，请导入图像以开始...", scrollArea);
    previewLabel->setObjectName("previewLabel"); // 用于QSS样式
    previewLabel->setAlignment(Qt::AlignCenter);
    scrollArea->setWidget(previewLabel);

    // --- 右侧：控制面板 ---
    const int rightPanelX = 580;
    const int rightPanelWidth = 310;
    const int labelWidth = 100; // 标签的统一宽度
    const int controlWidth = rightPanelWidth - labelWidth - 25; // 输入控件的统一宽度

    // --- 控制面板: 图像管理 ---
    const int importButtonWidth = rightPanelWidth - 45;
    importButton = new QPushButton("导入图像...", centralWidget);
    importButton->setGeometry(rightPanelX, 10, importButtonWidth, 35);

    helpButton = new QPushButton(centralWidget);
    helpButton->setGeometry(rightPanelX + importButtonWidth + 5, 10, 35, 35);
    helpButton->setToolTip("查看操作指南");

    imageListWidget = new QListWidget(centralWidget);
    imageListWidget->setGeometry(rightPanelX, 55, rightPanelWidth, 250);
    imageListWidget->setIconSize(QSize(64, 64));
    imageListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    moveUpButton = new QPushButton("上移", centralWidget);
    moveUpButton->setGeometry(rightPanelX, 315, (rightPanelWidth / 3) - 4, 30);

    moveDownButton = new QPushButton("下移", centralWidget);
    moveDownButton->setGeometry(rightPanelX + (rightPanelWidth / 3), 315, (rightPanelWidth / 3) - 4, 30);

    deleteButton = new QPushButton("删除", centralWidget);
    deleteButton->setGeometry(rightPanelX + 2 * (rightPanelWidth / 3) + 2, 315, (rightPanelWidth / 3) - 4, 30);

    // --- 控制面板: 合成参数设置 ---
    QGroupBox* settingsGroup = new QGroupBox("合成参数设置", centralWidget);
    settingsGroup->setGeometry(rightPanelX, 360, rightPanelWidth, 125);

    QLabel* directionLabel = new QLabel("切分方向:", settingsGroup);
    directionLabel->setGeometry(15, 30, labelWidth, 25);
    verticalRadio = new QRadioButton("纵向", settingsGroup);
    verticalRadio->setGeometry(labelWidth + 15, 30, 80, 25);
    horizontalRadio = new QRadioButton("横向", settingsGroup);
    horizontalRadio->setGeometry(labelWidth + 95, 30, 80, 25);
    verticalRadio->setChecked(true);

    QLabel* sliceWidthLabel = new QLabel("切片宽度(像素):", settingsGroup);
    sliceWidthLabel->setGeometry(15, 60, labelWidth, 25);
    sliceWidthSpinBox = new QSpinBox(settingsGroup);
    sliceWidthSpinBox->setGeometry(labelWidth + 15, 60, controlWidth, 25);
    sliceWidthSpinBox->setRange(1, 200);
    sliceWidthSpinBox->setValue(4);

    QLabel* outputSizeTextLabel = new QLabel("输出尺寸(像素):", settingsGroup);
    outputSizeTextLabel->setGeometry(15, 90, labelWidth, 25);
    outputPixelSizeLabel = new QLabel("N/A", settingsGroup);
    outputPixelSizeLabel->setGeometry(labelWidth + 15, 90, controlWidth, 25);
    outputPixelSizeLabel->setAlignment(Qt::AlignCenter);
    outputPixelSizeLabel->setObjectName("outputPixelSizeDisplay"); // 用于QSS样式
    outputPixelSizeLabel->setToolTip("此尺寸根据下方打印参数自动计算。\n请最后修改“打印宽度”，程序会自动调整此处的像素尺寸。");

    // --- 控制面板: 打印参数设置 ---
    QGroupBox* printSettingsGroup = new QGroupBox("打印参数设置", centralWidget);
    printSettingsGroup->setGeometry(rightPanelX, 495, rightPanelWidth, 125);

    QLabel* actualLpiLabel = new QLabel("光栅板实际LPI:", printSettingsGroup);
    actualLpiLabel->setGeometry(15, 30, labelWidth, 25);
    actualLpiSpinBox = new QDoubleSpinBox(printSettingsGroup);
    actualLpiSpinBox->setGeometry(labelWidth + 15, 30, controlWidth, 25);
    actualLpiSpinBox->setDecimals(2);
    actualLpiSpinBox->setRange(10.0, 1000.0);
    actualLpiSpinBox->setValue(90.00);

    QLabel* calibratedLpiLabel = new QLabel("打印机校准LPI:", printSettingsGroup);
    calibratedLpiLabel->setGeometry(15, 60, labelWidth, 25);
    calibratedLpiSpinBox = new QDoubleSpinBox(printSettingsGroup);
    calibratedLpiSpinBox->setGeometry(labelWidth + 15, 60, controlWidth, 25);
    calibratedLpiSpinBox->setDecimals(2);
    calibratedLpiSpinBox->setRange(10.0, 1000.0);
    calibratedLpiSpinBox->setValue(90.50);

    QLabel* printWidthLabel = new QLabel("打印宽度(厘米):", printSettingsGroup);
    printWidthLabel->setGeometry(15, 90, labelWidth, 25);
    desiredPrintSizeSpinBox = new QDoubleSpinBox(printSettingsGroup);
    desiredPrintSizeSpinBox->setGeometry(labelWidth + 15, 90, controlWidth - 30, 28);
    desiredPrintSizeSpinBox->setDecimals(2);
    desiredPrintSizeSpinBox->setSuffix(" 厘米");
    desiredPrintSizeSpinBox->setRange(0.0, 999.0);

    resetPrintSizeButton = new QPushButton("↺", printSettingsGroup);
    resetPrintSizeButton->setGeometry(labelWidth + 15 + controlWidth - 28, 90, 28, 28);
    resetPrintSizeButton->setToolTip("重置为自动计算的推荐尺寸");

    // --- 控制面板: 生成操作 ---
    saveButton = new QPushButton(" 生成并保存图像...", centralWidget);
    saveButton->setObjectName("saveButton");
    saveButton->setGeometry(rightPanelX, 630, rightPanelWidth, 40);
}

void MainWindow::setupStyles()
{
    // 加载样式表
    QFile styleFile(":/styles/stylesheet.qss");
    if (!styleFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning("警告: 无法打开样式表文件 ':/styles/stylesheet.qss'");
        return;
    }
    QTextStream stream(&styleFile);
    this->setStyleSheet(stream.readAll());

    // 为按钮设置图标、字体
    importButton->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
    helpButton->setIcon(this->style()->standardIcon(QStyle::SP_DialogHelpButton));
    deleteButton->setIcon(this->style()->standardIcon(QStyle::SP_TrashIcon));
    moveUpButton->setIcon(this->style()->standardIcon(QStyle::SP_ArrowUp));
    moveDownButton->setIcon(this->style()->standardIcon(QStyle::SP_ArrowDown));
    saveButton->setIcon(this->style()->standardIcon(QStyle::SP_DialogSaveButton));
    resetPrintSizeButton->setIcon(this->style()->standardIcon(QStyle::SP_RestoreDefaultsButton));

    QSize mainIconSize(20, 20);
    importButton->setIconSize(mainIconSize);
    saveButton->setIconSize(mainIconSize);

    QSize secondaryIconSize(16, 16);
    moveUpButton->setIconSize(secondaryIconSize);
    moveDownButton->setIconSize(secondaryIconSize);
    deleteButton->setIconSize(secondaryIconSize);
    helpButton->setIconSize(secondaryIconSize);

    resetPrintSizeButton->setFont(QFont("宋体", 14));
}

void MainWindow::setupConnections()
{
    // 连接UI控件的信号到槽函数
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importImages);
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::showHelpDialog);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedImage);
    connect(moveUpButton, &QPushButton::clicked, this, &MainWindow::moveImageUp);
    connect(moveDownButton, &QPushButton::clicked, this, &MainWindow::moveImageDown);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveFinalImage);
    connect(imageListWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::updateButtonStates);
    connect(resetPrintSizeButton, &QPushButton::clicked, this, &MainWindow::onResetPrintSizeClicked);

    // 当合成参数变化时，调度预览更新
    connect(verticalRadio, &QRadioButton::toggled, this, &MainWindow::schedulePreviewUpdate);
    connect(horizontalRadio, &QRadioButton::toggled, this, &MainWindow::schedulePreviewUpdate);
    connect(sliceWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::schedulePreviewUpdate);


    // 将所有核心参数的变化连接到新的重置槽函数
    connect(verticalRadio, &QRadioButton::toggled, this, &MainWindow::onCoreParametersChanged);
    connect(horizontalRadio, &QRadioButton::toggled, this, &MainWindow::onCoreParametersChanged);
    connect(sliceWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onCoreParametersChanged);
    connect(actualLpiSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onCoreParametersChanged);
    connect(calibratedLpiSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onCoreParametersChanged);

    // 当打印尺寸输入完成时，触发尺寸计算和确认
    connect(desiredPrintSizeSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::onPrintSizeEditingFinished);
}

// ===================================================================
//          核心工作流与状态管理函数
// ===================================================================

void MainWindow::importImages()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "选择源图像", "", "图像文件 (*.png *.jpg *.jpeg)");
    if (files.isEmpty()) return;

    // 如果列表中已有图像
    if (!imagePaths.isEmpty()) {
        int choice = QMessageBox::question(this, "导入模式",
                                           "您想追加到当前列表，还是清空并重新导入？",
                                           "追加", "清空");

        if (choice == 1) { // 如果清空
            imagePaths.clear();
        }
    }

    imagePaths.append(files);
    updateImageList(); // 先更新列表，以便后续计算获取正确的帧数

    // 导入后，重置为自动模式
    onCoreParametersChanged(true);

    if (!imagePaths.isEmpty()) {
        imageListWidget->setCurrentRow(0);
    }
}

void MainWindow::showHelpDialog()
{
    // 创建对话框和主要控件
    QDialog* helpDialog = new QDialog(this);
    helpDialog->setWindowTitle("操作指南 - GratingMagic v2.0.x");
    helpDialog->setMinimumSize(600, 500);
    QTextBrowser* textBrowser = new QTextBrowser(helpDialog);
    textBrowser->setOpenExternalLinks(true);

    // 加载帮助文本
    QFile helpFile(":/docs/help_guide.md");
    if (helpFile.open(QFile::ReadOnly | QFile::Text)) {
        textBrowser->setMarkdown(helpFile.readAll());
        helpFile.close();
    } else {
        textBrowser->setText("错误：无法加载帮助文件 ':/docs/help_guide.md'。");
    }

    // 底部按钮和布局
    QPushButton* licenseButton = new QPushButton("查看开源协议", helpDialog);
    QPushButton* aboutQtButton = new QPushButton("关于Qt", helpDialog);
    QPushButton* closeButton = new QPushButton("关闭", helpDialog);
    connect(licenseButton, &QPushButton::clicked, [](){
        QDesktopServices::openUrl(QUrl("https://github.com/ZhFuwe/GratingMagic/blob/main/LICENSE"));
    });
    connect(aboutQtButton, &QPushButton::clicked, this, [this](){
        QMessageBox::aboutQt(this, "关于 Qt");
    });
    connect(closeButton, &QPushButton::clicked, helpDialog, &QDialog::accept);

    // 设置对话框布局
    QVBoxLayout* mainLayout = new QVBoxLayout(helpDialog);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(licenseButton);
    buttonLayout->addWidget(aboutQtButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    mainLayout->addWidget(textBrowser);
    mainLayout->addLayout(buttonLayout);

    // 以模态方式显示对话框
    helpDialog->exec();
}

void MainWindow::onPrintSizeEditingFinished()
{
    desiredPrintSizeSpinBox->blockSignals(true);
    QScopedPointer<QObject> guard(new QObject());
    connect(guard.data(), &QObject::destroyed, [this](){
        if (desiredPrintSizeSpinBox) desiredPrintSizeSpinBox->blockSignals(false);
    });

    double userInputWidth = desiredPrintSizeSpinBox->value();

    if (imagePaths.isEmpty()) {
        onCoreParametersChanged(false); // 没有图像，直接重置
        return;
    }

    // 如果用户输入了0或负数，视为执行“重置”操作
    if (userInputWidth <= 0.0) {
        onCoreParametersChanged(true);
        return;
    }

    // 带容差的浮点数比较

    // 容差值
    const double epsilon = 0.005;
    // 获取旧值
    double previousValue = 0.0;
    if (currentSizeMode == SizeMode::ManualOverride) {
        // 手动模式，则与上次手动锁定的值比较
        previousValue = manualPrintWidthCm;
    } else {
        // 自动模式，与程序自动计算的精确值比较
        if (!imagePaths.isEmpty()) {
            QImage firstImage(imagePaths.first());
            if(!firstImage.isNull()) {
                previousValue = calculatePhysicalSize(firstImage.size()).width();
            }
        }
    }

    // 判断差的绝对值是否小于容差
    if (std::fabs(userInputWidth - previousValue) < epsilon) {
        qDebug() << "打印尺寸未发生有意义的改变，跳过确认。";
        desiredPrintSizeSpinBox->setValue(previousValue);
        return;
    }

    QImage firstImage(imagePaths.first());
    if (firstImage.isNull()) return;

    QSize targetPixelSize = calculateTargetPixels(userInputWidth, firstImage.size());
    if (targetPixelSize.width() < 1) {
        QMessageBox::warning(this, "警告", "宽度过小！");
        return;
    }

    QString message = QString("您希望将打印宽度手动指定为 %1 厘米吗？\n\n"
                              "此操作会将光栅图最终输出尺寸修改为【%2 x %3】像素，\n"
                              "在修改其他参数前，该尺寸将保持不变。")
                          .arg(QString::number(userInputWidth, 'f', 2))
                          .arg(targetPixelSize.width()).arg(targetPixelSize.height());

    if (QMessageBox::question(this, "尺寸修改确认", message, "确认修改", "取消") == 0) {
        // 用户确认，进入手动模式
        currentSizeMode = SizeMode::ManualOverride;
        manualPrintWidthCm = userInputWidth;
        qDebug() << "用户手动锁定尺寸为:" << manualPrintWidthCm << "cm";
        schedulePreviewUpdate();
    } else {
        // 用户取消，恢复
        schedulePreviewUpdate();
    }
}

void MainWindow::saveFinalImage()
{
    qDebug().noquote() << "\n\n=======================================================";
    qDebug().noquote() << "--- 开始执行 saveFinalImage ---";
    qDebug() << "时间戳:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 基本检查
    if (imagePaths.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先导入至少一张图像！");
        return;
    }

    // 收集所有参数
    QImage firstImage(imagePaths.first());
    if(firstImage.isNull()) {
        QMessageBox::critical(this, "错误", "无法加载第一张图像以获取尺寸信息。");
        return;
    }

    int frameCount = imagePaths.size();
    QString direction = verticalRadio->isChecked() ? "纵向" : "横向";
    int sliceWidth = sliceWidthSpinBox->value();

    double desiredSize = desiredPrintSizeSpinBox->value();

    // 确定最终尺寸
    QSize finalImageSize;
    if (currentSizeMode == SizeMode::ManualOverride) {
        finalImageSize = calculateTargetPixels(manualPrintWidthCm, firstImage.size());
    } else {
        finalImageSize = firstImage.size();
    }

    if (finalImageSize.width() < 1 || finalImageSize.height() < 1) {
        QMessageBox::critical(this, "尺寸错误", "计算出的最终图像尺寸无效（小于1像素），请检查参数。");
        return;
    }

    QSizeF finalPhysicalSize = calculatePhysicalSize(finalImageSize);
    double requiredDpi = calculateRequiredDPI();

    // 创建并显示报告单
    QString reportText = QString(
                             "请确认以下参数是否正确:"
                             "\n\n"
                             "帧数量: %1 \n"
                             "切分方向: %2 \n"
                             "切片宽度: %3 像素 \n"
                             "\n"
                             "输出图像尺寸: %4 x %5 像素 \n"
                             "物理打印尺寸: %6 x %7 厘米 \n"
                             "打印机精度要求: %8 DPI \n"
                             ).arg(imagePaths.size())
                             .arg(verticalRadio->isChecked() ? "纵向" : "横向")
                             .arg(sliceWidthSpinBox->value())
                             .arg(finalImageSize.width())
                             .arg(finalImageSize.height())
                             .arg(QString::number(finalPhysicalSize.width(), 'f', 2))
                             .arg(QString::number(finalPhysicalSize.height(), 'f', 2))
                             .arg(static_cast<int>(round(requiredDpi)));

    if (QMessageBox::question(this, "最终确认", reportText, "生成", "取消") == 1) { // 1是“取消”
        return;
    }

    // 超大图像警告
    const int largeImageThreshold = 20000;
    if (finalImageSize.width() > largeImageThreshold || finalImageSize.height() > largeImageThreshold) {
        QString warningText = QString(
                                  "<b>警告：即将尝试生成超大尺寸图像 (%1x%2)！</b>"
                                  "<p>处理此尺寸的图像需要大量系统资源和时间，并可能导致程序短暂无响应。</p>"
                                  "<p>处理时间取决于此设备的性能。</p>"
                                  "<ul>"
                                  "<li>请确保您已保存其他应用正在处理的工作。</li>"
                                  "<li>处理过程中请勿关闭程序。</li>"
                                  "</ul>"
                                  "<p>是否仍要继续？</p>"
                                  ).arg(finalImageSize.width()).arg(finalImageSize.height());

        if (QMessageBox::warning(this, "警告", warningText, "继续生成", "取消") == 1) {
            return;
        }
    }

    // 获取保存路径并执行最终的后台处理
    QString savePath = QFileDialog::getSaveFileName(this, "保存光栅图像", "", "PNG图像 (*.png)");
    if (savePath.isEmpty()) return;

    // --- 核心处理阶段 ---

    QList<QFile*> tempFiles;

    try
    {
        QTemporaryDir tempDir;
        if (!tempDir.isValid()) throw std::runtime_error("无法创建用于处理图像的临时目录。");
        qDebug() << "使用临时目录:" << tempDir.path();

        QProgressDialog progress("正在处理...", "取消", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(0);

        // --- 阶段一: 预处理并保存为临时文件 ---
        QList<QString> tempImagePaths;
        progress.setLabelText(QString("正在处理1/2: 预处理源图像 (共 %1 张)").arg(imagePaths.size()));
        for (int i = 0; i < imagePaths.size(); ++i) {
            progress.setValue(static_cast<int>((i * 1.0 / imagePaths.size()) * 50.0));
            QApplication::processEvents();
            if (progress.wasCanceled()) return;
            {
                QImage originalImg(imagePaths[i]);
                if (originalImg.isNull()) throw std::runtime_error("无法加载源文件。");

                QImage scaledImg = originalImg.scaled(finalImageSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                       .convertToFormat(QImage::Format_ARGB32);
                if (scaledImg.isNull()) throw std::runtime_error("在缩放图像时内存不足。");

                QString tempPath = tempDir.path() + QString("/scaled_%1.raw").arg(i);
                QFile tempFile(tempPath);
                if (!tempFile.open(QIODevice::WriteOnly)) throw std::runtime_error("无法创建临时文件。");

                tempFile.write(reinterpret_cast<const char*>(scaledImg.bits()), scaledImg.sizeInBytes());
                tempFile.close();
                tempImagePaths.append(tempPath);
            }
        }

        // --- 阶段二: 从临时文件合成 ---
        progress.setLabelText(QString("正在处理2/2: 合成最终图像..."));

        QImage resultImage(finalImageSize, QImage::Format_ARGB32);
        if (resultImage.isNull()) throw std::bad_alloc();

        // 一次性打开所有临时文件以供读取，并将文件指针存入列表
        for(const QString& path : tempImagePaths) {
            QFile* file = new QFile(path);
            if (!file->open(QIODevice::ReadOnly)) {
                throw std::runtime_error("无法打开预处理后的临时文件。");
            }
            tempFiles.append(file);
        }

        const qint64 bytesPerLine = finalImageSize.width() * 4;

        // 逐行处理
        for (int y = 0; y < finalImageSize.height(); ++y) {
            progress.setValue(50 + static_cast<int>((y * 1.0 / finalImageSize.height()) * 50.0));
            if(y % 16 == 0) QApplication::processEvents();
            if (progress.wasCanceled()) {
                qDeleteAll(tempFiles);
                return;
            }

            // 为当前行，从每个临时文件中读取对应的行数据
            QList<QByteArray> sourceScanlines;
            for(QFile* file : tempFiles) {
                file->seek(y * bytesPerLine); // 直接跳转到行开头，极快
                sourceScanlines.append(file->read(bytesPerLine));
            }

            // 调用合成算法
            generateLenticularStrip(resultImage, sourceScanlines, y, 1, verticalRadio->isChecked(), sliceWidthSpinBox->value());
        }

        // 清理文件句柄
        for(auto file : tempFiles) file->close();
        qDeleteAll(tempFiles);

        // 保存最终结果
        progress.setValue(100);
        if (!resultImage.save(savePath, "PNG", 80)) {
            throw std::runtime_error("保存最终文件失败！请检查路径或权限。");
        }

        QMessageBox::information(this, "成功", QString("图像已成功保存至:\n%1").arg(savePath));
    }
    catch (const std::exception &e)
    {
        // 确保即使发生异常，也关闭并清理已打开的文件
        for(auto file : tempFiles) file->close();
        qDeleteAll(tempFiles);
        QMessageBox::critical(this, "处理出错", e.what());
    }
}




// ===================================================================
//          列表操作函数
// ===================================================================

void MainWindow::deleteSelectedImage()
{
    if (imageListWidget->selectedItems().isEmpty() || imagePaths.isEmpty()) return;

    // 在操作前，记录下当前第一张图的尺寸
    QImageReader oldFirstImageReader(imagePaths.first());
    const QSize oldFirstImageSize = oldFirstImageReader.size();

    QList<int> rowsToDelete;
    for(auto item : imageListWidget->selectedItems()) {
        rowsToDelete.append(imageListWidget->row(item));
    }

    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());
    int selectionAnchor = rowsToDelete.last();
    for(int row : rowsToDelete) {
        imagePaths.removeAt(row);
    }

    updateImageList();
    int newCount = imagePaths.count();
    if (newCount > 0) {
        if (selectionAnchor >= newCount) {
            selectionAnchor = newCount - 1;
        }
        imageListWidget->setCurrentRow(selectionAnchor);
    }

    // 在操作后，判断第一张图的尺寸是否变化
    if (imagePaths.isEmpty()) {
        // 如果列表被清空，执行一次标准的预览更新
        schedulePreviewUpdate();
    } else {
        QImageReader newFirstImageReader(imagePaths.first());
        const QSize newFirstImageSize = newFirstImageReader.size();

        // 如果新旧第一张图尺寸不同，重置打印参数
        if (newFirstImageSize != oldFirstImageSize) {
            qDebug() << "第一张图像尺寸已改变，触发核心参数重置。";
            onCoreParametersChanged(true);
        } else {
            // 如果尺寸未变，则只更新预览即可
            schedulePreviewUpdate();
        }
    }
}

void MainWindow::moveImageUp()
{
    int currentIndex = imageListWidget->currentRow();
    if (currentIndex <= 0 || imagePaths.isEmpty()) return;

    QImageReader oldFirstImageReader(imagePaths.first());
    const QSize oldFirstImageSize = oldFirstImageReader.size();

    imagePaths.swapItemsAt(currentIndex, currentIndex - 1);
    updateImageList();
    imageListWidget->setCurrentRow(currentIndex - 1);

    QImageReader newFirstImageReader(imagePaths.first());
    const QSize newFirstImageSize = newFirstImageReader.size();

    if (newFirstImageSize != oldFirstImageSize) {
        qDebug() << "第一张图像尺寸已改变，触发核心参数重置。";
        onCoreParametersChanged(true);
    } else {
        schedulePreviewUpdate();
    }
}

void MainWindow::moveImageDown()
{
    int currentIndex = imageListWidget->currentRow();
    if (currentIndex == -1 || currentIndex >= imagePaths.count() - 1 || imagePaths.isEmpty()) return;

    QImageReader oldFirstImageReader(imagePaths.first());
    const QSize oldFirstImageSize = oldFirstImageReader.size();

    imagePaths.swapItemsAt(currentIndex, currentIndex + 1);
    updateImageList();
    imageListWidget->setCurrentRow(currentIndex + 1);

    QImageReader newFirstImageReader(imagePaths.first());
    const QSize newFirstImageSize = newFirstImageReader.size();

    if (newFirstImageSize != oldFirstImageSize) {
        qDebug() << "第一张图像尺寸已改变，触发核心参数重置。";
        onCoreParametersChanged(true);
    } else {
        schedulePreviewUpdate();
    }
}

// ===================================================================
//          核心辅助与计算函数
// ===================================================================

QSize MainWindow::calculateTargetPixels(double physical_width_cm, const QSize& original_image_size)
{
    if (physical_width_cm <= 0 || original_image_size.isEmpty() || imagePaths.isEmpty()) return QSize(0, 0);

    double aspectRatio = static_cast<double>(original_image_size.height()) / original_image_size.width();
    double calLpi = calibratedLpiSpinBox->value();
    int sliceWidth = sliceWidthSpinBox->value();
    int numFrames = imagePaths.size();

    double total_pixels_per_lenticule = sliceWidth * numFrames;
    double required_print_dpi = total_pixels_per_lenticule * calLpi;
    double physical_size_inch = physical_width_cm / 2.54;
    double calculated_pixels_w = physical_size_inch * required_print_dpi;
    double calculated_pixels_h = calculated_pixels_w * aspectRatio;

    return QSize(static_cast<int>(round(calculated_pixels_w)), static_cast<int>(round(calculated_pixels_h)));
}

QSizeF MainWindow::calculatePhysicalSize(const QSize& current_pixel_size)
{
    if (current_pixel_size.isEmpty() || imagePaths.isEmpty()) return QSizeF(0.0, 0.0);

    double aspectRatio = static_cast<double>(current_pixel_size.height()) / current_pixel_size.width();
    double total_pixels_w = current_pixel_size.width();
    double calLpi = calibratedLpiSpinBox->value();
    int sliceWidth = sliceWidthSpinBox->value();
    int numFrames = imagePaths.size();

    double total_pixels_per_lenticule = sliceWidth * numFrames;
    double required_print_dpi = total_pixels_per_lenticule * calLpi;

    if (required_print_dpi <= 0) return QSizeF(0.0, 0.0);

    double physical_size_inch_w = total_pixels_w / required_print_dpi;
    double physical_size_cm_w = physical_size_inch_w * 2.54;
    double physical_size_cm_h = physical_size_cm_w * aspectRatio;

    return QSizeF(physical_size_cm_w, physical_size_cm_h);
}

double MainWindow::calculateRequiredDPI()
{
    if (imagePaths.isEmpty()) {
        return 0.0;
    }

    double calLpi = calibratedLpiSpinBox->value();
    int sliceWidth = sliceWidthSpinBox->value();
    int numFrames = imagePaths.size();
    // 每个光栅单元下的总像素数
    double total_pixels_per_lenticule = sliceWidth * numFrames;

    return total_pixels_per_lenticule * calLpi;
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
    if (imagePaths.isEmpty()) {
        previewLabel->setText("请导入图像以开始...");
        previewLabel->setPixmap(QPixmap());
        return;
    }

    QImage firstImage(imagePaths.first());
    if(firstImage.isNull()) return;

    QSize finalPixelSize;
    double displayPhysicalWidth;

    // 决定最终的像素尺寸和要显示的物理宽度
    if (currentSizeMode == SizeMode::ManualOverride) {
        // 手动模式
        displayPhysicalWidth = manualPrintWidthCm;
        finalPixelSize = calculateTargetPixels(displayPhysicalWidth, firstImage.size());
    } else {
        // 自动模式
        finalPixelSize = firstImage.size();
        displayPhysicalWidth = calculatePhysicalSize(finalPixelSize).width();
    }

    // 更新参数显示
    outputPixelSizeLabel->setText(QString("%1 x %2").arg(finalPixelSize.width()).arg(finalPixelSize.height()));
    desiredPrintSizeSpinBox->blockSignals(true);
    desiredPrintSizeSpinBox->setValue(displayPhysicalWidth);
    desiredPrintSizeSpinBox->blockSignals(false);

    // 生成预览缩略图
    const int previewSize = 400;
    QSize previewTargetSize = finalPixelSize;
    previewTargetSize.scale(previewSize, previewSize, Qt::KeepAspectRatio);

    QList<QImage> previewThumbnails;
    for(const QString& path : imagePaths) {
        QImage img(path);
        if(!img.isNull()){
            // 所有缩略图基于统一的目标尺寸生成，保证一致性
            previewThumbnails.append(img.scaled(previewTargetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }
    if (previewThumbnails.isEmpty()) return;

    // 合成并显示预览
    QImage previewImage = generateLenticularPreview(previewThumbnails, verticalRadio->isChecked(), sliceWidthSpinBox->value());
    if(!previewImage.isNull()){
        previewLabel->setPixmap(QPixmap::fromImage(previewImage));
    }
}

QImage MainWindow::generateLenticularPreview(const QList<QImage>& thumbnailImages, bool isVertical, int sliceWidth)
{
    if (thumbnailImages.isEmpty() || sliceWidth <= 0) return QImage();

    const int numFrames = thumbnailImages.size();
    const QSize targetSize = thumbnailImages.first().size();

    QList<QImage> sourceImages;
    for (const QImage& originalImg : thumbnailImages) {
        sourceImages.append(originalImg.convertToFormat(QImage::Format_ARGB32));
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
                memcpy(resultLine + x, sourceLines[sourceImageIndex] + x * 4, 4);
            }
        }
    } else { // 横向切分
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

void MainWindow::generateLenticularStrip(QImage& resultImage, const QList<QByteArray>& sourceScanlines, int y, int stripHeight, bool isVertical, int sliceWidth)
{
    Q_UNUSED(stripHeight); // 在逐行模型中，此参数固定为1，因此标记为未使用
    if (sourceScanlines.isEmpty() || sliceWidth <= 0) return;

    const int numFrames = sourceScanlines.size();
    const int width = resultImage.width();
    const int bytesPerPixel = 4; // ARGB32格式
    uchar* resultLine = resultImage.scanLine(y);

    if (isVertical) {
        for (int x = 0; x < width; ++x) {
            int sourceImageIndex = (x / sliceWidth) % numFrames;
            // 从对应的源数据行中，复制4个字节（一个像素）
            memcpy(resultLine + (x * bytesPerPixel), sourceScanlines[sourceImageIndex].constData() + (x * bytesPerPixel), bytesPerPixel);
        }
    } else { // 横向切分
        int sourceImageIndex = (y / sliceWidth) % numFrames;
        // 将一整行裸数据直接复制过去
        memcpy(resultLine, sourceScanlines[sourceImageIndex].constData(), resultImage.bytesPerLine());
    }
}

void MainWindow::onResetPrintSizeClicked()
{
    qDebug() << "用户点击重置，打印尺寸恢复为自动计算模式。";
    onCoreParametersChanged(false);
    schedulePreviewUpdate();
}

void MainWindow::onCoreParametersChanged(bool showResetDialog)
{
    if (imagePaths.isEmpty()) return;

    // 如果之前是手动模式，提示用户。
    if (showResetDialog && currentSizeMode == SizeMode::ManualOverride) {
        QMessageBox::information(this, "重置", "由于参数变更，打印宽度已重置为根据原始图像尺寸的自动计算值。");
    }

    // 切换回自动
    currentSizeMode = SizeMode::Automatic;
    manualPrintWidthCm = 0.0;

    qDebug() << "状态已重置为自动模式。";

    // UI更新
    schedulePreviewUpdate();
}

void MainWindow::onTestButtonClicked()
{
    qDebug() << "--- 开始执行测试 ---";
}


