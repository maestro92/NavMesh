#pragma once

#include "platform_shared.h"

struct Rect
{
	glm::vec3 min;
	glm::vec3 max;
};

bool IsPointInsideRect(Rect rect, glm::vec3 point)
{
	return rect.min.x <= point.x && point.x < rect.max.x && rect.min.y <= point.y && point.y < rect.max.y;
}