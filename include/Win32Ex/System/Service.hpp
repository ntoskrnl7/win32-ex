﻿/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Service.hpp

Abstract:

    This Module implements the ServiceConfig, ServiceConfigW, ServiceConfigT, BasicServiceConfig /
                                Service,  ServiceW, ServiceT, BasicService /
                                Services, ServicesW, ServicesT, BasicServices class.

Author:

    Jung Kwang Lee (ntoskrnl7@gmail.com)

Environment:

    User mode

--*/

#pragma once

#ifndef _WIN32EX_SYSTEM_SERVICE_HPP_
#define _WIN32EX_SYSTEM_SERVICE_HPP_

#include "../Internal/version.h"
#define WIN32EX_SYSTEM_SERVICE_HPP_VERSION_MAJOR WIN32EX_VERSION_MAJOR
#define WIN32EX_SYSTEM_SERVICE_HPP_VERSION_MINOR WIN32EX_VERSION_MINOR
#define WIN32EX_SYSTEM_SERVICE_HPP_VERSION_PATCH WIN32EX_VERSION_PATCH

#include <functional>
#include <list>
#include <map>
#include <stdexcept>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#ifdef TWIN32EX_USE_SERVICE_SIMULATE_MODE
#include <wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")
#endif

#include "../Optional.hpp"
#include "../T/winsvc.hpp"
#include "../T/winuser.hpp"
#include "Process.hpp"

#ifdef _INC__MINGW_H
//
// winsvc.h
//

//
//  Service trigger data item
//
typedef struct _SERVICE_TRIGGER_SPECIFIC_DATA_ITEM
{
    DWORD dwDataType; // Data type -- one of SERVICE_TRIGGER_DATA_TYPE_* constants
#ifdef __midl
    [range(0, 1024)]
#endif
        DWORD cbData; // Size of trigger specific data
#ifdef __midl
    [size_is(cbData)]
#endif
        PBYTE pData; // Trigger specific data
} SERVICE_TRIGGER_SPECIFIC_DATA_ITEM, *PSERVICE_TRIGGER_SPECIFIC_DATA_ITEM;

//
//  Trigger-specific information
//
typedef struct _SERVICE_TRIGGER
{
    DWORD dwTriggerType;   // One of SERVICE_TRIGGER_TYPE_* constants
    DWORD dwAction;        // One of SERVICE_TRIGGER_ACTION_* constants
    GUID *pTriggerSubtype; // Provider GUID if the trigger type is SERVICE_TRIGGER_TYPE_CUSTOM
                           // Device class interface GUID if the trigger type is
                           // SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL
                           // Aggregate identifier GUID if type is aggregate.
#ifdef __midl
    [range(0, 64)]
#endif
        DWORD cDataItems; // Number of data items in pDataItems array
#ifdef __midl
    [size_is(cDataItems)]
#endif
        PSERVICE_TRIGGER_SPECIFIC_DATA_ITEM pDataItems; // Trigger specific data
} SERVICE_TRIGGER, *PSERVICE_TRIGGER;

//
// Service trigger information
//
typedef struct _SERVICE_TRIGGER_INFO
{
#ifdef __midl
    [range(0, 64)]
#endif
        DWORD cTriggers; // Number of triggers in the pTriggers array
#ifdef __midl
    [size_is(cTriggers)]
#endif
        PSERVICE_TRIGGER pTriggers; // Array of triggers
    PBYTE pReserved;                // Reserved, must be NULL
} SERVICE_TRIGGER_INFO, *PSERVICE_TRIGGER_INFO;

//
// Time change information
//
typedef struct _SERVICE_TIMECHANGE_INFO
{
    LARGE_INTEGER liNewTime; // New time
    LARGE_INTEGER liOldTime; // Old time
} SERVICE_TIMECHANGE_INFO, *PSERVICE_TIMECHANGE_INFO;
#endif

#include <dbt.h>
#include <shellapi.h>

#include "../Internal/misc.hpp"
#include "Process.h"

#if defined(_MSC_VER) && _MSC_VER < 1600
#define _STD_NS_ std::tr1
#else
#define _STD_NS_ std
#endif

namespace Win32Ex
{
namespace System
{
namespace Details
{
template <typename T> inline HANDLE CreateStopEvent(const T &Name);

template <> inline HANDLE CreateStopEvent(const String &Name)
{
    return CreateEventA(NULL, TRUE, FALSE, (Name + "StopEvent").c_str());
}

template <> inline HANDLE CreateStopEvent(const StringW &Name)
{
    return CreateEventW(NULL, TRUE, FALSE, (Name + L"StopEvent").c_str());
}

template <typename T> inline HANDLE OpenStopEvent(const T &Name);

template <> inline HANDLE OpenStopEvent(const String &Name)
{
    return OpenEventA(EVENT_ALL_ACCESS, FALSE, (Name + "StopEvent").c_str());
}

template <> inline HANDLE OpenStopEvent(const StringW &Name)
{
    return OpenEventW(EVENT_ALL_ACCESS, FALSE, (Name + L"StopEvent").c_str());
}
} // namespace Details

inline bool IsServiceMode()
{
    static bool isServiceMode = false;
    if (isServiceMode)
        return isServiceMode;

    std::string servicesProcessPath = "%SYSTEMROOT%\\System32\\services.exe";
    DWORD len = ExpandEnvironmentStringsA("%SYSTEMROOT%\\System32\\services.exe", &servicesProcessPath[0],
                                          (DWORD)servicesProcessPath.size());
    if (len > servicesProcessPath.size())
    {
        servicesProcessPath.resize(len);
        len = ExpandEnvironmentStringsA("%SYSTEMROOT%\\System32\\services.exe", &servicesProcessPath[0],
                                        (DWORD)servicesProcessPath.size());
    }
    servicesProcessPath.resize(len - 1);

    isServiceMode = lstrcmpiA(servicesProcessPath.c_str(), ThisProcess::Parent().ExecutablePath().c_str()) == 0;
    return isServiceMode;
}

template <typename _StringType> class BasicServices
{
  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;

#if defined(__cpp_variadic_templates)
    template <class... ServiceType> static bool Run(ServiceType &...service)
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(CharType *)service.Config_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<CharType>::Type)ServiceType::ServiceMain_}...,
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }
#else
    template <typename ServiceType0, typename ServiceType1>
    static bool Run(ServiceType0 &service0, ServiceType1 &service1)
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(CharType *)service0.Config_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<CharType>::Type)ServiceType0::ServiceMain_},
            {(CharType *)service1.Config_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<CharType>::Type)ServiceType1::ServiceMain_},
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }
    template <typename ServiceType0, typename ServiceType1, typename ServiceType2>
    static bool Run(ServiceType0 &service0, ServiceType1 &service1, ServiceType2 &service2)
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(CharType *)service0.Config_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<CharType>::Type)ServiceType0::ServiceMain_},
            {(CharType *)service1.Config_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<CharType>::Type)ServiceType1::ServiceMain_},
            {(CharType *)service2.Config_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<CharType>::Type)ServiceType2::ServiceMain_},
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }
#endif
};

