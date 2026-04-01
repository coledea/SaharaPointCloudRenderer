#include "AbstractNavigation.h"

namespace sahara::navigation
{

AbstractNavigation::AbstractNavigation(Camera* camera) noexcept
	: m_camera(camera)
{
}

}