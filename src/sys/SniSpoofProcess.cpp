#include "include/sys/SniSpoofProcess.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QThread>

namespace Configs_sys {
    SniSpoofProcess::SniSpoofProcess(QObject *parent) : QObject(parent) {}

    bool SniSpoofProcess::Start(const QString &binaryPath,
                                const QString &configPath,
                                const QString &argsTemplate,
                                const QString &listenHost,
                                int listenPort,
                                int timeoutMs,
                                QString &error) {
        if (IsRunning()) return true;

        QFileInfo binaryInfo(binaryPath);
        if (!binaryInfo.exists() || !binaryInfo.isFile()) {
            error = tr("SNI-Spoofing-Go binary was not found: %1").arg(binaryPath);
            return false;
        }

        QFile binary(binaryPath);
        auto permissions = binary.permissions();
        if (!(permissions & QFileDevice::ExeOwner)) {
            permissions |= QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther;
            if (!binary.setPermissions(permissions)) {
                error = tr("SNI-Spoofing-Go exists but executable permissions could not be set: %1").arg(binaryPath);
                return false;
            }
        }

        QFileInfo configInfo(configPath);
        if (!configInfo.exists() || !configInfo.isFile()) {
            error = tr("SNI Spoof config was not found: %1").arg(configPath);
            return false;
        }

        if (process != nullptr) {
            process->deleteLater();
            process = nullptr;
        }

        stderrTail.clear();
        stopping = false;
        process = new QProcess(this);
        process->setProgram(binaryPath);
        process->setArguments(BuildArgs(argsTemplate, configPath));
        process->setWorkingDirectory(configInfo.absolutePath());

        connect(process, &QProcess::readyReadStandardOutput, this, [this] {
            const auto text = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
            if (!text.isEmpty()) emit LogLine("[SNI] " + text);
        });
        connect(process, &QProcess::readyReadStandardError, this, [this] {
            const auto text = QString::fromUtf8(process->readAllStandardError()).trimmed();
            if (!text.isEmpty()) {
                AppendStderr(text);
                emit LogLine("[SNI error] " + text);
            }
        });
        connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError processError) {
            Q_UNUSED(processError)
            if (process != nullptr) emit LogLine("[SNI error] " + process->errorString());
        });
        connect(process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
            Q_UNUSED(exitStatus)
            auto *finishedProcess = process;
            process = nullptr;
            if (finishedProcess != nullptr) finishedProcess->deleteLater();
            emit StateChanged();
            if (!stopping) {
                QString message = tr("SNI-Spoofing-Go exited unexpectedly with code %1.").arg(exitCode);
                if (!stderrTail.isEmpty()) message += "\n" + stderrTail;
                emit Crashed(message);
            }
            stopping = false;
        });

        process->start();
        if (!process->waitForStarted(timeoutMs)) {
            error = tr("Failed to start SNI-Spoofing-Go: %1").arg(process->errorString());
            process->deleteLater();
            process = nullptr;
            return false;
        }

        if (!WaitForListen(listenHost, listenPort, timeoutMs, error)) {
            Stop(true);
            return false;
        }

        emit StateChanged();
        return true;
    }

    void SniSpoofProcess::Stop(bool block) {
        if (!IsRunning()) {
            stopping = false;
            emit StateChanged();
            return;
        }

        stopping = true;
        process->terminate();
        if (block || !process->waitForFinished(2000)) {
            if (process != nullptr && process->state() != QProcess::NotRunning) {
                process->kill();
                process->waitForFinished(1000);
            }
        }
        emit StateChanged();
    }

    bool SniSpoofProcess::IsRunning() const {
        return process != nullptr && process->state() != QProcess::NotRunning;
    }

    QStringList SniSpoofProcess::BuildArgs(const QString &argsTemplate, const QString &configPath) const {
        QString command = argsTemplate.trimmed();
        if (command.isEmpty()) command = "{config_path}";

        QString quotedConfigPath = configPath;
        quotedConfigPath.replace("\\", "\\\\");
        quotedConfigPath.replace("\"", "\\\"");
        quotedConfigPath = "\"" + quotedConfigPath + "\"";

        command.replace("{{config_path}}", quotedConfigPath);
        command.replace("{config_path}", quotedConfigPath);
        return QProcess::splitCommand(command);
    }

    bool SniSpoofProcess::WaitForListen(const QString &listenHost, int listenPort, int timeoutMs, QString &error) const {
        const QString probeHost = listenHost == "0.0.0.0" || listenHost == "::" ? "127.0.0.1" : listenHost;
        QElapsedTimer timer;
        timer.start();

        while (timer.elapsed() < timeoutMs) {
            if (!IsRunning()) {
                error = tr("SNI-Spoofing-Go exited before listening on %1:%2.").arg(probeHost).arg(listenPort);
                if (!stderrTail.isEmpty()) error += "\n" + stderrTail;
                return false;
            }

            QTcpSocket socket;
            socket.connectToHost(probeHost, listenPort);
            if (socket.waitForConnected(150)) {
                socket.disconnectFromHost();
                return true;
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
            QThread::msleep(50);
        }

        error = tr("SNI Spoofing did not start listening on %1:%2 within %3 ms.")
                    .arg(probeHost)
                    .arg(listenPort)
                    .arg(timeoutMs);
        return false;
    }

    void SniSpoofProcess::AppendStderr(const QString &data) {
        stderrTail += data + "\n";
        if (stderrTail.size() > 2000) stderrTail = stderrTail.right(2000);
    }
}
