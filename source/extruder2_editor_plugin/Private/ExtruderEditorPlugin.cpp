/* ---------- headers */

#include "ExtruderEditorPlugin.h"
#include <ExtruderPluginInterface.h>
#include <MountPluginInterface.h>

// Unigine Editor headers
#include <editor/UnigineConstants.h>
#include <editor/UnigineWindowManager.h>
#include <editor/UnigineEngineGuiWindow.h>
#include <editor/UnigineSelection.h>
#include <editor/UnigineSelector.h>

// Unigine headers
#include <UnigineLog.h>
#include <UnigineEditor.h>
#include <UnigineInterface.h>
#include <UnigineLogic.h>
#include <UnigineNode.h>
#include <UnigineInterpreter.h>

// Qt headers
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>

// Windows headers
#include <Windows.h>
#include <Psapi.h>
#include <shlwapi.h>

// STL headers
#include <filesystem>

using namespace Unigine;
using namespace UnigineEditor;

/* ---------- constants */

#ifndef STRINGIFY
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#endif

/* ---------- definitions */

/* ---------- prototypes */

static QMenuBar* GetUnigineMenuBar();

/* ---------- globals */

/* ---------- public code */

Extruder2EditorPlugin::Extruder2EditorPlugin() :
    m_plugin(nullptr),
    m_engine_gui_window(nullptr),
    m_connections(nullptr),
    m_menu(nullptr),
    m_activate_action(nullptr),
    m_show_window_action(nullptr)
{

}

Extruder2EditorPlugin::~Extruder2EditorPlugin()
{
    // should already be cleaned up in shutdown
    assert(m_plugin == nullptr);
    assert(m_engine_gui_window == nullptr);
    assert(m_connections == nullptr);
    assert(m_menu == nullptr);
    assert(m_activate_action == nullptr);
    assert(m_show_window_action == nullptr);
}

bool Extruder2EditorPlugin::init()
{
    Log::message("--- Extruder2EditorPlugin ---\n");
    Log::message("Initializing\n");

    bool result = init_plugin();
    if (result)
    {
        init_menu();
        init_window();
        init_mounts();
    }
    return result;
}

void Extruder2EditorPlugin::shutdown()
{
    Log::message("--- Extruder2EditorPlugin --\n");
    Log::message("Shutdown\n");

    shutdown_mounts();
    shutdown_window();
    shutdown_menu();
    shutdown_plugin();
}

void Extruder2EditorPlugin::show_window()
{
    assert(m_engine_gui_window != nullptr);
    UnigineEditor::WindowManager::show(m_engine_gui_window);
}

void Extruder2EditorPlugin::activate()
{
    assert(m_plugin != nullptr);
    m_plugin->activate();
}

/* ---------- private code */

static QMenuBar* GetUnigineMenuBar()
{
	// #TODO: Is there a better way to do this?

	QMenu* editor_help_menu = UnigineEditor::WindowManager::findMenu(Constants::MM_HELP);
	assert(editor_help_menu != nullptr);

	QObject* editor_help_menu_parent = editor_help_menu->parent(); // internal MainMenu type
	assert(editor_help_menu_parent != nullptr);

	QMenuBar* editor_menu_bar = qobject_cast<QMenuBar*>(editor_help_menu_parent);
	assert(editor_menu_bar != nullptr);

	return editor_menu_bar;
}

static QMenu* GetOrAddMenu(QMenuBar* menu_bar, QString const& menu_name, bool& owns_menu)
{
    // iterate over existing actions in the menu bar
    for (QAction* action : menu_bar->actions())
    {
        QMenu* menu = action->menu();
        if (menu != nullptr && menu->title() == menu_name)
        {
            owns_menu = false;
            return menu;
        }
    }

    // not found. make new menu.
    owns_menu = true;
    QMenu* menu = menu_bar->addMenu(menu_name);
    return menu;
}

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

#define checked(expression) (IsDebuggerPresent() ? !(!_checked((expression), #expression) && (__debugbreak(), 1)) : _checked(expression, #expression))


void getSelectedNodes(Unigine::Vector<Unigine::NodePtr>& nodes)
{
    nodes.clear();

    if (const auto selector{ UnigineEditor::Selection::getSelectorNodes() };
        selector != nullptr)
    {
        for (auto& node : selector->getNodes())
            nodes.append(node);
    }
}

static size_t selection_cb(Unigine::Vector<Unigine::NodePtr>& nodes)
{
    nodes.clear();

    Selection * selection_instance = Selection::instance();
    if (checked(selection_instance != nullptr))
    {
        SelectorNodes const* selection_nodes = selection_instance->getSelectorNodes();
        if(selection_nodes == nullptr)
            return 0;

        if (checked(selection_nodes != nullptr))
        {
            nodes = selection_nodes->getNodes();
        }
    }

    return nodes.size();
};

