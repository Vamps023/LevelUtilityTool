
#ifndef __EXTRUDER2EDITORPLUGIN_H__
#define __EXTRUDER2EDITORPLUGIN_H__

/* ---------- headers */

#include <editor/UniginePlugin.h>

#include <QObject>

/* ---------- constants */

/* ---------- definitions */

class IExtruder2Plugin;

namespace Unigine
{
    template <typename Type> class Ptr;
    typedef Ptr<class Gui> GuiPtr;
    class EventConnections;
}

namespace UnigineEditor
{
    class EngineGuiWindow;
}

class QAction;
class QMenu;

class Extruder2EditorPlugin :
    public QObject,
    public UnigineEditor::Plugin
{
    Q_OBJECT

public:
    Extruder2EditorPlugin();

    void show_window();
    void activate();

    /* ---------- UnigineEditor::Plugin interface */

    virtual ~Extruder2EditorPlugin() override;
    virtual bool init() override;
    virtual void shutdown() override;

private:
    bool init_plugin();
    void shutdown_plugin();
    void init_menu();
    void shutdown_menu();
    void init_window();
    void shutdown_window();
    bool init_mounts();
    void shutdown_mounts();

    void selection_changed();

    IExtruder2Plugin* m_plugin;
    UnigineEditor::EngineGuiWindow* m_engine_gui_window;
    Unigine::EventConnections* m_connections;
    QMenu* m_menu;
    bool m_owns_menu;
    QAction *m_activate_action;
    QAction *m_show_window_action;
};

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

#endif // __EXTRUDER2EDITORPLUGIN_H__
