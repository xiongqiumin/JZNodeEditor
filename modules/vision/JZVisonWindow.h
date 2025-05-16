#ifndef JZ_VISION_WINDOW_H_
#define JZ_VISION_WINDOW_H_

#include <QMainWindow>
#include "JZVisionWidget.h"
#include "jzWidgets/JZLogWidget.h"
#include "modules/communication/JZCommManager.h"
#include "modules/camera/JZCameraManager.h"
#include "modules/model/JZModelManager.h"

class JZVisonWindowConfig
{
public:
	JZCameraManagerConfig cameraConfig;
	JZModelManagerConfig modelConfig;
	JZCommManagerConfig commConfig;
};
QDataStream& operator<<(QDataStream& s, const JZVisonWindowConfig& param);
QDataStream& operator>>(QDataStream& s, JZVisonWindowConfig& param);

class JZVisonWindow : public QMainWindow
{
	Q_OBJECT
	
public:	
	JZVisonWindow();
	~JZVisonWindow();

	void init(JZVisonWindowConfig config);
	JZVisonWindowConfig config();

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

	JZCameraListWidget* m_list;
	JZCameraViewWidget* m_view;
	JZLogWidget* m_log;

	JZModelManager* m_modelManager;
	JZCameraManager* m_cameraManager;
	JZCommManager* m_commManager;

	JZVisonWindowConfig m_config;
};


#endif