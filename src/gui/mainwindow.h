#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QProgressBar>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QThread>
#include "core/secret_detector.h"

class ScanThread;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseClicked();
    void onOutputBrowseClicked();
    void onScanClicked();
    void onStopClicked();
    void onClearClicked();
    void onExportClicked();

    void onScanProgress(int current, int total);
    void onScanFinished(const ScanResult& result);
    void onScanError(const QString& error);
    void onResultTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void createCentralWidget();
    void createStatusBar();

    void loadResults(const ScanResult& result);
    void updateStatistics(const ScanStatistics& stats);

    void openInEditor(const QString& filePath, int lineNumber);
    QString findAvailableEditor();  // найти установленный редактор

    // UI Components
    QLineEdit* pathEdit;
    QPushButton* browseBtn;
    QPushButton* scanBtn;
    QPushButton* stopBtn;
    QPushButton* clearBtn;

    QLineEdit* outputEdit;
    QPushButton* outputBrowseBtn;

    QCheckBox* recursiveCheck;
    QCheckBox* respectGitignoreCheck;
    QCheckBox* strictModeCheck;

    QComboBox* formatCombo;
    QLineEdit* excludeEdit;
    QLineEdit* includeExtEdit;

    QTableWidget* resultsTable;
    QProgressBar* progressBar;
    QTextEdit* logText;

    QLabel* statsLabel;

    QAction* toolbarScanAction;
    QAction* toolbarStopAction;
    QAction* toolbarClearAction;

    // backend
    ScanThread* scanThread;
    SecretDetector detector;
    ScanResult lastResult;
    bool scanning;
};

// worker thread для сканирования
class ScanThread : public QThread {
    Q_OBJECT

public:
    ScanThread(SecretDetector* detector, const ScanOptions& options)
        : detector(detector), options(options) {}

signals:
    void progress(int current, int total);
    void finished(const ScanResult& result);
    void error(const QString& error);

protected:
    void run() override;

private:
    SecretDetector* detector;
    ScanOptions options;

    // UI Components
    QLineEdit* pathEdit;
    QPushButton* browseBtn;
    QPushButton* scanBtn;
    QPushButton* stopBtn;
    QPushButton* clearBtn;

    QAction* toolbarScanAction;
    QAction* toolbarStopAction;
    QAction* toolbarClearAction;
};

#endif // MAINWINDOW_H