typedef BasicServices<String> Services;
typedef BasicServices<StringW> ServicesW;
typedef BasicServices<StringT> ServicesT;

template <class _StringType> class BasicServiceConfig
{
  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;

    typedef std::function<bool()> AcceptStopCallback;
    typedef std::function<bool()> AcceptPauseCallback;

    typedef struct _SC_HANDLES
    {
        _SC_HANDLES(SC_HANDLE hSCManager = NULL) : hSCManager(hSCManager), hService(NULL)
        {
        }

        ~_SC_HANDLES()
        {
            if (hService)
                CloseServiceHandle(hService);
            if (hSCManager)
                CloseServiceHandle(hSCManager);
        }

        SC_HANDLE hSCManager;
        SC_HANDLE hService;

    } SC_HANDLES, *PSC_HANDLES;

    BasicServiceConfig(const StringType &Name, const Optional<const StringType &> &DisplayName = None(),
                       const Optional<const StringType &> &Description = None())
        : name_(Name)
    {
        if (DisplayName.IsSome())
            displayName_ = DisplayName;

        if (Description.IsSome())
            description_ = Description;

        QUERY_SERVICE_CONFIGT<CharType> *config = QueryConfig_();

        if (config)
        {
            serviceType_ = config->dwServiceType;
            startType_ = config->dwStartType;
            errorControl_ = config->dwErrorControl;
            binaryPathName_ = config->lpBinaryPathName;
            if (config->lpLoadOrderGroup)
                loadOrderGroup_ = config->lpLoadOrderGroup;
            TagId_ = config->dwTagId;
            if (config->lpDependencies)
                dependencies_ = config->lpDependencies;
            if (config->lpServiceStartName)
                serviceStartName_ = config->lpServiceStartName;
            displayName_ = config->lpDisplayName;

            FreeConfig_(config);
        }
    }

    bool Install(DWORD ServiceType = SERVICE_WIN32_OWN_PROCESS, DWORD StartType = SERVICE_AUTO_START,
                 const Optional<const StringType &> BinaryPathName = None(),
                 const Optional<const StringType &> DisplayName = None(),
                 const Optional<const StringType &> Description = None(), DWORD ErrorControl = SERVICE_ERROR_IGNORE,
                 const Optional<const StringType &> LoadOrderGroup = None(), _Out_ PDWORD TagId = NULL,
                 const std::list<StringType> Dependencies = std::list<StringType>(),
                 const Optional<const StringType &> ServiceStartName = None(),
                 const Optional<const StringType &> Password = None())
    {
        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CREATE_SERVICE));
        if (handles.hSCManager == NULL)
            return false;

        if (BinaryPathName.IsSome())
            binaryPathName_ = BinaryPathName;

        if (DisplayName.IsSome())
            displayName_ = DisplayName;

        if (LoadOrderGroup.IsSome())
            loadOrderGroup_ = LoadOrderGroup;

        errorControl_ = ErrorControl;

        StringType dependencies;
        for (typename std::list<StringType>::const_iterator it = Dependencies.begin(); it != Dependencies.end(); ++it)
        {
            dependencies.append(it->c_str());
            dependencies.push_back(0);
        }
        dependencies.push_back(0);

        handles.hService = CreateServiceT<CharType>(
            handles.hSCManager, name_.c_str(), displayName_.empty() ? NULL : displayName_.c_str(),
            SERVICE_CHANGE_CONFIG, ServiceType, StartType, ErrorControl, binaryPathName_.c_str(),
            loadOrderGroup_.empty() ? NULL : loadOrderGroup_.c_str(), TagId,
            dependencies.empty() ? NULL : dependencies.c_str(),
            ServiceStartName.IsSome() ? ServiceStartName.Get() : NULL, Password.IsSome() ? Password.Get() : NULL);

        if (handles.hService == NULL)
            return false;

        serviceType_ = ServiceType;
        startType_ = StartType;

        if (Description.IsSome())
            description_ = Description;

        if (!description_.empty())
        {
            SERVICE_DESCRIPTIONT<CharType> desc;
            desc.lpDescription = &description_[0];
            ChangeServiceConfig2T<CharType>(handles.hService, SERVICE_CONFIG_DESCRIPTION, &desc);
        }

        return true;
    }

    bool Uninstall(Duration Timeout = Duration::Second(30))
    {
        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_ALL_ACCESS));
        if (handles.hSCManager == NULL)
            return false;

        handles.hService = OpenServiceT<CharType>(handles.hSCManager, name_.c_str(), DELETE);
        if (handles.hService == NULL)
            return false;

        SERVICE_STATUS ss = {0};
        if (QueryServiceStatus(ss) && (ss.dwCurrentState != SERVICE_STOPPED) && (!Stop(Timeout)))
            return false;

        if (DeleteService(handles.hService))
        {
            CloseServiceHandle(handles.hService);
            handles.hService = OpenServiceT<CharType>(handles.hSCManager, name_.c_str(), DELETE);
            if (handles.hService == NULL)
                return true;

            if (DeleteService(handles.hService))
                return true;

            return GetLastError() == ERROR_SERVICE_MARKED_FOR_DELETE;
        }
        return false;
    }

    bool Control(DWORD ControlCode)
    {
        SERVICE_STATUS ss = {0};
        if (!QueryServiceStatus(ss))
            return false;

        if (ss.dwCurrentState == SERVICE_STOPPED)
            return false;

        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT));
        if (handles.hSCManager == NULL)
            return false;

        handles.hService = OpenServiceT<CharType>(handles.hSCManager, name_.c_str(),
                                                  SERVICE_QUERY_STATUS | SERVICE_USER_DEFINED_CONTROL);
        if (handles.hService == NULL)
            return false;

        return ControlService(handles.hService, ControlCode, &ss) == TRUE;
    }

    bool Start(Duration Timeout = Duration::Second(30))
    {
        SERVICE_STATUS ss = {0};
        if (!QueryServiceStatus(ss))
            return false;

        if (ss.dwCurrentState == SERVICE_RUNNING)
            return true;

        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT));
        if (handles.hSCManager == NULL)
            return false;

        handles.hService =
            OpenServiceT<CharType>(handles.hSCManager, name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_START);
        if (handles.hService == NULL)
            return false;

        if (!StartServiceA(handles.hService, 0, NULL))
            return false;

        return Wait(SERVICE_RUNNING, Timeout);
    }

    bool Stop(Duration Timeout = Duration::Second(30))
    {
        HANDLE stopEvent = Details::OpenStopEvent<StringType>(name_);
        if (stopEvent)
        {
            BOOL ret = SetEvent(stopEvent);
            ::CloseHandle(stopEvent);
            return ret == TRUE;
        }

        SERVICE_STATUS ss = {0};
        if (!QueryServiceStatus(ss))
            return false;

        if (ss.dwCurrentState == SERVICE_STOPPED)
            return true;

        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT));
        if (handles.hSCManager == NULL)
            return false;

        handles.hService =
            OpenServiceT<CharType>(handles.hSCManager, name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_STOP);
        if (handles.hService == NULL)
            return false;

        if (acceptStopCallback_ && (!acceptStopCallback_()))
            return false;

        if (!ControlService(handles.hService, SERVICE_CONTROL_STOP, &ss))
            return false;

        return Wait(SERVICE_STOPPED, Timeout);
    }

    bool Pause(Duration Timeout = Duration::Second(30))
    {
        SERVICE_STATUS ss = {0};
        if (!QueryServiceStatus(ss))
            return false;

        if (ss.dwCurrentState == SERVICE_PAUSED)
            return true;

        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT));
        if (handles.hSCManager == NULL)
            return false;

        handles.hService =
            OpenServiceT<CharType>(handles.hSCManager, name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_PAUSE_CONTINUE);
        if (handles.hService == NULL)
            return false;

        if (acceptPauseCallback_ && (!acceptPauseCallback_()))
            return false;

        if (!ControlService(handles.hService, SERVICE_CONTROL_PAUSE, &ss))
            return false;

        return Wait(SERVICE_PAUSED, Timeout);
    }

    bool Continue(Duration Timeout = Duration::Second(30))
    {
        SERVICE_STATUS ss = {0};
        if (!QueryServiceStatus(ss))
            return false;

        if (ss.dwCurrentState != SERVICE_PAUSED)
            return true;

        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT));
        if (handles.hSCManager == NULL)
            return false;

        handles.hService =
            OpenServiceT<CharType>(handles.hSCManager, name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_PAUSE_CONTINUE);
        if (handles.hService == NULL)
            return false;

        if (!ControlService(handles.hService, SERVICE_CONTROL_CONTINUE, &ss))
            return false;

        return Wait(SERVICE_RUNNING, Timeout);
    }

    bool Wait(DWORD ServiceState, Duration Timeout) const
    {
        SERVICE_STATUS ss = {0};
        for (;;)
        {
            if (!QueryServiceStatus(ss))
            {
                SetLastError(GetLastError());
                return false;
            }

            if (ss.dwCurrentState == ServiceState)
                return true;

            if (Timeout == 0)
                break;

            if (Timeout >= 100)
            {
                Sleep(100);
                Timeout -= 100;
            }
            else
            {
                Sleep(Timeout);
                Timeout = 0;
            }
        }

        SetLastError(WAIT_TIMEOUT);
        return false;
    }

    bool Installed() const
    {
        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT));
        if (handles.hSCManager == NULL)
            return false;

        handles.hService = OpenServiceT<CharType>(handles.hSCManager, name_.c_str(), SERVICE_INTERROGATE);
        return (handles.hService != NULL);
    }

    bool QueryServiceStatus(SERVICE_STATUS &ss) const
    {
        SC_HANDLES handles(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT));
        if (handles.hSCManager == NULL)
            return false;

        handles.hService = OpenServiceT<CharType>(handles.hSCManager, name_.c_str(), SERVICE_INTERROGATE);
        if (handles.hService == NULL)
            return false;

        if (ControlService(handles.hService, SERVICE_CONTROL_INTERROGATE, &ss) == FALSE)
        {
            if (GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
            {
                SetLastError(ERROR_SUCCESS);
                ss.dwCurrentState = SERVICE_STOPPED;
                return true;
            }
            return false;
        }

        return true;
    }

    const StringType &Name() const
    {
        return name_;
    }

    DWORD ServiceType() const
    {
        return serviceType_;
    }

    StringType &BinaryPathName() const
    {
        if (binaryPathName_.empty())
            binaryPathName_ = ThisProcess::ExecutablePathT<StringType>();

        return binaryPathName_;
    }

    SC_HANDLE ServiceHandle(DWORD DesiredAccess) const
    {
        SC_HANDLE hSCManager = OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT);
        if (hSCManager == NULL)
            return NULL;
        return OpenServiceT<CharType>(hSCManager, name_.c_str(), DesiredAccess);
    }

    bool CloseHandle(SC_HANDLE ServiceHandle) const
    {
        return ::CloseServiceHandle(ServiceHandle) == TRUE;
    }

    BasicServiceConfig &SetAcceptStop(AcceptStopCallback Callback)
    {
        acceptStopCallback_ = Callback;
        return *this;
    }

    BasicServiceConfig &SetAcceptPause(AcceptPauseCallback Callback)
    {
        acceptPauseCallback_ = Callback;
        return *this;
    }

    bool SetFailureActions(const SERVICE_FAILURE_ACTIONSA &FailureActions)
    {
        DWORD desireAccess = SERVICE_CHANGE_CONFIG;
        BOOL needToAcquireShutdownPrivilege = FALSE;
        for (DWORD i = 0; i < FailureActions.cActions; i++)
        {
            switch (FailureActions.lpsaActions[i].Type)
            {
            case SC_ACTION_RESTART:
                SetFlag(desireAccess, SERVICE_START);
                break;
            case SC_ACTION_REBOOT:
                needToAcquireShutdownPrivilege = TRUE;
                break;
            case SC_ACTION_NONE:
            case SC_ACTION_RUN_COMMAND:
            default:
                break;
            }
        }

        SC_HANDLE hService = ServiceHandle(desireAccess);

        if (hService != NULL)
        {
            PREVIOUS_TOKEN_PRIVILEGES prevState;
            if (needToAcquireShutdownPrivilege)
            {
                needToAcquireShutdownPrivilege = EnablePrivilege(TRUE, SE_SHUTDOWN_NAME, &prevState, NULL);
            }

            bool ret = ChangeServiceConfig2T<CharType>(hService, SERVICE_CONFIG_FAILURE_ACTIONS,
                                                       (PVOID)&FailureActions) == TRUE;

            if (needToAcquireShutdownPrivilege)
            {
                RevertPrivileges(&prevState);
            }

            CloseHandle(hService);
            return ret;
        }

        return false;
    }

    bool SetFailureActions(const SERVICE_FAILURE_ACTIONSA &FailureActions, SERVICE_FAILURE_ACTIONS_FLAG Flags)
    {
        if (!SetFailureActions(FailureActions))
        {
            return false;
        }

        SC_HANDLE hService = ServiceHandle(SERVICE_CHANGE_CONFIG);

        if (hService != NULL)
        {
            bool ret =
                ChangeServiceConfig2T<CharType>(hService, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, (PVOID)&Flags) == TRUE;
            CloseHandle(hService);
            return ret;
        }

        return false;
    }

