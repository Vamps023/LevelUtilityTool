
#ifndef __EXTRUDER2PLUGININTERFACE_H__
#define __EXTRUDER2PLUGININTERFACE_H__

/* ---------- headers */

/* ---------- constants */

/* ---------- definitions */

#include <Unigine.h> // $todo forward declare properly

#if defined(_MSC_VER)
    #if defined(LevelUtilityTool_EXPORTS)
        #define EXTRUDER2PLUGIN_EXPORT __declspec(dllexport)
    #else
        #define EXTRUDER2PLUGIN_EXPORT __declspec(dllimport)
    #endif
#else
    #define EXTRUDER2PLUGIN_EXPORT
#endif

class IExtruder2Plugin
{
public:
    virtual void activate() = 0;
    virtual void build_gui(Unigine::GuiPtr gui, Unigine::EventConnections& connections, bool is_editor) = 0;
    virtual void register_selection_callback(size_t(*callback)(Unigine::Vector<Unigine::NodePtr>& nodes)) = 0;
    virtual void selection_changed() = 0;
};

/* ---------- prototypes */

extern EXTRUDER2PLUGIN_EXPORT IExtruder2Plugin* GetExtruder2PluginInterface();

/* ---------- globals */

/* ---------- public code */

#endif // __EXTRUDER2PLUGININTERFACE_H__
