#pragma once

#include <QObject>
#include <UnigineNode.h>
#include <UnigineVector.h>

#include <deque>
#include <vector>

namespace LevelUtility
{

enum class GenerationType
{
	ObstacleBox,
	WorldOccluder
};

struct GenerationResult
{
	int requested = 0;
	int created = 0;
	int skipped = 0;
};

class NodeGenerationService final : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(NodeGenerationService)

public:
	explicit NodeGenerationService(QObject* parent = nullptr);
	~NodeGenerationService() override = default;

	std::vector<Unigine::NodePtr> collectSelectedNodeReferences() const;
	int countSelectedNodeReferences() const;

	bool beginGeneration(GenerationType type, bool groupUnderDummy);
	bool isProcessing() const { return processing_; }
	GenerationResult result() const { return result_; }

public slots:
	void processNextBatch();

signals:
	void batchProgress(int created, int requested);
	void generationFinished(const LevelUtility::GenerationResult& result);

private:
	Unigine::NodePtr ensureRoot();
	Unigine::NodePtr createGeneratedNode(const Unigine::NodePtr& source, GenerationType type);
	Unigine::Math::vec3 getSafeWorldBoundsSize(const Unigine::NodePtr& source) const;
	void configureGeneratedNode(const Unigine::NodePtr& node, const Unigine::NodePtr& source, const char* name) const;

	std::deque<Unigine::NodePtr> pending_;
	Unigine::NodePtr root_;
	GenerationResult result_;
	GenerationType activeType_ = GenerationType::ObstacleBox;
	bool groupUnderDummy_ = true;
	bool processing_ = false;
};

} // namespace LevelUtility
