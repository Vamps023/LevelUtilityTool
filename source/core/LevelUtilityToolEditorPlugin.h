#pragma once

#include <editor/UniginePlugin.h>
#include <QObject>

namespace LevelUtility
{
class NodeGenerationService;
}

class LevelUtilityToolPanel;

class LevelUtilityToolEditorPlugin final : public QObject, public UnigineEditor::Plugin
{
	Q_OBJECT
	Q_DISABLE_COPY(LevelUtilityToolEditorPlugin)
	Q_PLUGIN_METADATA(IID UNIGINE_EDITOR_PLUGIN_IID FILE "LevelUtilityTool.json")
	Q_INTERFACES(UnigineEditor::Plugin)

public:
	LevelUtilityToolEditorPlugin();
	~LevelUtilityToolEditorPlugin() override;

	bool init() override;
	void shutdown() override;

private:
	LevelUtility::NodeGenerationService* service_ = nullptr;
	LevelUtilityToolPanel* panel_ = nullptr;
};
