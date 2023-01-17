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
	bool IsPointInsideTriangle2(glm::vec3 testPoint, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		// Translate point and triangle so that point lies at origin
		p0 -= testPoint;
		p1 -= testPoint;
		p2 -= testPoint;

		float p0p1 = glm::dot(p0, p1);
		float p0p2 = glm::dot(p0, p2);
		float p1p2 = glm::dot(p1, p2);
		float p2p2 = glm::dot(p2, p2);

		if (p1p2 * p0p2 - p2p2 * p0p1 < 0.0f)
			return false;
		float p1p1 = glm::dot(p1, p1);
		if (p0p1 * p1p2 - p0p2 * p1p1 < 0.0f)
			return false;

		return true;
	}

	inline float TrigArea2D(float x0, float y0, float x1, float y1, float x2, float y2)
	{
		return (x0 - x1) * (y1 - y2) - (x1 - x2) * (y0 - y1);
	}

	void Barycentric(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p, float& u, float& v, float& w)
	{
		glm::vec3 m = glm::cross(p1 - p0, p2 - p0);

		float nu, nv, ood;

		float x = abs(m.x);
		float y = abs(m.y);
		float z = abs(m.z);

		if (x >= y && x >= z) 
		{
			nu = TrigArea2D(p.y, p.z, p1.y, p1.z, p2.y, p2.z);
			nv = TrigArea2D(p.y, p.z, p2.y, p2.z, p0.y, p0.z);
			ood = 1.0f / m.x;
		}
		else if (y >= x && y >= z)
		{
			nu = TrigArea2D(p.x, p.z, p1.x, p1.z, p2.x, p2.z);
			nv = TrigArea2D(p.x, p.z, p2.x, p2.z, p0.x, p0.z);
			ood = 1.0f / -m.y;
		}
		else
		{
			nu = TrigArea2D(p.x, p.y, p1.x, p1.y, p2.x, p2.y);
			nv = TrigArea2D(p.x, p.y, p2.x, p2.y, p0.x, p0.y);
			ood = 1.0f / m.z;
		}

		u = nu * ood;
		v = nv * ood;
		w = 1.0f - u - v;
	}


	// triangle is counter clockswise
	bool IsPointInsideTriangle_Barycentric(glm::vec3 testPoint, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		float u, v, w;
		Barycentric(p0, p1, p2, testPoint, u, v, w);
		return v >= 0.0f && w >= 0.0f && (v + w) <= 1.0f;
	}


	// triangle is counter clockswise
	bool IsPointInsideTriangle_Barycentric_NotOnTheLine(glm::vec3 testPoint, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
	{
		float u, v, w;
		Barycentric(p0, p1, p2, testPoint, u, v, w);
		return v > 0.0f && w > 0.0f && (v + w) < 1.0f;
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