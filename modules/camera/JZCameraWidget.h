#ifndef JZ_CAMERA_WIDGET_H_
#define JZ_CAMERA_WIDGET_H_

#include "JZCamera.h"
#include "JZPropertyDialog.h"

//JZCameraConfigDialog
class JZCameraConfigDialog : public JZPropertyDialog
{
    Q_OBJECT

public:
    JZCameraConfigDialog(QWidget *parent = nullptr);

    void setConfig(JZCameraConfigPtr cfg);
    JZCameraConfigPtr getConfig() const;

    private slots:

private:
    void accept();
    void addFilePage();
    void addHikPage();
    void addUvcPage();
    void addRtspPage();
    
    int m_type;
    QString m_name;

    JZProperty *m_propGroup;
    QMap<int,JZCameraConfigPtr> m_config;
};

#endif // !JZ_CAMERA_WIDGET_H_

