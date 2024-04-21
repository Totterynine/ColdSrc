#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <span>
#include <unordered_map>

using String = std::string;
using StringView = std::string_view;

// Yes, std::vector is Array, deal with it.
template <class T>
using Array = std::vector<T>;

template <class T, size_t N>
using ConstArray = std::array<T, N>;

template <class T>
using Span = std::span<T>;

template <class K, class T>
using Dict = std::unordered_map<K, T>;