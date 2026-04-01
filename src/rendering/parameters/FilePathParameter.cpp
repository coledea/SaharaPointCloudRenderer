#include "FilePathParameter.h"

namespace sahara::rendering
{

FilePathParameter::FilePathParameter(const QString& name, const QString& default_path, const QString& filetypes)
	: Parameter<QString>(name, default_path)
	, m_filetypes(filetypes)
{
}

const QString& FilePathParameter::filetypes() const noexcept
{
	return m_filetypes;
}

}