/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "DNSNameCache.h"

#include "network/Network.h"
#include "threads/CriticalSection.h"
#include "utils/log.h"

#include <mutex>

#if !defined(TARGET_WINDOWS) && defined(HAS_FILESYSTEM_SMB)
#include "ServiceBroker.h"

#include "platform/posix/filesystem/SMBWSDiscovery.h"
#endif

#if defined(TARGET_WINDOWS)
#include "platform/win32/CharsetConverter.h"
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#if defined(TARGET_FREEBSD)
#include <sys/socket.h>
#endif

CDNSNameCache g_DNSCache;

CCriticalSection CDNSNameCache::m_critical;

CDNSNameCache::CDNSNameCache(void) = default;

CDNSNameCache::~CDNSNameCache(void) = default;

bool CDNSNameCache::Lookup(const std::string& strHostName, std::string& strIpAddress)
{
  if (strHostName.empty() && strIpAddress.empty())
    return false;

  // first see if this is already an ip address
  in_addr addr4;
  in6_addr addr6;
  strIpAddress.clear();

  if (inet_pton(AF_INET, strHostName.c_str(), &addr4) ||
      inet_pton(AF_INET6, strHostName.c_str(), &addr6))
  {
    strIpAddress = strHostName;
    CLog::LogF(LOGDEBUG, "strHostname is IP address {}", strIpAddress);
    return true;
  }

  // check if there's a custom entry or if it's already cached
  if (g_DNSCache.GetCached(strHostName, strIpAddress))
  {
    CLog::LogF(LOGDEBUG, "strHostname found in cache [{}] > [{}]", strHostName, strIpAddress);
    return true;
  }

  // perform dns lookup
  addrinfo hints{};
  addrinfo* res;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  if (getaddrinfo(strHostName.c_str(), nullptr, &hints, &res) == 0)
  {
    strIpAddress = CNetworkBase::GetIpStr(res->ai_addr);
    addrinfo* ptr = res->ai_next;
    while (ptr)
    {
      CLog::LogF(LOGDEBUG, "getaddrinfo next: [{}]", CNetworkBase::GetIpStr(ptr->ai_addr));
      ptr = ptr->ai_next;
    }

    freeaddrinfo(res);
    CLog::LogF(LOGDEBUG, "getaddrinfo found [{}] > [{}]", strHostName, strIpAddress);
    //g_DNSCache.Add(strHostName, strIpAddress);
    //return true;
  }
  
#if defined(TARGET_WINDOWS)
  static DWORD nameSpaces[]{NS_DNS, NS_NETBT, NS_WINS, NS_NLA, NS_BTH, NS_NTDS, NS_ALL};

  const std::wstring wStrHostName = KODI::PLATFORM::WINDOWS::ToW(strHostName);
  ADDRINFOEXW wHints{};
  ADDRINFOEXW* wRes;

  wHints.ai_family = AF_UNSPEC;
  wHints.ai_socktype = SOCK_STREAM;
  wHints.ai_flags = AI_CANONNAME;

  for (auto ns : nameSpaces)
  {
    INT status = GetAddrInfoExW(wStrHostName.c_str(), NULL, ns, nullptr, &wHints, &wRes, 0,
                                nullptr, nullptr, 0);

    if (status == NO_ERROR)
    {
      strIpAddress = CNetworkBase::GetIpStr(wRes->ai_addr);
      ADDRINFOEXW* ptr = wRes->ai_next;
      while (ptr)
      {
        CLog::LogF(LOGDEBUG, "GetAddrInfoExW next: [{}]", CNetworkBase::GetIpStr(ptr->ai_addr));
        ptr = ptr->ai_next;
      }

      FreeAddrInfoExW(wRes);
      CLog::LogF(LOGDEBUG, "GetAddrInfoExW ns {} found [{}] > [{}]", ns, strHostName, strIpAddress);

      if (ns == NS_ALL)
      {
        g_DNSCache.Add(strHostName, strIpAddress);
        return true;
      }
    }
    else
    {
      CLog::Log(LOGDEBUG, "Lookup failed with namespace {}: error code {}", ns, WSAGetLastError());
    }
  }
#endif

  CLog::Log(LOGERROR, "Unable to lookup host: '{}'", strHostName);
  return false;
}

bool CDNSNameCache::GetCached(const std::string& strHostName, std::string& strIpAddress)
{
  {
    std::unique_lock<CCriticalSection> lock(m_critical);

    // loop through all DNSname entries and see if strHostName is cached
    for (const auto& DNSname : g_DNSCache.m_vecDNSNames)
    {
      if (DNSname.m_strHostName == strHostName)
      {
        strIpAddress = DNSname.m_strIpAddress;
        return true;
      }
    }
  }

#if !defined(TARGET_WINDOWS) && defined(HAS_FILESYSTEM_SMB)
  if (WSDiscovery::CWSDiscoveryPosix::IsInitialized())
  {
    WSDiscovery::CWSDiscoveryPosix& WSInstance =
        dynamic_cast<WSDiscovery::CWSDiscoveryPosix&>(CServiceBroker::GetWSDiscovery());
    if (WSInstance.GetCached(strHostName, strIpAddress))
      return true;
  }
  else
    CLog::Log(LOGDEBUG, LOGWSDISCOVERY,
              "CDNSNameCache::GetCached: CWSDiscoveryPosix not initialized");
#endif

  // not cached
  return false;
}

void CDNSNameCache::Add(const std::string& strHostName, const std::string& strIpAddress)
{
  CDNSName dnsName;

  dnsName.m_strHostName = strHostName;
  dnsName.m_strIpAddress  = strIpAddress;

  std::unique_lock<CCriticalSection> lock(m_critical);
  g_DNSCache.m_vecDNSNames.push_back(dnsName);
}

