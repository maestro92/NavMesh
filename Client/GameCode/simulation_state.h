#pragma once

#include <vector>
#include "world.h"
#include "geometry_core.h"
#include <math.h>

struct SelectionBox
{
	glm::vec2 p0;
	glm::vec2 p1;
	bool active;

	gmt::AABB box;

	void SetP0(glm::vec3 p)
	{
		p0.x = p.x;
		p0.y = p.y;
	}

	void SetP1(glm::vec3 p)
	{
		p1.x = p.x;
		p1.y = p.y;
	}

	void UpdateAABB()
	{
		box.min.x = std::min(p0.x, p1.x);
		box.max.x = std::max(p0.x, p1.x);

		box.min.y = std::min(p0.y, p1.y);
		box.max.y = std::max(p0.y, p1.y);
	}
};

// get rid of this
struct SimulationStateData
{
	std::vector<Entity*> selectedEntities;
};

struct SimulationState {
	SelectionBox selectionBox;
	SimulationStateData* data;
};