#ifdef SERVICE_CONFIG_TRIGGER_INFO
    bool SetTrigger(const SERVICE_TRIGGER_INFO &TriggerInfo) const
    {
        SC_HANDLE hService = ServiceHandle(SERVICE_CHANGE_CONFIG);
        if (hService != NULL)
        {
            bool ret =
                ChangeServiceConfig2T<CharType>(hService, SERVICE_CONFIG_TRIGGER_INFO, (PVOID)&TriggerInfo) == TRUE;
            CloseHandle(hService);
            return ret;
        }

        return false;
    }
#endif
#ifdef SERVICE_CONFIG_PRESHUTDOWN_INFO
    bool SetPreshutdownTimeout(Duration PreshutdownTimeout) const
    {
        SC_HANDLE hService = ServiceHandle(SERVICE_CHANGE_CONFIG);
        if (hService == NULL)
            return false;

        SERVICE_PRESHUTDOWN_INFO info = {PreshutdownTimeout};
        bool ret = ChangeServiceConfig2T<CharType>(hService, SERVICE_CONFIG_PRESHUTDOWN_INFO, &info) == TRUE;
        CloseHandle(hService);
        return ret;
    }
#endif
  private:
    QUERY_SERVICE_CONFIGT<CharType> *QueryConfig_() const
    {
        DWORD dwBytesNeeded;
        DWORD cbBufSize = 0;
        QUERY_SERVICE_CONFIGT<CharType> *lpsc = NULL;
        SC_HANDLES handles;

        SC_HANDLE schSCManager = OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT);
        SC_HANDLE schService = OpenServiceT<CharType>(schSCManager, name_.c_str(), SERVICE_QUERY_CONFIG);
        if (!schService)
            return NULL;

        if (!QueryServiceConfigT<CharType>(schService, NULL, 0, &dwBytesNeeded))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                cbBufSize = dwBytesNeeded;
                lpsc = (QUERY_SERVICE_CONFIGT<CharType> *)LocalAlloc(LMEM_FIXED, cbBufSize);
            }
            else
            {
                CloseServiceHandle(schService);
                return NULL;
            }
        }

        if (lpsc == NULL)
        {
            CloseServiceHandle(schService);
            return NULL;
        }

        if (!QueryServiceConfigT<CharType>(schService, lpsc, cbBufSize, &dwBytesNeeded))
        {
            if (lpsc)
                LocalFree(lpsc);

            CloseServiceHandle(schService);
            return NULL;
        }

        CloseServiceHandle(schService);
        return lpsc;
    }

    void FreeConfig_(QUERY_SERVICE_CONFIGT<CharType> *Config)
    {
        if (Config)
            LocalFree(Config);
    }

  private:
    StringType name_;
    DWORD serviceType_;
    DWORD startType_;
    DWORD errorControl_;
    mutable StringType binaryPathName_;
    StringType loadOrderGroup_;
    DWORD TagId_;
    StringType dependencies_;
    StringType serviceStartName_;
    StringType displayName_;

    StringType description_;

    AcceptStopCallback acceptStopCallback_;
    AcceptPauseCallback acceptPauseCallback_;
};

