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
    void applyScanResult(const QString &result);
    CS_SLOT_2(applyScanResult)

    CS_SLOT_1(Private)
    void applyDuplicatesResult(const QString &result);
    CS_SLOT_2(applyDuplicatesResult)

private:
    void runScan(const QString &dir);
    void runDuplicates(const QString &dir);

    QTabWidget *tabs;

    // Scanner Tab
    QWidget *scannerTab;
    QLineEdit *scanPathEdit;
    QPushButton *scanBtn;
    QLabel *scanResultLbl;

    // Duplicates Tab
    QWidget *dupesTab;
    QLineEdit *dupesPathEdit;
    QPushButton *dupesBtn;
    QLabel *dupesResultLbl;
};
