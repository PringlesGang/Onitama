#pragma once

#include "stopwatch.h"

template <class period>
Stopwatch<period>::Stopwatch(const bool start)
    : LastUpdateTime(std::chrono::system_clock::now()),
      Runtime(std::chrono::duration<size_t, period>::zero()),
      Paused(!start) {}

template <class period>
void Stopwatch<period>::Update() {
  if (Paused) return;

  const std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();

  Runtime += std::chrono::duration_cast<std::chrono::duration<size_t, period>>(
      std::chrono::time_point<std::chrono::system_clock>(now - LastUpdateTime)
          .time_since_epoch());

  LastUpdateTime = now;
}

template <class period>
void Stopwatch<period>::Set(const std::chrono::duration<size_t, period> time,
                            const bool pause) {
  Runtime = time;

  LastUpdateTime = std::chrono::system_clock::now();
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
