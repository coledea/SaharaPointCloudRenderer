#pragma once

#include <QLineEdit>
#include <QToolButton>

namespace sahara::ui
{

class FileSelector : public QWidget
{
	Q_OBJECT

public:
	FileSelector(const QString& initial_path, const QString& filetypes, QWidget* parent = nullptr);

	QString path() const;
	void setPath(const QString& new_path);

signals:
	void pathChanged(const QString& new_path);

private:
	QToolButton* m_file_dialog_button;
	QLineEdit* m_line_edit;
	QString m_filetypes;

	void onFileDialogButtonClicked();
};

}