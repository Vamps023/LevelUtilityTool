#include <UnigineEditor.h>
#include <UnigineLog.h>

// Include your plugin class header
#include "ExtruderEditorPlugin.h" 

extern "C"
{
    // UNIGINE_EXPORT handles the __declspec(dllexport) required on Windows
    UNIGINE_EXPORT void* CreatePlugin()
    {
        // Safety check to ensure we are in the Editor context
        // Extruder2EditorPlugin derives from Unigine::Editor::Plugin
        if (Unigine::Editor::isLoaded())
        {
            // Log the build configuration for debugging
#ifdef NDEBUG
            Unigine::Log::message("ExtruderEditor: Initializing Plugin in RELEASE mode.\n");
#else
            Unigine::Log::message("ExtruderEditor: Initializing Plugin in DEBUG mode.\n");
#endif

            Extruder2EditorPlugin* plugin = new Extruder2EditorPlugin();

            // Ensure the plugin was created
            if (plugin)
                return static_cast<UnigineEditor::Plugin*>(plugin);
        }

        return nullptr;
    }

    // Correctly release the Editor Plugin instance
    UNIGINE_EXPORT void ReleasePlugin(void* plugin_pointer)
    {
        if (plugin_pointer == nullptr)
            return;

        // Cast back to the derived type to ensure the correct destructor is called
        Extruder2EditorPlugin* plugin = static_cast<Extruder2EditorPlugin*>(plugin_pointer);
        delete plugin;
    }
}