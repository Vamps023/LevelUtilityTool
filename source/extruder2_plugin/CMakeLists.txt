unigine_plugin("Extruder2Plugin" "Extruder2" "Plugin")
target_link_libraries("Extruder2Plugin" PRIVATE "PluginOrder")
target_link_libraries("Extruder2Plugin" PUBLIC "unigine2_graphics")
target_link_libraries("Extruder2Plugin" PUBLIC "unigine2_extruder")