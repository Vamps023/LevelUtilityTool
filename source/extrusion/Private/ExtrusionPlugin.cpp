#include "ExtrusionPlugin.h"
#include "ExtrusionEngine.h"

#include <UnigineLog.h>
#include <UnigineEditor.h>
#include <UnigineNodes.h>
#include <UnigineWorld.h>
#include <UnigineGame.h>
#include <UnigineConsole.h>
#include <UnigineWidgets.h>
#include <UnigineProperties.h>

using namespace Unigine;

int ExtrusionPlugin::reference_count_ = 0;

ExtrusionPlugin::ExtrusionPlugin()
{
	++reference_count_;
	engine_ = std::make_unique<LevelUtility::ExtrusionEngine>();
}

ExtrusionPlugin::~ExtrusionPlugin()
{
	--reference_count_;
}

int ExtrusionPlugin::init()
{
	if (reference_count_ > 1)
		return 1;

	Log::message("ExtrusionPlugin: Initialized\n");
	return 1;
}

void ExtrusionPlugin::update()
{
	if (engine_ && extruding_ && extrude_node_)
	{
		Log::message("ExtrusionPlugin::update: extruding %s\n", extrude_node_->getName());
		engine_->update(extrude_node_);
	}
}

int ExtrusionPlugin::shutdown()
{
	if (reference_count_ > 0)
		return 1;

	node_deselected();
	Log::message("ExtrusionPlugin: Shutdown\n");
	return 1;
}

void ExtrusionPlugin::activate()
{
	Log::message("ExtrusionPlugin: Activated\n");
}

void ExtrusionPlugin::register_selection_callback(size_t(*callback)(Unigine::Vector<Unigine::NodePtr>& nodes))
{
	selection_callback_ = callback;
}

void ExtrusionPlugin::selection_changed()
{
	if (!selection_callback_)
		return;

	Vector<NodePtr> nodes;
	selection_callback_(nodes);

	if (nodes.empty())
	{
		Log::message("ExtrusionPlugin::selection_changed: no nodes selected\n");
		node_deselected();
		return;
	}

	NodePtr node = nodes[0];
	Log::message("ExtrusionPlugin::selection_changed: selected %s (type=%d)\n",
		node->getName(), node->getType());

	if (node == selected_object_)
	{
		Log::message("ExtrusionPlugin::selection_changed: same object, ignoring\n");
		return;
	}

	node_deselected();
	selected_object_ = node;

	NodePtr extrude = find_extrude_node(node);
	if (extrude)
	{
		Log::message("ExtrusionPlugin::selection_changed: found extrude node %s\n", extrude->getName());
		node_selected(extrude);
	}
	else
	{
		Log::message("ExtrusionPlugin::selection_changed: no extrude node found for %s\n", node->getName());
	}
}

NodePtr ExtrusionPlugin::find_extrude_node(const NodePtr& node) const
{
	if (!node)
		return NodePtr();

	NodePtr current = node;
	while (current)
	{
		// First try: match by "extrude" property
		if (current->getProperty(0) && current->getProperty(0)->getName() == "extrude")
			return current;

		// Fallback: match by node name pattern (e.g. "new_extrude", "extrude_01", etc.)
		// BUT exclude the generated "extruded" mesh child
		const char* name = current->getName();
		if (name && strcmp(name, "extruded") != 0 &&
		    (strstr(name, "extrude") != nullptr || strstr(name, "_extrude") != nullptr))
			return current;

		current = current->getParent();
	}
	return NodePtr();
}

void ExtrusionPlugin::node_selected(const NodePtr& node)
{
	extrude_node_ = node;
	extruding_ = true;

	if (engine_)
		engine_->begin_extrude(extrude_node_);

	Log::message("ExtrusionPlugin: Node selected for extrusion: %s\n", node->getName());
}

void ExtrusionPlugin::node_deselected()
{
	if (engine_)
		engine_->end_extrude();

	extruding_ = false;
	extrude_node_ = NodePtr();
	selected_object_ = NodePtr();
}

void ExtrusionPlugin::build_gui(GuiPtr gui, EventConnections& connections, bool is_editor)
{
	gui_ = gui;
	connections_ = &connections;
	is_editor_ = is_editor;

	WidgetVBoxPtr root = WidgetVBox::create(gui, 0, 0);
	root->setColor(Math::vec4(0.15f, 0.15f, 0.15f, 1.0f));

	tabs_widget_ = WidgetTabBox::create(gui, 0, 0);
	root->addChild(tabs_widget_, Gui::ALIGN_EXPAND);

	create_dynamic_node_ui();

	gui->addChild(root, Gui::ALIGN_EXPAND);
}

