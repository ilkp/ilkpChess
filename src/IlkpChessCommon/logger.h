#pragma once
#include <string>
#include <iostream>
#include <mutex>
#include <format>
#include <chrono>

inline std::mutex& logMutex()
{
	static std::mutex logMutex;
	return logMutex;
}

void writeLogStandalone(const std::string& msg);

std::string formatLogTime(const std::chrono::system_clock::time_point& timePoint);

class Logger
{
public:
	Logger(const std::string& name, bool enabled = true);
	Logger(Logger&& other) noexcept;
	Logger& operator=(Logger&& other) noexcept;

	void setName(const std::string& name);
	void setEnabled(bool enabled);
	void writeLog(const std::string& msg) const;

private:
	std::string _name;
	bool _enabled;
};