typedef BasicServiceConfig<String> ServiceConfig;
typedef BasicServiceConfig<StringW> ServiceConfigW;
typedef BasicServiceConfig<StringT> ServiceConfigT;

template <typename _StringType, const BasicServiceConfig<_StringType> &Config> class BasicService
{
    friend class BasicServices<String>;
    friend class BasicServices<StringW>;
    friend class BasicServices<StringT>;

  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;

    typedef std::function<void()> StartCallback;
    typedef std::function<DWORD(DWORD, CharType *[])> StartCallbackEx;
    typedef std::function<void()> StopCallback;
    typedef std::function<void()> PauseCallback;
    typedef std::function<void()> ContinueCallback;
    typedef std::function<void()> ShutdownCallback;
    typedef std::function<void()> ParamChangeCallback;
    typedef std::function<void()> NetbindAddCallback;
    typedef std::function<void()> NetbindRemoveCallback;
    typedef std::function<void()> NetbindEnableCallback;
    typedef std::function<void()> NetbindDisableCallback;
    typedef std::function<DWORD(DWORD, PDEV_BROADCAST_HDR)> DeviceEventCallback;
    typedef std::function<DWORD(DWORD)> HardwareProfileChangeCallback;
    typedef std::function<DWORD(DWORD, PPOWERBROADCAST_SETTING)> PowerEventCallback;
    typedef std::function<void(DWORD, PWTSSESSION_NOTIFICATION)> SessionChangeCallback;
#ifdef SERVICE_CONTROL_PRESHUTDOWN
    typedef std::function<void()> PreShutdownCallback;
#endif
#ifdef SERVICE_CONTROL_TIMECHANGE
    typedef std::function<void(PSERVICE_TIMECHANGE_INFO)> TimeChangeCallback;
#endif
#ifdef SERVICE_CONTROL_TRIGGEREVENT
    typedef std::function<void()> TriggerEventallback;
#endif
    typedef std::function<void()> OtherCallback;
    typedef std::function<DWORD(DWORD, PVOID)> OtherCallbackEx;
    typedef std::function<void(DWORD ErrorCode, PCSTR Message)> ErrorCallback;

  public:
    BasicService &On(DWORD Control, OtherCallback Callback)
    {
        otherCallbackMap_[Control] = Callback;
        return *this;
    }

    BasicService &OnEx(DWORD Control, OtherCallbackEx Callback)
    {
        otherCallbackExMap_[Control] = Callback;
        return *this;
    }

    BasicService &OnStart(StartCallback Callback)
    {
        startCallback_ = Callback;
        return *this;
    }

    BasicService &OnStartEx(StartCallbackEx Callback)
    {
        startCallbackEx_ = Callback;
        return *this;
    }

    BasicService &OnStop(StopCallback Callback = NULL)
    {
        stopCallback_ = Callback;

        if (stopCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_STOP);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_STOP);

        return *this;
    }

    BasicService &OnPause(PauseCallback Callback = NULL)
    {
        pauseCallback_ = Callback;

        if (pauseCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
        else if (continueCallback_ == NULL)
            ClearControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);

        return *this;
    }

    BasicService &OnContinue(ContinueCallback Callback = NULL)
    {
        continueCallback_ = Callback;

        if (continueCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
        else if (pauseCallback_ == NULL)
            ClearControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);

        return *this;
    }

    BasicService &OnShutdown(ShutdownCallback Callback = NULL)
    {
        shutdownCallback_ = Callback;

        if (shutdownCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_SHUTDOWN);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_SHUTDOWN);

        return *this;
    }

    BasicService &OnParamChange(ParamChangeCallback Callback = NULL)
    {
        paramChangeCallback_ = Callback;

        if (paramChangeCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_PARAMCHANGE);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_PARAMCHANGE);

        return *this;
    }

    BasicService &OnNetbindAdd(NetbindAddCallback Callback = NULL)
    {
        netbindAddCallback_ = Callback;

        if (netbindAddCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);
        else if ((netbindRemoveCallback_ == NULL) && (netbindEnableCallback_ == NULL) &&
                 (netbindDisableCallback_ == NULL))
            ClearControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);

        return *this;
    }

    BasicService &OnNetbindRemove(NetbindAddCallback Callback = NULL)
    {
        netbindRemoveCallback_ = Callback;

        if (netbindRemoveCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);
        else if ((netbindAddCallback_ == NULL) && (netbindEnableCallback_ == NULL) && (netbindDisableCallback_ == NULL))
            ClearControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);

        return *this;
    }

    BasicService &OnNetbindEnable(NetbindAddCallback Callback = NULL)
    {
        netbindEnableCallback_ = Callback;

        if (netbindEnableCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);
        else if ((netbindAddCallback_ == NULL) && (netbindRemoveCallback_ == NULL) && (netbindDisableCallback_ == NULL))
            ClearControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);

        return *this;
    }

    BasicService &OnNetbindDisable(NetbindAddCallback Callback = NULL)
    {
        netbindDisableCallback_ = Callback;

        if (netbindDisableCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);
        else if ((netbindAddCallback_ == NULL) && (netbindRemoveCallback_ == NULL) && (netbindEnableCallback_ == NULL))
            ClearControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);

        return *this;
    }

    BasicService &OnDeviceEvent(PVOID NotificationFilter = NULL, DeviceEventCallback Callback = NULL)
    {
        return OnDeviceEventT<CharType>(NotificationFilter, Callback);
    }

    template <typename T>
    BasicService &OnDeviceEventT(PVOID NotificationFilter = NULL, DeviceEventCallback Callback = NULL)
    {
        deviceEventCallback_ = Callback;

        if (deviceEventCallback_)
        {
            if (serviceStatusHandle_)
                deviceNotifyHandle_ = RegisterDeviceNotificationT<T>(serviceStatusHandle_, NotificationFilter,
                                                                     DEVICE_NOTIFY_SERVICE_HANDLE);
            else if (hWndConsoleService_)
                deviceNotifyHandle_ = RegisterDeviceNotificationT<T>(hWndConsoleService_, NotificationFilter,
                                                                     DEVICE_NOTIFY_WINDOW_HANDLE);
        }
        else
        {
            if (deviceNotifyHandle_)
            {
                UnregisterDeviceNotification(deviceNotifyHandle_);
                deviceNotifyHandle_ = NULL;
            }
        }

        return *this;
    }

    BasicService &OnHardwareProfileChange(HardwareProfileChangeCallback Callback = NULL)
    {
        hardwareProfileChangeCallback_ = Callback;

        if (hardwareProfileChangeCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_HARDWAREPROFILECHANGE);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_HARDWAREPROFILECHANGE);

        return *this;
    }

    BasicService &OnPowerEvent(PowerEventCallback Callback = NULL)
    {
        powerEventCallback_ = Callback;

        if (powerEventCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_POWEREVENT);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_POWEREVENT);

        return *this;
    }

    BasicService &OnSessionChanage(SessionChangeCallback Callback = NULL)
    {
        sessionChangeCallback_ = Callback;

        if (sessionChangeCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_SESSIONCHANGE);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_SESSIONCHANGE);

        return *this;
    }
