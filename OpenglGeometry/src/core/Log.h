#pragma once
#include <cstdint>
#include <deque>
#include <format>
#include <string>
#include <utility>

namespace Logger
{
	enum class Level
	{
		Trace,
		Info,
		Warning,
		Error
	};

	struct Entry
	{
		Level level = Level::Info;
		std::string message;
		double time = 0.0;
		std::uint64_t serial = 0;
		std::uint32_t repeat = 1;
	};

	void Push(Level level, std::string message);
	const std::deque<Entry>& Entries();
	std::uint64_t Revision();
	void Clear();
	void SetCapacity(std::size_t capacity);

	const char* ToString(Level level);

	template<typename... Args>
	void Trace(std::format_string<Args...> fmt, Args&&... args)
	{
		Push(Level::Trace, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void Info(std::format_string<Args...> fmt, Args&&... args)
	{
		Push(Level::Info, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void Warning(std::format_string<Args...> fmt, Args&&... args)
	{
		Push(Level::Warning, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void Error(std::format_string<Args...> fmt, Args&&... args)
	{
		Push(Level::Error, std::format(fmt, std::forward<Args>(args)...));
	}
}
