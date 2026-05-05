#pragma once

#include <UnigineNode.h>
#include <UnigineMathLib.h>
#include <UnigineObjects.h>
#include <UnigineVector.h>

#include <vector>
#include <memory>

namespace LevelUtility
{

/// Simple spline extrusion engine that works with Unigine nodes only.
/// No external dependencies beyond the Unigine SDK.
class ExtrusionEngine
{
public:
	ExtrusionEngine();
	~ExtrusionEngine();

	void begin_extrude(const Unigine::NodePtr& extrude_node);
	void update(const Unigine::NodePtr& extrude_node);
	void end_extrude();

private:
	struct Knot
	{
		Unigine::NodePtr node;
		Unigine::Math::Vec3 last_position;
	};

	std::vector<Knot> collect_knots(const Unigine::NodePtr& spline_node) const;
	bool knots_changed(const std::vector<Knot>& knots) const;
	void update_last_knots(const std::vector<Knot>& knots);

	std::vector<Unigine::Math::Vec3> build_spline(const std::vector<Knot>& knots) const;
	std::vector<Unigine::Math::Vec3> catmull_rom_spline(const std::vector<Unigine::Math::Vec3>& points, int segments_per_span) const;

	Unigine::NodePtr find_child_by_name(const Unigine::NodePtr& parent, const char* name) const;
	Unigine::ObjectMeshDynamicPtr create_or_update_extruded_mesh(const std::vector<Unigine::Math::Vec3>& path,
	                                                              const Unigine::NodePtr& template_node);

	std::vector<Unigine::Math::Vec3> extract_cross_section(const Unigine::NodePtr& template_node) const;
	Unigine::Math::vec3 perpendicular_xy(const Unigine::Math::vec3& dir) const;

	bool active_ = false;
	Unigine::NodePtr current_extrude_node_;
	Unigine::ObjectMeshDynamicPtr extruded_mesh_;
	std::vector<Knot> last_knots_;
};

} // namespace LevelUtility
