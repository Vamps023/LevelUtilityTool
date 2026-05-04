/* ---------- headers */

#include "ExtruderPlugin.h"
#include "ExtruderSystemLogic.h"
#include "ExtruderWorldLogic.h"
#include "ExtruderEditorLogic.h"

#include <UnigineInterface.h>
#include <PluginOrder.h>

#include <functional>

#include <vector>
#include <map>
#include <regex>

#include "unigine2_graphics/unigine2_utilities.h"
#include <UnigineWorld.h>
#include <UnigineObjects.h>
#include <UnigineMaterials.h>
#include <mutex>
#include "common/PropertiesProvider.h"
#include "pugixml/pugixml.hpp"
#include <UnigineFileSystem.h>
#include <Windows.h>


using namespace Unigine;
using namespace Unigine::Math;

static bool _checked(bool value, const char* expression)
{
    if (!value)
    {
        Unigine::Log::error("Check failed '%s'\n", expression);
    }
    else
    {
        Unigine::Log::message("Check passed '%s'\n", expression);
    }
    return value;
}

static void remove_widget_children(WidgetPtr parent)
{
    while (parent->getNumChildren() > 0)
    {
        WidgetPtr child = parent->getChild(0);
        parent->removeChild(child);
    }
}

#define checked(expression) (IsDebuggerPresent() ? !(!_checked(static_cast<bool>(expression), #expression) && (__debugbreak(), 1)) : _checked(static_cast<bool>(expression), #expression))

static int _debug_point;
#define debug_point (_debug_point++)

/* ---------- constants */

#ifndef STRINGIFY
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#endif

/* ---------- definitions */

/* ---------- prototypes */

template <class T> static T* GetUniginePlugin(const char* plugin_name);

/* ---------- globals */




int Extruder2Plugin::s_reference_count = 0;

/* ---------- public code */


namespace ExtrudeProperties
{
    const auto default_properties_file = "Extrude.ini";

   
    namespace TrackDataPath
    {
        const auto HEADER = "TrackData";
        const auto TrackDataPath_HEADER = "WorldTypes";
        const auto TrackDataPath_DEFAULT = "";
    }

    namespace FeaturePlacementPath
    {
        const auto HEADER = "FeaturePlacement";
        const auto FeaturePlacementPath_HEADER = "Network";
        const auto FeaturePlacementPath_DEFAULT = "";
    }
}

Extruder2Plugin* GetExtruder2Plugin()
{
    static Extruder2Plugin instance;
    return &instance;
}

Extruder2Plugin::Extruder2Plugin() :
    m_system_logic(nullptr),
    m_world_logic(nullptr),
    m_editor_logic(nullptr),
    m_selection_callback(nullptr),
    m_gui(),
    m_event_connections(),
    m_tabs_widget()
{
    const std::string ini_extension = ".ini";
    const auto        ini_extension_length = ini_extension.length();
    std::string       properties_file_name = ExtrudeProperties::default_properties_file;

    // Scan through our Batch file arguments for the -world_builer_ini so we can initialise our properties.
    for (int argument_index = 0; argument_index < Unigine::Engine::get()->getNumArgs(); ++argument_index)
    {
        const std::string argument(Unigine::Engine::get()->getArg(argument_index));

        const auto next_arg_should_be_ini = argument == "-Extrude.ini";
        if (next_arg_should_be_ini)
        {
            const auto next_arg_exists = argument_index + 1 < Unigine::Engine::get()->getNumArgs();
            if (next_arg_exists)
            {
                const std::string next_argument(Unigine::Engine::get()->getArg(++argument_index));
                if (next_argument.length() >= ini_extension_length &&
                    next_argument.substr(next_argument.length() - ini_extension_length) == ini_extension)
                    properties_file_name = next_argument;
            }
        }
    }

    auto data_path = Unigine::Engine::get()->getDataPath();

    auto abs_prop_path = Unigine::String::absname(data_path, properties_file_name.c_str());

    if (!Unigine::FileSystem::isFileExist(abs_prop_path))
    {
        Unigine::Log::error("Property file not found: %s\n", abs_prop_path.get());
        return;
    }

    auto tree = Common::Data::LoadPropertyTree(abs_prop_path.get());
    
    m_properties = std::make_unique<Common::Properties>(tree);
    m_extruder = std::make_unique<VisionStudio::ExtruderTool::Extruder>();

    LoadExtrudeProperties();

    InitExtrude();
}

