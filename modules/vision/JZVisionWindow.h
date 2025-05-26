#ifndef JZ_VISION_WINDOW_H_
#define JZ_VISION_WINDOW_H_

#include <QMainWindow>
#include "JZVisionWidget.h"
#include "jzWidgets/JZLogWidget.h"
#include "modules/communication/JZCommManager.h"
#include "modules/camera/JZCameraManager.h"
#include "modules/model/JZModelManager.h"

class JZVisionWindowConfig
{
public:	
};
QDataStream& operator<<(QDataStream& s, const JZVisionWindowConfig& param);
QDataStream& operator>>(QDataStream& s, JZVisionWindowConfig& param);

class JZVisionWindow : public QMainWindow
{
	Q_OBJECT
	
public:	
	JZVisionWindow();
	~JZVisionWindow();

	void init(JZVisionWindowConfig config);
	JZVisionWindowConfig config();
    void initView();

	JZModelManager* modelManager();
	JZCameraManager* cameraManager();
	JZCommManager* commManager();

	JZCameraListWidget* cameraList();
	JZCameraViewWidget* cameraView();
    
protected slots:
	void onActionClose();

	void onActionStartOnce();
	void onActionStart();
	void onActionStop();

	void onActionCameraSetting();
	void onActionModelSetting();
	void onActionCommSetting();
	void onActionDatabaseSetting();
	void onActionMesSetting();

	void onActionAbout();

protected:
	void initMenu();
	void initToolBar();
	void saveConfig();
    bool checkOpen(JZCamera *camera);

	JZCameraListWidget* m_list;
	JZCameraViewWidget* m_view;
	JZLogWidget* m_log;

	JZModelManager* m_modelManager;
	JZCameraManager* m_cameraManager;
	JZCommManager* m_commManager;

	JZVisionWindowConfig m_config;
};


#endif