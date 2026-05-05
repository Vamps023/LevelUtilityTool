#include "LevelUtilityToolEditorPlugin.h"
#include "../processing/NodeGenerationService.h"
#include "../ui/LevelUtilityToolPanel.h"
#include "../extrusion/Private/ExtrusionPlugin.h"

#include <UnigineLog.h>
#include <editor/UnigineWindowManager.h>
#include <editor/UnigineSelection.h>
#include <editor/UnigineSelector.h>

LevelUtilityToolEditorPlugin::LevelUtilityToolEditorPlugin() = default;

LevelUtilityToolEditorPlugin::~LevelUtilityToolEditorPlugin() = default;

static size_t selection_cb(Unigine::Vector<Unigine::NodePtr>& nodes)
{
	nodes.clear();
	UnigineEditor::Selection* sel = UnigineEditor::Selection::instance();
	if (!sel)
		return 0;

	UnigineEditor::SelectorNodes const* selection_nodes = sel->getSelectorNodes();
	if (!selection_nodes)
		return 0;

	nodes = selection_nodes->getNodes();
	return nodes.size();
}

bool LevelUtilityToolEditorPlugin::init()
{
	if (panel_ || service_)
		return true;

	service_ = new LevelUtility::NodeGenerationService();
	panel_ = new LevelUtilityToolPanel(service_);

	UnigineEditor::WindowManager::add(panel_, UnigineEditor::WindowManager::ROOT_AREA_RIGHT);
	UnigineEditor::WindowManager::restoreLastWindowConfig(panel_, panel_->objectName());
	UnigineEditor::WindowManager::show(panel_);

	// Initialize extrusion runtime plugin for live mesh generation
	extrusion_plugin_ = new ExtrusionPlugin();
	extrusion_plugin_->init();
	extrusion_plugin_->register_selection_callback(selection_cb);
	panel_->setExtrusionPlugin(extrusion_plugin_);

	// Connect editor selection changes to extrusion plugin
	UnigineEditor::Selection* sel = UnigineEditor::Selection::instance();
	if (sel)
	{
		connect(sel, &UnigineEditor::Selection::changed, [this]()
		{
			if (extrusion_plugin_)
				extrusion_plugin_->selection_changed();
		});
	}

	Unigine::Log::message("LevelUtilityTool: Plugin initialized\n");
	return true;
}

void LevelUtilityToolEditorPlugin::shutdown()
{
	if (extrusion_plugin_)
	{
		extrusion_plugin_->shutdown();
		delete extrusion_plugin_;
		extrusion_plugin_ = nullptr;
	}

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
