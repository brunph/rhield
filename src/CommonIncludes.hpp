#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <string>
#include <thread>
#include <string_view>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <functional>
#include <optional>
#include <chrono>
#include <any>
#include <format>
#include <ranges>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <expected>
#include <map>
#include <filesystem>
#include <fstream>
#include <random>
#include <numeric>
#include <set>

// dependencies

#define SPDLOG_ACTIVE_LEVEL 0
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
// not sure if i like this
#define TRACE(...)    SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__)
#define INFO(...)     SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)
#define WARN(...)     SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__)
#define ERR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__)
#define CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__)

#include <zasm/decoder/decoder.hpp>
#include <zasm/formatter/formatter.hpp>
#include <zasm/serialization/serializer.hpp>
#include <zasm/zasm.hpp>

#include <pepp/PELibrary.hpp>

// own includes

#include "util/Util.hpp"
#include "util/Random.hpp"