Extruder2Plugin::~Extruder2Plugin()
{
    // should already be cleaned up in shutdown
    assert(m_system_logic == nullptr);
    assert(m_world_logic == nullptr);
    assert(m_editor_logic == nullptr);
}

const char* Extruder2Plugin::get_name()
{
    return STRINGIFY(Extruder2Plugin);
}


int Extruder2Plugin::init()
{
    s_reference_count++;
    if (s_reference_count == 1)
    {
        Log::message("--- Extruder2Plugin ---\n");
        Log::message("Initializing\n");

        init_logic();
        init_interpreter();
    }
    return 1;
}

void Extruder2Plugin::update()
{
    if (m_extruder && m_object)
    {
        m_extruder->update();
    }
}

int Extruder2Plugin::get_order()
{
    return static_cast<int>(PluginOrder::Default);
}

int Extruder2Plugin::shutdown()
{
    s_reference_count--;
    if (s_reference_count == 0)
    {
        Log::message("--- Extruder2Plugin ---\n");
        Log::message("Shutdown\n");

        shutdown_interpreter();
        shutdown_logic();
    }
    return 1;
}

Extruder2SystemLogic * Extruder2Plugin::get_system_logic() const
{
    return m_system_logic;
}

Extruder2WorldLogic * Extruder2Plugin::get_world_logic() const
{
    return m_world_logic;
}

Extruder2EditorLogic * Extruder2Plugin::get_editor_logic() const
{
    return m_editor_logic;
}

void Extruder2Plugin::activate()
{
    Log::message("%s Activated\n", STRINGIFY(Extruder2Plugin));
}


void Extruder2Plugin::build_gui(GuiPtr gui, EventConnections& connections, bool is_editor)
{
    // shouldn't be trying to initialize gui twice
    if (checked(m_gui == nullptr))
    {
        m_gui = gui;

        WidgetVBoxPtr root = WidgetVBox::create();

        m_tabs_widget = WidgetTabBox::create();

        CreateDynamicNodeUIInit();

        m_tabs_widget->arrange();
        m_tabs_widget->setCurrentTab(0);
         
        
        root->setBackgroundColor(vec4(1.0, 0.0, 0.0, 1.0));

        root->addChild(m_tabs_widget, Gui::ALIGN_EXPAND | Gui::ALIGN_TOP);
        root->arrange();
       

        gui->addChild(root, Gui::ALIGN_EXPAND | Gui::ALIGN_TOP);

    }
}

void Extruder2Plugin::register_selection_callback(size_t(*callback)(Unigine::Vector<Unigine::NodePtr>&nodes))
{
    m_selection_callback = callback;
}

/* ---------- private code */

template <class T> static T* GetUniginePlugin(const char* plugin_name)
{
    T* plugin = nullptr;

    auto plugin_id = Engine::get()->findPlugin(plugin_name);
    if (plugin_id == -1)
    {
        Log::error("Cannot find '%s'\n", plugin_name);
        return plugin;
    }

    Unigine::Plugin* plugin_interface = Engine::get()->getPluginInterface(plugin_id);
    if (!plugin_interface)
    {
        Log::error("'%s' plugin interface is null\n", plugin_name);
        return plugin;
    }

    plugin = static_cast<T*>(plugin_interface);
    return plugin;
};

