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

    CS_SLOT_1(Private)
    void onSearchClicked();
    CS_SLOT_2(onSearchClicked)

    CS_SLOT_1(Private)
    void onFlowListClicked();
    CS_SLOT_2(onFlowListClicked)

    CS_SLOT_1(Private)
    void onScrubClicked();
    CS_SLOT_2(onScrubClicked)

    CS_SLOT_1(Private)
    void applySearchResult(const QString &result);
    CS_SLOT_2(applySearchResult)

    CS_SLOT_1(Private)
    void applyFlowResult(const QString &result);
    CS_SLOT_2(applyFlowResult)

    CS_SLOT_1(Private)
    void applyScrubResult(const QString &result);
    CS_SLOT_2(applyScrubResult)

    CS_SLOT_1(Private)
    void onExportClicked();
    CS_SLOT_2(onExportClicked)

    CS_SLOT_1(Private)
    void onCountClicked();
    CS_SLOT_2(onCountClicked)

    CS_SLOT_1(Private)
    void applyExportResult(const QString &result);
    CS_SLOT_2(applyExportResult)

    CS_SLOT_1(Private)
    void applyCountResult(const QString &result);
    CS_SLOT_2(applyCountResult)

    CS_SLOT_1(Private)
    void onHistoryClicked();
    CS_SLOT_2(onHistoryClicked)

    CS_SLOT_1(Private)
    void onIgnoreListClicked();
    CS_SLOT_2(onIgnoreListClicked)

    CS_SLOT_1(Private)
    void onIgnoreAddClicked();
    CS_SLOT_2(onIgnoreAddClicked)

    CS_SLOT_1(Private)
    void onOrganizeClicked();
    CS_SLOT_2(onOrganizeClicked)

    CS_SLOT_1(Private)
    void onDeleteDupesClicked();
    CS_SLOT_2(onDeleteDupesClicked)

    CS_SLOT_1(Private)
    void applyHistoryResult(const QString &result);
    CS_SLOT_2(applyHistoryResult)

    CS_SLOT_1(Private)
    void applyIgnoreResult(const QString &result);
    CS_SLOT_2(applyIgnoreResult)

    CS_SLOT_1(Private)
    void applyOrganizeResult(const QString &result);
    CS_SLOT_2(applyOrganizeResult)

    CS_SLOT_1(Private)
    void applyDeleteDupesResult(const QString &result);
    CS_SLOT_2(applyDeleteDupesResult)

private:
    void runScan(const QString &dir);
    void runDuplicates(const QString &dir);
    void runStats(const QString &dir);
    void runHash(const QString &filePath);
    void runMetadata(const QString &dir);
    void runLint(const QString &dir);
    void runSearch(const QString &query, const QString &dir);
    void runFlowList();
    void runScrub(const QString &dir);
    void runExport(const QString &dir);
    void runCount(const QString &dir);

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

    QWidget *searchTab;
    QLineEdit *searchQueryEdit;
    QLineEdit *searchPathEdit;
    QPushButton *searchBtn;
    QLabel *searchResultLbl;

    QWidget *flowTab;
    QPushButton *flowListBtn;
    QLabel *flowResultLbl;

    QWidget *scrubTab;
    QLineEdit *scrubPathEdit;
    QPushButton *scrubBtn;
    QLabel *scrubResultLbl;

    QWidget *exportTab;
    QLineEdit *exportPathEdit;
    QPushButton *exportBtn;
    QLabel *exportResultLbl;

    QWidget *countTab;
    QLineEdit *countPathEdit;
    QPushButton *countBtn;
    QLabel *countResultLbl;

    QWidget *historyTab;
    QPushButton *historyBtn;
    QLabel *historyResultLbl;

    QWidget *ignoreTab;
    QLineEdit *ignorePatternEdit;
    QPushButton *ignoreListBtn;
    QPushButton *ignoreAddBtn;
    QLabel *ignoreResultLbl;

    QWidget *organizeTab;
    QLineEdit *organizePathEdit;
    QLineEdit *organizeRuleEdit;
    QPushButton *organizeBtn;
    QLabel *organizeResultLbl;

    QWidget *deleteDupesTab;
    QLineEdit *deleteDupesPathEdit;
    QPushButton *deleteDupesBtn;
    QLabel *deleteDupesResultLbl;
};
