#pragma once
/* ----------------------------------------------------
 MIT License

 Copyright (c) 2020 SAE Institute Switzerland AG

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 Author : Canas Simon
 Co-Author :
 Date : 13.10.2020
---------------------------------------------------------- */
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "engine/assert.h"
#include "utils/service_locator.h"

namespace neko
{
//-----------------------------------------------------------------------------
// LogType
//-----------------------------------------------------------------------------
/// To differentiate log messages
enum class LogType : std::uint8_t
{
	DEBUG_ = 1,    // For develop debug messages // Renamed due to an already used name
	WARNING,       // For mistakes that don't cause instability
	ERROR_,        // For non-critical errors // Renamed due to an already used name
	LENGTH
};

//-----------------------------------------------------------------------------
// LogCategory
//-----------------------------------------------------------------------------
/// \brief To sort log messages into different categories
enum class LogCategory : std::uint8_t
{
	NONE = 1,
	ENGINE,
	MATH,
	GRAPHICS,
	IO,
	SOUND,
	TOOL,
	LENGTH
};

//-----------------------------------------------------------------------------
// LogMessage
//-----------------------------------------------------------------------------
/// \brief Struct representing a log message with its type and category
struct LogMessage
{
	LogCategory category = LogCategory::NONE;
	LogType type         = LogType::DEBUG_;
	std::string log;

	explicit LogMessage(std::string log) : log(std::move(log)) { Generate(); }

	explicit LogMessage(const LogType& logType, std::string log) : type(logType), log(std::move(log))
	{
		Generate();
	}

	explicit LogMessage(const LogCategory& category, const LogType& logType, std::string log)
	   : category(category), type(logType), log(std::move(log))
	{
		Generate();
	}

	void Generate();
	void Display() const { (type != LogType::ERROR_ ? std::cout : std::cerr) << log; }
};

//-----------------------------------------------------------------------------
// LogManagerInterface
//-----------------------------------------------------------------------------
/// \brief Used for the service locator
class ILogger
{
protected:
	~ILogger() = default;

public:
	/// Generate a log message.
	/// \param logType the type of the log message
	/// \param log the log message
	virtual void Log(LogType logType, const std::string& log) = 0;

	/// Generate a log message.
	/// \param logType the type of the log message
	/// \param category the category of the log message
	/// \param log the log message
	virtual void Log(LogCategory category, LogType logType, const std::string& log) = 0;

    /// Clear the log history
    virtual void ClearLogs() = 0;

	/// Retrieves the log history
	virtual const std::vector<LogMessage>& GetLogs() = 0;
};

//-----------------------------------------------------------------------------
// NullLogManager
//-----------------------------------------------------------------------------
/// \brief Used for the service locator
class NullLogger final : public ILogger
{
public:
	void Log(LogType logType, const std::string& log) override
	{
		Log(LogCategory::NONE, logType, log);
	}

	void Log(LogCategory category, LogType logType, const std::string& log) override
	{
		if (!hasWarningBeenIssued)
		{
			const LogMessage warningMessage(
				LogType::WARNING, "LogManager is null! History will NOT be available");
			warningMessage.Display();
		}

		const LogMessage logMessage(category, logType, log);
		logMessage.Display();

		hasWarningBeenIssued = true;
	}

	const std::vector<LogMessage>& GetLogs() override
	{
		neko_assert(false, "Impossible to get log history from a null LogManager");
	}

	void ClearLogs() override
	{
		std::cerr << "Impossible to clear log history from a null LogManager\n";
		assert(false);
	}

private:
    bool hasWarningBeenIssued = false;
};

//-----------------------------------------------------------------------------
// LogManager
//-----------------------------------------------------------------------------
/// \brief Creates and stores log messages
class Logger final : public ILogger
{
protected:
	//-----------------------------------------------------------------------------
	// LogManagerStatus
	//-----------------------------------------------------------------------------
	/// \brief To get the status of the engine
	enum LoggerStatus : std::uint8_t
	{
		IS_RUNNING     = 1u << 0u,    // To check if the LogManager is running
		IS_EMPTY       = 1u << 1u,    // To check if the LogManager has tasks
		IS_LOG_WAITING = 1u << 2u,    // To check if the LogManager is waiting for a task
		IS_WRITING     = 1u << 3u     // To check if the LogManager is writing its output to a file
	};

public:
	Logger();
	~Logger();

	void LogLoop();
	void Wait();
	void Destroy();

	void Log(LogType logType, const std::string& log) override;

	void Log(LogCategory category, LogType logType, const std::string& log) override;

	const std::vector<LogMessage>& GetLogs() override { return logHistory_; }

	void ClearLogs() override;

	void WriteToFile();

private:
	std::atomic<std::uint8_t> status_;

	std::vector<LogMessage> logHistory_;

	std::unique_ptr<std::thread> logThread_;
	std::mutex logMutex_;
	std::vector<std::function<void()>> tasks_;
	std::condition_variable conditionVariable_;
};

//-----------------------------------------------------------------------------
// Service Locator definition
//-----------------------------------------------------------------------------
using LoggerLocator = Locator<ILogger, NullLogger>;

//-----------------------------------------------------------------------------
// Shorthands
//-----------------------------------------------------------------------------
/// Generate a debug type log message
void LogDebug(const std::string& msg);

/// Generate a debug type log message
void LogDebug(LogCategory category, const std::string& msg);

/// Generate a warning type log message
void LogWarning(const std::string& msg);

/// Generate a warning type log message
void LogWarning(LogCategory category, const std::string& msg);

/// Generate an error type log message
void LogError(const std::string& msg);

/// Generate an error type log message
void LogError(LogCategory category, const std::string& msg);

/// Retrieves the log history
const std::vector<LogMessage>& GetLogs();
}
