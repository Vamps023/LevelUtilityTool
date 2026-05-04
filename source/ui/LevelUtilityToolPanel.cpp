#include "LevelUtilityToolPanel.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <editor/UnigineSelection.h>
#include <editor/UnigineSelector.h>
#include <UnigineEditor.h>
#include <UnigineNodes.h>
#include <UnigineWorld.h>

LevelUtilityToolPanel::LevelUtilityToolPanel(LevelUtility::NodeGenerationService* service,
                                             QWidget* parent)
	: QWidget(parent)
	, service_(service)
	, networkImportService_(std::make_unique<LevelUtility::NetworkImportService>())
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

	QFrame* selectionFrame = new QFrame(this);
	QHBoxLayout* selectionLayout = new QHBoxLayout(selectionFrame);
	selectionLayout->setContentsMargins(10, 10, 10, 10);
	selectionLayout->setSpacing(8);

	selectionLabel_ = new QLabel("Selected Node References: 0", selectionFrame);
	refreshButton_ = new QPushButton("Refresh", selectionFrame);
	refreshButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
	selectionLayout->addWidget(selectionLabel_, 1);
	selectionLayout->addWidget(refreshButton_);
	mainLayout->addWidget(selectionFrame);

	groupCheckBox_ = new QCheckBox("Group under DummyNode", this);
	groupCheckBox_->setChecked(true);
	mainLayout->addWidget(groupCheckBox_);

	obstacleButton_ = new QPushButton("Create ObstacleBox", this);
	obstacleButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
	mainLayout->addWidget(obstacleButton_);

	occluderButton_ = new QPushButton("Create WorldOccluder", this);
	occluderButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));
	mainLayout->addWidget(occluderButton_);

	statusLabel_ = new QLabel(this);
	statusLabel_->setWordWrap(true);
	mainLayout->addWidget(statusLabel_);
	mainLayout->addStretch(1);

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
	        this, &LevelUtilityToolPanel::refreshSelection);

	// Network Import connections
	connect(networkImportService_.get(), &LevelUtility::NetworkImportService::importFinished,
	        this, &LevelUtilityToolPanel::onNetworkImportFinished);
	connect(networkImportService_.get(), &LevelUtility::NetworkImportService::dataCleared,
	        this, &LevelUtilityToolPanel::onNetworkDataCleared);
	
	setupNetworkImportUI();
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

void LevelUtilityToolPanel::setupNetworkImportUI()
{
	QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
	if (!mainLayout)
		return;

	// Network Import section title
	QLabel* networkTitle = new QLabel("Network Import", this);
	networkTitle->setStyleSheet("font-size: 14px; font-weight: 700; margin-top: 10px;");
	mainLayout->addWidget(networkTitle);

	// File selection row
	QHBoxLayout* fileLayout = new QHBoxLayout();
	networkFileLabel_ = new QLabel("File: none selected", this);
	networkFileLabel_->setWordWrap(true);
	importXmlButton_ = new QPushButton("Import XML", this);
	clearNetworkButton_ = new QPushButton("Clear", this);
	clearNetworkButton_->setEnabled(false);
	
	fileLayout->addWidget(networkFileLabel_, 1);
	fileLayout->addWidget(importXmlButton_);
	fileLayout->addWidget(clearNetworkButton_);
	mainLayout->addLayout(fileLayout);

	// Status label
	networkStatusLabel_ = new QLabel("Status: Ready", this);
	networkStatusLabel_->setWordWrap(true);
	mainLayout->addWidget(networkStatusLabel_);

	// Visualization mode dropdown
	QHBoxLayout* modeLayout = new QHBoxLayout();
	QLabel* modeLabel = new QLabel("Mode:", this);
	visualizationModeCombo_ = new QComboBox(this);
	visualizationModeCombo_->addItem("Sequential Spline", static_cast<int>(LevelUtility::NetworkImportService::VisualizationMode::SequentialSpline));
	visualizationModeCombo_->addItem("Segment Lines", static_cast<int>(LevelUtility::NetworkImportService::VisualizationMode::SegmentLines));
	visualizationModeCombo_->setEnabled(false);
	
	modeLayout->addWidget(modeLabel);
	modeLayout->addWidget(visualizationModeCombo_, 1);
	mainLayout->addLayout(modeLayout);

	// Toggle visuals button
	toggleVisualsButton_ = new QPushButton("Hide Visuals", this);
	toggleVisualsButton_->setEnabled(false);
	mainLayout->addWidget(toggleVisualsButton_);

	// Segments tree view
	segmentsTree_ = new QTreeWidget(this);
	segmentsTree_->setHeaderLabel("Track Segments");
	segmentsTree_->setMaximumHeight(150);
	segmentsTree_->setEnabled(false);
	mainLayout->addWidget(segmentsTree_);

	// Connect signals
	connect(importXmlButton_, &QPushButton::clicked, this, &LevelUtilityToolPanel::importNetworkXml);
	connect(clearNetworkButton_, &QPushButton::clicked, this, &LevelUtilityToolPanel::clearNetwork);
	connect(visualizationModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
	        this, &LevelUtilityToolPanel::onVisualizationModeChanged);
	connect(toggleVisualsButton_, &QPushButton::clicked, this, &LevelUtilityToolPanel::toggleVisualsVisible);
}

