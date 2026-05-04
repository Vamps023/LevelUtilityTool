#include "LevelUtilityToolEditorPlugin.h"
#include "../processing/NodeGenerationService.h"
#include "../ui/LevelUtilityToolPanel.h"

#include <UnigineLog.h>
#include <editor/UnigineWindowManager.h>

LevelUtilityToolEditorPlugin::LevelUtilityToolEditorPlugin() = default;

LevelUtilityToolEditorPlugin::~LevelUtilityToolEditorPlugin() = default;

bool LevelUtilityToolEditorPlugin::init()
{
	if (panel_ || service_)
		return true;

	service_ = new LevelUtility::NodeGenerationService();
	panel_ = new LevelUtilityToolPanel(service_);

	UnigineEditor::WindowManager::add(panel_, UnigineEditor::WindowManager::ROOT_AREA_RIGHT);
	UnigineEditor::WindowManager::restoreLastWindowConfig(panel_, panel_->objectName());
	UnigineEditor::WindowManager::show(panel_);

	extruder_editor_plugin_ = new Extruder2EditorPlugin();
	extruder_editor_plugin_->init();

	GetExtruder2Plugin()->init();

	Unigine::Log::message("LevelUtilityTool: Plugin initialized\n");
	return true;
}

void LevelUtilityToolEditorPlugin::shutdown()
{
	if (extruder_editor_plugin_)
	{
		extruder_editor_plugin_->shutdown();
		delete extruder_editor_plugin_;
		extruder_editor_plugin_ = nullptr;
	}

	GetExtruder2Plugin()->shutdown();

	if (panel_)
	{
		UnigineEditor::WindowManager::remove(panel_);
		delete panel_;
		panel_ = nullptr;
	}

	delete service_;
	service_ = nullptr;

	Unigine::Log::message("LevelUtilityTool: Plugin shutdown\n");
}
