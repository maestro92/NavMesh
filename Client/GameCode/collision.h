#pragma once

#include "../PlatformShared/platform_shared.h"
#include "math.h"

namespace Collision
{
	float EPSILON = 1e-5;
	struct Rect
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	bool IsPointInsideRect(Rect rect, glm::vec3 point)
	{
		return rect.min.x <= point.x && point.x < rect.max.x&& rect.min.y <= point.y && point.y < rect.max.y;
	}

	// triangle is counter clockswise
	bool IsPointInsideTriangle(glm::vec3 testPoint, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		// Translate point and triangle so that point lies at origin
		p0 -= testPoint; 
		p1 -= testPoint; 
		p2 -= testPoint;

		// Compute normal vectors for triangles pab and pbc
		glm::vec3 u = glm::cross(p1, p2);
		glm::vec3 v = glm::cross(p2, p0);

		// Make sure they are both pointing in the same direction
		if (glm::dot(u, v) < 0.0f) 
			return false;
		
		// Compute normal vector for triangle pca
		glm::vec3 w = glm::cross(p0, p1);
		
		// Make sure it points in the same direction as the first two
		if (glm::dot(u, w) < 0.0f)
			return false;
		
		// Otherwise P must be in (or on) the triangle
		return true;
	}

	void ClosestPtPointLineSegment(glm::vec3 linePt0, glm::vec3 linePt1, glm::vec3 point, float& t, glm::vec3& closestPoint)
	{
		glm::vec3 dir = linePt1 - linePt0;

		t = glm::dot(point - linePt0, dir) / glm::dot(dir, dir);

		if (t < 0.0f)
			t = 0;
		if (t > 1.0f)
			t = 1.0f;

		closestPoint = linePt0 + t * dir;
	}

	bool IsPointOnLine(glm::vec3 linePt0, glm::vec3 linePt1, glm::vec3 point)
	{
		float t = 0;
		glm::vec3 closestPoint;
		ClosestPtPointLineSegment(linePt0, linePt1, point, t, closestPoint);

		glm::vec3 vec = point - closestPoint;
		float distSq = glm::dot(vec, vec);
		return distSq < EPSILON;
	}

}