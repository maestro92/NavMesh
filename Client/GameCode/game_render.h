#pragma once

#include "../PlatformShared/platform_shared.h"


namespace GameRender
{
	glm::vec4 COLOR_WHITE = glm::vec4(1, 1, 1, 1);
	glm::vec4 COLOR_RED = glm::vec4(1, 0, 0, 1);
	glm::vec4 COLOR_GREEN = glm::vec4(0, 1, 0, 1);
	glm::vec4 COLOR_BLUE = glm::vec4(0, 0, 1, 1);
	glm::vec4 COLOR_YELLOW = glm::vec4(1, 1, 0, 1);
	glm::vec4 COLOR_TEAL = glm::vec4(0, 1, 1, 1);
	glm::vec4 COLOR_ORANGE = glm::vec4(1, 0.5, 1, 1);

	enum AlignmentMode
	{
		Centered,
		Left,
		Right,
		Top,
		Bottom
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
		std::vector<glm::vec3>& vertices, 
		bool fakeLighting = false)
	{
		assert(vertices.size() == 3);

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
		glm::vec3 supportUpAXIS = glm::vec3(0, 1, 0);
		glm::vec3 right = glm::cross(dir, supportUpAXIS);
		if (right == glm::vec3(0))
		{
			supportUpAXIS = glm::vec3(0, 0, -1);
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

		// vertex
		glm::vec3 offset = glm::vec3(thickness, thickness, thickness);
		glm::vec3 min = pos - offset;
		glm::vec3 max = pos + offset;

		GameRender::PushCube(gameRenderCommands, renderGroup, bitmap, color, min, max, true);
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

		float cubeThickness = 0.5f;
		PushLine(gameRenderCommands, renderGroup, bitmap, COLOR_RED, p0, p1, cubeThickness);
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
};