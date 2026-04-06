#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class DemoWindow : public QWidget {
    CS_OBJECT(DemoWindow)
public:
    explicit DemoWindow(QWidget *parent = nullptr);

    CS_SLOT_1(Private)
    void onScanClicked();
    CS_SLOT_2(onScanClicked)

    CS_SLOT_1(Private)
    void onDuplicatesClicked();
    CS_SLOT_2(onDuplicatesClicked)

    CS_SLOT_1(Private)
    void onStatsClicked();
    CS_SLOT_2(onStatsClicked)

    CS_SLOT_1(Private)
    void onHashClicked();
    CS_SLOT_2(onHashClicked)

    CS_SLOT_1(Private)
    void onMetadataClicked();
    CS_SLOT_2(onMetadataClicked)

    CS_SLOT_1(Private)
    void onLintClicked();
    CS_SLOT_2(onLintClicked)

    CS_SLOT_1(Private)
    void applyScanResult(const QString &result);
    CS_SLOT_2(applyScanResult)

    CS_SLOT_1(Private)
    void applyDuplicatesResult(const QString &result);
    CS_SLOT_2(applyDuplicatesResult)

    CS_SLOT_1(Private)
    void applyStatsResult(const QString &result);
    CS_SLOT_2(applyStatsResult)

    CS_SLOT_1(Private)
    void applyHashResult(const QString &result);
    CS_SLOT_2(applyHashResult)

    CS_SLOT_1(Private)
    void applyMetadataResult(const QString &result);
    CS_SLOT_2(applyMetadataResult)

    CS_SLOT_1(Private)
    void applyLintResult(const QString &result);
    CS_SLOT_2(applyLintResult)

private:
    void runScan(const QString &dir);
    void runDuplicates(const QString &dir);
    void runStats(const QString &dir);
    void runHash(const QString &filePath);
    void runMetadata(const QString &dir);
    void runLint(const QString &dir);

    QTabWidget *tabs;

    QWidget *scannerTab;
    QLineEdit *scanPathEdit;
    QPushButton *scanBtn;
    QLabel *scanResultLbl;

    QWidget *dupesTab;
    QLineEdit *dupesPathEdit;
    QPushButton *dupesBtn;
    QLabel *dupesResultLbl;

    QWidget *statsTab;
    QLineEdit *statsPathEdit;
    QPushButton *statsBtn;
    QLabel *statsResultLbl;

    QWidget *hashTab;
    QLineEdit *hashPathEdit;
    QPushButton *hashBtn;
    QLabel *hashResultLbl;

    QWidget *metadataTab;
    QLineEdit *metadataPathEdit;
    QPushButton *metadataBtn;
    QLabel *metadataResultLbl;

    QWidget *lintTab;
    QLineEdit *lintPathEdit;
    QPushButton *lintBtn;
    QLabel *lintResultLbl;
};
