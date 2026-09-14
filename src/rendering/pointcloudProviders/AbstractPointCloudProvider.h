#pragma once

#include "DrawCommandBuffer.h"
#include "geometry/AttributeMetadata.h"
#include "navigation/Camera.h"
#include "rendering/RendererModuleTypes.h"
#include "rendering/parameters/AbstractParameter.h"

#include <QOpenGLBuffer>

namespace sahara::rendering
{

class AbstractPointCloudProvider : public QObject
{
	Q_OBJECT

public:
	AbstractPointCloudProvider(const QString& name, rendering::OpenGLContext* opengl_context, size_t number_of_draw_commands) noexcept;

	virtual ~AbstractPointCloudProvider() = default;

	virtual size_t numberOfPoints() const = 0;
	virtual int pointBudget() const = 0;
	virtual const geometry::BoundingBox& boundingBox() const = 0;
	virtual PointCloudProviderType type() const noexcept = 0;
	virtual void update() = 0;

	uint id() const noexcept;
	const QString& name() const noexcept;
	bool isValid() const noexcept;
	std::vector<AbstractParameter*>& parameters() noexcept;
	DrawCommandBuffer& drawCommandBuffer() noexcept;
	virtual std::vector<RasterizerType> supportedRasterizers() const noexcept = 0;

	bool hasAttribute(geometry::AttributeSemantic semantic) const;
	bool hasAttribute(const QString& attribute_name) const;
	const std::unordered_map<geometry::AttributeSemantic, geometry::AttributeMetadata*>& attributesMetadata() const;
	const geometry::AttributeMetadata* attributeMetadata(const QString& attribute_name) const;
	const geometry::AttributeMetadata* attributeMetadata(geometry::AttributeSemantic semantic) const;

	virtual void setRequiredAttributes(const std::set<geometry::AttributeSpecification>& attributes);

	template <typename T>
	const geometry::TypedAttributeMetadata<T>* typedAttributeMetadata(geometry::AttributeSemantic semantic);

	virtual void bindGPUBuffer(geometry::AttributeSemantic semantic) = 0;
	virtual GLuint getGPUBuffer(geometry::AttributeSemantic semantic) = 0;
	virtual void releaseGPUBuffer(geometry::AttributeSemantic semantic) = 0;

signals:
	void pointBudgetChanged(size_t new_budget);

protected:
	static uint m_next_id; // for keeping track of assigned IDs. We never release them, but we should never need more than uint can hold.

	uint m_id;		 // unique identifier
	QString m_name;	 // possibly non-unique name
	bool m_is_valid; // false if initialization failed (e.g., due to not being able to open/parse given files).

	OpenGLContext* m_opengl_context;
	std::unordered_map<geometry::AttributeSemantic, geometry::AttributeMetadata*> m_available_attributes;
	std::vector<const geometry::AttributeMetadata*> m_required_attributes;
	DrawCommandBuffer m_draw_command_buffer;
	std::vector<AbstractParameter*> m_parameters;
};

}

#include "AbstractPointCloudProvider.inl"