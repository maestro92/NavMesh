#pragma once

#include "../PlatformShared/platform_shared.h"


namespace GameRender
{
	glm::vec4 HALF_TRANS_COLOR_WHITE = glm::vec4(1, 1, 1, 0.5);
	glm::vec4 HALF_TRANS_COLOR_RED = glm::vec4(1, 0, 0, 0.5);


	glm::vec4 SELECTED_ENTITY_COLOR = glm::vec4(1, 0, 0, 0.25);
	glm::vec4 POTENTIAL_PATHING_START_COLOR = glm::vec4(1, 0, 0, 0.25);
	glm::vec4 POTENTIAL_PATHING_END_COLOR = glm::vec4(0, 1, 0, 0.25);

	glm::vec4 PATHING_START_COLOR = glm::vec4(1, 0, 0, 1);
	glm::vec4 PATHING_END_COLOR = glm::vec4(0, 1, 0, 1);


	glm::vec4 COLOR_WHITE = glm::vec4(1, 1, 1, 1);
	glm::vec4 COLOR_RED = glm::vec4(1, 0, 0, 1);
	glm::vec4 COLOR_GREEN = glm::vec4(0, 1, 0, 1);
	glm::vec4 COLOR_BLUE = glm::vec4(0, 0, 1, 1);
	glm::vec4 COLOR_YELLOW = glm::vec4(1, 1, 0, 1);
	glm::vec4 COLOR_TEAL = glm::vec4(0, 1, 1, 1);
	glm::vec4 COLOR_ORANGE = glm::vec4(1, 0.5, 1, 1);
	glm::vec4 COLOR_BLACK = glm::vec4(0, 0, 0, 1);

	glm::vec3 DEBUG_RENDER_OFFSET = glm::vec3(0, 0.1, 0);

	enum AlignmentMode
	{
		Centered,
		Left,
		Right,
		Top,
		Bottom
	};

	struct GameRenderState {
		RenderSystem::GameRenderCommands* gameRenderCommands;
		RenderSystem::RenderGroup* renderGroup;
		FontId debugFontId;
		LoadedFont* debugLoadedFont;
		GameAssets* gameAssets;
	};