void Extruder2Plugin::init_logic()
{
    Log::message("Initializing Logic\n");

    m_system_logic = new Extruder2SystemLogic();
    m_world_logic = new Extruder2WorldLogic();
    m_editor_logic = new Extruder2EditorLogic();

    Unigine::EnginePtr engine; // calls init() internally in default constructor
    engine->addSystemLogic(m_system_logic);
    engine->addWorldLogic(m_world_logic);
    engine->addEditorLogic(m_editor_logic);
}


void Extruder2Plugin::selection_changed()
{
     Unigine::Vector<Unigine::NodePtr> nodes;
     m_selection_callback(nodes);
     if (nodes.empty())
     {
         if (checked(nodes.empty()))
         {
             if(m_object)
                NodeDeselected(m_object);

             m_object = nullptr;
             return;
         }
     }
     

    auto size = nodes.size();
    int counter = 0;
    bool selection_changed = false;

    for (const auto &node : nodes)
    {
        counter++;
        if (counter == size)
        {
            if (m_object != node)
            {
                if(m_object)
                    NodeDeselected(m_object);

                 m_object = node;
                 selection_changed = true;
            }
        }
    }

    if(selection_changed)
        NodeSelected(m_object);
}

void Extruder2Plugin::shutdown_logic()
{
    Log::message("Shutdown Logic\n");

    assert(m_system_logic != nullptr);
    assert(m_world_logic != nullptr);
    assert(m_editor_logic != nullptr);

    Unigine::EnginePtr engine; // calls init() internally in default constructor
    engine->removeSystemLogic(m_system_logic);
    engine->removeWorldLogic(m_world_logic);
    engine->removeEditorLogic(m_editor_logic);

    delete m_system_logic;
    delete m_world_logic;
    delete m_editor_logic;

    m_system_logic = nullptr;
    m_world_logic = nullptr;
    m_editor_logic = nullptr;
}

void Extruder2Plugin::init_interpreter()
{
    Log::message("Initializing Interpreter\n");

    const char* group_name = get_name();
    int group_id = Interpreter::addGroup(group_name);
}

void Extruder2Plugin::shutdown_interpreter()
{
    Log::message("Shutdown Interpreter\n");

    const char* group_name = get_name();
    Interpreter::removeGroup(group_name);
}

