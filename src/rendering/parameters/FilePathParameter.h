#pragma once

#include "Parameter.h"

namespace sahara::rendering
{

class FilePathParameter : public Parameter<QString>
{
public:
	FilePathParameter(const QString& name, const QString& default_path, const QString& filetypes = "");

	const QString& filetypes() const noexcept;

private:
	QString m_filetypes;
};
}