void ExtrusionPlugin::create_dynamic_node_ui()
{
	if (!gui_)
		return;

	WidgetVBoxPtr tab = WidgetVBox::create(gui_, 0, 0);
	tab->setPadding(8, 8, 8, 8);
	tab->setSpace(4, 4);

	file_label_ = WidgetLabel::create(gui_, "File: none selected");
	tab->addChild(file_label_, Gui::ALIGN_LEFT);

	WidgetHBoxPtr buttons = WidgetHBox::create(gui_, 0, 0);
	buttons->setSpace(4, 4);

	create_button_ = WidgetButton::create(gui_, "Create New");
	create_button_->setEnabled(0);
	buttons->addChild(create_button_, Gui::ALIGN_LEFT);

	WidgetButtonPtr select_btn = WidgetButton::create(gui_, "Select Node...");
	buttons->addChild(select_btn, Gui::ALIGN_LEFT);

	WidgetButtonPtr clear_btn = WidgetButton::create(gui_, "Clear Selection");
	buttons->addChild(clear_btn, Gui::ALIGN_LEFT);

	tab->addChild(buttons, Gui::ALIGN_LEFT);

	create_button_->getEventClicked().connect(*connections_, this, &ExtrusionPlugin::create_clicked);
	select_btn->getEventClicked().connect(*connections_, this, &ExtrusionPlugin::select_node_clicked);
	clear_btn->getEventClicked().connect(*connections_, this, &ExtrusionPlugin::clear_selection_clicked);

	tabs_widget_->addTab("Create Dynamic Node");
}

void ExtrusionPlugin::create_clicked(const WidgetPtr&, int)
{
	if (selected_file_path_.empty())
		return;

	Math::Vec3 pos = Math::Vec3(0.0f, 0.0f, 2.0f);

	NodeDummyPtr root = NodeDummy::create();
	root->setName("new_extrude");
	root->setWorldPosition(pos);
	root->setSaveToWorldEnabled(true);
	root->setShowInEditorEnabled(true);
	root->setEnabled(true);

	PropertyPtr prop = Properties::findProperty("extrude");
	if (prop)
		root->setProperty(0, prop);

	NodeDummyPtr templ = NodeDummy::create();
	templ->setName("template");
	templ->setEnabled(false);
	templ->setSaveToWorldEnabled(true);
	root->addChild(templ);

	NodeDummyPtr spline = NodeDummy::create();
	spline->setName("spline");
	spline->setSaveToWorldEnabled(true);
	root->addChild(spline);

	NodeDummyPtr knot0 = NodeDummy::create();
	knot0->setName("knot0");
	knot0->setPosition(Math::Vec3(-2.0f, 0.0f, 0.0f));  // Local to spline
	knot0->setSaveToWorldEnabled(true);
	spline->addChild(knot0);

	NodeDummyPtr knot1 = NodeDummy::create();
	knot1->setName("knot1");
	knot1->setPosition(Math::Vec3(2.0f, 0.0f, 0.0f));  // Local to spline
	knot1->setSaveToWorldEnabled(true);
	spline->addChild(knot1);

	Unigine::Vector<Unigine::NodePtr> roots;
	World::getRootNodes(roots);
	if (roots.size() > 0)
		roots[0]->addChild(root);

	Log::message("ExtrusionPlugin: Created extrude node hierarchy from %s\n", selected_file_path_.c_str());
}

void ExtrusionPlugin::select_node_clicked(const WidgetPtr&, int)
{
	WidgetDialogFilePtr dialog = WidgetDialogFile::create(gui_, "Select Node File");
	dialog->setFilter("*.node");
	dialog->setHidden(false);
	while (!dialog->isDone())
	{
		// Modal dialog loop - in a real engine context this would be handled by the GUI system
		break;
	}
	const char* path = dialog->isOkClicked() ? dialog->getFile() : nullptr;
	if (path && path[0])
	{
		selected_file_path_ = path;
		file_label_->setText(("File: " + selected_file_path_).c_str());
		create_button_->setEnabled(1);
	}
}

void ExtrusionPlugin::clear_selection_clicked(const WidgetPtr&, int)
{
	selected_file_path_.clear();
	file_label_->setText("File: none selected");
	create_button_->setEnabled(0);
}
