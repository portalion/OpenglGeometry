#pragma once
#include <string>
#include <scene/Components.h>


class PathFileReader
{
public:
	PathComponent ReadFile(std::string file);
};