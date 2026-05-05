#include "ExtrusionToolWidget.h"
#include "ExtrusionQtEngine.h"

#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

#include <UnigineLog.h>

ExtrusionToolWidget::ExtrusionToolWidget(QWidget* parent)
	: QWidget(parent)
	, engine_(std::make_unique<LevelUtility::ExtrusionQtEngine>())
{
	setObjectName("ExtrusionToolWidget");
	setWindowTitle("Extrusion Tool");

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(14, 14, 14, 14);
	mainLayout->setSpacing(12);

	QLabel* titleLabel = new QLabel("Extrusion Tool", this);
	titleLabel->setStyleSheet("font-size: 16px; font-weight: 700;");
	mainLayout->addWidget(titleLabel);

	file_label_ = new QLabel("File: none selected", this);
	file_label_->setWordWrap(true);
	mainLayout->addWidget(file_label_);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->setSpacing(8);

	create_button_ = new QPushButton("Create New", this);
	create_button_->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
	create_button_->setEnabled(false);
	buttonsLayout->addWidget(create_button_);

	select_button_ = new QPushButton("Select Node...", this);
	select_button_->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
	buttonsLayout->addWidget(select_button_);

	clear_button_ = new QPushButton("Clear Selection", this);
	clear_button_->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogDiscardButton));
	buttonsLayout->addWidget(clear_button_);

	mainLayout->addLayout(buttonsLayout);

	status_label_ = new QLabel(this);
	status_label_->setWordWrap(true);
	mainLayout->addWidget(status_label_);
	mainLayout->addStretch(1);

	connect(select_button_, &QPushButton::clicked, this, &ExtrusionToolWidget::onSelectNodeFile);
	connect(clear_button_, &QPushButton::clicked, this, &ExtrusionToolWidget::onClearSelection);
	connect(create_button_, &QPushButton::clicked, this, &ExtrusionToolWidget::onCreateExtrudeNode);

	setStatusMessage("Select a .node template file to begin.");
}

ExtrusionToolWidget::~ExtrusionToolWidget() = default;

void ExtrusionToolWidget::onSelectNodeFile()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Select Node Template", QString(), "Node Files (*.node)");
	if (filePath.isEmpty())
		return;

	selected_file_path_ = filePath;
	file_label_->setText("File: " + selected_file_path_);
	create_button_->setEnabled(true);
	setStatusMessage("Template selected. Click 'Create New' to spawn extrude node.");
}

void ExtrusionToolWidget::onClearSelection()
{
	selected_file_path_.clear();
	file_label_->setText("File: none selected");
	create_button_->setEnabled(false);
	setStatusMessage("Select a .node template file to begin.");
}

void ExtrusionToolWidget::onCreateExtrudeNode()
{
	if (selected_file_path_.isEmpty())
		return;

	if (!engine_)
		return;

	bool ok = engine_->createExtrudeNode(selected_file_path_);
	if (ok)
		setStatusMessage(QString("Created extrude node from %1").arg(selected_file_path_));
	else
		setStatusMessage("Failed to create extrude node. Check console for details.", true);
}

void ExtrusionToolWidget::setStatusMessage(const QString& message, bool isError)
{
	status_label_->setText(message);
	status_label_->setStyleSheet(isError ? "color: #c62828;" : "color: #4a4a4a;");
}
