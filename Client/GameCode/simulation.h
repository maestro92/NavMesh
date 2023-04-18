#pragma once

#include "simulation.h"
#include "game_state.h"
#include "world.h"


namespace Sim
{

	void SimModeTick(SimulationState* simState,
		GameInputState* gameInputState,
		RenderSystem::GameRenderCommands* gameRenderCommands,
		World* world,
		glm::vec3 groundIntersectionPoint)
	{
		InteractWorldEntities(simState, gameInputState, gameRenderCommands, world, groundIntersectionPoint);

		SetDestination(simState, gameInputState, groundIntersectionPoint);

	}

	void SetDestination(SimulationState* simState,
		GameInputState* gameInputState,
		glm::vec3 groundIntersectionPoint)
	{
		if (simState->selectedEntity != NULL)
		{
			if (gameInputState->DidMouseRightButtonClicked())
			{
				simState->selectedEntity->destination = groundIntersectionPoint;
			}
		}
	}


	void InteractWorldEntities(SimulationState* simState,
		GameInputState* gameInputState,
		RenderSystem::GameRenderCommands* gameRenderCommands,
		World* world,
		glm::vec3 groundIntersectionPoint)
	{

		bool done = false;
		Entity* candidate = NULL;
		for (int i = 0; i < world->numEntities; i++)
		{
			Entity* entity = &world->entities[i];

			if (done)
			{
				break;
			}

			switch (entity->flag)
			{
				case EntityFlag::OBSTACLE:
				{
					std::vector<glm::vec3> absolutePos;

					for (int j = 0; j < entity->vertices.size(); j++)
					{
						absolutePos.push_back(entity->pos + entity->vertices[j]);
					}

					if (Collision::IsPointInsidePolygon2D(groundIntersectionPoint, absolutePos))
					{
						candidate = entity;
						done = true;

					}
				} break;

				case EntityFlag::AGENT:
				{
					if (Collision::IsPointInsideCircle(glm::vec2(groundIntersectionPoint), entity->agentRadius, glm::vec2(entity->pos)))
					{
						candidate = entity;
						done = true;
					}
				} break;

			}
		}


		if (candidate != NULL)
		{
			if (gameInputState->DidMouseLeftButtonClicked())
			{
				simState->selectedEntity = candidate;
			}
		}
	}
}