	// p0 p1 p2 p3 in clock wise order
	void PushQuad_Core(
		RenderSystem::GameRenderCommands* gameRenderCommands, 
		RenderSystem::RenderGroup* renderGroup, 
		LoadedBitmap* bitmap,
		glm::vec3 p0, glm::vec2 uv0, glm::vec4 color0,
		glm::vec3 p1, glm::vec2 uv1, glm::vec4 color1,
		glm::vec3 p2, glm::vec2 uv2, glm::vec4 color2,
		glm::vec3 p3, glm::vec2 uv3, glm::vec4 color3)
	{
		if (gameRenderCommands->HasSpaceForVertex(4))
		{
			RenderSystem::RenderGroupEntryTexturedQuads* entry = renderGroup->quads;
			entry->numQuads++;

			assert(gameRenderCommands->numBitmaps < gameRenderCommands->maxNumBitmaps);
			int index = gameRenderCommands->numBitmaps;
			gameRenderCommands->masterBitmapArray[index] = bitmap;
			gameRenderCommands->numBitmaps++;


			//	cout << "entry masterBitmapArray " << index << " " << bitmap->textureHandle << endl;
			//	cout << "gameRenderCommands->numBitmaps " << gameRenderCommands->numBitmaps << endl;

			int index2 = gameRenderCommands->numVertex;
			RenderSystem::TexturedVertex* vertexArray = &(gameRenderCommands->masterVertexArray[index2]);
			vertexArray[0].position = p0;
			vertexArray[0].normal = p0;
			vertexArray[0].uv = uv0;
			vertexArray[0].color = color0;

			vertexArray[1].position = p1;
			vertexArray[1].normal = p1;
			vertexArray[1].uv = uv1;
			vertexArray[1].color = color1;

			// Note that vertexArray[2] and vertexArray[3] has the points swapped since we want to
			// draw this with trangle strips.
			vertexArray[2].position = p3;
			vertexArray[2].normal = p3;
			vertexArray[2].uv = uv3;
			vertexArray[2].color = color3;

			vertexArray[3].position = p2;
			vertexArray[3].normal = p2;
			vertexArray[3].uv = uv2;
			vertexArray[3].color = color2;

			gameRenderCommands->numVertex += 4;
		}
	}

	
	// p0 p1 p2 p3 in clock wise order
	// p0 top left,		p3 top right
	// p1 bottom left,	p2 bottom right
	void PushQuad(
		RenderSystem::GameRenderCommands* gameRenderCommands, 
		RenderSystem::RenderGroup* renderGroup, 
		LoadedBitmap* bitmap,
		glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, 
		glm::vec4 color, 
		bool fakeLighting = false)
	{
		glm::vec2 t0 = glm::vec2(0, 0);
		glm::vec2 t1 = glm::vec2(1, 0);
		glm::vec2 t2 = glm::vec2(1, 1);
		glm::vec2 t3 = glm::vec2(0, 1);

		glm::vec4 topColor = color;
		glm::vec4 bottomColor = color;

		if (fakeLighting)
		{
			bottomColor *= 0.1;
			bottomColor.a = 1;
		}

		GameRender::PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p0, t0, topColor,
			p1, t1, bottomColor,
			p2, t2, bottomColor,
			p3, t3, topColor);
	}
	


	// right now, since the rendering pipeline only supports quads, im going to 
	// render triangles as quads and wasting one vertex
	void PushTriangle(
		RenderSystem::GameRenderCommands* gameRenderCommands, 
		RenderSystem::RenderGroup* renderGroup, 
		LoadedBitmap* bitmap,
		glm::vec4 color,
		glm::vec3 vertices[3],
		bool fakeLighting = false)
	{
		GameRender::PushQuad(gameRenderCommands, renderGroup, bitmap, 
			vertices[0],
			vertices[1],
			vertices[2],	// wasting one vertex here
			vertices[2],
			color, fakeLighting);
	}


	void PushCube(
		RenderSystem::GameRenderCommands* gameRenderCommands, 
		RenderSystem::RenderGroup* renderGroup, 
		LoadedBitmap* bitmap,
		std::vector<glm::vec3>& vertices, 
		glm::vec4 color, bool fakeLighting = false)
	{
		assert(vertices.size() == 8);
		// push the 6 sides

		/*
			y
			^
		  (-x,y,-z) p4 ------------ p5 (x,y,-z)
			|		|\              |\
			|		| \             | \
			|		| (-x,y,z)      |  \
			|		|	p0 ------------ p1 (x,y,z)
			|	    p6 -|----------	p7	|
		   (-x,-y,-z)\  |	(x,-y,-z)\	|
			|		  \	|		      \ |
			|		   \|			   \|
			|			p2 ------------ p3 (x,-y,z)
			|         (-x,-y,z)
			|
			------------------------------------------> x
			\
			 \
			  \
			   V z
		*/

		// 4 points on front face 
		glm::vec3 p0 = vertices[0];
		glm::vec3 p1 = vertices[1];
		glm::vec3 p2 = vertices[2];
		glm::vec3 p3 = vertices[3];

		// 4 points on back face 
		glm::vec3 p4 = vertices[4];
		glm::vec3 p5 = vertices[5];
		glm::vec3 p6 = vertices[6];
		glm::vec3 p7 = vertices[7];

		glm::vec2 t0 = glm::vec2(0, 0);
		glm::vec2 t1 = glm::vec2(1, 0);
		glm::vec2 t2 = glm::vec2(0, 1);
		glm::vec2 t3 = glm::vec2(1, 1);

		glm::vec4 topColor = color;
		glm::vec4 bottomColor = color;

		if (fakeLighting)
		{
			bottomColor *= 0.1;
			bottomColor.a = 1;
		}

		// front
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p0, t0, topColor,
			p1, t1, topColor,
			p3, t3, bottomColor,
			p2, t2, bottomColor);
		// top
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p4, t0, topColor,
			p5, t1, topColor,
			p1, t3, topColor,
			p0, t2, topColor);
		// left 
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p4, t0, topColor,
			p0, t1, topColor,
			p2, t3, bottomColor,
			p6, t2, bottomColor);
		// bottom
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p2, t0, bottomColor,
			p3, t1, bottomColor,
			p7, t3, bottomColor,
			p6, t2, bottomColor);
		// right 
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p1, t0, topColor,
			p5, t1, topColor,
			p7, t3, bottomColor,
			p3, t2, bottomColor);
		// back
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p5, t0, topColor,
			p4, t1, topColor,
			p6, t3, bottomColor,
			p7, t2, bottomColor);
	}


	void PushCube(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup,
		LoadedBitmap* bitmap,
		glm::vec4 color,
		glm::vec3 min, glm::vec3 max,
		bool fakeLighting = false)
	{
		/*
			y
			^
		  (-x,y,-z) p4 ------------ p5 (x,y,-z)
			|		|\              |\
			|		| \             | \
			|		| (-x,y,z)      |  \
			|		|	p0 ------------ p1 (x,y,z)
			|	    p6 -|----------	p7	|
		   (-x,-y,-z)\  |	(x,-y,-z)\	|
			|		  \	|		      \ |
			|		   \|			   \|
			|			p2 ------------ p3 (x,-y,z)
			|         (-x,-y,z)
			|
			------------------------------------------> x
			\
			 \
			  \
			   V z
		*/

		// 4 points on front face 
		glm::vec3 p0 = glm::vec3(min.x, max.y, max.z);
		glm::vec3 p1 = glm::vec3(max.x, max.y, max.z);
		glm::vec3 p2 = glm::vec3(min.x, min.y, max.z);
		glm::vec3 p3 = glm::vec3(max.x, min.y, max.z);

		// 4 points on back face 
		glm::vec3 p4 = glm::vec3(min.x, max.y, min.z);
		glm::vec3 p5 = glm::vec3(max.x, max.y, min.z);
		glm::vec3 p6 = glm::vec3(min.x, min.y, min.z);
		glm::vec3 p7 = glm::vec3(max.x, min.y, min.z);

		glm::vec2 t0 = glm::vec2(0, 0);
		glm::vec2 t1 = glm::vec2(1, 0);
		glm::vec2 t2 = glm::vec2(0, 1);
		glm::vec2 t3 = glm::vec2(1, 1);

		glm::vec4 topColor = color;
		glm::vec4 bottomColor = color;

		if (fakeLighting)
		{
			bottomColor *= 0.1;
			bottomColor.a = 1;
		}

		// front
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p0, t0, topColor,
			p1, t1, topColor,
			p3, t3, bottomColor,
			p2, t2, bottomColor);
		// top
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p4, t0, topColor,
			p5, t1, topColor,
			p1, t3, topColor,
			p0, t2, topColor);
		// left 
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p4, t0, topColor,
			p0, t1, topColor,
			p2, t3, bottomColor,
			p6, t2, bottomColor);
		// bottom
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p2, t0, bottomColor,
			p3, t1, bottomColor,
			p7, t3, bottomColor,
			p6, t2, bottomColor);
		// right 
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p1, t0, topColor,
			p5, t1, topColor,
			p7, t3, bottomColor,
			p3, t2, bottomColor);
		// back
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p5, t0, topColor,
			p4, t1, topColor,
			p6, t3, bottomColor,
			p7, t2, bottomColor);
	}


	void PushBitmap(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup,
		LoadedBitmap* bitmap,
		glm::vec4 color,
		glm::vec3 position,
		glm::vec3 halfDim, AlignmentMode hAlignment, AlignmentMode vAlignment)
	{
		glm::vec3 min = position;
		glm::vec3 max = position;
		switch (hAlignment)
		{
			case AlignmentMode::Centered:
				min.x -= halfDim.x;
				max.x += halfDim.x;
				break;
			case AlignmentMode::Left:
				max.x += halfDim.x * 2;
				break;
			case AlignmentMode::Right:
				min.x -= halfDim.x * 2;
				break;
			default:
				break;
		}

		switch (vAlignment)
		{
			case AlignmentMode::Centered:
				min.y -= halfDim.y;
				max.y += halfDim.y;
				break;
			case AlignmentMode::Top:
				min.y -= halfDim.y * 2;
				break;
			case AlignmentMode::Bottom:
				max.y += halfDim.y * 2;
				break;
			default:
				break;
		}

		glm::vec3 p0 = glm::vec3(min.x, max.y, max.z);
		glm::vec3 p1 = glm::vec3(max.x, max.y, max.z);
		glm::vec3 p2 = glm::vec3(min.x, min.y, max.z);
		glm::vec3 p3 = glm::vec3(max.x, min.y, max.z);

		glm::vec2 t0 = glm::vec2(0, 0);
		glm::vec2 t1 = glm::vec2(1, 0);
		glm::vec2 t2 = glm::vec2(0, 1);
		glm::vec2 t3 = glm::vec2(1, 1);

		glm::vec4 c0 = color;
		glm::vec4 c1 = color;
		glm::vec4 c2 = color;
		glm::vec4 c3 = color;


		// front
		PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p0, t0, c0,
			p1, t1, c1,
			p3, t3, c3,
			p2, t2, c2);
	}




	void PushLine(
		RenderSystem::GameRenderCommands* gameRenderCommands, 
		RenderSystem::RenderGroup* group, 
		LoadedBitmap* bitmap, 
		glm::vec4 color, glm::vec3 start, glm::vec3 end, float thickness)
	{
		if (end == start)
		{
			return;
		}
		glm::vec3 dir = glm::normalize(end - start);
		glm::vec3 supportUpAXIS = glm::vec3(0, 0, 1);
		glm::vec3 right = glm::cross(dir, supportUpAXIS);
		if (right == glm::vec3(0))
		{
			supportUpAXIS = glm::vec3(0, -1, 0);
			right = glm::cross(dir, supportUpAXIS);
		}

		glm::vec3 up = -glm::cross(dir, right);

		std::vector<glm::vec3> vertices(8);

		vertices[0] = start + thickness * (up - right);
		vertices[1] = start + thickness * (up + right);
		vertices[2] = start + thickness * (-up - right);
		vertices[3] = start + thickness * (-up + right);

		vertices[4] = end + thickness * (up - right);
		vertices[5] = end + thickness * (up + right);
		vertices[6] = end + thickness * (-up - right);
		vertices[7] = end + thickness * (-up + right);

		PushCube(gameRenderCommands, group, bitmap, vertices, color, false);
	}

	void PushLine(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* group,
		GameAssets* gameAssets,
		glm::vec4 color, glm::vec3 start, glm::vec3 end, float thickness)
	{
		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

		PushLine(gameRenderCommands, group, bitmap, color, start, end, thickness);
	}

	void PushDashedLine(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* group,
		GameAssets* gameAssets,
		glm::vec4 color, glm::vec3 start, glm::vec3 end, float thickness)
	{

		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

		float stepSize = 1;

		glm::vec3 dir = end - start;

		glm::vec3 normalizedDir = glm::normalize(dir);
		float totalDistSq = glm::distance(start, end);

		glm::vec3 curStart = start;


		while (totalDistSq > 0)
		{
			glm::vec3 segmentEnd = curStart + normalizedDir * stepSize;
			PushLine(gameRenderCommands, group, bitmap, color, curStart, segmentEnd, thickness);

			curStart = segmentEnd + normalizedDir * stepSize;

			totalDistSq -= 2 * stepSize;
		}
	}

	void PushLine3(
		RenderSystem::GameRenderCommands* gameRenderCommands, 
		RenderSystem::RenderGroup* group, 
		LoadedBitmap* bitmap, 
		glm::vec4 color, glm::vec3 start, glm::vec3 end, float thickness)
	{
		if (end == start)
		{
			return;
		}
		glm::vec3 dir = glm::normalize(end - start);
		glm::vec3 supportUpAXIS = glm::vec3(0, 1, 0);

		glm::vec3 right = glm::cross(dir, supportUpAXIS);
		if (right == glm::vec3(0))
		{
			supportUpAXIS = glm::vec3(0, 0, -1);
			right = glm::cross(dir, supportUpAXIS);
		}


		glm::vec3 up = -glm::cross(dir, right);

		//	std::cout << "right " << right << std::endl;
		//	std::cout << "up " << up << std::endl;

		std::vector<glm::vec3> vertices(8);

		vertices[0] = start + thickness * (up - right);
		vertices[1] = start + thickness * (up + right);
		vertices[2] = start + thickness * (-up - right);
		vertices[3] = start + thickness * (-up + right);

		vertices[4] = end + thickness * (up - right);
		vertices[5] = end + thickness * (up + right);
		vertices[6] = end + thickness * (-up - right);
		vertices[7] = end + thickness * (-up + right);

		GameRender::PushCube(gameRenderCommands, group, bitmap, vertices, color, false);
	}


	std::vector<Face> CreateCircleMesh(glm::vec3 center, float radius, float thickness)
	{
		// to limit the radius
		if (thickness > radius / 2)
			thickness = radius / 2;

		std::vector<Face> result;
		int step = 1;
		for (float i = 0; i < 360; i += step)
		{
			//phys
			float cos1 = cos(i * Math::DEGREE_TO_RADIAN);
			float sin1 = sin(i * Math::DEGREE_TO_RADIAN);

			float cos2 = cos((i + step) * Math::DEGREE_TO_RADIAN);
			float sin2 = sin((i + step) * Math::DEGREE_TO_RADIAN);

			// outer ring
			float wx = radius * cos1;
			float wz = radius * sin1;
			glm::vec3 simPos0 = glm::vec3(wx, 0, wz);

			wx = radius * cos2;
			wz = radius * sin2;
			glm::vec3 simPos1 = glm::vec3(wx, 0, wz);


			// inner ring
			float radius2 = radius - thickness;
			wx = radius2 * cos1;
			wz = radius2 * sin1;
			glm::vec3 simPos2 = glm::vec3(wx, 0, wz);

			wx = radius2 * cos2;
			wz = radius2 * sin2;
			glm::vec3 simPos3 = glm::vec3(wx, 0, wz);

			Face face;
			face.vertices = { simPos2, simPos3, simPos1, simPos0 };

			for (int i = 0; i < face.vertices.size(); i++)
			{
				face.vertices[i] += center;
			}

			result.push_back(face);
		}
		return result;
	}


	void RenderCircle(RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup,
		GameAssets* gameAssets,
		glm::vec4 color,
		glm::vec3 center,
		float radius,
		float thickness)
	{
		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);
		int step = 2;
		for (float i = 0; i < 360; i += step)
		{
			//phys
			float cos1 = cos(i * Math::DEGREE_TO_RADIAN);
			float sin1 = sin(i * Math::DEGREE_TO_RADIAN);

			float cos2 = cos((i + step) * Math::DEGREE_TO_RADIAN);
			float sin2 = sin((i + step) * Math::DEGREE_TO_RADIAN);

			// outer ring
			float wx = radius * cos1;
			float wy = radius * sin1;
			glm::vec3 simPos0 = center + glm::vec3(wx, wy, 0);

			wx = radius * cos2;
			wy = radius * sin2;
			glm::vec3 simPos1 = center + glm::vec3(wx, wy, 0);


			// inner ring
			float radius2 = radius - thickness;
			wx = radius2 * cos1;
			wy = radius2 * sin1;
			glm::vec3 simPos2 = center + glm::vec3(wx, wy, 0);

			wx = radius2 * cos2;
			wy = radius2 * sin2;
			glm::vec3 simPos3 = center + glm::vec3(wx, wy, 0);

			glm::vec3 vertices0[3] = { simPos1, simPos3, simPos2 };
			glm::vec3 vertices1[3] = { simPos1, simPos2, simPos0 };

			// PushTriangle(gameRenderCommands, renderGroup, bitmap, color, vertices0);
			// PushTriangle(gameRenderCommands, renderGroup, bitmap, color, vertices1);
			GameRender::PushQuad(gameRenderCommands, renderGroup, bitmap,
				simPos0,
				simPos2,
				simPos3,	// wasting one vertex here
				simPos1,
				color, false);
		}
	}


	void RenderPoint(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup,
		LoadedBitmap* bitmap,
		glm::vec4 color,
		glm::vec3 pos,
		float thickness)
	{
		// vertex
		glm::vec3 offset = glm::vec3(thickness, thickness, thickness);
		glm::vec3 min = pos - offset;
		glm::vec3 max = pos + offset;

		GameRender::PushCube(gameRenderCommands, renderGroup, bitmap, color, min, max, true);
	}


	void RenderPoint(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup,
		GameAssets* gameAssets,
		glm::vec4 color,
		glm::vec3 pos,
		float thickness)
	{
		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

		GameRender::RenderPoint(gameRenderCommands, renderGroup, bitmap, color, pos, thickness);
	}


	void RenderLine(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup,
		GameAssets* gameAssets,
		glm::vec4 color,
		glm::vec3 p0,
		glm::vec3 p1,
		float thickness)
	{
		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

		PushLine(gameRenderCommands, renderGroup, bitmap, color, p0, p1, thickness);
	}

	void RenderCoordinateSystem(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup,
		GameAssets* gameAssets)
	{
		float scale = 20;
		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

		glm::vec3 origin = glm::vec3(0);
		glm::vec3 dim = glm::vec3(scale, scale, scale);

		glm::vec3 xAxisEnd = origin + dim.x * glm::vec3(1, 0, 0);
		glm::vec3 yAxisEnd = origin + dim.y * glm::vec3(0, 1, 0);
		glm::vec3 zAxisEnd = origin + dim.z * glm::vec3(0, 0, 1);

		float cubeThickness = 0.5f;
		PushLine(gameRenderCommands, renderGroup, bitmap, COLOR_RED, origin, xAxisEnd, cubeThickness);
		PushLine(gameRenderCommands, renderGroup, bitmap, COLOR_GREEN, origin, yAxisEnd, cubeThickness);
		PushLine(gameRenderCommands, renderGroup, bitmap, COLOR_BLUE, origin, zAxisEnd, cubeThickness);
	}

	void DEBUGTextLine(const char* s,
		GameRenderState* gameRenderState,
		glm::vec3 position, float bitmapScale)
	{
		// how big do we want char to be displayed
		float DEBUG_CHAR_BITMAP_SCALE = bitmapScale;

		LoadedFont* debugLoadedFont = gameRenderState->debugLoadedFont;
		GameAssets* gameAssets = gameRenderState->gameAssets;

		int ascent = 0;
		int descent = 0;
		int lineGap = 0;
		stbtt_GetFontVMetrics(&gameRenderState->debugLoadedFont->fontInfo, &ascent, &descent, &lineGap);
		float scale = DEBUG_CHAR_BITMAP_SCALE * stbtt_ScaleForPixelHeight(&debugLoadedFont->fontInfo, FONT_SCALE);

		int lineGapBetweenNextBaseline = (ascent - descent + lineGap);
		int scaledLineGap = (int)(lineGapBetweenNextBaseline * scale);

		float xPos = position.x;
		int yBaselinePos = position.y;

		// This is essentially following the example from stb library
	//	for (int i = 0; i < size; i++)

		int i = 0;
		while (s[i] != '\0')
		{
			int advance, leftSideBearing;
			stbtt_GetCodepointHMetrics(&debugLoadedFont->fontInfo, s[i], &advance, &leftSideBearing);

			GlyphId glyphID = GetGlyph(gameAssets, debugLoadedFont, s[i]);
			LoadedGlyph* glyphBitmap = GetGlyph(gameAssets, glyphID);

			if (s[i] == '\n')
			{
				xPos = position.x;
				yBaselinePos -= scaledLineGap;
			}
			else
			{
				float height = DEBUG_CHAR_BITMAP_SCALE * glyphBitmap->bitmap.height;
				float width = glyphBitmap->bitmap.width / (float)glyphBitmap->bitmap.height * height;

				// i dont know why i only multiple scale here... but i'll investigate later
				float x = xPos + (float)(glyphBitmap->bitmapXYOffsets.x) * scale;
				float y = yBaselinePos - (float)(glyphBitmap->bitmapXYOffsets.y);

				// 0.2 so that its slightly above the z plane
				glm::vec3 leftTopPos = glm::vec3(x, y, 0.2);

				GameRender::PushBitmap(
					gameRenderState->gameRenderCommands,
					gameRenderState->renderGroup,
					&glyphBitmap->bitmap,
					GameRender::COLOR_WHITE,
					leftTopPos,
					glm::vec3(width / 2.0, height / 2.0, 0),
					GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Top);


				xPos += (advance * scale);
				xPos += scale * stbtt_GetCodepointKernAdvance(&debugLoadedFont->fontInfo, s[i], s[i + 1]);
			}
			i++;
		}

	}




	// right now, since the rendering pipeline only supports quads, im going to 
	// render triangles as quads and wasting one vertex
	void PushTriangleOutline(
		RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup,
		LoadedBitmap* bitmap,
		glm::vec4 color,
		glm::vec3 vertices[3],
		float thickness,
		bool fakeLighting = false)
	{
		const int numVertex = 3;

		for (int i = 0; i < numVertex; i++)
		{
			glm::vec3 p = vertices[i];
			glm::vec3 p1;
			if (i == numVertex - 1)
			{
				p1 = vertices[0];
			}
			else
			{
				p1 = vertices[i + 1];
			}

			RenderPoint(gameRenderCommands, renderGroup, bitmap, color, p, thickness);
			PushLine(gameRenderCommands, renderGroup, bitmap, color, p, p1, thickness);
		}
	}

};