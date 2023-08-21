#pragma once

#include "simulation.h"
#include "game_state.h"
#include "world.h"
#include "pathfinding.h"

namespace Sim
{


	void SetDestination(SimulationState* simState,
		GameInputState* gameInputState,
		World* world,
		glm::vec3 groundIntersectionPoint)
	{
		Entity* entity = simState->selectedEntity;
		if (entity != NULL)
		{
			if (gameInputState->DidMouseRightButtonClicked())
			{
				entity->destination = groundIntersectionPoint;

				float diameter = entity->agentRadius * 2;
				PathFinding::PathfindingResult pathingResult = PathFinding::FindPath(world->pathingDebug, diameter, world, entity->pos, entity->destination);
				entity->waypoints = pathingResult.waypoints;
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

	void SimModeTick(SimulationState* simState,
		GameInputState* gameInputState,
		RenderSystem::GameRenderCommands* gameRenderCommands,
		World* world,
		glm::vec3 groundIntersectionPoint)
	{
		InteractWorldEntities(simState, gameInputState, gameRenderCommands, world, groundIntersectionPoint);

		SetDestination(simState, gameInputState, world, groundIntersectionPoint);


		for (int i = 0; i < world->numEntities; i++)
		{
			Entity* entity = &world->entities[i];
			switch (entity->flag)
			{
				case EntityFlag::AGENT:
				{


				} break;
			}
		}
	}
}