void Extruder2Plugin::CreateDynamicNodeUIInit()
{
    int dynamic_node_tool_index = m_tabs_widget->addTab("Create Dynamic Node Tool");
   
    // Global vbox
    Unigine::WidgetVBoxPtr vbox = Unigine::WidgetVBox::create();
    vbox->setBackground(1);

    m_tabs_widget->addChild(vbox, Gui::ALIGN_EXPAND | Gui::ALIGN_TOP);

    // Label for selected node
    m_file_name = Unigine::WidgetLabel::create("No file selected yet");
    vbox->addChild(m_file_name, Unigine::Gui::ALIGN_TOP);

    // Spacer
    Unigine::WidgetSpacerPtr spacer = Unigine::WidgetSpacer::create();
    vbox->addChild(spacer, Unigine::Gui::ALIGN_TOP);

    // Buttons in an hbox
    Unigine::WidgetHBoxPtr hbox = Unigine::WidgetHBox::create();

    Unigine::WidgetButtonPtr build_button = Unigine::WidgetButton::create("Create New");
    build_button->getEventClicked().connect(m_event_connections, [this] { create_clicked(); });
    

    build_button->setEnabled(false);
    hbox->addChild(build_button, Unigine::Gui::ALIGN_LEFT | Unigine::Gui::ALIGN_EXPAND);

    Unigine::WidgetButtonPtr select_node_button = Unigine::WidgetButton::create("Select Node...");
    select_node_button->getEventClicked().connect(m_event_connections, [this] { select_node_clicked(); });
    hbox->addChild(select_node_button, Unigine::Gui::ALIGN_LEFT | Unigine::Gui::ALIGN_EXPAND);

    Unigine::WidgetButtonPtr cancel_button = Unigine::WidgetButton::create("Clear Selection");
    cancel_button->getEventClicked().connect(m_event_connections, [this] { cancel_clicked(); });

    hbox->addChild(cancel_button, Unigine::Gui::ALIGN_LEFT | Unigine::Gui::ALIGN_EXPAND);

    vbox->addChild(hbox, Unigine::Gui::ALIGN_TOP| Unigine::Gui::ALIGN_EXPAND);

    // Enable the build button only when a file has been selected.
    m_file_name->getEventChanged().connect(
        m_event_connections,
        [build_button](const Unigine::Ptr<Unigine::Widget>& widget_ptr)
        {
            const auto label{Unigine::static_ptr_cast<Unigine::WidgetLabel>(widget_ptr)};
            build_button->setEnabled(Unigine::FileSystem::isFileExist(label->getText()));
        });

}

 void Extruder2Plugin::create_clicked()
{
    std::string path = m_file_name->getText();
    if (!Unigine::FileSystem::isFileExist(path.c_str()))
        return;

    Unigine::NodeDummyPtr extrude_node = Unigine::NodeDummy::create();
    extrude_node->setName("new_extrude");
    extrude_node->setProperty("extrude");
    extrude_node->setSaveToWorldEnabled(true);

    Unigine::PlayerPtr editor_player = Unigine::Editor::getPlayer();
    extrude_node->setWorldPosition(editor_player->getWorldPosition());

    Unigine::NodeReferencePtr template_node = Unigine::NodeReference::create(path.c_str());
    template_node->setName("template");
    template_node->setEnabled(false);
    template_node->setSaveToWorldEnabled(true);
    extrude_node->addChild(template_node);

    Unigine::NodeDummyPtr spline_node = Unigine::NodeDummy::create();
    spline_node->setName("spline");
    spline_node->setSaveToWorldEnabled(true);
    extrude_node->addChild(spline_node);

    Unigine::NodeDummyPtr knot0_node = Unigine::NodeDummy::create();
    knot0_node->setName("knot0");
    knot0_node->setSaveToWorldEnabled(true);
    spline_node->addChild(knot0_node);
    knot0_node->setPosition(Unigine::Math::Vec3(
        editor_player->getDirection(editor_player->isPlayer() ? Unigine::Math::AXIS_NZ : Unigine::Math::AXIS_Y) +
        editor_player->getDirection(Unigine::Math::AXIS_NX)));

    Unigine::NodeDummyPtr knot1_node = Unigine::NodeDummy::create();
    knot1_node->setName("knot1");
    knot1_node->setSaveToWorldEnabled(true);
    spline_node->addChild(knot1_node);
    knot1_node->setPosition(Unigine::Math::Vec3(
        editor_player->getDirection(editor_player->isPlayer() ? Unigine::Math::AXIS_NZ : Unigine::Math::AXIS_Y) -
        editor_player->getDirection(Unigine::Math::AXIS_NX)));

    extrude_node->setOwner(false);
    extrude_node->setShowInEditorEnabledRecursive(true);
}

void Extruder2Plugin::cancel_clicked()
{
    m_file_name->setText("No file selected yet");
}

Unigine::EngineWindowViewportPtr Extruder2Plugin::getFileSelectionWindow()
{
    if (m_file_selection_window == nullptr)
    {
        m_file_selection_window = Unigine::EngineWindowViewport::create("Select a file...", 600, 500);
        m_file_selection_window->setCanBeNested(false);
        m_file_selection_window->setCanCreateGroup(false);

        auto widget = Unigine::WidgetDialogFile::create("Select a file...");
        widget->setFilter(".node");
        widget->setPermanentFocus();
        widget->setMoveable(false);
        widget->setPath(m_file_selection_path.c_str());

        widget->getOkButton()->getEventClicked().connect([widget, this] {
            if (Unigine::FileSystem::isFileExist(widget->getFile()))
                m_file_name->setText(widget->getFile());

            m_file_selection_path = widget->getPath();
            m_file_selection_window.deleteLater();
        });

        widget->getCancelButton()->getEventClicked().connect([this] {
            m_file_selection_window.deleteLater();
        });

        m_file_selection_window->getGui()->addChild(widget, Unigine::Gui::ALIGN_EXPAND | Unigine::Gui::ALIGN_OVERLAP);
    }

    return m_file_selection_window;
}

