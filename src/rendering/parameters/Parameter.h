#pragma once

#include "AbstractParameter.h"

#include <QString>

namespace sahara::rendering
{

template <class T>
class Parameter : public AbstractParameter
{
public:
	Parameter(const QString& name, const T& value);
	virtual ~Parameter() = default;

	const T& value() const;

	virtual void setValue(const T& value);
	void reset() override;

protected:
	T m_initial_value;
	T m_value;
};

}

#include "Parameter.inl"