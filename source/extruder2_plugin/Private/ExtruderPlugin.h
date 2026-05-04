
#ifndef __EXTRUDER2PLUGIN_H__
#define __EXTRUDER2PLUGIN_H__

/* ---------- headers */

#include <UniginePlugin.h>
#include <memory>
#include "ExtruderPluginInterface.h"


#include <unigine2_extruder/extruder.h>


/* ---------- constants */

/* ---------- definitions */

class Extruder2SystemLogic;
class Extruder2WorldLogic;
class Extruder2EditorLogic;


namespace Common
{
    class Properties;
}

class Extruder2Plugin :
    public Unigine::Plugin,
    public IExtruder2Plugin
{
public:
    Extruder2Plugin();

    /* ---------- Unigine::Plugin interface */

    virtual ~Extruder2Plugin() override;
    virtual const char *get_name() override;
    virtual int init() override;
    virtual int shutdown() override;
    virtual void update() override;
    virtual int get_order() override;
    virtual void selection_changed() override;

    /* ---------- IExtruder2Plugin interface */

    virtual void activate() override;
    virtual void build_gui(Unigine::GuiPtr gui, Unigine::EventConnections& connections, bool is_editor) override;
    virtual void register_selection_callback(size_t(*callback)(Unigine::Vector<Unigine::NodePtr>& nodes)) override;

    Extruder2SystemLogic* get_system_logic() const;
    Extruder2WorldLogic* get_world_logic() const;
    Extruder2EditorLogic* get_editor_logic() const;
    

private:
    void init_logic();
    void shutdown_logic();
    void init_interpreter();
    void shutdown_interpreter();

    void CreateDynamicNodeUIInit();
    void create_clicked();
    void cancel_clicked();
    void select_node_clicked();
    Unigine::EngineWindowViewportPtr getFileSelectionWindow();
    void NodeSelected(Unigine::NodePtr node);
    void NodeDeselected(Unigine::NodePtr node);
    void InitExtrude();
    void LoadExtrudeProperties();

    Extruder2SystemLogic* m_system_logic;
    Extruder2WorldLogic* m_world_logic;
    Extruder2EditorLogic* m_editor_logic;
    size_t(*m_selection_callback)(Unigine::Vector<Unigine::NodePtr>& nodes);

    Unigine::GuiPtr m_gui;
    Unigine::EventConnections m_event_connections;
    Unigine::WidgetTabBoxPtr m_tabs_widget;
    Unigine::WidgetLabelPtr          m_file_name;
    Unigine::EngineWindowViewportPtr m_file_selection_window;

    std::string m_file_selection_path{Unigine::Engine::get()->getDataPath()};
    std::unique_ptr<VisionStudio::ExtruderTool::Extruder>  m_extruder;
    Unigine::NodePtr    m_object;
    bool                m_updating_splines;
    std::unique_ptr<Common::Properties>      m_properties;
    std::string    m_world_type_path;
    std::string    m_network_data_path;
    std::string    m_world_type_data;
    std::string    m_network_data;

    static int s_reference_count;
};

/* ---------- prototypes */

extern Extruder2Plugin* GetExtruder2Plugin();

/* ---------- globals */

/* ---------- public code */

#endif // __EXTRUDER2PLUGIN_H__