void LevelUtilityToolPanel::importNetworkXml()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Import Network XML", QString(), "XML Files (*.xml)");
	if (filePath.isEmpty())
		return;
	
	networkImportService_->importXml(filePath);
}

void LevelUtilityToolPanel::clearNetwork()
{
	networkImportService_->clear();
}

void LevelUtilityToolPanel::onNetworkImportFinished(bool success, const QString& message)
{
	if (success)
	{
		networkFileLabel_->setText("File: " + networkImportService_->filePath());
		networkStatusLabel_->setText("Status: " + message);
		
		// Enable controls
		clearNetworkButton_->setEnabled(true);
		visualizationModeCombo_->setEnabled(true);
		toggleVisualsButton_->setEnabled(true);
		segmentsTree_->setEnabled(true);
		
		// Populate tree
		segmentsTree_->clear();
		const auto& segments = networkImportService_->trackSegments();
		for (const auto& segment : segments)
		{
			QString itemText = QString("%1: %2").arg(segment.segmentId).arg(QString::fromStdString(segment.name));
			QTreeWidgetItem* item = new QTreeWidgetItem(segmentsTree_);
			item->setText(0, itemText);
		}
		
		// Generate visuals
		networkImportService_->generateVisuals();
	}
	else
	{
		networkStatusLabel_->setText("Status: " + message);
		networkStatusLabel_->setStyleSheet("color: #c62828;");
	}
}

void LevelUtilityToolPanel::onNetworkDataCleared()
{
	networkFileLabel_->setText("File: none selected");
	networkStatusLabel_->setText("Status: Ready");
	networkStatusLabel_->setStyleSheet("");
	
	// Disable controls
	clearNetworkButton_->setEnabled(false);
	visualizationModeCombo_->setEnabled(false);
	toggleVisualsButton_->setEnabled(false);
	toggleVisualsButton_->setText("Hide Visuals");
	segmentsTree_->setEnabled(false);
	segmentsTree_->clear();
}

void LevelUtilityToolPanel::onVisualizationModeChanged(int index)
{
	if (index < 0)
		return;
	
	auto mode = static_cast<LevelUtility::NetworkImportService::VisualizationMode>(
		visualizationModeCombo_->itemData(index).toInt());
	
	networkImportService_->setVisualizationMode(mode);
}

void LevelUtilityToolPanel::toggleVisualsVisible()
{
	bool currentlyVisible = networkImportService_->areVisualsVisible();
	networkImportService_->setVisualsVisible(!currentlyVisible);
	
	toggleVisualsButton_->setText(currentlyVisible ? "Show Visuals" : "Hide Visuals");
}
