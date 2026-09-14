#pragma once

#include <QObject>
#include <QString>

namespace sahara::rendering
{

// This base class contains the API of a parameter and all QObject-related stuff.
// The templated stuff is held separately in the (templated) Parameter class, as Qts moc can't handle template classes

class AbstractParameter : public QObject
{
	Q_OBJECT

public:
	AbstractParameter(const QString& name);
	virtual ~AbstractParameter() = default;

	const QString& name() const;
	bool isVisible() const noexcept;
	void setVisible(bool visible);

	virtual void reset() = 0;

signals:
	void valueChanged();
	void visibilityChanged(bool visible);

protected:
	QString m_name;
	bool m_visible;
};

}
