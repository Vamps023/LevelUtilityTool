/* ---------- headers */

#include "ExtruderPluginInterface.h"
#include "ExtruderPlugin.h"

/* ---------- constants */

/* ---------- definitions */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

EXTRUDER2PLUGIN_EXPORT IExtruder2Plugin* GetExtruder2PluginInterface()
{
    Extruder2Plugin* extruder_2_plugin = GetExtruder2Plugin();
    return extruder_2_plugin;
}

/* ---------- private code */
