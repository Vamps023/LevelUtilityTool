#include "ExtrusionEngine.h"

#include <UnigineLog.h>
#include <UnigineNodes.h>
#include <UnigineWorld.h>
#include <UnigineMesh.h>
#include <UnigineProperties.h>
#include <UnigineMaterials.h>

#include <algorithm>
#include <cmath>

using namespace Unigine;
using namespace Unigine::Math;

namespace LevelUtility
{

ExtrusionEngine::ExtrusionEngine() = default;
ExtrusionEngine::~ExtrusionEngine() = default;

void ExtrusionEngine::begin_extrude(const NodePtr& extrude_node)
{
	current_extrude_node_ = extrude_node;
	active_ = true;
	last_knots_.clear();
	extruded_mesh_.clear();

	Log::message("ExtrusionEngine: begin_extrude on %s\n", extrude_node->getName());
}

void ExtrusionEngine::end_extrude()
{
	active_ = false;
	current_extrude_node_.clear();
	last_knots_.clear();
	extruded_mesh_.clear();
}

void ExtrusionEngine::update(const NodePtr& extrude_node)
{
	if (!active_ || !extrude_node)
	{
		Log::message("ExtrusionEngine::update: not active or no extrude_node\n");
		return;
	}

	NodePtr spline = find_child_by_name(extrude_node, "spline");
	if (!spline)
	{
		Log::message("ExtrusionEngine::update: no 'spline' child found in %s\n", extrude_node->getName());
		return;
	}

	std::vector<Knot> knots = collect_knots(spline);
	Log::message("ExtrusionEngine::update: found %d knots\n", static_cast<int>(knots.size()));

	if (knots.size() < 2)
	{
		Log::message("ExtrusionEngine::update: not enough knots (< 2)\n");
		return;
	}

	if (!knots_changed(knots))
	{
		Log::message("ExtrusionEngine::update: knots unchanged\n");
		return;
	}

	Log::message("ExtrusionEngine::update: knots CHANGED, rebuilding mesh\n");
	update_last_knots(knots);

	std::vector<Vec3> path = build_spline(knots);
	Log::message("ExtrusionEngine::update: spline has %d points\n", static_cast<int>(path.size()));

	if (path.size() < 2)
	{
		Log::message("ExtrusionEngine::update: path too short\n");
		return;
	}

	NodePtr templ = find_child_by_name(extrude_node, "template");
	extruded_mesh_ = create_or_update_extruded_mesh(path, templ);

	if (extruded_mesh_)
		Log::message("ExtrusionEngine::update: extruded mesh created/updated, vertices=%d indices=%d\n",
			extruded_mesh_->getNumVertex(), extruded_mesh_->getNumIndices());
	else
		Log::message("ExtrusionEngine::update: FAILED to create extruded mesh\n");
}

NodePtr ExtrusionEngine::find_child_by_name(const NodePtr& parent, const char* name) const
{
	if (!parent)
		return NodePtr();

	for (int i = 0; i < parent->getNumChildren(); ++i)
	{
		NodePtr child = parent->getChild(i);
		if (child && strcmp(child->getName(), name) == 0)
			return child;
	}
	return NodePtr();
}

std::vector<ExtrusionEngine::Knot> ExtrusionEngine::collect_knots(const NodePtr& spline_node) const
{
	std::vector<Knot> result;
	if (!spline_node)
		return result;

	for (int i = 0; i < spline_node->getNumChildren(); ++i)
	{
		NodePtr child = spline_node->getChild(i);
		if (!child)
			continue;

		const char* name = child->getName();
		if (name && strncmp(name, "knot", 4) == 0)
		{
			Knot k;
			k.node = child;
			k.last_position = child->getWorldPosition();
			result.push_back(k);
		}
	}

	std::sort(result.begin(), result.end(), [](const Knot& a, const Knot& b)
	{
		return strcmp(a.node->getName(), b.node->getName()) < 0;
	});

	return result;
}

bool ExtrusionEngine::knots_changed(const std::vector<Knot>& knots) const
{
	if (knots.size() != last_knots_.size())
	{
		Log::message("ExtrusionEngine::knots_changed: size mismatch %d vs %d\n",
			static_cast<int>(knots.size()), static_cast<int>(last_knots_.size()));
		return true;
	}

	for (size_t i = 0; i < knots.size(); ++i)
	{
		float dist = (knots[i].last_position - last_knots_[i].last_position).length();
		if (dist > 0.001f)
		{
			Log::message("ExtrusionEngine::knots_changed: knot %d moved by %.4f\n", static_cast<int>(i), dist);
			return true;
		}
	}
	return false;
}

void ExtrusionEngine::update_last_knots(const std::vector<Knot>& knots)
{
	last_knots_.resize(knots.size());
	for (size_t i = 0; i < knots.size(); ++i)
		last_knots_[i].last_position = knots[i].node->getWorldPosition();
}

std::vector<Vec3> ExtrusionEngine::build_spline(const std::vector<Knot>& knots) const
{
	std::vector<Vec3> points;
	points.reserve(knots.size());
	for (const auto& k : knots)
		points.push_back(k.node->getWorldPosition());

	return catmull_rom_spline(points, 8);
}

std::vector<Vec3> ExtrusionEngine::catmull_rom_spline(const std::vector<Vec3>& points, int segments_per_span) const
{
	std::vector<Vec3> result;
	if (points.size() < 2)
		return result;

	if (points.size() == 2)
	{
		result.push_back(points[0]);
		result.push_back(points[1]);
		return result;
	}

	auto get_point = [&](int index) -> Vec3
	{
		int clamped = std::max(0, std::min(static_cast<int>(points.size()) - 1, index));
		return points[clamped];
	};

	for (size_t i = 0; i + 1 < points.size(); ++i)
	{
		Vec3 p0 = get_point(static_cast<int>(i) - 1);
		Vec3 p1 = get_point(static_cast<int>(i));
		Vec3 p2 = get_point(static_cast<int>(i) + 1);
		Vec3 p3 = get_point(static_cast<int>(i) + 2);

		for (int s = 0; s < segments_per_span; ++s)
		{
			float t = static_cast<float>(s) / static_cast<float>(segments_per_span);

			Vec3 pt = (
				(p1 * 2.0f) +
				((p2 - p0) * t) +
				((p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * (t * t)) +
				((p0 * -1.0f + p1 * 3.0f - p2 * 3.0f + p3) * (t * t * t))) * 0.5f;

			result.push_back(pt);
		}
	}

	result.push_back(points.back());
	return result;
}

std::vector<Vec3> ExtrusionEngine::extract_cross_section(const NodePtr& template_node) const
{
	std::vector<Vec3> section;
	if (!template_node)
	{
		// Default 1m x 1m square cross-section for visibility
		section.push_back(Vec3(-0.5f, -0.5f, 0.0f));
		section.push_back(Vec3(0.5f, -0.5f, 0.0f));
		section.push_back(Vec3(0.5f, 0.5f, 0.0f));
		section.push_back(Vec3(-0.5f, 0.5f, 0.0f));
		return section;
	}

	// Try to extract from ObjectMeshDynamic
	ObjectMeshDynamicPtr mesh = template_node->getType() == Node::OBJECT_MESH_DYNAMIC
		? static_ptr_cast<ObjectMeshDynamic>(template_node)
		: ObjectMeshDynamicPtr();

	if (!mesh)
	{
		section.push_back(Vec3(-0.1f, -0.1f, 0.0f));
		section.push_back(Vec3(0.1f, -0.1f, 0.0f));
		section.push_back(Vec3(0.1f, 0.1f, 0.0f));
		section.push_back(Vec3(-0.1f, 0.1f, 0.0f));
		return section;
	}

	int num_verts = mesh->getNumVertex();
	if (num_verts == 0)
	{
		section.push_back(Vec3(-0.1f, -0.1f, 0.0f));
		section.push_back(Vec3(0.1f, -0.1f, 0.0f));
		section.push_back(Vec3(0.1f, 0.1f, 0.0f));
		section.push_back(Vec3(-0.1f, 0.1f, 0.0f));
		return section;
	}

	for (int i = 0; i < std::min(num_verts, 8); ++i)
	{
		vec3 v = mesh->getVertex(i);
		section.push_back(Vec3(v.x, v.y, v.z));
	}

	return section;
}

vec3 ExtrusionEngine::perpendicular_xy(const vec3& dir) const
{
	if (length(dir) < 0.0001f)
		return vec3(1.0f, 0.0f, 0.0f);

	vec3 d = normalize(dir);
	vec3 side(-d.y, d.x, 0.0f);
	if (length(side) < 0.001f)
		side = vec3(1.0f, 0.0f, 0.0f);
	else
		side = normalize(side);
	return side;
}

ObjectMeshDynamicPtr ExtrusionEngine::create_or_update_extruded_mesh(const std::vector<Vec3>& path,
                                                                      const NodePtr& template_node)
{
	if (path.size() < 2)
		return ObjectMeshDynamicPtr();

	std::vector<Vec3> cross_section = extract_cross_section(template_node);
	if (cross_section.size() < 2)
		return ObjectMeshDynamicPtr();

	Log::message("ExtrusionEngine::create_or_update_extruded_mesh: path=%d sections=%d\n",
		static_cast<int>(path.size()), static_cast<int>(cross_section.size()));

	ObjectMeshDynamicPtr mesh;
	NodePtr existing = find_child_by_name(current_extrude_node_, "extruded");
	if (existing && existing->getType() == Node::OBJECT_MESH_DYNAMIC)
	{
		mesh = static_ptr_cast<ObjectMeshDynamic>(existing);
		Log::message("ExtrusionEngine: reusing existing extruded mesh\n");
	}
	else
	{
		mesh = ObjectMeshDynamic::create();
		if (!mesh)
		{
			Log::error("ExtrusionEngine: ObjectMeshDynamic::create() returned null\n");
			return ObjectMeshDynamicPtr();
		}
		mesh->setName("extruded");
		mesh->setSaveToWorldEnabled(true);
		mesh->setShowInEditorEnabled(true);
		mesh->setEnabled(true);
		if (current_extrude_node_)
			current_extrude_node_->addChild(mesh);

		// Add a surface before setting material
		mesh->addSurface("extruded");
		Log::message("ExtrusionEngine: created new ObjectMeshDynamic with surface\n");

		// Assign a default material so the mesh is visible
		// Search all materials (including library materials) for one that supports ObjectMeshDynamic
		MaterialPtr mat;
		int num_mats = Materials::getNumMaterials();
		for (int i = 0; i < num_mats; ++i)
		{
			MaterialPtr candidate = Materials::getMaterial(i);
			if (!candidate)
				continue;

			// Skip editor brushes and other unsupported materials
			if (!candidate->isNodeTypeSupported(Node::OBJECT_MESH_DYNAMIC))
				continue;

			const char* ns_name = candidate->getNamespaceName();
			if (!ns_name)
				continue;

			// Prefer mesh_base or any mesh-related material
			if (strstr(ns_name, "mesh_base") != nullptr ||
			    strstr(ns_name, "primitives") != nullptr ||
			    strstr(ns_name, "sphere_mesh") != nullptr ||
			    strstr(ns_name, "plane_mesh") != nullptr)
			{
				mat = candidate;
				Log::message("ExtrusionEngine: found renderable material '%s'\n", ns_name);
				break;
			}
		}

		// If no mesh material found, use any material that supports ObjectMeshDynamic
		if (!mat)
		{
			for (int i = 0; i < num_mats; ++i)
			{
				MaterialPtr candidate = Materials::getMaterial(i);
				if (candidate && candidate->isNodeTypeSupported(Node::OBJECT_MESH_DYNAMIC))
				{
					mat = candidate;
					Log::message("ExtrusionEngine: using fallback material '%s'\n",
						candidate->getNamespaceName() ? candidate->getNamespaceName() : "unknown");
					break;
				}
			}
		}

		if (mat)
		{
			mesh->setMaterial(mat, 0);
			Log::message("ExtrusionEngine: assigned material successfully\n");
		}
		else
		{
			Log::warning("ExtrusionEngine: NO compatible material found. Mesh will be invisible.\n");
		}
	}

	// Rebuild mesh geometry
	int num_path = static_cast<int>(path.size());
	int num_section = static_cast<int>(cross_section.size());

	// Safety: need at least 2 points in path and 2 points in cross-section
	if (num_path < 2 || num_section < 2)
	{
		Log::warning("ExtrusionEngine: insufficient data: path=%d section=%d\n", num_path, num_section);
		return mesh;
	}

	mesh->clearVertex();
	mesh->clearIndices();

	// Pre-calculate UV divisors to avoid division by zero
	float u_div = static_cast<float>(num_section - 1);
	float v_div = static_cast<float>(num_path - 1);

	// Generate vertices by sweeping cross-section along path
	for (int i = 0; i < num_path; ++i)
	{
		Vec3 pos = path[i];
		Vec3 forward;
		if (i == 0)
			forward = path[1] - path[0];
		else if (i == num_path - 1)
			forward = path[i] - path[i - 1];
		else
			forward = path[i + 1] - path[i - 1];

		vec3 f = normalize(vec3(forward.x, forward.y, forward.z));
		vec3 side = perpendicular_xy(f);
		vec3 up = cross(f, side);

		for (int j = 0; j < num_section; ++j)
		{
			Vec3 local = cross_section[j];
			vec3 world = vec3(pos.x, pos.y, pos.z)
				+ side * local.x
				+ up * local.y
				+ f * local.z;
			mesh->addVertex(world);
			mesh->addTexCoord(vec4(
				static_cast<float>(j) / u_div,
				static_cast<float>(i) / v_div,
				0.0f, 0.0f));
			mesh->addTangent(quat(0.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	// Generate indices for triangle strips between path segments
	for (int i = 0; i < num_path - 1; ++i)
	{
		int base0 = i * num_section;
		int base1 = (i + 1) * num_section;

		for (int j = 0; j < num_section - 1; ++j)
		{
			int a = base0 + j;
			int b = base0 + j + 1;
			int c = base1 + j;
			int d = base1 + j + 1;

			mesh->addIndex(a);
			mesh->addIndex(c);
			mesh->addIndex(b);

			mesh->addIndex(b);
			mesh->addIndex(c);
			mesh->addIndex(d);
		}
	}

	// Only flush if we have valid data
	int num_verts = mesh->getNumVertex();
	int num_indices = mesh->getNumIndices();

	if (num_verts > 0 && num_indices > 0)
	{
		mesh->updateBounds();
		mesh->flushVertex();
		mesh->flushIndices();
		Log::message("ExtrusionEngine: mesh built with %d vertices, %d indices\n", num_verts, num_indices);
	}
	else
	{
		Log::warning("ExtrusionEngine: mesh has no data, skipping flush (verts=%d, indices=%d)\n",
			num_verts, num_indices);
	}

	return mesh;
}

} // namespace LevelUtility
