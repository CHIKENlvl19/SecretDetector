#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>
#include <QSplitter>
#include <QProcess>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QFileInfo>
#include <QUrl>
#include <QSettings>
#include <QDialog>
#include <QListWidget>
#include <QDialogButtonBox>
#include "utils/export_manager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scanThread(nullptr), scanning(false) {
    
    setWindowTitle("Secret Detector");
    resize(1200, 800);
    
    // инициализация настроек
    settings = new QSettings("SecretDetector", "GUI", this);

    setupUI();

    // excludeEdit->setText("build, config, .git, node_modules, __pycache__, .venv");

    excludeEdit->setText("build, config, .git, node_modules, __pycache__, .venv, .pdf, .mp4, .djvu, .docx, .xlsx, .pptx, .odt, .zip, .tar, .gz, .rar, .7z, .png, .jpg, .jpeg, .gif, .bmp, .mp3, .wav, .avi, .mkv, .iso");

    // excludeEdit->setText("build, config, .git, node_modules, __pycache__, .venv, pdf, mp4,
    //     djvu, docx, xlsx, pptx, odt, zip, tar, gz, rar, 7z, png, jpg, jpeg, gif, bmp, mp3, wav, avi, mkv");

    createMenuBar();
    createToolBar();
    createStatusBar();
    
    // попробовать несколько путей
    std::vector<std::string> config_paths = {
        "/opt/secret-detector/config/patterns.json"
    };
    
    bool loaded = false;
    for (const auto& path : config_paths) {
        if (detector.initialize(path)) {
            logText->append(QString("[INFO] Loaded config from: %1").arg(QString::fromStdString(path)));
            loaded = true;
            break;
        }
    }
    
    if (!loaded) {
        // использовать дефолтные паттерны
        detector.initialize("");
        logText->append("[WARNING] Using default patterns");
    }
}

