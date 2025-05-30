/**
 * @file timer.hpp
 * @brief
 * @copyright Copyright (c) 2024
 */

#pragma once

#include <chrono>

class Timer {
 public:
  /**
   * @brief Starts the timer
   */
  void start() { _start = std::chrono::steady_clock::now(); }

  /**
   * @brief Stops the timer
   */
  void stop() { _end = std::chrono::steady_clock::now(); }

  /**
   * @brief Returns the start time
   * @return std::chrono::steady_clock::time_point object
   */
  auto getStart() { return _start; }

  /**
   * @brief Returns the end time
   * @return std::chrono::steady_clock::time_point object
   */
  auto getStop() { return _end; }

  /**
   * @brief Returns the elapsed time in seconds
   * @return std::chrono::duration object
   */
  std::chrono::duration<double> elapsed() const { return (_end - _start); }

 private:
  /// @brief Start time
  std::chrono::steady_clock::time_point _start;

  /// @brief End time
  std::chrono::steady_clock::time_point _end;
};
