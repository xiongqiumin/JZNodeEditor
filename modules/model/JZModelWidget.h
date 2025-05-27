#ifndef JZMODEL_WIDGET_H_
#define JZMODEL_WIDGET_H_

#include "JZPropertyDialog.h"
#include "JZModelManager.h"
#include "../JZModuleConfigWidget.h"

//JZModelConfigDialog
class JZModelConfigDialog : public JZPropertyDialog
{
    Q_OBJECT

public:
    JZModelConfigDialog(QWidget *parent = nullptr);
    ~JZModelConfigDialog();

    void setConfig(JZModelConfigEnum cfg);
    JZModelConfigEnum getConfig() const;

private slots:


private:
    void addYolo();

    void accept();

    int m_type;
    QString m_name;
    JZProperty *m_propGroup;    
    QMap<int, JZModelConfigEnum*> m_config;

};

//JZModelConfigWidget
class JZModelConfigWidget : public JZModuleConfigWidget
{
    Q_OBJECT

public:
    JZModelConfigWidget(QWidget* parent = nullptr);

    void setConfig(JZModelManagerConfig cfg);
    JZModelManagerConfig config();

    virtual void addConfig() override;
    virtual void removeConfig(int index) override;
    virtual void settingConfig(int index) override;
    virtual void updateConfig() override;

signals:
    void sigModelChanged();

protected:
    JZModelManagerConfig m_config;
    QStringList m_modelTypeList;
};

#endif