MainWindow::~MainWindow() {

}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    
    // секция выбора пути
    QGroupBox* pathGroup = new QGroupBox("Scan Target", this);
    QHBoxLayout* pathLayout = new QHBoxLayout(pathGroup);
    
    pathEdit = new QLineEdit(this);
    pathEdit->setPlaceholderText("Select file or directory to scan...");
    
    browseBtn = new QPushButton("Browse...", this);
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseClicked);
    
    pathLayout->addWidget(new QLabel("Path:", this));
    pathLayout->addWidget(pathEdit, 1);
    pathLayout->addWidget(browseBtn);
    
    mainLayout->addWidget(pathGroup);
    
    // секция опций
    QGroupBox* optionsGroup = new QGroupBox("Scan Options", this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    // чекбоксы
    QHBoxLayout* checkLayout = new QHBoxLayout();
    recursiveCheck = new QCheckBox("Recursive scan", this);
    recursiveCheck->setChecked(true);
    
    respectGitignoreCheck = new QCheckBox("Respect .gitignore", this);
    respectGitignoreCheck->setChecked(true);
    
    strictModeCheck = new QCheckBox("Strict mode", this);
    strictModeCheck->setToolTip("Fail on any match (not just CRITICAL)");
    
    checkLayout->addWidget(recursiveCheck);
    checkLayout->addWidget(respectGitignoreCheck);
    checkLayout->addWidget(strictModeCheck);
    checkLayout->addStretch();
    
    optionsLayout->addLayout(checkLayout);
    
    // фильтры
    QHBoxLayout* filterLayout = new QHBoxLayout();
    
    excludeEdit = new QLineEdit(this);
    excludeEdit->setPlaceholderText("Exclude patterns (comma-separated): build, .git, node_modules");
    
    includeExtEdit = new QLineEdit(this);
    includeExtEdit->setPlaceholderText("Include extensions (comma-separated): cpp, h, py");
    
    filterLayout->addWidget(new QLabel("Exclude:", this));
    filterLayout->addWidget(excludeEdit, 1);
    filterLayout->addWidget(new QLabel("Include ext:", this));
    filterLayout->addWidget(includeExtEdit, 1);
    
    optionsLayout->addLayout(filterLayout);
    
    mainLayout->addWidget(optionsGroup);
    
    // секция вывода
    QGroupBox* outputGroup = new QGroupBox("Output Options", this);
    QHBoxLayout* outputLayout = new QHBoxLayout(outputGroup);
    
    outputEdit = new QLineEdit(this);
    outputEdit->setPlaceholderText("Output directory for reports (optional)");
    
    outputBrowseBtn = new QPushButton("Browse...", this);
    connect(outputBrowseBtn, &QPushButton::clicked, this, &MainWindow::onOutputBrowseClicked);
    
    formatCombo = new QComboBox(this);
    formatCombo->addItems({"Text", "JSON", "CSV", "HTML"});
    
    QComboBox* severityFilter = new QComboBox(this);
    severityFilter->addItems({"All", "CRITICAL", "HIGH", "MEDIUM", "LOW"});
    severityFilter->setToolTip("Filter results by severity");

    outputLayout->addWidget(new QLabel("Show:", this));
    outputLayout->addWidget(severityFilter);


    outputLayout->addWidget(new QLabel("Output:", this));
    outputLayout->addWidget(outputEdit, 1);
    outputLayout->addWidget(outputBrowseBtn);
    outputLayout->addWidget(new QLabel("Format:", this));
    outputLayout->addWidget(formatCombo);
    
    mainLayout->addWidget(outputGroup);
    
    // кнопки действий
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    scanBtn = new QPushButton("Start Scan", this);
    scanBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    connect(scanBtn, &QPushButton::clicked, this, &MainWindow::onScanClicked);
    
    stopBtn = new QPushButton("Stop", this);
    stopBtn->setEnabled(false);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    
    clearBtn = new QPushButton("Clear Results", this);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    
    QPushButton* exportBtn = new QPushButton("Export Report", this);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::onExportClicked);
    
    buttonLayout->addWidget(scanBtn);
    buttonLayout->addWidget(stopBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // progress Bar
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);
    
    // splitter с результатами и логом
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    
    // таблица результатов
    resultsTable = new QTableWidget(this);
    resultsTable->setColumnCount(6);
    resultsTable->setHorizontalHeaderLabels({"File", "Line", "Severity", "Pattern", "Match", "Preview"});
    resultsTable->horizontalHeader()->setStretchLastSection(true);
    resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTable->setAlternatingRowColors(true);
    resultsTable->setSortingEnabled(true);
    
    connect(resultsTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onResultTableDoubleClicked);

    splitter->addWidget(resultsTable);
    
    // лог
    logText = new QTextEdit(this);
    logText->setReadOnly(true);
    logText->setMaximumHeight(150);
    logText->setPlaceholderText("Scan logs will appear here...");
    
    splitter->addWidget(logText);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter, 1);
}

