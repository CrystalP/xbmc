/*
 *  Copyright (C) 2024 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "WinErrorUtils.h"

#include "CharsetConverter.h"
#include "dxerr.h"

#include <Audioclient.h>
#ifdef TARGET_WINDOWS_DESKTOP 
#include <dsound.h>
#endif
#include <format>
#include <string>
#include <xaudio2.h>

using namespace KODI::PLATFORM::WINDOWS;

#define ERRTOSTR(err) \
  case err: \
    return #err

namespace
{
static const std::string GetDSoundErrorString(HRESULT hr)
{
#ifdef TARGET_WINDOWS_DESKTOP
  switch (hr)
  {
    ERRTOSTR(DS_OK);
    ERRTOSTR(DS_NO_VIRTUALIZATION);
    ERRTOSTR(DSERR_ALLOCATED);
    ERRTOSTR(DSERR_CONTROLUNAVAIL);
    ERRTOSTR(DSERR_INVALIDPARAM);
    ERRTOSTR(DSERR_INVALIDCALL);
    ERRTOSTR(DSERR_GENERIC);
    ERRTOSTR(DSERR_PRIOLEVELNEEDED);
    ERRTOSTR(DSERR_OUTOFMEMORY);
    ERRTOSTR(DSERR_BADFORMAT);
    ERRTOSTR(DSERR_UNSUPPORTED);
    ERRTOSTR(DSERR_NODRIVER);
    ERRTOSTR(DSERR_ALREADYINITIALIZED);
    ERRTOSTR(DSERR_NOAGGREGATION);
    ERRTOSTR(DSERR_BUFFERLOST);
    ERRTOSTR(DSERR_OTHERAPPHASPRIO);
    ERRTOSTR(DSERR_UNINITIALIZED);
    ERRTOSTR(DSERR_NOINTERFACE);
    ERRTOSTR(DSERR_ACCESSDENIED);
    ERRTOSTR(DSERR_BUFFERTOOSMALL);
    ERRTOSTR(DSERR_DS8_REQUIRED);
    ERRTOSTR(DSERR_SENDLOOP);
    ERRTOSTR(DSERR_BADSENDBUFFERGUID);
    ERRTOSTR(DSERR_OBJECTNOTFOUND);
    ERRTOSTR(DSERR_FXUNAVAILABLE);
    default:
      break;
  }
#endif
  return {};
}

static const std::string GetWasapiErrorString(HRESULT hr)
{
  switch (hr)
  {
    ERRTOSTR(AUDCLNT_E_NOT_INITIALIZED);
    ERRTOSTR(AUDCLNT_E_ALREADY_INITIALIZED);
    ERRTOSTR(AUDCLNT_E_WRONG_ENDPOINT_TYPE);
    ERRTOSTR(AUDCLNT_E_DEVICE_INVALIDATED);
    ERRTOSTR(AUDCLNT_E_NOT_STOPPED);
    ERRTOSTR(AUDCLNT_E_BUFFER_TOO_LARGE);
    ERRTOSTR(AUDCLNT_E_OUT_OF_ORDER);
    ERRTOSTR(AUDCLNT_E_UNSUPPORTED_FORMAT);
    ERRTOSTR(AUDCLNT_E_INVALID_SIZE);
    ERRTOSTR(AUDCLNT_E_DEVICE_IN_USE);
    ERRTOSTR(AUDCLNT_E_BUFFER_OPERATION_PENDING);
    ERRTOSTR(AUDCLNT_E_THREAD_NOT_REGISTERED);
    ERRTOSTR(AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED);
    ERRTOSTR(AUDCLNT_E_ENDPOINT_CREATE_FAILED);
    ERRTOSTR(AUDCLNT_E_SERVICE_NOT_RUNNING);
    ERRTOSTR(AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED);
    ERRTOSTR(AUDCLNT_E_EXCLUSIVE_MODE_ONLY);
    ERRTOSTR(AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL);
    ERRTOSTR(AUDCLNT_E_EVENTHANDLE_NOT_SET);
    ERRTOSTR(AUDCLNT_E_INCORRECT_BUFFER_SIZE);
    ERRTOSTR(AUDCLNT_E_BUFFER_SIZE_ERROR);
    ERRTOSTR(AUDCLNT_E_CPUUSAGE_EXCEEDED);
    ERRTOSTR(AUDCLNT_E_BUFFER_ERROR);
    ERRTOSTR(AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED);
    ERRTOSTR(AUDCLNT_E_INVALID_DEVICE_PERIOD);
    default:
      break;
  }
  return {};
}

static const std::string GetXAudio2ErrorString(HRESULT hr)
{
  switch (hr)
  {
    ERRTOSTR(XAUDIO2_E_INVALID_CALL);
    ERRTOSTR(XAUDIO2_E_XMA_DECODER_ERROR);
    ERRTOSTR(XAUDIO2_E_XAPO_CREATION_FAILED);
    ERRTOSTR(XAUDIO2_E_DEVICE_INVALIDATED);
    default:
      break;
  }
  return {};
}
} // namespace

const std::string CWinError::FormatDSoundError(HRESULT hr)
{
  std::string code{GetDSoundErrorString(hr)};
  if (!code.length())
    code = FromW(DXGetErrorStringW(hr));

  return std::format("0x{:X} {}", static_cast<uint32_t>(hr), code);
}

const std::string CWinError::FormatHRESULT(HRESULT hr)
{
  std::string code = FromW(DXGetErrorStringW(hr));
  WCHAR buff[2048];
  DXGetErrorDescriptionW(hr, buff, 2048);

  return std::format("HRESULT 0x{:X} Code: {} ({})", static_cast<uint32_t>(hr), code, FromW(buff));
}

const std::string CWinError::FormatWasapiError(HRESULT hr)
{
  std::string code{GetWasapiErrorString(hr)};
  if (!code.length())
    code = FromW(DXGetErrorStringW(hr));

  return std::format("HRESULT 0x{:X} Code: {}", static_cast<uint32_t>(hr), code);
}

const std::string CWinError::FormatXAudio2Error(HRESULT hr)
{
  std::string code{GetXAudio2ErrorString(hr)};
  if (!code.length())
    code = FromW(DXGetErrorStringW(hr));

  return std::format("HRESULT 0x{:X} Code: {}", static_cast<uint32_t>(hr), code);
}
