#pragma once

#include "../processing/NodeGenerationService.h"

#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;
class QTimer;

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

private:
	void startGeneration(LevelUtility::GenerationType type);
	void setBusy(bool busy);
	void setStatusMessage(const QString& message, bool isError = false);

	LevelUtility::NodeGenerationService* service_ = nullptr;

	QLabel* selectionLabel_ = nullptr;
	QLabel* statusLabel_ = nullptr;
	QCheckBox* groupCheckBox_ = nullptr;
	QPushButton* refreshButton_ = nullptr;
	QPushButton* obstacleButton_ = nullptr;
	QPushButton* occluderButton_ = nullptr;
	QTimer* batchTimer_ = nullptr;
};