#ifdef SERVICE_CONTROL_PRESHUTDOWN
    BasicService &OnPreShutdown(Duration PreshutdownTimeout, PreShutdownCallback Callback = NULL)
    {
        Config.SetPreshutdownTimeout(PreshutdownTimeout);
        return OnPreShutdown(Callback);
    }

    BasicService &OnPreShutdown(PreShutdownCallback Callback = NULL)
    {
        preShutdownCallback_ = Callback;

        if (preShutdownCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_PRESHUTDOWN);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_PRESHUTDOWN);

        return *this;
    }
#endif
#ifdef SERVICE_CONTROL_TIMECHANGE
    BasicService &OnTimeChange(TimeChangeCallback Callback = NULL)
    {
        timeChangeCallback_ = Callback;

        if (timeChangeCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_TIMECHANGE);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_TIMECHANGE);

        return *this;
    }
#endif
#ifdef SERVICE_CONTROL_TRIGGEREVENT
    BasicService &OnTriggerEvent(const SERVICE_TRIGGER_INFO &TriggerInfo, TriggerEventallback Callback = NULL)
    {
        Config.SetTrigger(TriggerInfo);
        return OnTriggerEvent(Callback);
    }

    BasicService &OnTriggerEvent(TriggerEventallback Callback = NULL)
    {
        triggerEventCallback_ = Callback;

        if (triggerEventCallback_)
            SetControlsAccepted(SERVICE_ACCEPT_TRIGGEREVENT);
        else
            ClearControlsAccepted(SERVICE_ACCEPT_TRIGGEREVENT);

        return *this;
    }
#endif
    BasicService &OnError(ErrorCallback Callback)
    {
        errorCallback_ = Callback;
        return *this;
    }

    bool SetWin32ExitCode(DWORD Win32ExitCode)
    {
        serviceStatus_.dwServiceType = Config.ServiceType();
        serviceStatus_.dwWin32ExitCode = Win32ExitCode;
        return UpdateServiceStatus_();
    }

    bool SetServiceSpecificExitCode(DWORD ServiceSpecificExitCode)
    {
        serviceStatus_.dwServiceType = Config.ServiceType();
        serviceStatus_.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        serviceStatus_.dwServiceSpecificExitCode = ServiceSpecificExitCode;
        return UpdateServiceStatus_();
    }

    bool IsRunning() const
    {
        return serviceStatus_.dwCurrentState == SERVICE_RUNNING;
    }

    DWORD Status() const
    {
        return serviceStatus_.dwCurrentState;
    }

    HANDLE StatusHandle() const
    {
        return (IsServiceMode() ? (HANDLE)serviceStatusHandle_ : (HANDLE)hWndConsoleService_);
    }

    DWORD ControlsAccepted() const
    {
        return serviceStatus_.dwControlsAccepted;
    }

    bool SetStatus(DWORD Status, DWORD Win32ExitCode = ERROR_SUCCESS)
    {
        serviceStatus_.dwServiceType = Config.ServiceType();
        serviceStatus_.dwCurrentState = Status;
        serviceStatus_.dwWin32ExitCode = Win32ExitCode;
        return UpdateServiceStatus_();
    }

    bool ClearControlsAccepted(DWORD ControlsAccepted)
    {
        ClearFlag(serviceStatus_.dwControlsAccepted, ControlsAccepted);
        return UpdateServiceStatus_();
    }

    bool SetControlsAccepted(DWORD ControlsAccepted)
    {
        SetFlag(serviceStatus_.dwControlsAccepted, ControlsAccepted);
        return UpdateServiceStatus_();
    }

    bool Run()
    {
        if (IsServiceMode())
        {
            typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
                {(CharType *)Config.Name().c_str(), (typename LPSERVICE_MAIN_FUNCTIONT<CharType>::Type)ServiceMain_},
                {NULL, NULL}};

            return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
        }
