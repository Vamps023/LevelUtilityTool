#pragma once

#include <QWidget>
#include <memory>

class QLabel;
class QPushButton;

namespace LevelUtility
{
class ExtrusionQtEngine;
}

class ExtrusionToolWidget final : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(ExtrusionToolWidget)

public:
	explicit ExtrusionToolWidget(QWidget* parent = nullptr);
	~ExtrusionToolWidget() override;

private slots:
	void onSelectNodeFile();
	void onClearSelection();
	void onCreateExtrudeNode();

private:
	void setStatusMessage(const QString& message, bool isError = false);

	std::unique_ptr<LevelUtility::ExtrusionQtEngine> engine_;

	QLabel* file_label_ = nullptr;
	QLabel* status_label_ = nullptr;
	QPushButton* create_button_ = nullptr;
	QPushButton* select_button_ = nullptr;
	QPushButton* clear_button_ = nullptr;

	QString selected_file_path_;
};
