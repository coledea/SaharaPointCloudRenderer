#include "RangeParameter.h"
#include "utils/QVector3DUtilities.h"

namespace sahara::rendering
{

template <class T>
inline RangeParameter<T>::RangeParameter(const QString& name, const T& value, const T& min, const T& max, const T& stepsize)
	: Parameter<T>(name, value)
	, m_minimum(min)
	, m_maximum(max)
	, m_stepsize(stepsize)
{
}

template <class T>
inline void RangeParameter<T>::setValue(const T& value)
{
	using namespace std; // Necessary for the compiler to find overloads for native datatypes
	this->m_value = round(value / m_stepsize) * m_stepsize;
	this->m_value = max(min(m_maximum, this->m_value), m_minimum);
	emit this->valueChanged();
}

template <class T>
inline const T& RangeParameter<T>::minimum() const
{
	return m_minimum;
}

template <class T>
inline const T& RangeParameter<T>::maximum() const
{
	return m_maximum;
}

template <class T>
inline const T& RangeParameter<T>::stepsize() const
{
	return m_stepsize;
}

}