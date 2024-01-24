#pragma once

#include <vector>
#include "world.h"
#include "geometry_core.h"
#include <math.h>
#include <unordered_set>

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


namespace sim
{
	struct Group
	{
		int id;
		// brute forcing it now
		std::vector<int> entityIds;

		// todo: make this more efficient
	//	std::unordered_map<int, int> idLookup;

		bool IsEmpty()
		{
			return entityIds.size() == 0;
		}

		bool HasUnit(int id)
		{
		//	return idLookup.find(id) != idLookup.end();
			for (int i = 0; i < entityIds.size(); i++)
			{
				if (entityIds[i] == id)
				{
					return true;
				}
			}
			return false;
		}

		void Add(int entityId)
		{
			entityIds.push_back(entityId);

			/*
			if (!HasUnit(entityId))
			{
				ids.push_back(entityId);
				int index = ids.size() - 1;
				idLookup[entityId] = index;
			}
			*/
		}

		void Remove(int entityId)
		{
			if (entityIds.size() > 1)
			{
				int lastIndex = entityIds.size() - 1;
				for (int i = 0; i < entityIds.size(); i++)
				{
					if (entityIds[i] == entityId)
					{
						int temp = entityIds[lastIndex];
						entityIds[i] = temp;
						break;
					}
				}
			}

			entityIds.pop_back();

			/*
			if (HasUnit(entityId))
			{
				int index = idLookup[entityId];
				ids.
				idLookup.erase(entityId);
			}
			*/
		}
	};

	struct GroupManager
	{
		int groupIdCounter;

		std::vector<Group> groups;

		GroupManager()
		{
			groupIdCounter = 0;
		}

		Group* GetOrCreateNewGroup()
		{
			for (int i = 0; i < groups.size(); i++)
			{
				if (groups[i].IsEmpty())
				{
					return &groups[i];
				}
			}

			Group group = Group();
			group.id = groupIdCounter++;
			groups.push_back(group);

			int index = groups.size()-1;
			return &groups[index];
		}

		Group* GetGroup(int groupId)
		{
			for (int i = 0; i < groups.size(); i++)
			{
				if (groups[i].id == groupId)
				{
					return &groups[i];
				}
			}
			return NULL;
		}

		/*
		void TryRemove(Group* group)
		{
			if (!group->IsEmpty())
			{
				return;
			}

			if (groups.size() > 1)
			{
				int lastIndex = groups.size() - 1;
				for (int i = 0; i < groups.size(); i++)
				{
					if (groups[i].id == group->id)
					{
						Group temp = groups[lastIndex];
						groups[i] = temp;
						break;
					}
				}
			}

			groups.pop_back();
		}
		*/
	};
}

// get rid of this
struct SimulationStateData
{
	std::vector<Entity*> selectedEntities;
	sim::GroupManager groupManager;
};

struct SimulationState {
	SelectionBox selectionBox;
	SimulationStateData* data;
};
