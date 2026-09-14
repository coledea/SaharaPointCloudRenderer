#pragma once

#include <QString>
#include <unordered_map>
#include <vector>

namespace sahara::rendering
{

enum class RendererModule
{
	PointCloudProvider,
	Rasterizer,
	Colorizer,
	Postprocessor
};

enum class PointCloudProviderType
{
	StaticPointCloudProvider,
	MultiTemporalPointCloudProvider,
	OOCSinglePointCloudProvider,
	OOCMultiTemporalPointCloudProvider
};

enum class RasterizerType
{
	PointPrimitiveRasterizer,
	PointComputeRasterizer,
	HQComputeRasterizer,
	MultiTemporalPointPrimitiveRasterizer,
	OOCPointPrimitiveRasterizer
};

enum class ColorizerType
{
	SingleColor,
	AttributeBased
};

enum class PostprocessorType
{
	EyeDomeLighting,
	HoleFilling
};

const std::unordered_map<RendererModule, std::vector<QString>> RendererModuleNames = {
	{ RendererModule::PointCloudProvider, { "Static Point Cloud", "Multi-temporal Point Cloud", "Out-of-core Point Cloud", "Multi-temporal Out-of-Core Point Cloud" } },
	{ RendererModule::Rasterizer, { "Point Primitive Rasterizer", "Point Compute", "HQ Point Compute", "Multi-temporal Point Primitive Rasterizer", "OOC Point Primitive Rasterizer" } },
	{ RendererModule::Colorizer, { "Single Color", "Attribute-based" } },
	{ RendererModule::Postprocessor, { "EDL", "Hole-filling" } }
};

}