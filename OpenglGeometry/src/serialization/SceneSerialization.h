#pragma once
#include <string>

class Scene;

namespace Serialization
{
	struct Result
	{
		bool ok = false;
		std::string message;
	};

	Result SaveScene(Scene& scene, const std::string& path);
	Result LoadScene(Scene& scene, const std::string& path);
}
