/*
 *  Copyright (C) 2025 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <string>
#include <string_view>

namespace
{
constexpr unsigned long StringToLongCode(std::string_view a)
{
  const int len = a.length();

  if (len == 0 || len > 4)
    return -1;

  return static_cast<unsigned long>(len >= 4 ? a[len - 4] : 0) << 24 |
         static_cast<unsigned long>(len >= 3 ? a[len - 3] : 0) << 16 |
         static_cast<unsigned long>(len >= 2 ? a[len - 2] : 0) << 8 |
         static_cast<unsigned long>(len >= 1 ? a[len - 1] : 0);
}

  /*!
   * \brief Converts a language code given as a long, see #MAKECODE(a, b, c, d)
   *        to its string representation.
   * \param[in] code The language code given as a long, see #MAKECODE(a, b, c, d).
   * \return The string representation of the given language code code.
   */
std::string LongCodeToString(unsigned long code)
{
  std::string ret;
  for (unsigned int j = 0; j < 4; j++)
  {
    char c = (char)code & 0xFF;
    if (c == '\0')
      break;

    ret.insert(0, 1, c);
    code >>= 8;
  }
  return ret;
}
} // namespace

struct LCENTRY
{
  unsigned long code;
  std::string_view name;
};

/*!
 * \brief Returns an array of ISO 639 codes sorted by code
 * \param codes array to sort
 * \return sorted array
 */
template<typename T>
constexpr auto CreateIso639ByCode(T codes)
{
  std::ranges::sort(codes, {}, &LCENTRY::code);
  return codes;
}

/*!
 * \brief Returns an array of ISO 639 codes sorted by name (case-insensitive)
 * \param codes array to sort
 * \return sorted array
 */
template<typename T>
constexpr auto CreateIso639ByName(T codes)
{
  //! @todo create the array with lower-cased names to avoid case-insentive comparison by users later.
  std::ranges::sort(
      codes, [](std::string_view a, std::string_view b)
      { return StringUtils::CompareNoCase(a, b, 0) < 0; }, &LCENTRY::name);
  return codes;
}

template<typename T, std::size_t... Ns>
constexpr auto concat_impl(const std::array<T, Ns>&... arrays)
{
  constexpr std::size_t total_size = (Ns + ...); // Sum of sizes
  std::array<T, total_size> result{};
  std::size_t current_index = 0;
  (
      [&](const auto& arr)
      {
        for (const T& val : arr)
        {
          result[current_index++] = val;
        }
      }(arrays),
      ...); // Apply lambda to each array in the pack
  return result;
}

/*!
 * \brief Concatenate two arrays containing the same type of elements
 * \param a1 first array
 * \param a2 second array
 * \return concatenated array
 */
template<typename T, std::size_t N1, std::size_t N2>
constexpr auto concat(const std::array<T, N1>& a1, const std::array<T, N2>& a2)
{
  return concat_impl(a1, a2);
}
