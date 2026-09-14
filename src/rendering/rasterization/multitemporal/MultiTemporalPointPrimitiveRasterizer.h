#pragma once

#include "ChangeHighlightingRasterizerPass.h"
#include "SideBySideRasterizerPass.h"
#include "SplitViewRasterizerPass.h"
#include "rendering/Framebuffer.h"
#include "rendering/InputEventCache.h"
#include "rendering/ShaderProgramFactory.h"
#include "rendering/parameters/EnumParameter.h"
#include "rendering/parameters/RangeParameter.h"
#include "rendering/rasterization/AbstractRasterizer.h"
#include "utils/FullscreenGeometry.h"

#include <QOpenGLVertexArrayObject>

namespace sahara::rendering
{

class MultiTemporalPointPrimitiveRasterizer : public AbstractRasterizer
{
	Q_OBJECT

public:
	MultiTemporalPointPrimitiveRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept;
	virtual ~MultiTemporalPointPrimitiveRasterizer();

	virtual void recompileShaders(const std::set<geometry::AttributeSpecification>& required_outputs) override;
	virtual void run(Framebuffer* framebuffer, int read_fbo_index) override;

	void setFramebuffer(Framebuffer* framebuffer);

	virtual RasterizerType type() const noexcept override;
	std::set<geometry::AttributeSpecification> requiredAttributes(const std::set<geometry::AttributeSpecification>& required_outputs) const override;

protected slots:
	void onPointSizeChanged();
	void onComparisonModeChanged();
	void onAddedColorChanged();
	void onRemovedColorChanged();
	void onEpsilonChanged();
	void onLensSizeChanged();

protected:
	std::unique_ptr<RangeParameter<int>> m_point_size_parameter;
	std::unique_ptr<EnumParameter> m_comparison_mode_parameter;
	std::unique_ptr<Parameter<QColor>> m_added_color_parameter;
	std::unique_ptr<Parameter<QColor>> m_removed_color_parameter;
	std::unique_ptr<RangeParameter<float>> m_epsilon_parameter;
	std::unique_ptr<RangeParameter<float>> m_lens_size_parameter;

	QOpenGLVertexArrayObject m_vao; // Vertex Array Object (VAO).

	std::unique_ptr<QOpenGLShaderProgram> m_shader_program;

	SideBySideRasterizerPass m_side_by_side_pass;
	SplitViewRasterizerPass m_split_view_pass;
	ChangeHighlightingRasterizerPass m_change_highlighting_pass;
	ChangeHighlightingRasterizerPass m_precomputed_change_highlighting_pass;

	void drawStandardView();
	ShaderPaths getShaderPaths() const noexcept;
	void initializeVAO();
	void setVertexAttribute(const geometry::AttributeSpecification& attribute); // requires current context and shader program being bound
};

}