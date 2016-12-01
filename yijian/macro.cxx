#include "macro.h"

void initConsoleLog() {
  static auto console = spdlog::stdout_logger_mt("console");
  console->set_level(spdlog::level::trace);
}
