#pragma once

#include "simulation.h"
#include "game_state.h"
#include "world.h"
#include "pathfinding.h"

namespace Sim
{


	void SetDestinationTick(SimulationState* simState,
		GameInputState* gameInputState,
		World* world,
		glm::vec3 groundIntersectionPoint)
	{
		
		if (gameInputState->DidMouseRightButtonClickedDown())
		{
			for(int i=0; i<simState->data->selectedEntities.size(); i++)
			{
				Entity* entity = simState->data->selectedEntities[i];
				entity->pathingState.destination = groundIntersectionPoint;

				float diameter = entity->agentRadius * 2;
				PathFinding::PathfindingResult pathingResult = PathFinding::FindPath(world->pathingDebug, diameter, world, entity->pos, entity->pathingState.destination);
				entity->pathingState.BeginPathingToDestination(pathingResult.waypoints);
			}
		}
		
	}


	void AgentPathingTick(Entity* entity)
	{
		
		float speed = 2.0f;
		float dt = 0.1f;

		PathingState* state = &(entity->pathingState);

		if (state->curTargetWaypointIndex >= state->waypoints.size())
		{
			return;
		}

		float distTravelled = dt * speed;

		while (distTravelled > 0)
		{
			if (state->curTargetWaypointIndex >= state->waypoints.size())
			{
				break;
			}

			glm::vec3 vecToNextWaypoint = state->waypoints[state->curTargetWaypointIndex] - entity->pos;
			float distToNextWayPoint = glm::length(vecToNextWaypoint);

			// i can communte to the next waypoint
			if (distTravelled >= distToNextWayPoint)
			{
				distTravelled -= distToNextWayPoint;
				entity->pos = state->waypoints[state->curTargetWaypointIndex];
				state->curTargetWaypointIndex++;
			}
			else
			{
				glm::vec3 dir = glm::normalize(vecToNextWaypoint);
				entity->pos = entity->pos + dir * distTravelled;
				entity->facingDirection = dir;
				distTravelled = 0;
			}
		}
	}

	void SimModeTick(SimulationState* simState,
		GameInputState* gameInputState,
		RenderSystem::GameRenderCommands* gameRenderCommands,
		World* world,
		glm::vec3 groundIntersectionPoint)
	{
		// InteractWorldEntities(simState, gameInputState, gameRenderCommands, world, groundIntersectionPoint);

		SetDestinationTick(simState, gameInputState, world, groundIntersectionPoint);

		for (int i = 0; i < world->numEntities; i++)
		{
			Entity* entity = &world->entities[i];
			switch (entity->flag)
			{
				case EntityFlag::AGENT:
				{
					AgentPathingTick(entity);




				} break;
			}
		}
	}
}