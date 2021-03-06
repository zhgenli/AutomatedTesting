#include "MtcpFileHelper.h"
#include "Util.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QApplication>

std::mutex MtcpFileHelper::mtcpFileMutex;

const QString K_CONFIG = "/Config/";
const QString K_BACKUP = "/Config/Backup/";

const QString lotStartFile = "LotStart.csv";
const QString lotEndFile = "LotEnd.csv";
const QString pivotRequest = "PivotRequest.csv";

const QString buildConfig = "BuildConfig.csv";

MtcpFileHelper::MtcpFileHelper() {}

bool MtcpFileHelper::fileChecksum(const QString filePath, MTCP_FILE_TYPE type)
{
    const QString TMPath = QApplication::applicationDirPath();
    QString backupPath = TMPath + K_BACKUP;
    switch (type) {
    case M_START:
        backupPath += lotStartFile;
        break;
    case M_END:
        backupPath += lotEndFile;
        break;
    case M_PIVOT:
        backupPath += pivotRequest;
        break;
    case M_CONFIG:
        backupPath += buildConfig;
        break;
    default:
        return false;
    }

    QFile file_backup(backupPath);
    if (!file_backup.exists()) {
        return false;
    }

    QFile file(filePath);
    if (file.exists()) {
        file.remove();
    }

    return QFile::copy(backupPath, filePath);
}

const QMap<QString, QString> MtcpFileHelper::inputBuildConfig(bool retry)
{
    std::unique_lock<std::mutex> lock(mtcpFileMutex);
    const QString TMPath = QApplication::applicationDirPath();
    try {
        QFile file(TMPath + K_CONFIG + buildConfig);
        if (!file.open(QIODevice::ReadOnly)) {
            std::string err = std::string("OPEN FILE FAILED, ") + (TMPath + K_CONFIG + buildConfig).toStdString();
            throw std::runtime_error(err);
        }

        QTextStream out(&file);
        QStringList csvLines = out.readAll().split("\n");

        QMap<QString, QString> bulidConfigMap;
        for (int i = 1; i < csvLines.length(); i++) {
            QStringList csvline = csvLines.at(i).split(',');
            if (csvline.length() > 3) {
                bulidConfigMap.insert(csvline.at(2), csvline.at(1));
            }
        }
        return bulidConfigMap;
    }
    catch (std::runtime_error& e) {
        fileChecksum(TMPath + K_CONFIG + buildConfig, M_CONFIG);
        if (retry) {
            lock.unlock();
            inputBuildConfig(false);
        } else {
            throw e;
        }
    }
}

const QString MtcpFileHelper::inputCurrLotName(bool retry)
{
    std::unique_lock<std::mutex> lock(mtcpFileMutex);
    const QString TMPath = QApplication::applicationDirPath();
    try {
        QFile file(TMPath + K_CONFIG + lotStartFile);
        if (!file.open(QIODevice::ReadOnly)) {
            std::string err = std::string("OPEN FILE FAILED, ") + (TMPath + K_CONFIG + lotStartFile).toStdString();
            throw std::runtime_error(err);
        }

        QTextStream out(&file);
        QStringList csvLines = out.readAll().split("\n");
        if (csvLines.length() < 4) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }

        QStringList csvline = csvLines.at(3).split(',');
        if (csvline.length() < 12) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }
        return csvline.at(10);
    }
    catch (std::runtime_error& e) {
        fileChecksum(TMPath + K_CONFIG + lotStartFile, M_START);
        if (retry) {
            lock.unlock();
            inputCurrLotName(false);
        } else {
            throw e;
        }
    }
}

