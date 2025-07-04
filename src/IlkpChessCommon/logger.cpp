#include "logger.h"

std::mutex& logMutex()
{
	static std::mutex logMutex;
	return logMutex;
}

inline void writeLogStandalone(const std::string& msg)
{
	std::unique_lock logLock(logMutex());
	std::cout
		<< formatLogTime(std::chrono::system_clock::now()) << " "
		<< msg
		<< (msg.back() == '\n' ? "" : "\n");
}

std::string formatLogTime(const std::chrono::system_clock::time_point& timePoint)
{
	const std::chrono::zoned_time localTimePoint{ std::chrono::current_zone(), timePoint };
	const std::string time = std::format("{:%F %T} ", localTimePoint);
	return time.substr(0, 23);
}

Logger::Logger(const std::string& name, bool enabled) noexcept :
	_name(name),
	_enabled(enabled)
{
}

Logger::Logger(const Logger& other) noexcept :
	_name(other._name),
	_enabled(other._enabled)
{
}

Logger::Logger(Logger&& other) noexcept :
	_name(std::move(other._name)),
	_enabled(std::move(other._enabled))
{
}

Logger& Logger::operator=(Logger&& other) noexcept
{
	if (this != &other)
	{
		_name = std::move(other._name);
		_enabled = std::move(other._enabled);
	}
	return *this;
}

Logger& Logger::operator=(const Logger& other) noexcept
{
	_name = other._name;
	_enabled = other._enabled;
	return *this;
}

void Logger::setName(const std::string& name)
{
	_name = name;
}

void Logger::setEnabled(bool enabled)
{
	_enabled = enabled;
}

void Logger::writeLog(const std::string& msg) const
{
	if (!_enabled)
		return;

	std::unique_lock logLock(logMutex());
	try
	{
		std::cout
			<< formatLogTime(std::chrono::system_clock::now()) << " "
			<< _name << ": "
			<< msg
			<< (msg.back() == '\n' ? "" : "\n");
	}
	catch (std::exception e)
	{
		std::cout << e.what();
	}
}
