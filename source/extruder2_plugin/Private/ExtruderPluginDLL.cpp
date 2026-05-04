#include "ExtruderPlugin.h"

extern "C" UNIGINE_EXPORT void* CreatePlugin()
{
    Extruder2Plugin* plugin = new Extruder2Plugin();
    assert(plugin != nullptr);
    return plugin;
}

extern "C" UNIGINE_EXPORT void ReleasePlugin(void* plugin_pointer)
{
    assert(plugin_pointer != nullptr);
    Extruder2Plugin* plugin = static_cast<Extruder2Plugin *>(plugin_pointer);
    delete plugin;
}
