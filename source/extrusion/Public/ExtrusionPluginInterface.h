#pragma once

#include <UnigineNode.h>
#include <UnigineGui.h>
#include <UnigineEvent.h>

#ifdef EXTRUSIONPLUGIN_EXPORT
	#define EXTRUSION_API __declspec(dllexport)
#else
	#define EXTRUSION_API __declspec(dllimport)
#endif

class IExtrusionPlugin
{
public:
	virtual ~IExtrusionPlugin() = default;

	virtual void activate() = 0;
	virtual void build_gui(Unigine::GuiPtr gui, Unigine::EventConnections& connections, bool is_editor) = 0;
	virtual void register_selection_callback(size_t(*callback)(Unigine::Vector<Unigine::NodePtr>& nodes)) = 0;
	virtual void selection_changed() = 0;
};

// GetExtrusionPluginInterface() is defined in the runtime plugin DLL.
// When both runtime and editor are in the same binary, use the local instance directly.
extern "C" EXTRUSION_API IExtrusionPlugin* GetExtrusionPluginInterface();