void MainWindow::createMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu("&File");
    
    QAction* openAction = fileMenu->addAction("&Open Folder...");
    connect(openAction, &QAction::triggered, this, &MainWindow::onBrowseClicked);
    
    QAction* exportAction = fileMenu->addAction("&Export Report...");
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportClicked);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    QMenu* settingsMenu = menuBar()->addMenu("&Settings");
    
    QAction* resetEditorAction = settingsMenu->addAction("&Change Default Editor...");
    connect(resetEditorAction, &QAction::triggered, this, &MainWindow::onResetEditorClicked);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QDialog* aboutDialog = new QDialog(this);
        aboutDialog->setWindowTitle("About Secret Detector");
        aboutDialog->setFixedSize(450, 250);
        
        QVBoxLayout* layout = new QVBoxLayout(aboutDialog);
        
        // заголовок
        QLabel* titleLabel = new QLabel("<h2>Secret Detector v1.0.0</h2>");
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);
        
        // описание
        QLabel* descLabel = new QLabel(
            "Find secrets in your code: API keys, tokens, passwords, and more.<br><br>"
            "Built with C++17 and Qt5<br><br>"
            "© 2025 Berdnikov Alexey. Licensed under the MIT License."
        );
        descLabel->setAlignment(Qt::AlignCenter);
        descLabel->setWordWrap(true);
        layout->addWidget(descLabel);
        
        // ссылка на GitHub
        QLabel* linkLabel = new QLabel(
            "<p><b>GitHub Repository:</b><br>"
            "<a href='https://github.com/CHIKENlvl19/SecretDetector'>"
            "https://github.com/CHIKENlvl19/SecretDetector</a></p>"
        );
        linkLabel->setAlignment(Qt::AlignCenter);
        linkLabel->setTextFormat(Qt::RichText);
        linkLabel->setOpenExternalLinks(true);  // автоматически открывать ссылки
        layout->addWidget(linkLabel);
        
        // Кнопка OK
        QPushButton* okButton = new QPushButton("OK");
        connect(okButton, &QPushButton::clicked, aboutDialog, &QDialog::accept);
        layout->addWidget(okButton, 0, Qt::AlignCenter);
        
        aboutDialog->exec();
    });
}

void MainWindow::createToolBar() {
    QToolBar* toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    
    toolbarScanAction = toolbar->addAction("▶ Scan");
    toolbarScanAction->setEnabled(true);  // Изначально включена
    connect(toolbarScanAction, &QAction::triggered, this, &MainWindow::onScanClicked);
    
    toolbarStopAction = toolbar->addAction("⏹ Stop");
    toolbarStopAction->setEnabled(false);  // Изначально отключена
    connect(toolbarStopAction, &QAction::triggered, this, &MainWindow::onStopClicked);
    
    toolbar->addSeparator();
    
    toolbarClearAction = toolbar->addAction("🗑 Clear");
    connect(toolbarClearAction, &QAction::triggered, this, &MainWindow::onClearClicked);
}


void MainWindow::createStatusBar() {
    statsLabel = new QLabel("Ready", this);
    statusBar()->addPermanentWidget(statsLabel);
    statusBar()->showMessage("Ready to scan");
}

void MainWindow::onBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory to Scan",
        pathEdit->text().isEmpty() ? QDir::homePath() : pathEdit->text());
    
    if (!dir.isEmpty()) {
        pathEdit->setText(dir);
    }
}

void MainWindow::onOutputBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory",
        outputEdit->text().isEmpty() ? QDir::homePath() : outputEdit->text());
    
    if (!dir.isEmpty()) {
        outputEdit->setText(dir);
    }
}

