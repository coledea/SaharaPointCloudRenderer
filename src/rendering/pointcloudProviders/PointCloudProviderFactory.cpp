#include "PointCloudProviderFactory.h"

#include "MultiTemporalPointCloudProvider.h"
#include "OOCMultiTemporalPointCloudProvider.h"
#include "OOCSinglePointCloudProvider.h"
#include "StaticPointCloudProvider.h"
#include "io/CSVLoadingDialog.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace sahara::rendering
{

std::unique_ptr<AbstractPointCloudProvider> PointCloudProviderFactory::createPointCloudProvider(const std::filesystem::path& filepath, OpenGLContext* opengl_context, navigation::Camera* camera)
{
	const auto file_extension = filepath.extension();

	std::unique_ptr<AbstractPointCloudProvider> provider;
	if (file_extension == ".csv" || file_extension == ".txt" || file_extension == ".xyz" || file_extension == ".ascii")
	{
		auto filepath_string = QString::fromStdString(filepath.generic_string());
		auto loading_dialog = sahara::io::CSVLoadingDialog(filepath_string);
		if (loading_dialog.exec() == QDialog::Rejected)
		{
			return nullptr;
		}

		auto& column_settings = loading_dialog.columnSettings();
		provider = std::make_unique<StaticPointCloudProvider>(filepath, column_settings, opengl_context);
	}
	else if (file_extension == ".ply")
	{
		provider = std::make_unique<StaticPointCloudProvider>(filepath, sahara::io::CSVColumnSettings::empty, opengl_context);
	}
	else if (file_extension == ".mtpc")
	{
		provider = std::make_unique<MultiTemporalPointCloudProvider>(filepath, opengl_context);
	}
	else if (file_extension == ".json")
	{
		QFile file(filepath);
		if (!file.open(QIODevice::ReadOnly))
		{
			qWarning() << "Could not open file " << filepath.c_str();
			return nullptr;
		}
		const auto metadata_json = QJsonDocument::fromJson(file.readAll());
		file.close();

		auto type = metadata_json.object().value("type").toString();

		if (type == "multi-temporal")
		{
			provider = std::make_unique<OOCMultiTemporalPointCloudProvider>(filepath, opengl_context, camera);
		}
		else
		{
			provider = std::make_unique<OOCSinglePointCloudProvider>(filepath, opengl_context, camera);
		}
	}
	else
	{
		assert(false && "Unsupported point cloud type");
		return nullptr;
	}

	if (provider->isValid())
	{
		return provider;
	}
	else
	{
		return nullptr;
	}
}
}