bool Extruder2EditorPlugin::init_plugin()
{
    Log::message("Init Plugin\n");

    bool result = false;

    m_plugin = GetExtruder2PluginInterface();
    if (m_plugin != nullptr)
    {
        Log::message("Found %s plugin\n", STRINGIFY(Extruder2Plugin));

        m_plugin->register_selection_callback(selection_cb);

        UnigineEditor::Selection *sel = UnigineEditor::Selection::instance();
        connect(sel, &UnigineEditor::Selection::changed,
                this, &Extruder2EditorPlugin::selection_changed);

        result = true;
    }
    else
    {
        Log::message("Failed to find %s plugin\n", STRINGIFY(Extruder2Plugin));
        result = false;
    }

    return result;
}

void Extruder2EditorPlugin::selection_changed()
{
    if(m_plugin)
        m_plugin->selection_changed();
}

void Extruder2EditorPlugin::shutdown_plugin()
{
    Log::message("Shutdown Plugin\n");

    UnigineEditor::Selection *sel = UnigineEditor::Selection::instance();
    disconnect(sel, &UnigineEditor::Selection::changed,
           this, &Extruder2EditorPlugin::selection_changed);

    m_plugin = nullptr;
}

void Extruder2EditorPlugin::init_menu()
{
    Log::message("Init Menu\n");

    // adding an item for the Assets Plugin to the Windows menu of the UnigineEditor
    // QMenu *editor_menu = UnigineEditor::WindowManager::findMenu(Constants::MM_TOOLS);
    QMenuBar* editor_menu_bar = GetUnigineMenuBar();
    assert(editor_menu_bar != nullptr);

    m_menu = GetOrAddMenu(editor_menu_bar, "World Builder", m_owns_menu);
    assert(m_menu != nullptr);

    m_show_window_action = m_menu->addAction("Extrusion Tool (Show)", this, &Extruder2EditorPlugin::show_window);
    assert(m_show_window_action != nullptr);
}

void Extruder2EditorPlugin::shutdown_menu()
{
    Log::message("Shutdown Menu\n");

    if (m_menu)
    {
        if (m_activate_action)
        {
            m_menu->removeAction(m_activate_action);
            m_activate_action = nullptr;
        }

        if (m_show_window_action)
        {
            m_menu->removeAction(m_show_window_action);
            m_show_window_action = nullptr;
        }

        if (m_owns_menu)
        {
            delete m_menu;
        }
        m_menu = nullptr;
    }
}

void Extruder2EditorPlugin::init_window()
{
    if (m_engine_gui_window == nullptr)
    {
        m_engine_gui_window = new EngineGuiWindow();
        m_engine_gui_window->setWindowTitle("Extrusion Tool");
        m_engine_gui_window->setRenderingPolicy(EngineGuiWindow::RENDERING_POLICY_WINDOW_VISIBLE);

        GuiPtr gui = m_engine_gui_window->getGui();

        m_connections = new EventConnections();

        assert(m_plugin != nullptr);
        assert(m_connections != nullptr);
        m_plugin->build_gui(gui, *m_connections, true);

        UnigineEditor::WindowManager::add(m_engine_gui_window, UnigineEditor::WindowManager::LAST_USED_AREA);
    }
}

void Extruder2EditorPlugin::shutdown_window()
{
    if (m_engine_gui_window != nullptr)
    {
        assert(m_engine_gui_window != nullptr);
        UnigineEditor::WindowManager::remove(m_engine_gui_window);

        assert(m_connections != nullptr);
        delete m_connections;
        m_connections = nullptr;

        delete m_engine_gui_window;
        m_engine_gui_window = nullptr;
    }
}

static char const* get_current_module_path()
{
    static HMODULE current_module = nullptr;
    static bool get_module_handle_result = GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&get_current_module_path),
        &current_module);
    static char current_module_path[MAX_PATH];
    static DWORD get_module_file_name_result = GetModuleFileNameExA(GetCurrentProcess(), current_module, current_module_path, MAX_PATH);
    static BOOL path_remove_file_spec_result = PathRemoveFileSpecA(current_module_path);
    return current_module_path;
}

bool Extruder2EditorPlugin::init_mounts()
{
    bool result = false;

    IMountPlugin* mount_plugin = GetMountPluginInterface();
    if (mount_plugin != nullptr)
    {
        std::filesystem::path module_path = get_current_module_path();
        std::string module_path_str = module_path.u8string();
        result = mount_plugin->mount_path("plugins", module_path_str.c_str());
    }

    return result;
}

void Extruder2EditorPlugin::shutdown_mounts()
{
    if (m_engine_gui_window != nullptr)
    {
        assert(m_engine_gui_window != nullptr);
        UnigineEditor::WindowManager::remove(m_engine_gui_window);

        assert(m_connections != nullptr);
        delete m_connections;
        m_connections = nullptr;

        delete m_engine_gui_window;
        m_engine_gui_window = nullptr;
    }
}
