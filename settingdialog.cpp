#include "settingdialog.h"
#include "ui_settingdialog.h"

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    ui_initConnect();
}

SettingDialog::SettingDialog(SettingConfig config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    this->settingConfig = config;

    ui_init(this->settingConfig);

    ui_initConnect();
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::ui_init(SettingConfig config)
{
    ui_log_setLogPath(config.logConfig.filePath);
    ui_log_enableSaveLog(config.logConfig.enableSaveLog);
    ui_log_enableLogBuffer(config.logConfig.enableBuffer);
    ui_show_enableShowColor(config.showConfig.enableShowColor);
    ui_show_setRColorStr(config.showConfig.recColor);
    ui_show_setSColorStr(config.showConfig.sendColor);
    ui_show_setFontStr(config.showConfig.font);

    this->ui->leLogPath->setText(config.logConfig.filePath);
    this->ui->sbLogBufferSize->setValue(config.logConfig.bufferSize);

    this->ui->sbShowBufferSize->setValue(config.showConfig.bufferSize);
//    QString fontStr = QString::fromUtf8("%1 %2 %3").arg(config.showConfig.font.family()).arg(config.showConfig.font.pointSize()).arg(config.showConfig.font.weight());
//    this->ui->leShowFont->setText(config.showConfig.font.toString());

    this->ui->sbLogLimit->setValue(config.logConfig.fileLimit);
    this->ui->sbLogBufferSize->setValue(config.logConfig.bufferSize);


}

void SettingDialog::ui_initConnect()
{
    QObject::connect(this->ui->tbtShowFont, &QToolButton::clicked, [=](){
        bool ok;
        QFont font = QFontDialog::getFont(&ok, this->settingConfig.showConfig.font, this);
        if(ok){
            this->settingConfig.showConfig.font = font;
//            this->ui->leShowFont->setText(font.toString());
            ui_show_setFontStr(font);
        }
    });

    QObject::connect(this->ui->tbtShowRColor, &QToolButton::clicked, [=](){
        QColor color = QColorDialog::getColor(this->settingConfig.showConfig.recColor, this);
        this->settingConfig.showConfig.recColor = color;
        ui_show_setRColorStr(color);
    });
    QObject::connect(this->ui->tbtShowSColor, &QToolButton::clicked, [=](){
        QColor color = QColorDialog::getColor(this->settingConfig.showConfig.sendColor, this);
        this->settingConfig.showConfig.sendColor = color;
        ui_show_setSColorStr(color);
    });

    QObject::connect(this->ui->tbtLogPathSelete, &QToolButton::clicked, [=](){
        QString _p = settingConfig.logConfig.filePath;
        if(_p.isEmpty()){
            _p = QApplication::applicationDirPath();
        }else{
            int pos = _p.lastIndexOf('/');
            QString _pp = _p.mid(pos + 1);
            if(_pp.contains(".txt")){
                _p = _p.mid(0, pos);
            }
        }
        QFileDialog dialog(this, QString::fromUtf8("保存日志文件"), _p, QString::fromUtf8("文本文件 (*.txt)"));
        QString dt_str = QDateTime::currentDateTime().toString(QString::fromUtf8("log_yyyyMMddhhmmss"));
        dt_str = dt_str + QString::fromUtf8(".txt");
        dialog.selectFile(dt_str);
        int res = dialog.exec();
        if(res == QFileDialog::Accepted){
            const QString p = dialog.selectedFiles().first();
            this->settingConfig.logConfig.filePath = p;
            ui_log_setLogPath(p);
        }
    });

    QObject::connect(this->ui->buttonBox, &QDialogButtonBox::clicked, [=](QAbstractButton *button){
          if(this->ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole){
              handle_dialog_apply();
          }
          if(this->ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole){
              handle_dialog_apply();
          }
    });

    QObject::connect(this->ui->cbLogEnable, &QCheckBox::stateChanged, [=](int state){
        if(state == Qt::Checked){
            settingConfig.logConfig.enableSaveLog = true;
            QString _lp = ui_log_getLogPath();
            int pos = _lp.lastIndexOf('/');
            QString _ln = _lp.mid(pos + 1);
            if(!_ln.contains(".txt")){  // 不含txt文件, 说明是目录
                 _lp.append(QString::fromUtf8("/log_%1.txt").arg(QDateTime::currentDateTime().toString(QString::fromUtf8("yyyyMMddhhmmss"))));
            }
            settingConfig.logConfig.filePath = _lp;
            ui_log_setLogPath(_lp);
        }
        if(state == Qt::Unchecked){
            settingConfig.logConfig.enableSaveLog = false;
        }
    });

//    QObject::connect(this->ui->cbLogEnableBuffer, &QCheckBox::stateChanged, [=](int state){
//        if(state == Qt::Checked){
//            settingConfig.logConfig.enableBuffer = true;
//        }
//        if(state == Qt::Unchecked){
//            settingConfig.logConfig.enableBuffer = false;
//        }
//    });
//    QObject::connect(this->ui->cbShowEnableColor, &QCheckBox::stateChanged, [=](int state){
//        if(state == Qt::Checked){
//            settingConfig.showConfig.enableShowColor = true;
//        }
//        if(state == Qt::Unchecked){
//            settingConfig.showConfig.enableShowColor = false;
//        }
//    });

}


void SettingDialog::ui_log_setLogPath(const QString &p)
{
    this->ui->leLogPath->setText(p);
    this->ui->leLogPath->setToolTip(p);
}

const QString SettingDialog::ui_log_getLogPath()
{
    return this->ui->leLogPath->text();
}

bool SettingDialog::ui_log_isEnableSaveLog()
{
    return this->ui->cbLogEnable->isChecked();
}

bool SettingDialog::ui_log_isEnableLogBuffer()
{
    return this->ui->cbLogEnableBuffer->isChecked();
}

int SettingDialog::ui_log_getBufferSize()
{
    return this->ui->sbLogBufferSize->value();
}

int SettingDialog::ui_log_getLogFileLimit()
{
    return this->ui->sbLogLimit->value();
}

void SettingDialog::ui_log_enableSaveLog(bool save)
{
    Qt::CheckState s = save ? Qt::Checked : Qt::Unchecked;
    this->ui->cbLogEnable->setCheckState(s);
}

void SettingDialog::ui_log_enableLogBuffer(bool en)
{
     Qt::CheckState s = en ? Qt::Checked : Qt::Unchecked;
     this->ui->cbLogEnableBuffer->setCheckState(s);
}

void SettingDialog::ui_show_enableShowColor(bool en)
{
    Qt::CheckState s = en ? Qt::Checked : Qt::Unchecked;
    this->ui->cbShowEnableColor->setCheckState(s);
}

void SettingDialog::ui_show_setFontStr(QFont font)
{
    QString fontStr = QString::fromUtf8("%1 %2 ").arg(font.family()).arg(font.pointSize());

    this->ui->leShowFont->setText(fontStr);
}

void SettingDialog::ui_show_setRColorStr(QColor color)
{
    QString cStr = color.name(QColor::HexRgb).toUpper();
    this->ui->leShowRColor->setText(cStr);
}

void SettingDialog::ui_show_setSColorStr(QColor color)
{
    QString cStr = color.name(QColor::HexRgb).toUpper();
    this->ui->leShowSColor->setText(cStr);
}

const int SettingDialog::ui_show_getBufferSize()
{
    return this->ui->sbShowBufferSize->value();
}

bool SettingDialog::ui_show_isEnableShowColor()
{
    return this->ui->cbShowEnableColor->isChecked();
}

const QColor SettingDialog::ui_show_getRecieveColor()
{
    return QColor(QLatin1String(this->ui->leShowRColor->text().toLatin1()));
}

const QColor SettingDialog::ui_show_getSendColor()
{
    return QColor(QLatin1String(this->ui->leShowSColor->text().toLatin1()));
}

void SettingDialog::handle_dialog_apply()
{
    this->settingConfig.showConfig.bufferSize = ui_show_getBufferSize();
    this->settingConfig.showConfig.enableShowColor = ui_show_isEnableShowColor();


    this->settingConfig.logConfig.bufferSize = ui_log_getBufferSize();
    this->settingConfig.logConfig.enableBuffer = ui_log_isEnableLogBuffer();
    this->settingConfig.logConfig.enableSaveLog = ui_log_isEnableSaveLog();
    this->settingConfig.logConfig.filePath = ui_log_getLogPath();
    this->settingConfig.logConfig.fileLimit = ui_log_getLogFileLimit();

    emit setting_changed(this->settingConfig);
}
