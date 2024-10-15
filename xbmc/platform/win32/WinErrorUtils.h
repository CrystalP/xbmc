/*
 *  Copyright (C) 2024 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <string>

class CWinError
{
private:
  CWinError() = default;
  ~CWinError() = default;

public:

  static const std::string FormatDSoundError(HRESULT hr);
  static const std::string FormatHRESULT(HRESULT hr);
  static const std::string FormatWasapiError(HRESULT hr);
  static const std::string FormatXAudio2Error(HRESULT hr);
};
