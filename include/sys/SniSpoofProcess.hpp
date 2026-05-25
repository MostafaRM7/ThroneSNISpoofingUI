#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

namespace Configs_sys {
    class SniSpoofProcess : public QObject {
        Q_OBJECT

    public:
        explicit SniSpoofProcess(QObject *parent = nullptr);

        bool Start(const QString &binaryPath,
                   const QString &configPath,
                   const QString &argsTemplate,
                   const QString &listenHost,
                   int listenPort,
                   int timeoutMs,
                   QString &error);
        void Stop(bool block = false);
        [[nodiscard]] bool IsRunning() const;

    signals:
        void LogLine(const QString &line);
        void StateChanged();
        void Crashed(const QString &message);

    private:
        QProcess *process = nullptr;
        bool stopping = false;
        QString stderrTail;

        QStringList BuildArgs(const QString &argsTemplate, const QString &configPath) const;
        bool WaitForListen(const QString &listenHost, int listenPort, int timeoutMs, QString &error) const;
        void AppendStderr(const QString &data);
    };
}
