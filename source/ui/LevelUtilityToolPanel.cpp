#include "LevelUtilityToolPanel.h"
#include "../extrusion_editor/Private/ExtrusionToolWidget.h"
#include "../extrusion/Private/ExtrusionPlugin.h"

#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QMetaObject>

#include <editor/UnigineSelection.h>
#include <editor/UnigineSelector.h>
#include <UnigineEditor.h>
#include <UnigineNodes.h>
#include <UnigineWorld.h>

LevelUtilityToolPanel::LevelUtilityToolPanel(LevelUtility::NodeGenerationService* service,
                                             QWidget* parent)
	: QWidget(parent)
	, service_(service)
{
	setObjectName("LevelUtilityToolPanel");
	setWindowTitle("Level Utility Tool");
	setMinimumWidth(340);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(14, 14, 14, 14);
	mainLayout->setSpacing(12);

	QLabel* titleLabel = new QLabel("Level Utility Tool", this);
	titleLabel->setStyleSheet("font-size: 16px; font-weight: 700;");
	mainLayout->addWidget(titleLabel);

	// Tab widget
	tabWidget_ = new QTabWidget(this);
	mainLayout->addWidget(tabWidget_);

	// --- Tab 1: Obstacle / Occluder ---
	QWidget* obstacleTab = new QWidget(this);
	QVBoxLayout* obstacleLayout = new QVBoxLayout(obstacleTab);
	obstacleLayout->setContentsMargins(10, 10, 10, 10);
	obstacleLayout->setSpacing(12);

	QFrame* selectionFrame = new QFrame(obstacleTab);
	QHBoxLayout* selectionLayout = new QHBoxLayout(selectionFrame);
	selectionLayout->setContentsMargins(10, 10, 10, 10);
	selectionLayout->setSpacing(8);

	selectionLabel_ = new QLabel("Selected Node References: 0", selectionFrame);
	refreshButton_ = new QPushButton("Refresh", selectionFrame);
	refreshButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
	selectionLayout->addWidget(selectionLabel_, 1);
	selectionLayout->addWidget(refreshButton_);
	obstacleLayout->addWidget(selectionFrame);

	groupCheckBox_ = new QCheckBox("Group under DummyNode", obstacleTab);
	groupCheckBox_->setChecked(true);
	obstacleLayout->addWidget(groupCheckBox_);

	obstacleButton_ = new QPushButton("Create ObstacleBox", obstacleTab);
	obstacleButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
	obstacleLayout->addWidget(obstacleButton_);

	occluderButton_ = new QPushButton("Create WorldOccluder", obstacleTab);
	occluderButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));
	obstacleLayout->addWidget(occluderButton_);

	statusLabel_ = new QLabel(obstacleTab);
	statusLabel_->setWordWrap(true);
	obstacleLayout->addWidget(statusLabel_);
	obstacleLayout->addStretch(1);

	tabWidget_->addTab(obstacleTab, "Obstacle / Occluder");

	// --- Tab 2: Extrusion Tool ---
	extrusionWidget_ = new ExtrusionToolWidget(this);
	tabWidget_->addTab(extrusionWidget_, "Extrusion Tool");

	batchTimer_ = new QTimer(this);
	batchTimer_->setInterval(0);

	connect(refreshButton_, &QPushButton::clicked, this, &LevelUtilityToolPanel::refreshSelection);
	connect(obstacleButton_, &QPushButton::clicked, this, &LevelUtilityToolPanel::createObstacleBoxes);
	connect(occluderButton_, &QPushButton::clicked, this, &LevelUtilityToolPanel::createWorldOccluders);
	connect(batchTimer_, &QTimer::timeout, service_, &LevelUtility::NodeGenerationService::processNextBatch);
	connect(service_, &LevelUtility::NodeGenerationService::batchProgress,
	        this, &LevelUtilityToolPanel::onBatchProgress);
	connect(service_, &LevelUtility::NodeGenerationService::generationFinished,
	        this, &LevelUtilityToolPanel::onGenerationFinished);
	connect(UnigineEditor::Selection::instance(), &UnigineEditor::Selection::changed,
	        this, &LevelUtilityToolPanel::onSelectionChanged);

	refreshSelection();
}

void LevelUtilityToolPanel::refreshSelection()
{
	if (!service_)
		return;

	const int count = service_->countSelectedNodeReferences();
	selectionLabel_->setText(QString("Selected Node References: %1").arg(count));
	obstacleButton_->setEnabled(count > 0 && !service_->isProcessing());
	occluderButton_->setEnabled(count > 0 && !service_->isProcessing());

	if (count == 0)
		setStatusMessage("Select one or more Node References.");
	else
		setStatusMessage(QString("Ready to process %1 Node Reference%2.")
			.arg(count)
			.arg(count == 1 ? "" : "s"));
}

void LevelUtilityToolPanel::createObstacleBoxes()
{
	startGeneration(LevelUtility::GenerationType::ObstacleBox);
}

void LevelUtilityToolPanel::createWorldOccluders()
{
	startGeneration(LevelUtility::GenerationType::WorldOccluder);
}

void LevelUtilityToolPanel::startGeneration(LevelUtility::GenerationType type)
{
	if (!service_ || service_->isProcessing())
		return;

	if (!service_->beginGeneration(type, groupCheckBox_->isChecked()))
	{
		QMessageBox::warning(this, "Level Utility Tool",
			"Select at least one Node Reference before generating nodes.");
		refreshSelection();
		return;
	}

	setBusy(true);
	setStatusMessage("Generating nodes...");
	batchTimer_->start();
}

void LevelUtilityToolPanel::onBatchProgress(int created, int requested)
{
	setStatusMessage(QString("Generated %1 of %2 nodes...").arg(created).arg(requested));
}

void LevelUtilityToolPanel::onGenerationFinished(const LevelUtility::GenerationResult& result)
{
	batchTimer_->stop();
	setBusy(false);

	if (result.created == 0)
	{
		refreshSelection();
		setStatusMessage("No nodes were generated. Check the selection and source bounds.", true);
		return;
	}

	refreshSelection();
	setStatusMessage(QString("Created %1 node%2. Skipped %3.")
		.arg(result.created)
		.arg(result.created == 1 ? "" : "s")
		.arg(result.skipped));
}

void LevelUtilityToolPanel::setBusy(bool busy)
{
	refreshButton_->setEnabled(!busy);
	groupCheckBox_->setEnabled(!busy);
	obstacleButton_->setEnabled(!busy);
	occluderButton_->setEnabled(!busy);
}

void LevelUtilityToolPanel::setStatusMessage(const QString& message, bool isError)
{
	statusLabel_->setText(message);
	statusLabel_->setStyleSheet(isError ? "color: #c62828;" : "color: #4a4a4a;");
}

void LevelUtilityToolPanel::setExtrusionPlugin(ExtrusionPlugin* plugin)
{
	extrusion_plugin_ = plugin;
}

void LevelUtilityToolPanel::onExtrusionUpdate()
{
	if (extrusion_plugin_)
		extrusion_plugin_->update();
}

void LevelUtilityToolPanel::onSelectionChanged()
{
	// First do the normal refresh
	refreshSelection();

	// Then notify extrusion plugin of selection change
	if (extrusion_plugin_)
	{
		extrusion_plugin_->selection_changed();
		// Defer mesh update to avoid modifying nodes during editor's selection callback
		QTimer::singleShot(0, this, &LevelUtilityToolPanel::onExtrusionUpdate);
	}
}
