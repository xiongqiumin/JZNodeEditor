#ifndef JZ_MOTIONL_WIDGET_H_
#define JZ_MOTION_WIDGET_H_

#include "JZPropertyDialog.h"
#include "JZMotionManager.h"
#include "../JZModuleConfigWidget.h"

//JZMotionConfigDialog
class JZMotionConfigDialog : public JZPropertyDialog
{
    Q_OBJECT

public:
    JZMotionConfigDialog(QWidget *parent = nullptr);
    ~JZMotionConfigDialog();

    void setConfig(JZMotionConfigEnum cfg);
    JZMotionConfigEnum getConfig() const;

private slots:
    

private:
    void addMoteanPage();
    void accept();

    int m_type;
    QString m_name;
    JZProperty *m_propGroup;    
    QMap<int, JZMotionConfigEnum*> m_config;

};

//JZMotionConfigWidget
class JZMotionConfigWidget : public JZModuleConfigWidget
{
    Q_OBJECT

public:
    JZMotionConfigWidget(QWidget* parent = nullptr);

    void setConfig(JZMotionManagerConfig cfg);
    JZMotionManagerConfig config();

    virtual void addConfig() override;
    virtual void removeConfig(int index) override;
    virtual void settingConfig(int index) override;
    virtual void updateConfig() override;

signals:
    void sigModelChanged();

protected:
    JZMotionManagerConfig m_config;
    QStringList m_modelTypeList;
};

#endif