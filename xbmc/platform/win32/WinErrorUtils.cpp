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

#define ERRTODESC(err, desc) \
  case err: \
    return desc

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

static const std::string GetDSoundErrorDescription(HRESULT hr)
{
#ifdef TARGET_WINDOWS_DESKTOP
  switch (hr)
  {
    ERRTODESC(DS_OK, "");
    ERRTODESC(DS_NO_VIRTUALIZATION,
              "The call succeeded, but we had to substitute the 3D algorithm");
    ERRTODESC(DSERR_ALLOCATED, "The call failed because resources (such as a priority level) were "
                               "already being used by another caller");
    ERRTODESC(DSERR_CONTROLUNAVAIL,
              "The control (vol, pan, etc.) requested by the caller is not available");
    ERRTODESC(DSERR_INVALIDPARAM, "DSERR_INVALIDPARAM");
    ERRTODESC(DSERR_INVALIDCALL, "This call is not valid for the current state of this object");
    ERRTODESC(DSERR_GENERIC, "DSERR_GENERIC");
    ERRTODESC(DSERR_PRIOLEVELNEEDED,
              "The caller does not have the priority level required for the function to succeed");
    ERRTODESC(DSERR_OUTOFMEMORY, "Not enough free memory is available to complete the operation");
    ERRTODESC(DSERR_BADFORMAT, "The specified WAVE format is not supported");
    ERRTODESC(DSERR_UNSUPPORTED, "DSERR_UNSUPPORTED");
    ERRTODESC(DSERR_NODRIVER, "No sound driver is available for use");
    ERRTODESC(DSERR_ALREADYINITIALIZED, "This object is already initialized");
    ERRTODESC(DSERR_NOAGGREGATION, "DSERR_NOAGGREGATION");
    ERRTODESC(DSERR_BUFFERLOST, "The buffer memory has been lost, and must be restored");
    ERRTODESC(DSERR_OTHERAPPHASPRIO,
              "Another app has a higher priority level, preventing this call from succeeding");
    ERRTODESC(DSERR_UNINITIALIZED, "This object has not been initialized");
    ERRTODESC(DSERR_NOINTERFACE, "DSERR_NOINTERFACE");
    ERRTODESC(DSERR_ACCESSDENIED, "DSERR_ACCESSDENIED");
    ERRTODESC(DSERR_BUFFERTOOSMALL,
              "Tried to create a DSBCAPS_CTRLFX buffer shorter than DSBSIZE_FX_MIN milliseconds");
    ERRTODESC(DSERR_DS8_REQUIRED,
              "Attempt to use DirectSound 8 functionality on an older DirectSound object");
    ERRTODESC(DSERR_SENDLOOP, "A circular loop of send effects was detected");
    ERRTODESC(DSERR_BADSENDBUFFERGUID,
              "The GUID specified in an audiopath file does not match a valid MIXIN buffer");
    ERRTODESC(DSERR_OBJECTNOTFOUND,
              "The object requested was not found (numerically equal to DMUS_E_NOT_FOUND)");
    ERRTODESC(DSERR_FXUNAVAILABLE, "Requested effects are not available");
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

  std::string desc{GetDSoundErrorDescription(hr)};
  if (!desc.length())
  {
    WCHAR buff[2048];
    DXGetErrorDescriptionW(hr, buff, 2048);
    desc = FromW(buff);
  }

  return std::format("0x{:X} {} ({})", static_cast<uint32_t>(hr), code, desc);
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

