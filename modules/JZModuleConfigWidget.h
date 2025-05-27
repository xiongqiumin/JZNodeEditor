#ifndef JZ_MODULE_CONFIG_WIDGET_H_
#define JZ_MODULE_CONFIG_WIDGET_H_

#include <QTableWidget>
#include "JZModuleConfigFactory.h"

class JZModuleConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JZModuleConfigWidget(QWidget *parent = nullptr);
    ~JZModuleConfigWidget();
    
protected slots:
    void onBtnAddClicked();
    void onBtnRemoveClicked();  
    void onBtnSettingClicked();
    void onItemDoubleClicked(QTableWidgetItem *item);

protected:
    virtual void addConfig() = 0;
    virtual void removeConfig(int index) = 0;
    virtual void settingConfig(int index) = 0;
    virtual void updateConfig() = 0;

    QTableWidget *m_table;
};

#endif