void Extruder2Plugin::select_node_clicked()
{
    auto window{getFileSelectionWindow()};

    window->show();
    window->setSystemFocus();
    window->setAlwaysOnTop(true);
}


void Extruder2Plugin::NodeSelected(Unigine::NodePtr node)
{
    if (!m_extruder)
        return;

    auto extrude_node = node;
    std::string node_name = node->getName();

    while (extrude_node)
    {
        auto num_properties = extrude_node->getNumProperties();
        for (auto i = 0; i < num_properties; i++)
        {
            if (auto property = extrude_node->getProperty(i))
            {
                std::string property_name = property->getName();
                if (property_name == "extrude")
                {
                    m_extruder->begin_extrude(extrude_node);
                    m_updating_splines = true;
                    return;
                }
            }
        }

        extrude_node = extrude_node->getParent();
    }

    // Didn't find the "extrude" property, begin extrude using "_extrudes" parent

    auto ancestor = node;
    while (ancestor->getParent())
    {
        ancestor = ancestor->getParent();
        if (std::string(ancestor->getName()) == "_extrudes")
        {
            // Something is happening with the extruder
            // Find which spline is being updated

            auto spline = node;
            if (spline != ancestor)
            {
                while (spline->getParent() != ancestor)
                    spline = spline->getParent();

                // Begin updating the spline
                m_extruder->begin_extrude(spline);
                m_updating_splines = true;
                return;
            }
        }
    }
}

void Extruder2Plugin::NodeDeselected(Unigine::NodePtr node)
{
    if (m_updating_splines && m_extruder)
    {
        m_extruder->end_extrude();
    }

    m_updating_splines = false;
}

void Extruder2Plugin::InitExtrude()
{
    if(m_extruder)
        m_extruder->LoadNetworkData(m_network_data, m_world_type_data);
}

void Extruder2Plugin::LoadExtrudeProperties()
{
    m_world_type_path = m_properties->GetProperty<std::string>(ExtrudeProperties::TrackDataPath::HEADER,
        ExtrudeProperties::TrackDataPath::TrackDataPath_HEADER,
        ExtrudeProperties::TrackDataPath::TrackDataPath_DEFAULT);


     m_network_data_path = m_properties->GetProperty<std::string>(ExtrudeProperties::FeaturePlacementPath::HEADER,
        ExtrudeProperties::FeaturePlacementPath::FeaturePlacementPath_HEADER,
        ExtrudeProperties::FeaturePlacementPath::FeaturePlacementPath_DEFAULT);

    pugi::xml_document base_network_doc;

    // Load the XML file into the document
    pugi::xml_parse_result base_network_result = base_network_doc.load_file(m_network_data_path.c_str());

    // Check if loading was successful
    if (!base_network_result) {
        return;
    }

     // Create an output stream to hold the XML string
    std::ostringstream base_network_stringstream;

    // Save the XML document into the output stream, which will preserve its structure
    base_network_doc.save(base_network_stringstream, "");  // Adding indentation for better readability

    // Get the XML content as a string
     m_network_data = base_network_stringstream.str();


     pugi::xml_document world_type_doc;

    // Load the XML file into the document
    pugi::xml_parse_result world_type_result = world_type_doc.load_file(m_world_type_path.c_str());

    // Check if loading was successful
    if (!world_type_result) {
        return;
    }

     // Create an output stream to hold the XML string
    std::ostringstream world_type_stringstream;

    // Save the XML document into the output stream, which will preserve its structure
    world_type_doc.save(world_type_stringstream, "");  // Adding indentation for better readability

    // Get the XML content as a string
    m_world_type_data = world_type_stringstream.str();
}