void MainWindow::onScanClicked() {
    if (pathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "No Path Selected", "Please select a file or directory to scan.");
        return;
    }
    
    // КРИТИЧЕСКАЯ ПРОВЕРКА - ПРЕДОТВРАЩАЕТ КРАШ, НЕ ДАЙ БОГ НЕ СРАБОТАЕТ, Я НЕ ЗНАЮ УЖЕ ЧТО С СОБОЙ СДЕЛАЮ
    if (scanThread) {
        if (scanThread->isRunning()) {
            QMessageBox::warning(this, "Scan in Progress", 
                "Please wait for the current scan to finish.");
            return;
        }
        // cтарый поток есть, но не запущен - обнулить обнуляй типа ыыы хвхахв
        scanThread = nullptr;
    }
    
    // подготовить опции
    ScanOptions options;
    options.scan_path = pathEdit->text().toStdString();
    options.recursive = recursiveCheck->isChecked();
    options.respect_gitignore = respectGitignoreCheck->isChecked();
    options.num_threads = 0;
    
    // exclude patterns
    QString excludeStr = excludeEdit->text().trimmed();
    if (!excludeStr.isEmpty()) {
        QStringList excludeList = excludeStr.split(',', Qt::SkipEmptyParts);
        for (const QString& pattern : excludeList) {
            options.exclude_patterns.push_back(pattern.trimmed().toStdString());
        }
    }
    
    // include extensions
    QString includeStr = includeExtEdit->text().trimmed();
    if (!includeStr.isEmpty()) {
        QStringList includeList = includeStr.split(',', Qt::SkipEmptyParts);
        for (const QString& ext : includeList) {
            options.include_extensions.push_back(ext.trimmed().toStdString());
        }
    }
    
    // ОТКЛЮЧИТЬ ВСЕ КНОПКИ СКАНИРОВАНИЯ
    scanning = true;
    scanBtn->setEnabled(false);
    stopBtn->setEnabled(true);
    
    // КРИТИЧЕСКИ ВАЖНО - ОТКЛЮЧИТЬ TOOLBAR КНОПКИ А ТО ОПЯТЬ КРАШНЕТСЯ ВСЁ НУ СКОКА МОЖНО
    toolbarScanAction->setEnabled(false);
    toolbarStopAction->setEnabled(true);
    
    progressBar->setVisible(true);
    progressBar->setMaximum(100);
    progressBar->setValue(0);
    
    logText->append("[INFO] Starting scan...");
    statusBar()->showMessage("Scanning...");
    
    scanThread = new ScanThread(&detector, options);
    connect(scanThread, &ScanThread::progress, this, &MainWindow::onScanProgress);
    connect(scanThread, &ScanThread::finished, this, &MainWindow::onScanFinished);
    connect(scanThread, &ScanThread::error, this, &MainWindow::onScanError);
    connect(scanThread, &QThread::finished, scanThread, &QObject::deleteLater);
    
    scanThread->start();
}


void MainWindow::onStopClicked() {
    if (scanThread && scanThread->isRunning()) {
        disconnect(scanThread, nullptr, this, nullptr);
        
        scanThread->quit();
        if (!scanThread->wait(1000)) {
            scanThread->terminate();
            scanThread->wait();
        }
        
        logText->append("[WARNING] Scan stopped by user");
        statusBar()->showMessage("Scan stopped");
        
        scanning = false;
        scanBtn->setEnabled(true);
        stopBtn->setEnabled(false);
        
        // ВКЛЮЧИТЬ TOOLBAR КНОПКИ А ТО Ж НЕ ВИДНО ИХ БУДЕТ
        toolbarScanAction->setEnabled(true);
        toolbarStopAction->setEnabled(false);
        
        progressBar->setVisible(false);
        
        scanThread = nullptr;
    }
}



void MainWindow::onClearClicked() {
    resultsTable->setRowCount(0);
    logText->clear();
    lastResult = ScanResult();
    statsLabel->setText("Ready");
    statusBar()->showMessage("Results cleared");
}

void MainWindow::onExportClicked() {
    if (lastResult.matches.empty()) {
        QMessageBox::information(this, "No Results", "No scan results to export.");
        return;
    }
    
    QString format = formatCombo->currentText().toLower();
    QString filter;
    QString defaultExt;
    
    if (format == "json") {
        filter = "JSON Files (*.json)";
        defaultExt = ".json";
    } else if (format == "csv") {
        filter = "CSV Files (*.csv)";
        defaultExt = ".csv";
    } else if (format == "html") {
        filter = "HTML Files (*.html)";
        defaultExt = ".html";
    } else {
        filter = "Text Files (*.txt)";
        defaultExt = ".txt";
    }
    
    // диалог сохранения
    QString fileName = QFileDialog::getSaveFileName(
        this, 
        "Export Report", 
        QDir::homePath() + "/secret_detector_report" + defaultExt,
        filter
    );
    
    if (fileName.isEmpty()) {
        return; // пользователь отменил
    }
    
    // убедиться что есть расширение
    if (!fileName.endsWith(defaultExt)) {
        fileName += defaultExt;
    }
    
    try {
        std::string filePathStr = fileName.toStdString();
        nlohmann::json data = lastResult.to_json();
        
        bool success = false;
        
        // экспорт в зависимости от формата (статические методы)
        if (format == "json") {
            success = ExportManager::exportToJson(data, filePathStr);
        } else if (format == "csv") {
            success = ExportManager::exportToCsv(data, filePathStr);
        } else if (format == "html") {
            success = ExportManager::exportToHtml(data, filePathStr);
        } else {
            success = ExportManager::exportToText(data, filePathStr);
        }
        
        if (success) {
            logText->append("[INFO] Exported to: " + fileName);
            statusBar()->showMessage("Report exported successfully", 3000);
            
            QMessageBox::information(this, "Export Successful", 
                QString("Report exported to:\n%1\n\nFormat: %2\nMatches: %3")
                .arg(fileName)
                .arg(format.toUpper())
                .arg(lastResult.matches.size()));
        } else {
            throw std::runtime_error("Export failed");
        }
        
    } catch (const std::exception& e) {
        logText->append(QString("[ERROR] Export failed: %1").arg(e.what()));
        QMessageBox::critical(this, "Export Failed", 
            QString("Failed to export report:\n%1").arg(e.what()));
    }
}



