#pragma once

#include "simulation.h"
#include "game_state.h"
#include "world.h"
#include "pathfinding.h"

namespace sim
{
	float c_dt = 1;

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


	void PathingTick(Entity* entity)
	{
		
		float speed = 0.2f;

		PathingState* state = &(entity->pathingState);

		if (state->curTargetWaypointIndex >= state->waypoints.size())
		{
			return;
		}

		float maxDistTravelled = c_dt * speed;

		while (maxDistTravelled > 0)
		{
			if (state->curTargetWaypointIndex >= state->waypoints.size())
			{
				break;
			}

			glm::vec3 vecToNextWaypoint = state->waypoints[state->curTargetWaypointIndex] - entity->pos;
			float distToNextWayPoint = glm::length(vecToNextWaypoint);
			glm::vec3 dir = glm::normalize(vecToNextWaypoint);
		//	entity->facingDirection = dir;

			// i can communte to the next waypoint
			if (maxDistTravelled >= distToNextWayPoint)
			{
				maxDistTravelled -= distToNextWayPoint;

				entity->velocity = vecToNextWaypoint;

//				entity->pos = state->waypoints[state->curTargetWaypointIndex];
				state->curTargetWaypointIndex++;
			}
			else
			{
				entity->velocity = dir * speed;

//				entity->pos = entity->pos + dir * maxDistTravelled;

				maxDistTravelled = 0;
			}
			entity->facingDirection = dir;
		}

		std::cout << entity->id << " " << entity->velocity.x << " " << entity->velocity.y << " " << std::endl;
	}

	// https://www.youtube.com/watch?v=nvx-13zWTPI&t=1156s&ab_channel=AndrewSmith
	// https://people.ece.cornell.edu/land/courses/ece4760/labs/s2021/Boids/Boids.html
	// https://ap011y0n.github.io/Group-Movement/

	// https://www.jdxdev.com/blog/2021/03/19/boids-for-rts/
	// replaced the boid separation force with a custom avoidance force
	void BoidsTick(Entity* entity,
		World* world)
	{

		const float c_avoidFactor = 0.05;
		const float c_protenctedRangeSq = 4; // 5 squared
		
		const float c_visibleRangeSq = 400; // 10 squared
		const float c_alignmentFactor = 0.05;

		const float c_centeringFactor = 0.0005;

		glm::vec2 closeDx = glm::vec2(0);

		glm::vec2 avgNeighborPos = glm::vec2(0);
		glm::vec2 avgNeighborVel = glm::vec2(0);
		float numNeighbors = 0;
		


		for (int i = 0; i < world->numEntities; i++)
		{
			Entity* entity2 = &world->entities[i];

			if (entity2->flag != EntityFlag::AGENT)
			{
				continue;
			}

			if (entity2->id == entity->id)
			{
				continue;
			}

			float distSq = glm::length2(entity->pos - entity2->pos);

			float dist = glm::length(entity->pos - entity2->pos);
		//	std::cout << " distSq " << distSq << std::endl;
		//	std::cout << " dist " << dist << std::endl;

			if (distSq < c_protenctedRangeSq)
			{
				// Separation
			//	closeDx.x += entity->pos.x - entity2->pos.x;
			//	closeDx.y += entity->pos.y - entity2->pos.y;
			}

			if (distSq < c_visibleRangeSq)
			{
				// Alignment
				avgNeighborVel.x += entity2->velocity.x;
				avgNeighborVel.y += entity2->velocity.y;

				// Cohesion
				avgNeighborPos.x += entity2->pos.x;
				avgNeighborPos.y += entity2->pos.y;

				numNeighbors++;
			}
		}

		entity->velocity.x += closeDx.x * c_avoidFactor;
		entity->velocity.y += closeDx.y * c_avoidFactor;

		if (numNeighbors > 0)
		{
			avgNeighborVel = avgNeighborVel / numNeighbors;
			entity->velocity.x += (avgNeighborVel.x - entity->velocity.x) * c_alignmentFactor;
			entity->velocity.y += (avgNeighborVel.y - entity->velocity.y) * c_alignmentFactor;

			avgNeighborPos = avgNeighborPos / numNeighbors;
			entity->velocity.x += (avgNeighborPos.x - entity->pos.x) * c_centeringFactor;
			entity->velocity.y += (avgNeighborPos.y - entity->pos.y) * c_centeringFactor;
		}

		std::cout << entity->id << " " << entity->velocity.x << " " << entity->velocity.y << " " << std::endl;
	}

