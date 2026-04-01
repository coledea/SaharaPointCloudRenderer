#pragma once

#include "Parameter.h"

namespace sahara::rendering
{

/*
 * Example for creating a range parameter:
 *    RangeParameter<QVector3D> param("Parameter Name", QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 0.0, 0.0), QVector3D(1.0, 1.0, 1.0), QVector3D(0.01, 0.01, 0.01));
 */

template <class T>
class RangeParameter : public Parameter<T>
{
public:
	RangeParameter(const QString& name, const T& value, const T& min, const T& max, const T& stepsize);

	void setValue(const T& value) override;
	const T& maximum() const;
	const T& minimum() const;
	const T& stepsize() const;

private:
	T m_minimum;
	T m_maximum;
	T m_stepsize;
};
}

#include "RangeParameter.inl"