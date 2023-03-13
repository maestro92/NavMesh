#pragma once

#include "..\PlatformShared\platform_shared.h"
#include <fstream>

#include "..\NavMesh\include\json_spirit\json_spirit.h"
#include "..\NavMesh\include\json_spirit\json_spirit_reader_template.h"
#include "..\NavMesh\include\json_spirit\json_spirit_writer_template.h"


#ifndef JSON_SPIRIT_MVALUE_ENABLED
#error Please define JSON_SPIRIT_MVALUE_ENABLED for the mValue type to be enabled 
#endif


using namespace std;
using namespace json_spirit;

namespace GameIO
{
	mValue readJsonFileToMap(char* filename);
	Value readJsonFileToVector(char* filename);
	const mValue& findValue(const mObject& obj, const std::string& name);
	glm::vec3 findVec3(const mObject& obj, const std::string& name);
	glm::vec2 findVec2(const mObject& obj, const std::string& name);
}