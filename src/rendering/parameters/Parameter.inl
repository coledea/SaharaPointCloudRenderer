#include "Parameter.h"

namespace sahara::rendering
{

template <class T>
inline Parameter<T>::Parameter(const QString& name, const T& value)
	: AbstractParameter(name)
	, m_initial_value(value)
	, m_value(value)
{
}

template <class T>
inline const T& Parameter<T>::value() const
{
	return m_value;
}

template <class T>
inline void Parameter<T>::setValue(const T& value)
{
	m_value = value;
	emit valueChanged();
}

template <class T>
inline void Parameter<T>::reset()
{
	setValue(m_initial_value);
}

}