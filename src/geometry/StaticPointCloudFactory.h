#pragma once

#include "StaticPointCloud.h"

#include <filesystem>

namespace sahara::geometry::StaticPointCloudFactory
{

std::unique_ptr<StaticPointCloud> loadFromPointCloudFile(const std::filesystem::path& filepath);
std::unique_ptr<StaticPointCloud> loadFromMeshFile(const std::filesystem::path& filepath);
std::unique_ptr<StaticPointCloud> createSyntheticPointCloud(uint num_points);

}