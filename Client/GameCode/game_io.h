#pragma once

#include "..\PlatformShared\platform_shared.h"
#include <fstream>

#include "json_spirit\json_spirit.h"
#include "json_spirit\json_spirit_reader_template.h"
#include "json_spirit\json_spirit_writer_template.h"


#ifndef JSON_SPIRIT_MVALUE_ENABLED
#error Please define JSON_SPIRIT_MVALUE_ENABLED for the mValue type to be enabled 
#endif

using namespace std;
using namespace json_spirit;

namespace GameIO
{
	mValue ReadJsonFileToMap(const char* filename)
	{
		ifstream is(filename);

		mValue content;
		try
		{
			read_or_throw(is, content);
		}
		catch (Error_position errorPosition)
		{
			cout << "	error reading " << filename << endl;
			cout << "	line " << errorPosition.line_ << endl;
			cout << "	column " << errorPosition.column_ << endl;
			cout << "	" << errorPosition.reason_ << endl;
			while (true)
			{

			}
			exit(1);
		}
		return content;
	}

	Value ReadJsonFileToVector(char* filename)
	{
		ifstream is(filename);

		Value content;
		try
		{
			read_or_throw(is, content);
		}
		catch (Error_position errorPosition)
		{
			cout << "	error reading " << filename << endl;
			cout << "	line " << errorPosition.line_ << endl;
			cout << "	column " << errorPosition.column_ << endl;
			cout << "	" << errorPosition.reason_ << endl;
			while (true)
			{

			}
			exit(1);
		}
		return content;
	}

	const mValue& FindValue(const mObject& obj, const std::string& name)
	{
		mObject::const_iterator it = obj.find(name);

		if (it == obj.end() || it->first != name)
		{
			cout << "error findingValue " << name << endl;
			//		assert(it != obj.end());
			//		assert(it->first == name);
			while (true)
			{

			}
			exit(1);
		}

		return it->second;
	}

	glm::vec2 FindVec2(const mObject& obj, const std::string& name)
	{
		const mObject vec3Obj = FindValue(obj, name).get_obj();

		float x = FindValue(vec3Obj, "x").get_real();
		float y = FindValue(vec3Obj, "y").get_real();

		glm::vec2 vec2Value(x, y);

		return vec2Value;
	}

	glm::vec3 FindVec3(const mObject& obj, const std::string& name)
	{
		const mObject vec3Obj = FindValue(obj, name).get_obj();

		float x = FindValue(vec3Obj, "x").get_real();
		float y = FindValue(vec3Obj, "y").get_real();
		float z = FindValue(vec3Obj, "z").get_real();

		glm::vec3 vec3Value(x, y, z);

		return vec3Value;
	}

	Object SerializePoint(glm::vec2 p)
	{
		Object pointObj;

		pointObj.push_back(Pair("x", p.x));
		pointObj.push_back(Pair("y", p.y));

		return pointObj;
	}

	Object SerializePoint3D(glm::vec3 p)
	{
		Object pointObj;

		pointObj.push_back(Pair("x", p.x));
		pointObj.push_back(Pair("y", p.y));
		pointObj.push_back(Pair("z", p.z));
		return pointObj;
	}

	glm::vec2 DeserializePoint2D(const mObject& obj)
	{
		glm::vec2 point;
		point.x = GameIO::FindValue(obj, "x").get_real();
		point.y = GameIO::FindValue(obj, "y").get_real();
		return point;
	}

	glm::vec3 DeserializePoint3D(const mObject& obj)
	{
		glm::vec3 point;
		point.x = GameIO::FindValue(obj, "x").get_real();
		point.y = GameIO::FindValue(obj, "y").get_real();
		point.z = GameIO::FindValue(obj, "z").get_real();
		return point;
	}

	/*
	mValue ReadJsonFileToMap(const char* filename);
	Value ReadJsonFileToVector(char* filename);
	const mValue& FindValue(const mObject& obj, const std::string& name);
	glm::vec2 FindVec2(const mObject& obj, const std::string& name);
	glm::vec3 FindVec3(const mObject& obj, const std::string& name);

	Object SerializePoint(glm::vec2 p);
	Object SerializePoint3D(glm::vec3 p);
	glm::vec2 DeserializePoint2D(const mObject& obj);
	glm::vec3 DeserializePoint3D(const mObject& obj);
	*/
}