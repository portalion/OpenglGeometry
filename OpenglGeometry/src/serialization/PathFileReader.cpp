#include "PathFileReader.h"
#include <core/Log.h>
#include <fstream>
#include <optional>

namespace
{
	std::optional<float> ReadCoordinate(const std::string& line, char axis, std::size_t from)
	{
		std::size_t position = line.find(axis, from);
		if (position == std::string::npos)
			return std::nullopt;

		try
		{
			return std::stof(line.substr(position + 1));
		}
		catch (const std::exception&)
		{
			return std::nullopt;
		}
	}
}

PathComponent PathFileReader::ReadFile(std::string file)
{
	PathComponent result;
	result.m_Tool.m_Diameter = std::stoi(file.substr(file.find_last_of('.') + 2, 2));
	result.m_Tool.m_IsFlat = file[file.find_last_of('.') + 1] == 'f';

	std::ifstream stream(file);
	if (!stream.is_open())
	{
		Logger::Error("Could not open path file: {}", file);
		return result;
	}

	std::string line;
	std::size_t lineIndex = 0;
	while (std::getline(stream, line))
	{
		lineIndex++;
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			continue;

		std::size_t operationStart = line.find('G');
		if (line[0] != 'N' || operationStart == std::string::npos)
		{
			Logger::Warning("Path file line {}: invalid format '{}', skipping", lineIndex, line);
			continue;
		}

		std::string operation = line.substr(operationStart, 3);
		if (operation != "G01")
		{
			Logger::Warning("Path file line {}: unsupported operation '{}', skipping", lineIndex, operation);
			continue;
		}

		std::size_t coordinatesStart = operationStart + 3;
		auto x = ReadCoordinate(line, 'X', coordinatesStart);
		auto y = ReadCoordinate(line, 'Y', coordinatesStart);
		auto z = ReadCoordinate(line, 'Z', coordinatesStart);
		if (!x || !y || !z)
		{
			Logger::Warning("Path file line {}: missing coordinates '{}', skipping", lineIndex, line);
			continue;
		}

		result.m_Positions.emplace_back(*x, *y, *z, 1.f);
	}

	Logger::Info("Loaded path {} with {} positions", file, result.m_Positions.size());
	return result;
}
