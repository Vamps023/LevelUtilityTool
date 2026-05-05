#pragma once

#include "../processing/NodeGenerationService.h"

#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;
class QTimer;
class QTabWidget;
class ExtrusionToolWidget;

class ExtrusionPlugin;

class LevelUtilityToolPanel final : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(LevelUtilityToolPanel)

public:
	explicit LevelUtilityToolPanel(LevelUtility::NodeGenerationService* service,
	                               QWidget* parent = nullptr);
	~LevelUtilityToolPanel() override = default;

	void setExtrusionPlugin(ExtrusionPlugin* plugin);

private slots:
	void refreshSelection();
	void createObstacleBoxes();
	void createWorldOccluders();
	void onBatchProgress(int created, int requested);
	void onGenerationFinished(const LevelUtility::GenerationResult& result);
	void onExtrusionUpdate();
	void onSelectionChanged();

private:
	void startGeneration(LevelUtility::GenerationType type);
	void setBusy(bool busy);
	void setStatusMessage(const QString& message, bool isError = false);

	LevelUtility::NodeGenerationService* service_ = nullptr;

	// Obstacle/Occluder tab widgets
	QLabel* selectionLabel_ = nullptr;
	QLabel* statusLabel_ = nullptr;
	QCheckBox* groupCheckBox_ = nullptr;
	QPushButton* refreshButton_ = nullptr;
	QPushButton* obstacleButton_ = nullptr;
	QPushButton* occluderButton_ = nullptr;
	QTimer* batchTimer_ = nullptr;

	// Tab container
	QTabWidget* tabWidget_ = nullptr;
	ExtrusionToolWidget* extrusionWidget_ = nullptr;

	// Extrusion runtime
	ExtrusionPlugin* extrusion_plugin_ = nullptr;
};
