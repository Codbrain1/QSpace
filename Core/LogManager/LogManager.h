#pragma once
namespace QSpace::Core {
class LogManager {
  public:
    static void setup();
    static void setVerbose(bool enabled);
};
} // namespace QSpace::Core