void MainWindow::onScanProgress(int current, int total) {
    int percent = (current * 100) / total;
    progressBar->setValue(percent);
    statusBar()->showMessage(QString("Scanning... %1/%2 files (%3%)")
        .arg(current).arg(total).arg(percent));
}

void MainWindow::onScanFinished(const ScanResult& result) {
    lastResult = result;
    scanning = false;
    
    // ВКЛЮЧИТЬ КНОПКИ ОБРАТНО НУ КОНЕЧНО МЫ ЖЕ ИХ ВЫРУБИЛИ
    scanBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    
    // КРИТИЧЕСКИ ВАЖНО - ВКЛЮЧИТЬ TOOLBAR КНОПКИ ААААААААААААААА
    toolbarScanAction->setEnabled(true);
    toolbarStopAction->setEnabled(false);
    
    progressBar->setVisible(false);
    
    logText->append(QString("[INFO] Scan completed in %1 seconds")
        .arg(result.statistics.scan_time_seconds, 0, 'f', 2));
    
    loadResults(result);
    updateStatistics(result.statistics);
    
    // КРИТИЧЕСКИ ВАЖНО - ОБНУЛИТЬ УКАЗАТЕЛЬ НУ ЭТО ТОЖЕ ТИПА ЛОМАЕТ 
    scanThread = nullptr;
    
    // показать уведомление посмотри тебе пришло важное сообщение
    if (result.has_critical) {
        QMessageBox::critical(this, "Critical Secrets Found",
            QString("Found %1 CRITICAL secrets!\nPlease review the results.")
            .arg(result.statistics.critical_count));
    } else if (result.statistics.total_matches_found > 0) {
        QMessageBox::warning(this, "Secrets Found",
            QString("Found %1 potential secrets.\nPlease review the results.")
            .arg(result.statistics.total_matches_found));
    } else {
        QMessageBox::information(this, "Scan Complete", "No secrets detected!");
    }
}


void MainWindow::onScanError(const QString& error) {
    scanning = false;
    
    scanBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    
    toolbarScanAction->setEnabled(true);
    toolbarStopAction->setEnabled(false);
    
    progressBar->setVisible(false);
    
    logText->append("[ERROR] " + error);
    QMessageBox::critical(this, "Scan Error", "Error during scan:\n" + error);
    
    scanThread = nullptr;
}

