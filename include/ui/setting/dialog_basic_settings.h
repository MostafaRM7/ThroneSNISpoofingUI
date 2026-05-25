#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QJsonObject>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTimer>
#include "ui_dialog_basic_settings.h"

namespace Ui {
    class DialogBasicSettings;
}

class DialogBasicSettings : public QDialog {
    Q_OBJECT

public:
    explicit DialogBasicSettings(QWidget *parent = nullptr);

    ~DialogBasicSettings();

public slots:

    void accept();

private:
    Ui::DialogBasicSettings *ui;

    void applyRegexHighlighting();
    void setupSniSpoofSettings();
    void updateSniSpoofJsonPreview();
    QString defaultSniSpoofBinaryPath() const;

    QCheckBox *snispoof_enable = nullptr;
    QLineEdit *snispoof_binary_path = nullptr;
    QLineEdit *snispoof_listen_host = nullptr;
    QLineEdit *snispoof_listen_port = nullptr;
    QLineEdit *snispoof_connect_ip = nullptr;
    QLineEdit *snispoof_connect_port = nullptr;
    QLineEdit *snispoof_fake_sni = nullptr;
    QLineEdit *snispoof_cli_args = nullptr;
    QPlainTextEdit *snispoof_config_json = nullptr;

    struct {
        QString custom_inbound;
        bool needRestart = false;
        bool updateDisableTray = false;
        bool updateTrayIcon = false;
        bool updateSystemDns = false;
        bool updateMaxLogLines = false;
        bool updateDisableAdmin = false;
    } CACHE;

private slots:
    void on_core_settings_clicked();
    void on_backup_create_clicked();
    void on_backup_restore_clicked();
};