#ifdef TWIN32EX_USE_SERVICE_SIMULATE_MODE
        else
        {
#if _CRT_DECLARE_GLOBAL_VARIABLES_DIRECTLY
#if (defined(__argc) && defined(__argv))
            if (typeid(CharType) == typeid(CHAR))
            {
                ServiceMain_(__argc, __argv);
            }
            else
            {
                ServiceMain_(0, NULL);
            }

#elif (defined(__argc) && defined(__wargv))
            if (typeid(CharType) == typeid(WCHAR))
            {
                ServiceMain_(__argc, __argv);
            }
            else
            {
                ServiceMain_(0, NULL);
            }
#endif
#else
            ServiceMain_(0, NULL);
#endif
            return true;
        }
#endif
        return false;
    }

  protected:
    BasicService()
        : serviceStatusHandle_(NULL), deviceNotifyHandle_(NULL), hCleanupCompleteEvent_(NULL), hStopedEvent_(NULL),
          hWndConsoleService_(NULL), Config_(Config)
    {
        ZeroMemory(&serviceStatus_, sizeof(serviceStatus_));
    }

    ~BasicService()
    {
        if (deviceNotifyHandle_)
            UnregisterDeviceNotification(deviceNotifyHandle_);

        if (hStopedEvent_)
            SetEvent(hStopedEvent_);

        Cleanup_();
    }

  private:
    void Cleanup_()
    {
        if (hStopedEvent_)
        {
            CloseHandle(hStopedEvent_);
            hStopedEvent_ = NULL;
        }

        if (hCleanupCompleteEvent_)
        {
            SetEvent(hCleanupCompleteEvent_);
            CloseHandle(hCleanupCompleteEvent_);
            hCleanupCompleteEvent_ = NULL;
        }
    }

    bool UpdateServiceStatus_()
    {
        if (serviceStatusHandle_ == NULL)
        {
            SetLastError(ERROR_SERVICE_NOT_ACTIVE);
            return false;
        }
        return ::SetServiceStatus(serviceStatusHandle_, &serviceStatus_) == TRUE;
    }

  public:
    static BasicService &Instance()
    {
        static BasicService service;
        return service;
    }

  private:
    void RaiseError_(DWORD ErrorCode, PCSTR Message)
    {
        if (errorCallback_)
            errorCallback_(ErrorCode, Message);
        SetWin32ExitCode(ErrorCode);
    }

#ifdef TWIN32EX_USE_SERVICE_SIMULATE_MODE
    static BOOL WINAPI ConsoleHandlerRoutine_(DWORD dwCtrlType)
    {
        ServiceHandlerEx_(SERVICE_CONTROL_STOP, dwCtrlType, NULL, NULL);
        return TRUE;
    }

    static LRESULT CALLBACK ConsoleWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_DEVICECHANGE:
            ServiceHandlerEx_(SERVICE_CONTROL_DEVICEEVENT, (DWORD)wParam, (LPVOID)lParam, NULL);
            break;

        case WM_WTSSESSION_CHANGE:
            WTSSESSION_NOTIFICATION sessionNotification;
            sessionNotification.cbSize = sizeof(sessionNotification);
            sessionNotification.dwSessionId = (DWORD)lParam;
            ServiceHandlerEx_(SERVICE_CONTROL_SESSIONCHANGE, (DWORD)wParam, &sessionNotification, NULL);
            break;
#ifdef SERVICE_CONTROL_TIMECHANGE
        case WM_TIMECHANGE:

            SERVICE_TIMECHANGE_INFO info;

            SYSTEMTIME st;
            GetSystemTime(&st);

            FILETIME ft;
            SystemTimeToFileTime(&st, &ft);

            info.liOldTime.QuadPart = 0; // Not yet. :-(
            info.liNewTime.LowPart = ft.dwLowDateTime;
            info.liNewTime.HighPart = ft.dwHighDateTime;

            ServiceHandlerEx_(SERVICE_CONTROL_TIMECHANGE, 0, &info, NULL);
            break;