void MainWindow::loadResults(const ScanResult& result) {
    resultsTable->setRowCount(0);
    
    // ограничить до 1000 результатов для UI
    size_t max_display = 1000;
    size_t display_count = std::min(result.matches.size(), max_display);
    
    if (result.matches.size() > max_display) {
        logText->append(QString("[WARNING] Too many results (%1). Showing first %2")
            .arg(result.matches.size()).arg(max_display));
    }
    
    resultsTable->setUpdatesEnabled(false);  // отключить обновление для скорости, а то оператива дорогая щас
    
    for (size_t i = 0; i < display_count; ++i) {
        const Match& match = result.matches[i];
        
        int row = resultsTable->rowCount();
        resultsTable->insertRow(row);
        
        // безопасное создание items
        auto fileItem = new QTableWidgetItem(QString::fromStdString(match.file_path));
        auto lineItem = new QTableWidgetItem(QString::number(match.line_number));
        auto severityItem = new QTableWidgetItem(QString::fromStdString(match.severity));
        auto patternItem = new QTableWidgetItem(QString::fromStdString(match.pattern_name));
        auto matchItem = new QTableWidgetItem(QString::fromStdString(match.matched_text));
        auto previewItem = new QTableWidgetItem(QString::fromStdString(match.preview));
        
        // Цвет severity
        if (match.severity == "CRITICAL") {
            severityItem->setBackground(QColor(255, 200, 200));
            severityItem->setForeground(QColor(139, 0, 0));
        } else if (match.severity == "HIGH") {
            severityItem->setBackground(QColor(255, 230, 200));
            severityItem->setForeground(QColor(184, 92, 0));
        } else if (match.severity == "MEDIUM") {
            severityItem->setBackground(QColor(255, 255, 200));
        }
        
        resultsTable->setItem(row, 0, fileItem);
        resultsTable->setItem(row, 1, lineItem);
        resultsTable->setItem(row, 2, severityItem);
        resultsTable->setItem(row, 3, patternItem);
        resultsTable->setItem(row, 4, matchItem);
        resultsTable->setItem(row, 5, previewItem);
    }
    
    resultsTable->setUpdatesEnabled(true);  // включить обратно :)))))
    resultsTable->resizeColumnsToContents();
}


void MainWindow::updateStatistics(const ScanStatistics& stats) {
    QString statsText = QString("Files: %1 | Matches: %2 (🔴 %3, 🟠 %4, 🟡 %5, 🟢 %6) | Time: %7s")
        .arg(stats.total_files_scanned)
        .arg(stats.total_matches_found)
        .arg(stats.critical_count)
        .arg(stats.high_count)
        .arg(stats.medium_count)
        .arg(stats.low_count)
        .arg(stats.scan_time_seconds, 0, 'f', 2);
    
    statsLabel->setText(statsText);
    statusBar()->showMessage("Scan complete");
}

// обработчик двойного клика на результат
void MainWindow::onResultTableDoubleClicked(int row, int column) {
    Q_UNUSED(column);  // не используем колонку
    
    if (row < 0 || row >= resultsTable->rowCount()) {
        return;
    }
    
    // gолучить путь к файлу и номер строки
    QTableWidgetItem* fileItem = resultsTable->item(row, 0);  // колонка File
    QTableWidgetItem* lineItem = resultsTable->item(row, 1);  // колонка Line
    
    if (!fileItem || !lineItem) {
        return;
    }
    
    QString filePath = fileItem->text();
    int lineNumber = lineItem->text().toInt();
    
    // проверить что файл существует
    if (!QFile::exists(filePath)) {
        logText->append(QString("[ERROR] File not found: %1").arg(filePath));
        QMessageBox::warning(this, "File Not Found",
                           QString("The file does not exist:\n%1").arg(filePath));
        return;
    }
    
    // открыть в редакторе
    openInEditor(filePath, lineNumber);
}