	void ResolveCollision(Entity* entity, Entity* entity2, Collision::ContactData contactData)
	{
		// do it by mass

		if (entity->flag == AGENT && entity2->flag == AGENT)
		{
			entity->velocity += contactData.normal * contactData.penetrationDepth / 2.0f;
			entity2->velocity -= contactData.normal * contactData.penetrationDepth / 2.0f;
		}
		else if (entity->flag == AGENT && entity2->flag == OBSTACLE)
		{
			entity->velocity += contactData.normal * contactData.penetrationDepth;
		}

	}


	// https://github.com/erincatto/box2d/blob/main/src/collision/b2_collide_circle.cpp
	// can just copy box 2d's algorithm
	// b2CollidePolygonAndCircle
	void TestAgentObtacleCollision(Entity* entity, Entity* entity2)
	{
		gmt::Sphere sa = gmt::Sphere();
		sa.center = entity->pos;
		sa.radius = entity->agentRadius;

		Collision::ContactData contactData;
		if (Collision::PolygonCircleCollision(sa, entity2->physBody, entity2->pos, contactData))
		{
			ResolveCollision(entity, entity2, contactData);
		}
	}


	void PhysicsTick(Entity* entity, World* world)
	{

		for (int i = 0; i < world->numEntities; i++)
		{
			Entity* entity2 = &world->entities[i];

			if (entity2->id == entity->id)
			{
				continue;
			}

			if (entity2->flag == EntityFlag::AGENT)
			{
				gmt::Sphere sa = gmt::Sphere();
				sa.center = entity->pos;
				sa.radius = entity->agentRadius;

				gmt::Sphere sb = gmt::Sphere();
				sb.center = entity2->pos;
				sb.radius = entity2->agentRadius;

				Collision::ContactData contactData;
				if (Collision::SphereSphereIntersection(sa, sb, contactData))
				{
					ResolveCollision(entity, entity2, contactData);
				}
			}
			else if (entity2->flag == EntityFlag::OBSTACLE)
			{
				TestAgentObtacleCollision(entity, entity2);


				/*
				gmt::Sphere sa = gmt::Sphere();
				sa.center = entity->pos;
				sa.radius = entity->agentRadius;

				gmt::Sphere sb = gmt::Sphere();
				sb.center = entity2->pos;
				sb.radius = entity2->agentRadius;

				Collision::ContactData contactData;
				if (Collision::SphereSphereIntersection(sa, sb, contactData))
				{
					ResolveCollision(entity, entity2, contactData);
				}
				*/
			}
		}
	}


	void TransformTick(Entity* entity)
	{
		float c_maxSpeed = 2;

		// clamp the max velocity
		float speed = glm::length(entity->velocity);
		glm::vec3 dir = glm::normalize(entity->velocity);

		if (speed > c_maxSpeed)
		{
			entity->velocity = dir * c_maxSpeed;
		}


		entity->pos = entity->pos + entity->velocity;
	}

	void EntityTick(Entity* entity,
		SimulationState* simState,
		GameInputState* gameInputState,
		RenderSystem::GameRenderCommands* gameRenderCommands,
		World* world)
	{

		std::cout << ">>>> entity Tick " << entity->id << " " << entity->pos.x << " " << entity->pos.y << " " << std::endl;


		entity->velocity = glm::vec3(0);

		PathingTick(entity);
	//	BoidsTick(entity, world);
		PhysicsTick(entity, world);
		TransformTick(entity);

		std::cout << "				" << entity->id << " " << entity->pos.x << " " << entity->pos.y << " " << std::endl;
		

	}


	void SimModeTick(SimulationState* simState,
		GameInputState* gameInputState,
		RenderSystem::GameRenderCommands* gameRenderCommands,
		World* world,
		glm::vec3 groundIntersectionPoint)
	{
		std::cout << ">>>> SimModeTick " << std::endl;

		// InteractWorldEntities(simState, gameInputState, gameRenderCommands, world, groundIntersectionPoint);

		SetDestinationTick(simState, gameInputState, world, groundIntersectionPoint);

		for (int i = 0; i < world->numEntities; i++)
		{
			Entity* entity = &world->entities[i];
			switch (entity->flag)
			{
				case EntityFlag::AGENT:
				{
					EntityTick(entity, simState, gameInputState, gameRenderCommands, world);
				} break;
			}
		}
	}



}