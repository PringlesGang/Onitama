#pragma once

#include <chrono>

template <class period = std::ratio<1>>
class Stopwatch {
 public:
  Stopwatch(bool start = false);

  void Update();

  void Set(std::chrono::duration<size_t, period> time, bool pause = false);
  void Reset(bool pause = false) {
    Set(std::chrono::duration<size_t, period>::zero(), pause);
  }

  void Play();
  void Pause();

  std::chrono::duration<size_t, period> GetRuntime() const { return Runtime; }
  size_t GetRuntimeCount() const { return Runtime.count(); }

 private:
  std::chrono::time_point<std::chrono::system_clock> LastUpdateTime;
  std::chrono::duration<size_t, period> Runtime;

  bool Paused;
};

#include "stopwatch.tpp"