// открыть файл в текстовом редакторе
void MainWindow::openInEditor(const QString& filePath, int lineNumber) {
    QString editor = getPreferredEditor();
    
    // если редактор не выбран или не существует - показать диалог
    if (editor.isEmpty() || !QFile::exists(editor)) {
        if (!editor.isEmpty()) {
            logText->append(QString("[WARNING] Previously selected editor not found: %1").arg(editor));
        }
        
        editor = showEditorSelectionDialog();
        
        if (editor.isEmpty()) {
            // пользователь отменил выбор
            return;
        }
    }
    
    // запустить редактор с переходом на строку
    QStringList args;
    QFileInfo editorInfo(editor);
    QString editorName = editorInfo.fileName();
    
    if (editorName.contains("code")) {
        args << "--goto" << QString("%1:%2:1").arg(filePath).arg(lineNumber);
    } else if (editorName.contains("gedit")) {
        args << QString("+%1").arg(lineNumber) << filePath;
    } else if (editorName.contains("kate")) {
        args << filePath << "-l" << QString::number(lineNumber);
    } else if (editorName.contains("subl")) {
        args << QString("%1:%2").arg(filePath).arg(lineNumber);
    } else if (editorName.contains("vim") || editorName.contains("nvim")) {
        args << QString("+%1").arg(lineNumber) << filePath;
    } else if (editorName.contains("nano")) {
        args << QString("+%1").arg(lineNumber) << filePath;
    } else if (editorName.contains("emacs")) {
        args << QString("+%1").arg(lineNumber) << filePath;
    } else {
        args << filePath;
    }
    
    bool success = QProcess::startDetached(editor, args);
    
    if (success) {
        logText->append(QString("[INFO] Opened in %1: %2:%3")
                        .arg(editorName)
                        .arg(filePath)
                        .arg(lineNumber));
        statusBar()->showMessage(QString("Opened: %1 (line %2)")
                                .arg(QFileInfo(filePath).fileName())
                                .arg(lineNumber), 3000);
    } else {
        QMessageBox::warning(this, "Cannot Launch Editor",
                           QString("Failed to launch: %1\n\nWould you like to select a different editor?").arg(editorName));
        // сбросить редактор и попробовать снова
        setPreferredEditor("");
        openInEditor(filePath, lineNumber);
    }
}


// получить сохраненный редактор
QString MainWindow::getPreferredEditor() {
    return settings->value("editor/preferred", "").toString();
}

// сохранить выбранный редактор
void MainWindow::setPreferredEditor(const QString& editorPath) {
    settings->setValue("editor/preferred", editorPath);
}

// получить список доступных редакторов
QList<QPair<QString, QString>> MainWindow::getAvailableEditors() {
    QList<QPair<QString, QString>> editors;
    
    // список известных редакторов <имя, команда>
    QStringList editorCommands = {
        "code", "gedit", "kate", "subl", "nvim", "vim", 
        "nano", "emacs", "mousepad", "pluma", "geany"
    };
    
    QStringList editorNames = {
        "Visual Studio Code", "Gedit", "Kate", "Sublime Text", 
        "Neovim", "Vim", "Nano", "Emacs", "Mousepad", "Pluma", "Geany"
    };
    
    for (int i = 0; i < editorCommands.size(); ++i) {
        QString fullPath = QStandardPaths::findExecutable(editorCommands[i]);
        if (!fullPath.isEmpty()) {
            editors.append(qMakePair(editorNames[i], fullPath));
        }
    }
    
    return editors;
}

