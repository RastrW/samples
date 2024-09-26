#ifndef FORMSETTINGS_H
#define FORMSETTINGS_H

#include <QtWidgets>





class FormSettings : public QWidget
{
    Q_OBJECT
    struct _tree_item;
    using _v_tree_items = std::vector<_tree_item>;
public:
    explicit FormSettings(QWidget *parent = nullptr);
    int init();
    void populateSettingsTree( _tree_item& ti_root, QTreeWidgetItem* ptwi_parent );
signals:
private:
    _tree_item*      pti_settings_root_= nullptr;
    QTreeWidget*     ptw_sections_     = nullptr;
    QStackedWidget*  psw_              = nullptr;
};

#endif // FORMSETTINGS_H
