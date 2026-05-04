#include "NodeGenerationService.h"

#include <UnigineLog.h>
#include <UnigineMathLib.h>
#include <UnigineNodes.h>
#include <UniginePathFinding.h>
#include <UnigineWorld.h>
#include <UnigineWorlds.h>
#include <editor/UnigineSelection.h>
#include <editor/UnigineSelector.h>

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace
{
constexpr const char* ROOT_NAME = "Generated_Obstacle_Root";
constexpr int BATCH_SIZE = 16;
constexpr float MIN_SIZE = 0.001f;

Unigine::Math::vec3 clampSize(const Unigine::Math::vec3& size)
{
	return Unigine::Math::vec3(
		std::max(std::abs(size.x), MIN_SIZE),
		std::max(std::abs(size.y), MIN_SIZE),
		std::max(std::abs(size.z), MIN_SIZE));
}

const char* typeName(LevelUtility::GenerationType type)
{
	return type == LevelUtility::GenerationType::ObstacleBox ? "ObstacleBox" : "WorldOccluder";
}
}

namespace LevelUtility
{

NodeGenerationService::NodeGenerationService(QObject* parent)
	: QObject(parent)
{
}

std::vector<Unigine::NodePtr> NodeGenerationService::collectSelectedNodeReferences() const
{
	std::vector<Unigine::NodePtr> results;
	std::unordered_set<int> ids;

	const UnigineEditor::SelectorNodes* selector = UnigineEditor::Selection::getSelectorNodes();
	if (!selector || selector->empty())
		return results;

	const Unigine::Vector<Unigine::NodePtr>& selected = selector->getNodes();
	results.reserve(static_cast<size_t>(selected.size()));

	for (int i = 0; i < selected.size(); ++i)
	{
		Unigine::NodePtr node = selected[i];
		if (!node || node->getType() != Unigine::Node::NODE_REFERENCE)
			continue;

		const int id = node->getID();
		if (!ids.insert(id).second)
			continue;

		results.push_back(node);
	}

	return results;
}

int NodeGenerationService::countSelectedNodeReferences() const
{
	return static_cast<int>(collectSelectedNodeReferences().size());
}

bool NodeGenerationService::beginGeneration(GenerationType type, bool groupUnderDummy)
{
	if (processing_)
		return false;

	std::vector<Unigine::NodePtr> selected = collectSelectedNodeReferences();
	if (selected.empty())
	{
		result_ = {};
		emit generationFinished(result_);
		return false;
	}

	pending_.clear();
	for (const Unigine::NodePtr& node : selected)
		pending_.push_back(node);

	result_ = {};
	result_.requested = static_cast<int>(pending_.size());
	activeType_ = type;
	groupUnderDummy_ = groupUnderDummy;
	root_ = nullptr;
	processing_ = true;

	Unigine::Log::message("LevelUtilityTool: Queued %d Node References for %s generation\n",
		result_.requested,
		typeName(type));

	return true;
}

void NodeGenerationService::processNextBatch()
{
	if (!processing_)
		return;

	int processed = 0;
	while (!pending_.empty() && processed < BATCH_SIZE)
	{
		Unigine::NodePtr source = pending_.front();
		pending_.pop_front();
		++processed;

		if (!source || source->getType() != Unigine::Node::NODE_REFERENCE)
		{
			++result_.skipped;
			continue;
		}

		Unigine::NodePtr generated = createGeneratedNode(source, activeType_);
		if (!generated)
		{
			++result_.skipped;
			continue;
		}

		if (groupUnderDummy_)
		{
			Unigine::NodePtr root = ensureRoot();
			if (root)
				root->addChild(generated);
		}

		++result_.created;
	}

	emit batchProgress(result_.created, result_.requested);

	if (pending_.empty())
	{
		processing_ = false;
		Unigine::World::saveWorld();
		Unigine::Log::message("LevelUtilityTool: Created %d nodes, skipped %d nodes\n",
			result_.created,
			result_.skipped);
		emit generationFinished(result_);
	}
}

Unigine::NodePtr NodeGenerationService::ensureRoot()
{
	if (root_)
		return root_;

	Unigine::NodeDummyPtr root = Unigine::NodeDummy::create();
	if (!root)
	{
		Unigine::Log::error("LevelUtilityTool: Failed to create %s\n", ROOT_NAME);
		return nullptr;
	}

	root->setName(ROOT_NAME);
	root->setSaveToWorldEnabled(true);
	root->setShowInEditorEnabled(true);
	root->setEnabled(true);
	root_ = root;
	return root_;
}

Unigine::NodePtr NodeGenerationService::createGeneratedNode(const Unigine::NodePtr& source, GenerationType type)
{
	const Unigine::Math::vec3 size = getSafeWorldBoundsSize(source);

	if (type == GenerationType::ObstacleBox)
	{
		Unigine::ObstacleBoxPtr obstacle = Unigine::ObstacleBox::create(size);
		if (!obstacle)
			return nullptr;

		configureGeneratedNode(obstacle, source, "Generated_ObstacleBox");
		return obstacle;
	}

	Unigine::WorldOccluderPtr occluder = Unigine::WorldOccluder::create(size);
	if (!occluder)
		return nullptr;

	configureGeneratedNode(occluder, source, "Generated_WorldOccluder");
	return occluder;
}

Unigine::Math::vec3 NodeGenerationService::getSafeWorldBoundsSize(const Unigine::NodePtr& source) const
{
	if (!source)
		return Unigine::Math::vec3_one;

	Unigine::Math::WorldBoundBox bounds = source->getHierarchyWorldBoundBox(true);
	if (!bounds.isValid())
		bounds = source->getWorldBoundBox();

	if (!bounds.isValid())
		return Unigine::Math::vec3_one;

	const Unigine::Math::dvec3 world_size = bounds.getSize();
	const Unigine::Math::vec3 world_scale = clampSize(source->getWorldScale());

	return clampSize(Unigine::Math::vec3(
		static_cast<float>(world_size.x) / world_scale.x,
		static_cast<float>(world_size.y) / world_scale.y,
		static_cast<float>(world_size.z) / world_scale.z));
}

void NodeGenerationService::configureGeneratedNode(const Unigine::NodePtr& node,
                                                   const Unigine::NodePtr& source,
                                                   const char* name) const
{
	if (!node || !source)
		return;

	node->setName(name);
	node->setWorldTransform(source->getWorldTransform());
	node->setSaveToWorldEnabled(true);
	node->setShowInEditorEnabled(true);
	node->setEnabled(true);
}

} // namespace LevelUtility
