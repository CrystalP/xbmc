/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "VideoSyncD3D.h"

#include "Utils/MathUtils.h"
#include "Utils/TimeUtils.h"
#include "cores/VideoPlayer/VideoReferenceClock.h"
#include "rendering/dx/DeviceResources.h"
#include "rendering/dx/RenderContext.h"
#include "utils/StringUtils.h"
#include "utils/XTimeUtils.h"
#include "utils/log.h"
#include "windowing/GraphicContext.h"

#include <mutex>

#ifdef TARGET_WINDOWS_STORE
#include <winrt/Windows.Graphics.Display.Core.h>
#endif

using namespace std::chrono_literals;

void CVideoSyncD3D::OnLostDisplay()
{
  if (!m_displayLost)
  {
    m_displayLost = true;
    m_lostEvent.Wait();
  }
}

void CVideoSyncD3D::OnResetDisplay()
{
  m_displayReset = true;
}

void CVideoSyncD3D::RefreshChanged()
{
  m_displayReset = true;
}

bool CVideoSyncD3D::Setup()
{
  CLog::Log(LOGDEBUG, "CVideoSyncD3D: Setting up Direct3d");
  std::unique_lock<CCriticalSection> lock(CServiceBroker::GetWinSystem()->GetGfxContext());
  DX::Windowing()->Register(this);
  m_displayLost = false;
  m_displayReset = false;
  m_lostEvent.Reset();

  // we need a high priority thread to get accurate timing
  if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
    CLog::Log(LOGDEBUG, "CVideoSyncD3D: SetThreadPriority failed");

  DX::DeviceResources::Get()->GetOutput(m_output.ReleaseAndGetAddressOf());
  m_output->GetDesc(&m_outputDesc);

  return true;
}

void CVideoSyncD3D::Run(CEvent& stopEvent)
{
  int64_t Now;
  int64_t LastVBlankTime;
  int NrVBlanks;
  double VBlankTime;
  const int64_t systemFrequency = CurrentHostFrequency();
  Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
  
  CreateDXGIFactory1(IID_PPV_ARGS(factory.ReleaseAndGetAddressOf()));

  // init the vblanktime
  Now = CurrentHostCounter();
  LastVBlankTime = Now;

  while (!stopEvent.Signaled() && !m_displayLost && !m_displayReset)
  {
    // sleep until vblank
    m_output->WaitForVBlank();

    DX::DeviceResources::Get()->GetOutput(m_output.ReleaseAndGetAddressOf());

    m_output->GetDesc(&m_outputDesc);

    // calculate how many vblanks happened
    Now = CurrentHostCounter();
    VBlankTime = (double)(Now - LastVBlankTime) / (double)systemFrequency;
    NrVBlanks = MathUtils::round_int(VBlankTime * m_fps);

    // update the vblank timestamp, update the clock and send a signal that we got a vblank
    m_refClock->UpdateClock(NrVBlanks, Now);

    // save the timestamp of this vblank so we can calculate how many vblanks happened next time
    LastVBlankTime = Now;

    if (!factory->IsCurrent())
    {
      float fps = m_fps;
      if (fps != GetFps())
        break;

      CreateDXGIFactory1(IID_PPV_ARGS(factory.ReleaseAndGetAddressOf()));
    }

    // Some of the code above can take a non-negligible amount of time
    Now = CurrentHostCounter();

    // because we had a vblank, sleep until half the refreshrate period because i think WaitForVBlank block any rendering stuf
    // without sleeping we have freeze rendering
    int SleepTime = (int)((LastVBlankTime + (systemFrequency / MathUtils::round_int(m_fps) / 2) - Now) * 1000 / systemFrequency);
    if (SleepTime > 50)
      SleepTime = 50; //failsafe
    if (SleepTime > 0)
      ::Sleep(SleepTime);
  }

  m_output.Reset();
  m_lostEvent.Set();
  while (!stopEvent.Signaled() && m_displayLost && !m_displayReset)
  {
    KODI::TIME::Sleep(10ms);
  }
}

void CVideoSyncD3D::Cleanup()
{
  CLog::Log(LOGDEBUG, "CVideoSyncD3D: Cleaning up Direct3d");

  m_lostEvent.Set();
  DX::Windowing()->Unregister(this);
}

float CVideoSyncD3D::GetFps()
{
#ifdef TARGET_WINDOWS_DESKTOP
  DEVMODEW sDevMode = {};
  sDevMode.dmSize = sizeof(sDevMode);

  if (EnumDisplaySettingsW(m_outputDesc.DeviceName, ENUM_CURRENT_SETTINGS, &sDevMode))
  {
    if ((sDevMode.dmDisplayFrequency + 1) % 24 == 0 || (sDevMode.dmDisplayFrequency + 1) % 30 == 0)
      m_fps = static_cast<float>(sDevMode.dmDisplayFrequency + 1) / 1.001f;
    else
      m_fps = static_cast<float>(sDevMode.dmDisplayFrequency);

    if (sDevMode.dmDisplayFlags & DM_INTERLACED)
      m_fps *= 2;
  }
#else
  using namespace winrt::Windows::Graphics::Display::Core;

  auto hdmiInfo = HdmiDisplayInformation::GetForCurrentView();
  if (hdmiInfo) // Xbox only
  {
    auto currentMode = hdmiInfo.GetCurrentDisplayMode();
    m_fps = currentMode.RefreshRate();
  }
#endif

  if (m_fps == 0.0)
    m_fps = 60.0f;

  return m_fps;
}
