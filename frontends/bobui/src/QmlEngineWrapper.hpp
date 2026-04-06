#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class QmlEngineWrapper : public QObject {
    Q_OBJECT
public:
    explicit QmlEngineWrapper(QObject *parent = nullptr);

signals:
    void scanFinished(const QVariantList &results, const QString &stats);
    void duplicatesFinished(const QVariantList &results, const QString &stats);
    void statsFinished(const QString &report, const QString &stats);
    void hashFinished(const QString &report, const QString &stats);
    void metadataFinished(const QString &report, const QString &stats);
    void errorOccurred(const QString &scope, const QString &errorMsg);

public slots:
    void runScan(const QString &directoryPath);
    void runDuplicates(const QString &directoryPath);
    void runStats(const QString &directoryPath);
    void runHash(const QString &filePath);
    void runMetadata(const QString &directoryPath);
};
