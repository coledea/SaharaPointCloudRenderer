#pragma once

#include "AbstractParameter.h"

#include <QString>

namespace sahara::rendering
{

// a parameter that can be used to trigger events (e.g., a button that triggers an action after whose completion the parameter resets itself)
class TriggerParameter : public AbstractParameter
{
public:
	TriggerParameter(const QString& name);
	virtual ~TriggerParameter() = default;

	void activate() noexcept;
	void deactivate() noexcept;
	void toggle() noexcept;
	bool isActive() const noexcept;

	void reset() override;

protected:
	bool m_active;
};

}