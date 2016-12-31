#include "macro.h"

void initConsoleLog() {
  static auto console = spdlog::stdout_color_mt("console");
  console->set_level(spdlog::level::debug);
}