const QString MtcpFileHelper::outputLotStart(const QString& lotName, bool retry)
{
    std::unique_lock<std::mutex> lock(mtcpFileMutex);
    const QString TMPath = QApplication::applicationDirPath();
    try {
        QFile file(TMPath + K_CONFIG + lotStartFile);
        if (!file.open(QIODevice::ReadWrite)) {
            std::string err = std::string("OPEN FILE FAILED, ") + (TMPath + K_CONFIG + lotStartFile).toStdString();
            throw std::runtime_error(err);
        }

        QTextStream out(&file);
        QStringList csvLines = out.readAll().split("\n");
        if (csvLines.length() < 4) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }

        QStringList csvline = csvLines.at(3).split(',');
        if (csvline.length() < 12) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }
        csvline.replace(10, lotName);
        csvLines.replace(3, csvline.join(","));

        file.resize(0);
        out << csvLines.join("\n");
        file.close();
        return TMPath + K_CONFIG + lotStartFile;
    }
    catch (std::runtime_error& e) {
        fileChecksum(TMPath + K_CONFIG + lotStartFile, M_START);
        if (retry) {
            lock.unlock();
            outputLotStart(lotName, false);
        } else {
            throw e;
        }
    }
}

const QString MtcpFileHelper::outputLotEnd(const QString& lotName, bool retry)
{
    std::unique_lock<std::mutex> lock(mtcpFileMutex);
    const QString TMPath = QApplication::applicationDirPath();
    try {
        QFile file(TMPath + K_CONFIG + lotEndFile);
        if (!file.open(QIODevice::ReadWrite)) {
            std::string err = std::string("OPEN FILE FAILED, ") + (TMPath + K_CONFIG + lotEndFile).toStdString();
            throw std::runtime_error(err);
        }

        QTextStream out(&file);
        QStringList csvLines = out.readAll().split("\n");
        if (csvLines.length() < 2) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }

        QStringList csvline = csvLines.at(1).split(',');
        if (csvline.length() < 12) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }
        csvline.replace(10, lotName);
        csvLines.replace(1, csvline.join(","));

        file.resize(0);
        out << csvLines.join("\n");
        file.close();
        return TMPath + K_CONFIG + lotEndFile;
    }
    catch (std::runtime_error& e) {
        fileChecksum(TMPath + K_CONFIG + lotEndFile, M_END);
        if (retry) {
            lock.unlock();
            outputLotEnd(lotName, false);
        } else {
            throw e;
        }
    }
}

const QString MtcpFileHelper::outputPivotRequest(const QString& sn, const QString& lotName, const QString& version,
                                                 bool retry)
{
    std::unique_lock<std::mutex> lock(mtcpFileMutex);
    const QString TMPath = QApplication::applicationDirPath();
    try {
        QFile file(TMPath + K_CONFIG + pivotRequest);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            std::string err = std::string("OPEN FILE FAILED, ") + (TMPath + K_CONFIG + pivotRequest).toStdString();
            throw std::runtime_error(err);
        }

        QTextStream out(&file);
        QStringList csvLines = out.readAll().split("\n");
        if (csvLines.length() < 88) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }

        QStringList csvline = csvLines.at(2).split(',');
        if (csvline.length() < 12) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }
        csvline.replace(10, sn);
        csvLines.replace(2, csvline.join(","));

        csvline = csvLines.at(4).split(',');
        if (csvline.length() < 12) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }
        csvline.replace(10, lotName);
        csvLines.replace(4, csvline.join(","));

        csvline = csvLines.at(15).split(',');
        if (csvline.length() < 12) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }
        csvline.replace(10, version);
        csvLines.replace(15, csvline.join(","));

        csvline = csvLines.at(87).split(',');
        if (csvline.length() < 12) {
            file.close();
            throw std::runtime_error("READ FILE FAILED, File format error.");
        }
        csvline.replace(10, sn);
        csvLines.replace(87, csvline.join(","));

        file.resize(0);
        out << csvLines.join("\n");
        file.close();
        return TMPath + K_CONFIG + pivotRequest;
    }
    catch (std::runtime_error& e) {
        fileChecksum(TMPath + K_CONFIG + pivotRequest, M_PIVOT);
        if (retry) {
            lock.unlock();
            outputPivotRequest(sn, lotName, version, false);
        } else {
            throw e;
        }
    }
}
