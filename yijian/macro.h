#ifndef MACRO_H_YIJIAN
#define MACRO_H_YIJIAN

#include <stdexcept>
#include <string>
#include <stdio.h>
#include <assert.h>
#include "spdlog/spdlog.h"
#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

namespace yijian {

class noncopyable {
public:
	noncopyable() = default;
	noncopyable(const noncopyable &) = delete;
	noncopyable& operator=(const noncopyable&) = delete;
	noncopyable(noncopyable &&) = default;
	noncopyable& operator=(noncopyable&&) = default;
	virtual ~noncopyable() = default;
};

}

// c++11 thread-safe
static std::shared_ptr<spdlog::logger>& getConsole() {
  auto static console = spdlog::stdout_logger_mt("console", true);
  console->set_level(spdlog::level::critical);
  return console;
}

#ifdef YILOG_ON
#define YILOG_STR_H(x) #x
#define YILOG_STR_HELPER(x) YILOG_STR_H(x)
#define YILOG_TRACE(...) getConsole()->trace("[" __FILE__ " line #" YILOG_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#define YILOG_DEBUG(...) getConsole()->debug(__VA_ARGS__)
#define YILOG_INFO(...) getConsole()->info(__VA_ARGS__)
#define YILOG_WARN(...) getConsole()->warn(__VA_ARGS__)
#define YILOG_ERROR(...) getConsole()->error(__VA_ARGS__)
#define YILOG_CRITICAL(...) getConsole()->critical(__VA_ARGS__)
#else
#define YILOG_TRACE(...)
#define YILOG_DEBUG(...)
#define YILOG_INFO(...)
#define YILOG_WARN(...)
#define YILOG_ERROR(...)
#define YILOG_CRITICAL(...)
#endif

#define MQTT_BUFFER_SIZE (1024 * 2)
#define MQTT_SESSION_COUNT (20 * 1000)

#endif


