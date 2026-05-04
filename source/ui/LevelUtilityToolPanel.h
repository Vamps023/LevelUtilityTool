#pragma once

#include "../processing/NodeGenerationService.h"
#include "../extrusion/NetworkImportService.h"

#include <QWidget>
#include <memory>

class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

class LevelUtilityToolPanel final : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(LevelUtilityToolPanel)

public:
	explicit LevelUtilityToolPanel(LevelUtility::NodeGenerationService* service,
	                               QWidget* parent = nullptr);
	~LevelUtilityToolPanel() override = default;

private slots:
	void refreshSelection();
	void createObstacleBoxes();
	void createWorldOccluders();
	void onBatchProgress(int created, int requested);
	void onGenerationFinished(const LevelUtility::GenerationResult& result);
	
	// Network Import slots
	void importNetworkXml();
	void clearNetwork();
	void onNetworkImportFinished(bool success, const QString& message);
	void onNetworkDataCleared();
	void onVisualizationModeChanged(int index);
	void toggleVisualsVisible();

private:
	void startGeneration(LevelUtility::GenerationType type);
	void setBusy(bool busy);
	void setStatusMessage(const QString& message, bool isError = false);
	void setupNetworkImportUI();

	LevelUtility::NodeGenerationService* service_ = nullptr;
	std::unique_ptr<LevelUtility::NetworkImportService> networkImportService_;

	QLabel* selectionLabel_ = nullptr;
	QLabel* statusLabel_ = nullptr;
	QCheckBox* groupCheckBox_ = nullptr;
	QPushButton* refreshButton_ = nullptr;
	QPushButton* obstacleButton_ = nullptr;
	QPushButton* occluderButton_ = nullptr;
	QTimer* batchTimer_ = nullptr;
	
	// Network Import UI elements
	QLabel* networkFileLabel_ = nullptr;
	QPushButton* importXmlButton_ = nullptr;
	QPushButton* clearNetworkButton_ = nullptr;
	QComboBox* visualizationModeCombo_ = nullptr;
	QTreeWidget* segmentsTree_ = nullptr;
	QPushButton* toggleVisualsButton_ = nullptr;
	QLabel* networkStatusLabel_ = nullptr;
};
