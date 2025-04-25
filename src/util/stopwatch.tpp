#pragma once

#include "stopwatch.h"

template <class period>
Stopwatch<period>::Stopwatch(const bool start)
    : StartTime(std::chrono::system_clock::now()),
      LastUpdateTime(StartTime),
      Paused(!start) {}

template <class period>
void Stopwatch<period>::Update() {
  if (Paused) return;
  LastUpdateTime = std::chrono::system_clock::now();
}

template <class period>
void Stopwatch<period>::Set(const std::chrono::duration<size_t, period> time,
                            const bool pause) {
  LastUpdateTime = std::chrono::system_clock::now();
  StartTime = LastUpdateTime - time;

  Paused |= pause;  // Only pause; don't unpause
}

template <class period>
void Stopwatch<period>::Play() {
  if (!Paused) return;

  LastUpdateTime = std::chrono::system_clock::now();
  Paused = false;
};

template <class period>
void Stopwatch<period>::Pause() {
  Update();
  Paused = true;
}

template <class period>
std::chrono::duration<size_t, period> Stopwatch<period>::GetRuntime() const {
  return std::chrono::duration_cast<std::chrono::duration<size_t, period>>(
      LastUpdateTime - StartTime);
}
