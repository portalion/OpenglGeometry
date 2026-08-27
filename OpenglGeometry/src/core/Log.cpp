#include "Log.h"

#include <chrono>
#include <mutex>

namespace
{
	std::mutex g_Mutex;
	std::deque<Logger::Entry> g_Entries;
	std::size_t g_Capacity = 500;
	std::uint64_t g_Serial = 0;
	std::uint64_t g_Revision = 0;

	double Now()
	{
		using namespace std::chrono;
		static const steady_clock::time_point start = steady_clock::now();
		return duration<double>(steady_clock::now() - start).count();
	}
}

void Logger::Push(Level level, std::string message)
{
	std::lock_guard lock(g_Mutex);

	if (!g_Entries.empty())
	{
		Entry& last = g_Entries.back();
		if (last.level == level && last.message == message)
		{
			last.repeat++;
			last.time = Now();
			g_Revision++;
			return;
		}
	}

	Entry entry;
	entry.level = level;
	entry.message = std::move(message);
	entry.time = Now();
	entry.serial = ++g_Serial;

	g_Entries.push_back(std::move(entry));

	while (g_Entries.size() > g_Capacity)
	{
		g_Entries.pop_front();
	}

	g_Revision++;
}

const std::deque<Logger::Entry>& Logger::Entries()
{
	return g_Entries;
}

std::uint64_t Logger::Revision()
{
	return g_Revision;
}

void Logger::Clear()
{
	std::lock_guard lock(g_Mutex);
	g_Entries.clear();
	g_Revision++;
}

void Logger::SetCapacity(std::size_t capacity)
{
	std::lock_guard lock(g_Mutex);
	g_Capacity = capacity == 0 ? 1 : capacity;
	while (g_Entries.size() > g_Capacity)
	{
		g_Entries.pop_front();
	}
	g_Revision++;
}

const char* Logger::ToString(Level level)
{
	switch (level)
	{
	case Level::Trace:   return "trace";
	case Level::Info:    return "info";
	case Level::Warning: return "warning";
	case Level::Error:   return "error";
	default:             return "info";
	}
}
