#pragma once

#include <QWidget>

namespace sahara::ui
{

class ScenePanelItemWidget : public QWidget
{
	Q_OBJECT

public:
	ScenePanelItemWidget(const QString& text);

signals:
	void visibilityToggled(bool visible);

private:
};

}