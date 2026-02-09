#ifndef FORMSETTINGS_H
#define FORMSETTINGS_H
#pragma once

#include <QtWidgets>
class QAstra;
class FormSettings
    : public QWidget{
    Q_OBJECT
    struct _tree_item;
    using _v_tree_items = std::vector<_tree_item>;
public:
    explicit FormSettings(QWidget *parent = nullptr);
    int init(const std::shared_ptr<QAstra>& sp_qastra);
    void populateSettingsTree( _tree_item& ti_root, QTreeWidgetItem* ptwi_parent );
    void setButtonSaveEnabled(bool bl_new_val);
    void setAppSettingsChanged();
    void closeEvent(QCloseEvent *event) override;
signals:
public slots:
    void onBtnSaveClick();
private:
    std::shared_ptr<QAstra> sp_qastra_;
    _tree_item*      pti_settings_root_ = nullptr;
    QPushButton*     ppb_save_settings_ = nullptr;
    QTreeWidget*     ptw_sections_      = nullptr;
    QStackedWidget*  psw_               = nullptr;
    bool             bl_app_settings_chnged_ = false;
};

#endif // FORMSETTINGS_H
