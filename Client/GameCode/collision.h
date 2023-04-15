#pragma once

#include "../PlatformShared/platform_shared.h"
#include "math.h"
#include <vector>
#include "geometry_core.h"
#include <algorithm>

namespace Collision
{
	float EPSILON = 1e-5;

	bool IsPointInsideRect(GeoCore::AABB rect, glm::vec3 point)
	{
		return rect.min.x <= point.x && point.x < rect.max.x&& rect.min.y <= point.y && point.y < rect.max.y;
	}

	bool IsPointInsidePolygon(std::vector<glm::vec3> polygon, glm::vec3 point)
	{
		return false;
	}

	/*
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
	*/

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


	float Signed2DTrigArea(glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
	}


	bool DoesAABBAABBIntersect2D(GeoCore::AABB a, GeoCore::AABB b)
	{
		if (a.max.x < b.min.x || a.min.x > b.max.x)
		{
			return false;
		}

		if (a.max.y < b.min.y || a.min.y > b.max.y)
		{
			return false;
		}

		return true;
	}


	bool GetLineLineIntersectionPoint(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2& intersectionPoint)
	{
		float a1 = Signed2DTrigArea(p0, p1, p3);
		float a2 = Signed2DTrigArea(p0, p1, p2);

		if (a1 * a2 < 0.0f)
		{
			float a3 = Signed2DTrigArea(p2, p3, p0);
			float a4 = a3 + a2 - a1;

			if (a3 * a4 < 0.0f)
			{
				float t = a3 / (a3 - a4);
				intersectionPoint = p0 + t * (p1 - p0);
				return true;
			}
		}
		return false;
	}


	bool GetLineLineIntersectionPoint_CheckOnlyXY2D(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec2& intersectionPoint)
	{
		glm::vec2 a2 = glm::vec2(a.x, a.y);
		glm::vec2 b2 = glm::vec2(b.x, b.y);
		glm::vec2 c2 = glm::vec2(c.x, c.y);
		glm::vec2 d2 = glm::vec2(d.x, d.y);

		return GetLineLineIntersectionPoint(a2, b2, c2, d2, intersectionPoint);
	}

	bool GetRayRayIntersection2D(glm::vec2 p0, glm::vec2 d0, glm::vec2 p1, glm::vec2 d1, glm::vec2& intersectionPoint)
	{
		float dx = p1.x - p0.x;
		float dy = p1.y - p0.y;

		float det = d1.x * d0.y - d1.y * d0.x;
		if (det == 0)
		{
			return false;
		}

		float u = (dy * d1.x - dx * d1.y) / det;
		float v = (dy * d0.x - dx * d0.y) / det;

		intersectionPoint = p0 + u * d0;
		return true;
	}

	bool IsConvex(const std::vector<glm::vec3>& polygon)
	{
		int v0 = 0;
		int v1 = 0;
		int v2 = 0;

		for (int i = 0; i < polygon.size(); i++)
		{
			v0 = i - 1;
			v1 = i;
			v2 = i + 1;

			if (i == 0)
			{
				v0 = polygon.size() - 1;
			}
			else if (i == polygon.size() - 1)
			{
				v2 = 0;
			}

			float angle2 = Math::CalculateInteriorAngle(polygon[v0], polygon[v1], polygon[v2]);

			// not a convex vertex
			if (angle2 >= 180)
			{
				return false;
			}
		}

		return true;
	}


	bool IsPointInsideCircle(glm::vec2 center, float radius, glm::vec2 point)
	{
		glm::vec2 dx = center - point;
		float distSquared = glm::dot(dx, dx);
		return radius * radius > distSquared;
	}


	struct Plane
	{
		glm::vec3 normal;
		float dist;
	};

	struct Ray
	{
		glm::vec3 p;
		glm::vec3 dir;
	};

	Plane GetPlane(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 normal = glm::cross(ab, ac);
		float dist = glm::dot(normal, a);
		return { normal, dist };
	}

	bool RayPlaneIntersection3D(Plane plane, Ray ray, glm::vec3& intersectionPoint)
	{
		// check if p is above the plane, and approaching the plane
		// or if p is below the plane, and approaching the plane.
		// similar to the moving sphere to plane test
		float dist = glm::dot(plane.normal, ray.p) - plane.dist;

		if (dist == 0)
		{
			intersectionPoint = ray.p;
			return true;
		}
		else
		{
			float denom = glm::dot(plane.normal, ray.dir);
			if (denom * dist >= 0.0f)
			{
				// no intersection
				return 0;
			}
			else
			{
				// ray is moving towards the plane

				//       distance from point to plane 
				// t = --------------------------------------
				//      distance covered by direction to plane

				float t = -(glm::dot(ray.p, plane.normal) + plane.dist) / glm::dot(ray.dir, plane.normal);
				intersectionPoint = ray.p + t * ray.dir;
				return true;
			}
		}
	}