#endif
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    struct CONSOLE_WND_THREAD_PARAM
    {
        CONSOLE_WND_THREAD_PARAM(BasicService &Service, HINSTANCE hInstance, PCSTR ClassName)
            : Service(Service), hWnd(NULL), hInstance(hInstance), ClassName(ClassName), hEventWndCreatedOrFailed(NULL)
        {
        }
        ~CONSOLE_WND_THREAD_PARAM()
        {
            if (hEventWndCreatedOrFailed)
                CloseHandle(hEventWndCreatedOrFailed);
        }

        HINSTANCE hInstance;
        PCSTR ClassName;
        HANDLE hEventWndCreatedOrFailed;
        HWND hWnd;
        BasicService &Service;
    };

    static DWORD WINAPI ConsoleWndThreadProc_(LPVOID lpThreadParameter)
    {
        CONSOLE_WND_THREAD_PARAM *param = (CONSOLE_WND_THREAD_PARAM *)lpThreadParameter;

        DWORD errorCode;
        BasicService &service = param->Service;

        HWND hWndSvc =
            CreateWindowExA(0, param->ClassName, "ConsoleServiceWindow", 0, 0, 0, 0, 0, 0, 0, param->hInstance, 0);

        if (hWndSvc == NULL)
        {
            errorCode = GetLastError();
            service.RaiseError_(errorCode, "Failed to CreateWindowExA");
            SetEvent(param->hEventWndCreatedOrFailed);
            return GetLastError();
        }

        if (!WTSRegisterSessionNotification(hWndSvc, NOTIFY_FOR_ALL_SESSIONS))
        {
            errorCode = GetLastError();
            DestroyWindow(hWndSvc);
            service.RaiseError_(errorCode, "Failed to WTSRegisterSessionNotification");
            SetEvent(param->hEventWndCreatedOrFailed);
            return GetLastError();
        }

        param->hWnd = hWndSvc;
        SetEvent(param->hEventWndCreatedOrFailed);

        BOOL bRet;
        MSG msg;

        while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
        {
            if (bRet == -1)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        WTSUnRegisterSessionNotification(hWndSvc);
        DestroyWindow(hWndSvc);
        return 0;
    }
#endif
    static DWORD WINAPI ServiceHandlerEx_(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
    {
        DWORD result = NO_ERROR;
        BasicService &service = BasicService::Instance();

        UNREFERENCED_PARAMETER(lpContext);

        switch (dwControl)
        {
        case SERVICE_CONTROL_STOP:
            service.SetStatus(SERVICE_STOP_PENDING);

            if (service.stopCallback_)
                service.stopCallback_();

            SetEvent(service.hStopedEvent_);
            service.SetStatus(SERVICE_STOPPED);
            WaitForSingleObject(service.hCleanupCompleteEvent_, INFINITE);
            result = NO_ERROR;
            break;

        case SERVICE_CONTROL_PAUSE:
            service.SetStatus(SERVICE_PAUSE_PENDING);
            if (service.pauseCallback_)
                service.pauseCallback_();
            service.SetStatus(SERVICE_PAUSED);
            break;

        case SERVICE_CONTROL_CONTINUE:
            service.SetStatus(SERVICE_CONTINUE_PENDING);
            if (service.continueCallback_)
                service.continueCallback_();
            service.SetStatus(SERVICE_RUNNING);
            break;

        case SERVICE_CONTROL_INTERROGATE:
            result = NO_ERROR;
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            if (service.shutdownCallback_)
                service.shutdownCallback_();
            result = NO_ERROR;
            break;

        case SERVICE_CONTROL_PARAMCHANGE:
            if (service.paramChangeCallback_)
                service.paramChangeCallback_();
            break;

        case SERVICE_CONTROL_NETBINDADD:
            if (service.netbindAddCallback_)
                service.netbindAddCallback_();
            break;

        case SERVICE_CONTROL_NETBINDREMOVE:
            if (service.netbindRemoveCallback_)
                service.netbindRemoveCallback_();
            break;

        case SERVICE_CONTROL_NETBINDENABLE:
            if (service.netbindEnableCallback_)
                service.netbindEnableCallback_();
            break;

        case SERVICE_CONTROL_NETBINDDISABLE:
            if (service.netbindDisableCallback_)
                service.netbindDisableCallback_();
            break;

        case SERVICE_CONTROL_DEVICEEVENT:
            if (service.deviceEventCallback_)
                result = service.deviceEventCallback_(dwEventType, (PDEV_BROADCAST_HDR)lpEventData);
            break;

        case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
            if (service.hardwareProfileChangeCallback_)
                result = service.hardwareProfileChangeCallback_(dwEventType);
            break;

        case SERVICE_CONTROL_POWEREVENT:
            if (service.powerEventCallback_)
                result = service.powerEventCallback_(dwEventType, (PPOWERBROADCAST_SETTING)lpEventData);
            break;

        case SERVICE_CONTROL_SESSIONCHANGE:
            if (service.sessionChangeCallback_)
                service.sessionChangeCallback_(dwEventType, (PWTSSESSION_NOTIFICATION)lpEventData);
            break;
#ifdef SERVICE_CONTROL_PRESHUTDOWN
        case SERVICE_CONTROL_PRESHUTDOWN:
            if (service.preShutdownCallback_)
                service.preShutdownCallback_();
            break;
#endif
#ifdef SERVICE_CONTROL_TIMECHANGE
        case SERVICE_CONTROL_TIMECHANGE:
            if (service.timeChangeCallback_)
                service.timeChangeCallback_((PSERVICE_TIMECHANGE_INFO)lpEventData);
            break;
#endif
#ifdef SERVICE_CONTROL_TRIGGEREVENT
        case SERVICE_CONTROL_TRIGGEREVENT:
            if (service.triggerEventCallback_)
                service.triggerEventCallback_();
            break;
#endif
        default:
            if (service.otherCallbackExMap_[dwControl])
                return service.otherCallbackExMap_[dwControl](dwEventType, lpEventData);

            if (service.otherCallbackMap_[dwControl])
                service.otherCallbackMap_[dwControl]();
            break;
        }

        return result;
    }

    static VOID WINAPI ServiceMain_(DWORD dwNumServicesArgs, CharType **lpServiceArgVectors)
    {
        DWORD errorCode;
        BasicService &service = BasicService::Instance();

        service.hCleanupCompleteEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (service.hCleanupCompleteEvent_ == NULL)
        {
            service.RaiseError_(GetLastError(), "Failed to CreateEventW");
            CloseHandle(service.hStopedEvent_);
            return;
        }

        if (IsServiceMode())
        {
            service.hStopedEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (service.hStopedEvent_ == NULL)
            {
                service.RaiseError_(GetLastError(), "Failed to CreateEventW");
                return;
            }

            service.serviceStatusHandle_ =
                RegisterServiceCtrlHandlerExT<CharType>((CharType *)Config.Name().c_str(), ServiceHandlerEx_, &service);

            if (!service.serviceStatusHandle_)
            {
                errorCode = GetLastError();
                CloseHandle(service.hStopedEvent_);
                service.RaiseError_(GetLastError(), "Failed to RegisterServiceCtrlHandlerExW");
                service.Cleanup_();
                return;
            }

            service.SetStatus(SERVICE_START_PENDING);

            if (service.startCallbackEx_)
            {
                errorCode = service.startCallbackEx_(dwNumServicesArgs, lpServiceArgVectors);
                if (errorCode != ERROR_SUCCESS)
                {
                    service.RaiseError_(errorCode, "Failed to start service.");
                    service.SetStatus(SERVICE_STOPPED, errorCode);
                    service.Cleanup_();
                    return;
                }
            }
            else if (service.startCallback_)
            {
                service.startCallback_();
            }

            service.SetStatus(SERVICE_RUNNING);
            WaitForSingleObject(service.hStopedEvent_, INFINITE);

            service.Cleanup_();
            return;
        }
#ifdef TWIN32EX_USE_SERVICE_SIMULATE_MODE
        else
        {
            service.hStopedEvent_ = Details::CreateStopEvent(service.Config_.Name());
            if (service.hStopedEvent_ == NULL)
            {
                service.RaiseError_(GetLastError(), "Failed to CreateEventW");
                return;
            }

            if (!SetConsoleCtrlHandler(ConsoleHandlerRoutine_, TRUE))
            {
                errorCode = GetLastError();
                service.RaiseError_(GetLastError(), "Failed to SetConsoleCtrlHandler");
                service.Cleanup_();
                if (!IsServiceMode())
                    SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                return;
            }

            using namespace Convert::String;

            std::string className =
                typeid(StringType) == typeid(String) ? (PCSTR)Config.Name().c_str() : (PCSTR)(!Config.Name()).c_str();
            className.append("Class");

            HINSTANCE hInstance = GetModuleHandle(NULL);

            WNDCLASSEXA wndClass = {sizeof(WNDCLASSEXA),
                                    CS_DBLCLKS,
                                    ConsoleWndProc_,
                                    0,
                                    0,
                                    hInstance,
                                    LoadIcon(0, IDI_APPLICATION),
                                    LoadCursor(0, IDC_ARROW),
                                    HBRUSH(COLOR_WINDOW + 1),
                                    0,
                                    className.c_str(),
                                    LoadIcon(0, IDI_APPLICATION)};

            if (!RegisterClassExA(&wndClass))
            {
                errorCode = GetLastError();
                service.RaiseError_(errorCode, "Failed to RegisterClassExA");
                service.Cleanup_();
                if (!IsServiceMode())
                    SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                return;
            }

            CONSOLE_WND_THREAD_PARAM param(service, hInstance, className.c_str());

            param.hEventWndCreatedOrFailed = CreateEvent(NULL, FALSE, FALSE, NULL);

            if (!param.hEventWndCreatedOrFailed)
            {
                errorCode = GetLastError();
                UnregisterClassA(className.c_str(), hInstance);
                service.RaiseError_(errorCode, "Failed to CreateEvent");
                service.Cleanup_();
                if (!IsServiceMode())
                    SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                return;
            }

            HANDLE hThraed = CreateThread(NULL, 0, ConsoleWndThreadProc_, &param, 0, NULL);
            if (!hThraed)
            {
                errorCode = GetLastError();
                UnregisterClassA(className.c_str(), hInstance);
                service.RaiseError_(errorCode, "Failed to WTSRegisterSessionNotification");
                service.Cleanup_();
                if (!IsServiceMode())
                    SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                return;
            }

            service.SetStatus(SERVICE_START_PENDING);
            WaitForSingleObject(param.hEventWndCreatedOrFailed, INFINITE);

            if (param.hWnd == NULL)
            {
                service.Cleanup_();
                if (!IsServiceMode())
                    SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                return;
            }

            service.hWndConsoleService_ = param.hWnd;
            if (service.startCallbackEx_)
            {
                errorCode = service.startCallbackEx_(dwNumServicesArgs, lpServiceArgVectors);
                if (errorCode != ERROR_SUCCESS)
                {
                    service.RaiseError_(errorCode, "Failed to start service.");

                    service.SetStatus(SERVICE_STOPPED, errorCode);
                    SendMessage(service.hWndConsoleService_, WM_DESTROY, 0, 0);
                    WaitForSingleObject(hThraed, INFINITE);
                    CloseHandle(hThraed);

                    service.Cleanup_();
                    if (!IsServiceMode())
                        SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                    return;
                }
            }
            else if (service.startCallback_)
            {
                service.startCallback_();
            }

            service.SetStatus(SERVICE_RUNNING);
            ResumeThread(hThraed);
            WaitForSingleObject(service.hStopedEvent_, INFINITE);

            SendMessage(service.hWndConsoleService_, WM_DESTROY, 0, 0);
            WaitForSingleObject(hThraed, INFINITE);
            CloseHandle(hThraed);
            UnregisterClassA(className.c_str(), hInstance);

            service.Cleanup_();
            if (!IsServiceMode())
                SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
            return;
        }
#endif // TWIN32EX_USE_SERVICE_SIMULATE_MODE
    }

  protected:
    StartCallback startCallback_;
    StartCallbackEx startCallbackEx_;
    StopCallback stopCallback_;
    PauseCallback pauseCallback_;
    ContinueCallback continueCallback_;
    ShutdownCallback shutdownCallback_;
    ParamChangeCallback paramChangeCallback_;
    NetbindAddCallback netbindAddCallback_;
    NetbindRemoveCallback netbindRemoveCallback_;
    NetbindEnableCallback netbindEnableCallback_;
    NetbindDisableCallback netbindDisableCallback_;
    DeviceEventCallback deviceEventCallback_;
    HardwareProfileChangeCallback hardwareProfileChangeCallback_;
    PowerEventCallback powerEventCallback_;
    SessionChangeCallback sessionChangeCallback_;
#ifdef SERVICE_CONTROL_PRESHUTDOWN
    PreShutdownCallback preShutdownCallback_;
#endif
#ifdef SERVICE_CONTROL_TIMECHANGE
    TimeChangeCallback timeChangeCallback_;
#endif
#ifdef SERVICE_CONTROL_TRIGGEREVENT
    TriggerEventallback triggerEventCallback_;
#endif
    ErrorCallback errorCallback_;

    std::map<DWORD, OtherCallback> otherCallbackMap_;
    std::map<DWORD, OtherCallbackEx> otherCallbackExMap_;

    volatile SERVICE_STATUS_HANDLE serviceStatusHandle_;
    volatile HWND hWndConsoleService_;

    HANDLE hStopedEvent_;
    HANDLE hCleanupCompleteEvent_;

    SERVICE_STATUS serviceStatus_;

    HDEVNOTIFY deviceNotifyHandle_;

    const BasicServiceConfig<StringType> &Config_;
};

template <const ServiceConfig &Config> class Service : public BasicService<String, Config>
{
  public:
    typedef String StringType;
    typedef typename StringType::value_type CharType;

    static Service &Instance()
    {
        return (Service &)BasicService<StringType, Config>::Instance();
    }

  private:
    Service() : BasicService<StringType, Config>()
    {
    }
};

template <const ServiceConfigW &Config> class ServiceW : public BasicService<StringW, Config>
{
  public:
    typedef StringW StringType;
    typedef typename StringType::value_type CharType;

    static ServiceW &Instance()
    {
        return (ServiceW &)BasicService<StringType, Config>::Instance();
    }

  private:
    ServiceW() : BasicService<StringType, Config>()
    {
    }
};

template <const ServiceConfigT &Config> class ServiceT : public BasicService<StringT, Config>
{
  public:
    typedef StringT StringType;
    typedef typename StringType::value_type CharType;

    static ServiceT &Instance()
    {
        return (ServiceT &)BasicService<StringType, Config>::Instance();
    }

  private:
    ServiceT() : BasicService<StringType, Config>()
    {
    }
};
} // namespace System
} // namespace Win32Ex

#undef _STD_NS_

#endif // _WIN32EX_SYSTEM_SERVICE_HPP_