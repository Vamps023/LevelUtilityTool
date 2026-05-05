#pragma once

#include "../Public/ExtrusionPluginInterface.h"

#include <UniginePlugin.h>
#include <UnigineNode.h>
#include <UnigineMathLib.h>

#include <memory>
#include <vector>
#include <string>

namespace LevelUtility
{
class ExtrusionEngine;
}

class ExtrusionPlugin final : public Unigine::Plugin, public IExtrusionPlugin
{
public:
	ExtrusionPlugin();
	~ExtrusionPlugin() override;

	// Unigine::Plugin
	int init() override;
	void update() override;
	int shutdown() override;

	// IExtrusionPlugin
	void activate() override;
	void build_gui(Unigine::GuiPtr gui, Unigine::EventConnections& connections, bool is_editor) override;
	void register_selection_callback(size_t(*callback)(Unigine::Vector<Unigine::NodePtr>& nodes)) override;
	void selection_changed() override;

private:
	void node_selected(const Unigine::NodePtr& node);
	void node_deselected();
	void create_clicked(const Unigine::WidgetPtr& widget, int button);
	void select_node_clicked(const Unigine::WidgetPtr& widget, int button);
	void clear_selection_clicked(const Unigine::WidgetPtr& widget, int button);
	Unigine::NodePtr find_extrude_node(const Unigine::NodePtr& node) const;
	void create_dynamic_node_ui();

	std::unique_ptr<LevelUtility::ExtrusionEngine> engine_;

	Unigine::NodePtr selected_object_;
	Unigine::NodePtr extrude_node_;

	size_t (*selection_callback_)(Unigine::Vector<Unigine::NodePtr>& nodes) = nullptr;

	// GUI
	Unigine::GuiPtr gui_;
	Unigine::EventConnections* connections_ = nullptr;
	Unigine::WidgetTabBoxPtr tabs_widget_;
	Unigine::WidgetLabelPtr file_label_;
	Unigine::WidgetButtonPtr create_button_;

	std::string selected_file_path_;
	bool is_editor_ = false;
	bool extruding_ = false;

	static int reference_count_;
};