	bool RayTriangleIntersection3D(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3& intersectionPoint)
	{
		Plane plane = GetPlane(a, b, c);
		Ray ray = { rayOrigin, rayDir };

		// first check if ray plane intersection happens
		if (!RayPlaneIntersection3D(plane, ray, intersectionPoint))
		{
			return false;
		}

		if (IsPointInsideTriangle_Barycentric(intersectionPoint, a, b, c))
		{
			return true;
		}			
	}

	float GetDeterminant(glm::vec2 a, glm::vec2 b)
	{
		return a.x * b.y - a.y * b.x;
	}


	//Where is p in relation to a-b
	// < 0 -> to the right
	// = 0 -> on the line
	// > 0 -> to the left
	// a 2D cross product
	bool IsPointOnTheLeftOfLineSegment2D(glm::vec2 point, glm::vec2 a, glm::vec2 b)
	{
		return GetDeterminant(b-a, point-a) > 0;
	}

	bool IsPointOnTheRightOfLineSegment2D(glm::vec2 point, glm::vec2 a, glm::vec2 b)
	{
		return GetDeterminant(b-a, point-a) < 0;
	}	
	


	// https://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf
	// separating axis theorem
	bool TestTriangleAABB2D(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, GeoCore::AABB b)
	{
		// p0, p1, p2, is projecting the triangle points onto an axis
		float p0, p1, p2, r;

		glm::vec3 c = (b.min + b.max) * 0.5f;
		float e0 = (b.max.x - b.min.x) * 0.5f;
		float e1 = (b.max.y - b.min.y) * 0.5f;

		v0 = v0 - c;
		v1 = v1 - c;
		v2 = v2 - c;

		// triangle edges
		glm::vec3 f0 = v1 - v0;
		glm::vec3 f1 = v2 - v1;
		glm::vec3 f2 = v0 - v2;

		glm::vec3 zAxis = glm::vec3(0, 0, 1);

		glm::vec3 edges[3];
		edges[0] = f0;
		edges[1] = f1;
		edges[2] = f2;

		// 3 separating axis of the triangle edges
		for (int i = 0; i < 3; i++)
		{
			// recall the separating axis is perpendicular to the edge
			// so we just use the zAxis to get it.
			glm::vec3 axis = glm::normalize(glm::cross(edges[i], zAxis));

			p0 = glm::dot(v0, axis);
			p1 = glm::dot(v1, axis);
			p2 = glm::dot(v2, axis);

			r = e0 * std::abs(glm::dot(glm::vec3(1, 0, 0), axis)) + 
				e1 * std::abs(glm::dot(glm::vec3(0, 1, 0), axis));

			if (std::max(-Math::Max(p0, p1, p2), Math::Min(p0, p1, p2)) > r)
			{
				return false;
			}
		}

		// 2 axes to the face normals of aabb b
		if (Math::Max(v0.x, v1.x, v2.x) < -e0 || Math::Min(v0.x, v1.x, v2.x) > e0)
		{
			return false;
		}

		if (Math::Max(v0.y, v1.y, v2.y) < -e1 || Math::Min(v0.y, v1.y, v2.y) > e1)
		{
			return false;
		}

		return true;
	}
	

	// https://erich.realtimerendering.com/ptinpoly/
	// assume polygon is counter clockwise
	bool IsPointInsidePolygon2D(glm::vec3 point, std::vector<glm::vec3> polygon)
	{
		int windingNumber = 0;
		glm::vec2 point2D = glm::vec2(point.x, point.y);

		for (int i = 0; i < polygon.size(); i++)
		{
			glm::vec2 v0 = glm::vec2(polygon[i].x, polygon[i].y);
			glm::vec2 v1;
			if (i == polygon.size() - 1)
			{
				v1 = glm::vec2(polygon[0].x, polygon[0].y);
			}
			else
			{
				v1 = glm::vec2(polygon[i+1].x, polygon[i+1].y);
			}


			// an upward crossing
			if (v0.y <= point2D.y)
			{
				if (v1.y > point2D.y)
				{
					if (IsPointOnTheLeftOfLineSegment2D(point2D, v0, v1))
					{
						windingNumber++;
					}
				}
			}
			else
			{
				if (v1.y <= point2D.y)
				{
					if (IsPointOnTheRightOfLineSegment2D(point2D, v0, v1))
					{
						windingNumber--;
					}
				}
			}
		}


		return windingNumber;
	}

}