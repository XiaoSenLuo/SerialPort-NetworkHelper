#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QFont>
#include <QColor>
#include <QFontDialog>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QSettings>

#include <QDebug>


namespace Ui {
class SettingDialog;
}

#include "settingconfig.h"

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    SettingDialog(SettingConfig config, QWidget *parent = nullptr);
    ~SettingDialog();

    SettingConfig getSetting(void){
        return this->settingConfig;
    }

signals:
    void setting_changed(SettingConfig config);

private:
    Ui::SettingDialog *ui;

    SettingConfig settingConfig;

    void ui_init(SettingConfig config);
    void ui_initConnect(void);

    void ui_log_setLogPath(const QString &p);
    const QString ui_log_getLogPath();
    void ui_log_enableSaveLog(bool save);
    bool ui_log_isEnableSaveLog(void);
    void ui_log_enableLogBuffer(bool en);
    bool ui_log_isEnableLogBuffer(void);
    int ui_log_getBufferSize(void);
    int ui_log_getLogFileLimit(void);

    void ui_show_enableShowColor(bool en);
    bool ui_show_isEnableShowColor(void);
    void ui_show_setFontStr(QFont font);
    void ui_show_setRColorStr(QColor color);
    void ui_show_setSColorStr(QColor color);

    const int ui_show_getBufferSize(void);
    const QColor ui_show_getRecieveColor(void);
    const QColor ui_show_getSendColor(void);

    void handle_dialog_apply(void);
};

#endif // SETTINGDIALOG_H
