#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QTranslator>

namespace Ui
{
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = 0);
    ~PreferencesDialog();
    void initPreferencesDialogFormConfig();

private slots:
    void onOk();
    void onCancel();

public slots:
    void clickLanguageAction(QTranslator& tran);

private:
    Ui::PreferencesDialog* ui;
    bool checkPreferencesSettingIsChange();
};

#endif  // PREFERENCESDIALOG_H