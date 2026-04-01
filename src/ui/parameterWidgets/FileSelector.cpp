#include "FileSelector.h"

#include <QFileDialog>
#include <QHBoxLayout>

namespace sahara::ui
{

FileSelector::FileSelector(const QString& initial_path, const QString& filetypes, QWidget* parent)
	: QWidget(parent)
	, m_file_dialog_button{ new QToolButton(this) }
	, m_line_edit{ new QLineEdit(initial_path, this) }
	, m_filetypes(filetypes)
{
	m_file_dialog_button->setIcon(QIcon(":/icons/file-open.png"));
	m_file_dialog_button->setAutoRaise(true);
	m_file_dialog_button->setAutoFillBackground(false);

	m_line_edit->setReadOnly(true);

	auto layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(5);

	layout->addWidget(m_line_edit);
	layout->addWidget(m_file_dialog_button);

	setLayout(layout);

	connect(m_file_dialog_button, &QToolButton::clicked, this, &FileSelector::onFileDialogButtonClicked);
}

QString FileSelector::path() const
{
	return m_line_edit->text();
}

void FileSelector::setPath(const QString& new_path)
{
	m_line_edit->setText(new_path);
}

void FileSelector::onFileDialogButtonClicked()
{
	m_line_edit->setText(QFileDialog::getOpenFileName(this, "Select Camera Path", "./", m_filetypes));
	emit pathChanged(m_line_edit->text());
}

}