// диалог выбора редактора
QString MainWindow::showEditorSelectionDialog(bool showRememberCheckbox) {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Select Code Editor");
    dialog->setMinimumWidth(500);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    // заголовок
    QLabel* titleLabel = new QLabel("<h3>Choose Your Preferred Code Editor</h3>");
    layout->addWidget(titleLabel);
    
    QLabel* descLabel = new QLabel("Select an editor to open files when you double-click on results:");
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);
    
    // список доступных редакторов
    QListWidget* editorList = new QListWidget(dialog);
    editorList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    QList<QPair<QString, QString>> availableEditors = getAvailableEditors();
    
    if (availableEditors.isEmpty()) {
        QLabel* noEditorsLabel = new QLabel(
            "<p style='color: red;'>No supported text editors found!</p>"
            "<p>Please install one of the following:</p>"
            "<ul>"
            "<li>Visual Studio Code (code)</li>"
            "<li>Gedit (gedit)</li>"
            "<li>Kate (kate)</li>"
            "<li>Sublime Text (subl)</li>"
            "<li>Vim/Neovim (vim/nvim)</li>"
            "</ul>"
        );
        noEditorsLabel->setWordWrap(true);
        layout->addWidget(noEditorsLabel);
        
        QPushButton* okButton = new QPushButton("OK", dialog);
        connect(okButton, &QPushButton::clicked, dialog, &QDialog::reject);
        layout->addWidget(okButton);
        
        dialog->exec();
        delete dialog;
        return QString();
    }
    
    for (const auto& editor : availableEditors) {
        QListWidgetItem* item = new QListWidgetItem(
            QString("%1\n   %2").arg(editor.first).arg(editor.second)
        );
        item->setData(Qt::UserRole, editor.second);  // сохранить путь
        editorList->addItem(item);
    }
    
    editorList->setCurrentRow(0);
    layout->addWidget(editorList);
    
    // чекбокс "Запомнить выбор"
    QCheckBox* rememberCheck = nullptr;
    if (showRememberCheckbox) {
        rememberCheck = new QCheckBox("Remember my choice (can be changed in Settings menu)", dialog);
        rememberCheck->setChecked(true);
        layout->addWidget(rememberCheck);
    }
    
    // кнопка "Открыть с помощью другого..."
    QPushButton* browseButton = new QPushButton("Browse for Editor...", dialog);
    connect(browseButton, &QPushButton::clicked, [&]() {
        QString customEditor = QFileDialog::getOpenFileName(
            dialog,
            "Select Code Editor Executable",
            "/usr/bin",
            "All Files (*)"
        );
        
        if (!customEditor.isEmpty()) {
            QListWidgetItem* customItem = new QListWidgetItem(
                QString("Custom: %1").arg(QFileInfo(customEditor).fileName())
            );
            customItem->setData(Qt::UserRole, customEditor);
            editorList->addItem(customItem);
            editorList->setCurrentItem(customItem);
        }
    });
    layout->addWidget(browseButton);
    
    // кнопки OK/Cancel
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        dialog
    );
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    // двойной клик = OK
    connect(editorList, &QListWidget::itemDoubleClicked, dialog, &QDialog::accept);
    
    int result = dialog->exec();
    
    QString selectedEditor;
    if (result == QDialog::Accepted && editorList->currentItem()) {
        selectedEditor = editorList->currentItem()->data(Qt::UserRole).toString();
        
        // сохранить выбор если включен чекбокс
        if (!showRememberCheckbox || (rememberCheck && rememberCheck->isChecked())) {
            setPreferredEditor(selectedEditor);
            logText->append(QString("[INFO] Default editor set to: %1").arg(selectedEditor));
        }
    }
    
    delete dialog;
    return selectedEditor;
}

// сброс редактора
void MainWindow::onResetEditorClicked() {
    QString currentEditor = getPreferredEditor();
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Change Default Editor");
    msgBox.setText("Current default editor:");
    msgBox.setInformativeText(currentEditor.isEmpty() ? 
        "Not set" : 
        QFileInfo(currentEditor).fileName() + "\n" + currentEditor);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.button(QMessageBox::Ok)->setText("Change Editor");
    
    if (msgBox.exec() == QMessageBox::Ok) {
        QString newEditor = showEditorSelectionDialog(true);
        if (!newEditor.isEmpty()) {
            statusBar()->showMessage(QString("Editor changed to: %1")
                .arg(QFileInfo(newEditor).fileName()), 3000);
        }
    }
}

// ScanThread Implementation

void ScanThread::run() {
    try {
        // Setup progress callback
        detector->setProgressCallback([this](size_t current, size_t total) {
            emit progress(current, total);
        });
        
        ScanResult result = detector->scan(options);
        emit finished(result);
        
    } catch (const std::exception& e) {
        emit error(QString::fromStdString(e.what()));
    }
}
