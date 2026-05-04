/// @file  extruder.h
/// @brief Stub replacement for Oksygen's VisionStudio::ExtruderTool::Extruder class.
///        Provides the same public interface so ExtruderPlugin compiles standalone.
///        All heavy operations are no-ops until the real library is linked.

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <UnigineNode.h>
#include <UnigineLog.h>

namespace VisionStudio
{
    namespace ExtruderTool
    {
        class Extruder
        {
        public:
            Extruder()
            {
                Unigine::Log::message("Extruder: Stub created (standalone mode)\n");
            }

            ~Extruder() = default;

            Extruder(const Extruder&) = delete;

            bool LoadNetworkData(const std::string& network_definition_data, const std::string& world_types_data)
            {
                Unigine::Log::message("Extruder: LoadNetworkData stub called (network=%zu bytes, world_types=%zu bytes)\n",
                    network_definition_data.size(), world_types_data.size());
                return !network_definition_data.empty() && !world_types_data.empty();
            }

            void begin_extrude(const Unigine::NodePtr& spline_parent_node)
            {
                Unigine::Log::message("Extruder: begin_extrude stub called\n");
            }

            void end_extrude()
            {
                Unigine::Log::message("Extruder: end_extrude stub called\n");
            }

            void update()
            {
                // No-op in stub mode
            }

            void update_snap_to_track(double xOffset, double yOffset, bool saveToWorld = true)
            {
                Unigine::Log::message("Extruder: update_snap_to_track stub called\n");
            }

            void Invalidate()
            {
                Unigine::Log::message("Extruder: Invalidate stub called\n");
            }
        };
    }
}
