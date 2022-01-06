/*++

Copyright (c) Win32Ex Authors. All rights reserved.

Module Name:

    Service.hpp

Abstract:

    This Module implements the Service, ServiceW, ServiceT class.

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
#include "../Result.hpp"
#include "../Security/Privilege.hpp"
#include "../T/winsvc.hpp"
#include "../T/winuser.hpp"
#include "Process.hpp"

#if !defined(WIN32EX_DO_NOT_INCLUDE_MISSING_WINSVC_HEADER_DEFINITIONS)
#if defined(_WIN32)
#if !defined(SERVICE_CONTROL_PRESHUTDOWN)
#define SERVICE_CONTROL_PRESHUTDOWN 0x0000000F
#endif // !defined(SERVICE_CONTROL_PRESHUTDOWN)

#if !defined(SERVICE_CONTROL_TRIGGEREVENT)
#define SERVICE_CONTROL_TRIGGEREVENT 0x00000020
#endif // !defined(SERVICE_CONTROL_TRIGGEREVENT)

#if !defined(SERVICE_CONFIG_PRESHUTDOWN_INFO)
#define SERVICE_CONFIG_PRESHUTDOWN_INFO 7
//
// Service preshutdown timeout setting
//
typedef struct _SERVICE_PRESHUTDOWN_INFO
{
    DWORD dwPreshutdownTimeout; // Timeout in msecs
} SERVICE_PRESHUTDOWN_INFO, *LPSERVICE_PRESHUTDOWN_INFO;
#endif // !defined(SERVICE_CONFIG_PRESHUTDOWN_INFO)

#if !defined(SERVICE_CONTROL_TIMECHANGE)
#define SERVICE_CONTROL_TIMECHANGE 0x00000010
//
// Time change information
//
typedef struct _SERVICE_TIMECHANGE_INFO
{
    LARGE_INTEGER liNewTime; // New time
    LARGE_INTEGER liOldTime; // Old time
} SERVICE_TIMECHANGE_INFO, *PSERVICE_TIMECHANGE_INFO;
#endif // !defined(SERVICE_CONTROL_TIMECHANGE)

#if defined(SERVICE_CONFIG_TRIGGER_INFO)
#if !defined(SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT)
#define SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT 6
#endif // !defined(SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT)
#if !defined(SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE)
#define SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE 7
#endif // !defined(SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE)
#if !defined(SERVICE_TRIGGER_TYPE_AGGREGATE)
#define SERVICE_TRIGGER_TYPE_AGGREGATE 30
#endif // !defined(SERVICE_TRIGGER_TYPE_AGGREGATE)
#else  // !defined(SERVICE_CONFIG_TRIGGER_INFO)
#define SERVICE_CONFIG_TRIGGER_INFO 8

//
// Service trigger types
//
#define SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL 1
#define SERVICE_TRIGGER_TYPE_IP_ADDRESS_AVAILABILITY 2
#define SERVICE_TRIGGER_TYPE_DOMAIN_JOIN 3
#define SERVICE_TRIGGER_TYPE_FIREWALL_PORT_EVENT 4
#define SERVICE_TRIGGER_TYPE_GROUP_POLICY 5
#define SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT 6
#define SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE 7
#define SERVICE_TRIGGER_TYPE_CUSTOM 20
#define SERVICE_TRIGGER_TYPE_AGGREGATE 30

//
// Service trigger actions
//
#define SERVICE_TRIGGER_ACTION_SERVICE_START 1
#define SERVICE_TRIGGER_ACTION_SERVICE_STOP 2

//
// argv[1] passed into ServiceMain of trigger started services
//
#define SERVICE_TRIGGER_STARTED_ARGUMENT L"TriggerStarted"

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
#endif // !defined(SERVICE_CONFIG_TRIGGER_INFO)

#if !defined(SERVICE_CONFIG_PREFERRED_NODE)
#define SERVICE_CONFIG_PREFERRED_NODE 9
//
// Preferred node information
//
typedef struct _SERVICE_PREFERRED_NODE_INFO
{
    USHORT usPreferredNode; // Preferred node
    BOOLEAN fDelete;        // Delete the preferred node setting
} SERVICE_PREFERRED_NODE_INFO, *LPSERVICE_PREFERRED_NODE_INFO;
#endif // !defined(SERVICE_CONFIG_PREFERRED_NODE)

#if !defined(SERVICE_CONFIG_LAUNCH_PROTECTED)
#define SERVICE_CONFIG_LAUNCH_PROTECTED 12

//
// Service LaunchProtected types supported
//
#define SERVICE_LAUNCH_PROTECTED_NONE 0
#define SERVICE_LAUNCH_PROTECTED_WINDOWS 1
#define SERVICE_LAUNCH_PROTECTED_WINDOWS_LIGHT 2
#define SERVICE_LAUNCH_PROTECTED_ANTIMALWARE_LIGHT 3

//
// Service launch protected setting
//
typedef struct _SERVICE_LAUNCH_PROTECTED_INFO
{
    DWORD dwLaunchProtected; // Service launch protected
} SERVICE_LAUNCH_PROTECTED_INFO, *PSERVICE_LAUNCH_PROTECTED_INFO;
#endif // !defined(SERVICE_CONFIG_LAUNCH_PROTECTED)
#endif // defined(_WIN32)
#if defined(_INC__MINGW_H)
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

//
// Preferred node information
//
typedef struct _SERVICE_PREFERRED_NODE_INFO
{
    USHORT usPreferredNode; // Preferred node
    BOOLEAN fDelete;        // Delete the preferred node setting
} SERVICE_PREFERRED_NODE_INFO, *LPSERVICE_PREFERRED_NODE_INFO;

//
// Service LaunchProtected types supported
//
#define SERVICE_LAUNCH_PROTECTED_NONE 0
#define SERVICE_LAUNCH_PROTECTED_WINDOWS 1
#define SERVICE_LAUNCH_PROTECTED_WINDOWS_LIGHT 2
#define SERVICE_LAUNCH_PROTECTED_ANTIMALWARE_LIGHT 3

//
// Service launch protected setting
//
typedef struct _SERVICE_LAUNCH_PROTECTED_INFO
{
    DWORD dwLaunchProtected; // Service launch protected
} SERVICE_LAUNCH_PROTECTED_INFO, *PSERVICE_LAUNCH_PROTECTED_INFO;

//
// Service trigger actions
//
#define SERVICE_TRIGGER_ACTION_SERVICE_START 1
#define SERVICE_TRIGGER_ACTION_SERVICE_STOP 2

//
// Service trigger types
//
#define SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL 1
#define SERVICE_TRIGGER_TYPE_IP_ADDRESS_AVAILABILITY 2
#define SERVICE_TRIGGER_TYPE_DOMAIN_JOIN 3
#define SERVICE_TRIGGER_TYPE_FIREWALL_PORT_EVENT 4
#define SERVICE_TRIGGER_TYPE_GROUP_POLICY 5
#define SERVICE_TRIGGER_TYPE_NETWORK_ENDPOINT 6
#define SERVICE_TRIGGER_TYPE_CUSTOM_SYSTEM_STATE_CHANGE 7
#define SERVICE_TRIGGER_TYPE_CUSTOM 20
#define SERVICE_TRIGGER_TYPE_AGGREGATE 30
#endif // defined(_INC__MINGW_H)
#endif // defined(WIN32EX_DO_NOT_INCLUDE_MISSING_WINSVC_HEADER_DEFINITIONS)

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
namespace Service
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

class Handle
{
  public:
    Handle(SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT))
        : hSCManager(hSCManager), hService(NULL)
    {
    }

    ~Handle()
    {
        Close();
    }

    template <typename CharType> bool Open(CONST CharType *lpServiceName, DWORD dwDesiredAccess = SERVICE_QUERY_CONFIG)
    {
        if (hSCManager == NULL)
            hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);

        if (hSCManager == NULL)
            return false;

        if (hService)
            CloseServiceHandle(hService);
        hService = OpenServiceT<CharType>(hSCManager, lpServiceName, dwDesiredAccess);
        return hService != NULL;
    }

    void Close()
    {
        if (hService)
        {
            CloseServiceHandle(hService);
            hService = NULL;
        }
        if (hSCManager)
        {
            CloseServiceHandle(hSCManager);
            hSCManager = NULL;
        }
    }

    bool Attach(SC_HANDLE ServiceHandle)
    {
        if (ServiceHandle == NULL)
            return false;

        if (hService)
            CloseServiceHandle(hService);

        hService = ServiceHandle;
        return true;
    }

    operator SC_HANDLE() const
    {
        return hService;
    }

    bool IsValid() const
    {
        return hSCManager != NULL;
    }

  public:
    SC_HANDLE hSCManager;
    SC_HANDLE hService;
};
} // namespace Service
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

template <class _StringType = StringT> class ServiceT
{
  public:
    typedef _StringType StringType;
    typedef typename StringType::value_type CharType;

    typedef std::function<bool()> AcceptStopCallback;
    typedef std::function<bool()> AcceptPauseCallback;

    ServiceT(const StringType &Name, const Optional<const StringType &> &DisplayName = None(),
             const Optional<const StringType &> &Description = None())
        : name_(Name)
    {
        if (DisplayName.IsSome())
            displayName_ = DisplayName;

        if (Description.IsSome())
            description_ = Description;

        Details::Service::Handle handle;
        if (!handle.Open<CharType>(name_.c_str()))
            return;

        SharedPtr<QUERY_SERVICE_CONFIGT<CharType>> config = QueryConfig_(handle);
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
                Convert::MultiSzToList<ServiceT>(config->lpDependencies, dependencies_);
            if (config->lpServiceStartName)
                serviceStartName_ = config->lpServiceStartName;
            displayName_ = config->lpDisplayName;
        }

        SharedPtr<SERVICE_DESCRIPTIONT<CharType>> desc = QueryConfig2_<SERVICE_DESCRIPTIONT<CharType>>(handle);
        if (desc && desc->lpDescription)
            description_ = desc->lpDescription;
    }

    bool Install(DWORD ServiceType = SERVICE_WIN32_OWN_PROCESS, DWORD StartType = SERVICE_AUTO_START,
                 const Optional<const StringType &> BinaryPathName = None(),
                 const Optional<const StringType &> DisplayName = None(),
                 const Optional<const StringType &> Description = None(), DWORD ErrorControl = SERVICE_ERROR_IGNORE,
                 const Optional<const StringType &> LoadOrderGroup = None(), _Out_ PDWORD TagId = NULL,
                 const std::list<ServiceT> Dependencies = std::list<ServiceT>(),
                 const Optional<const StringType &> ServiceStartName = None(),
                 const Optional<const StringType &> Password = None())
    {
        Details::Service::Handle handle(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CREATE_SERVICE));
        if (handle.hSCManager == NULL)
            return false;

        if (BinaryPathName.IsSome())
            binaryPathName_ = BinaryPathName;

        if (DisplayName.IsSome())
            displayName_ = DisplayName;

        if (LoadOrderGroup.IsSome())
            loadOrderGroup_ = LoadOrderGroup;

        errorControl_ = ErrorControl;

        StringType dependencies;
        for (typename std::list<ServiceT>::const_iterator it = Dependencies.begin(); it != Dependencies.end(); ++it)
        {
            dependencies.append(it->Name().c_str());
            dependencies.push_back(0);
        }
        dependencies.push_back(0);

        if (!handle.Attach(CreateServiceT<CharType>(
                handle.hSCManager, name_.c_str(), displayName_.empty() ? NULL : displayName_.c_str(),
                SERVICE_CHANGE_CONFIG, ServiceType, StartType, ErrorControl, binaryPathName_.c_str(),
                loadOrderGroup_.empty() ? NULL : loadOrderGroup_.c_str(), TagId,
                dependencies.empty() ? NULL : dependencies.c_str(),
                ServiceStartName.IsSome() ? ServiceStartName.Get() : NULL, Password.IsSome() ? Password.Get() : NULL)))
            return false;

        serviceType_ = ServiceType;
        startType_ = StartType;
        dependencies_ = Dependencies;

        if (Description.IsSome())
            description_ = Description;

        if (!description_.empty())
        {
            SERVICE_DESCRIPTIONT<CharType> desc;
            desc.lpDescription = &description_[0];
            ChangeServiceConfig2T<CharType>(handle, SERVICE_CONFIG_DESCRIPTION, &desc);
        }

        return true;
    }

    bool Uninstall(Duration Timeout = Duration::Second(30))
    {
        Details::Service::Handle handle(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_ALL_ACCESS));
        if (handle.hSCManager == NULL)
            return false;

        if (!handle.Open(name_.c_str(), DELETE))
            return false;

        SERVICE_STATUS ss = {0};
        if (QueryServiceStatus(ss) && (ss.dwCurrentState != SERVICE_STOPPED) && (!Stop(Timeout)))
            return false;

        if (DeleteService(handle))
        {
            handle.Close();
            if (!handle.Open(name_.c_str(), DELETE))
                return true;

            if (DeleteService(handle.hService))
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

        Details::Service::Handle handle;
        if (handle.hSCManager == NULL)
            return false;

        if (!handle.Open(name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_USER_DEFINED_CONTROL))
            return false;

        return ControlService(handle.hService, ControlCode, &ss) == TRUE;
    }

    bool Start(Duration Timeout = Duration::Second(30))
    {
        SERVICE_STATUS ss = {0};
        if (!QueryServiceStatus(ss))
            return false;

        if (ss.dwCurrentState == SERVICE_RUNNING)
            return true;

        Details::Service::Handle handle;
        if (handle.hSCManager == NULL)
            return false;

        if (!handle.Open(name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_START))
            return false;

        if (!StartServiceA(handle.hService, 0, NULL))
            return false;

        return Wait(SERVICE_RUNNING, Timeout);
    }

    bool Stop(Duration Timeout = Duration::Second(30))
    {
        HANDLE stopEvent = Details::Service::OpenStopEvent<StringType>(name_);
        if (stopEvent)
        {
            BOOL ret = SetEvent(stopEvent);
            CloseHandle(stopEvent);
            return ret == TRUE;
        }

        SERVICE_STATUS ss = {0};
        if (!QueryServiceStatus(ss))
            return false;

        if (ss.dwCurrentState == SERVICE_STOPPED)
            return true;

        Details::Service::Handle handle;
        if (handle.hSCManager == NULL)
            return false;

        if (!handle.Open(name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_STOP))
            return false;

        if (acceptStopCallback_ && (!acceptStopCallback_()))
            return false;

        if (!ControlService(handle.hService, SERVICE_CONTROL_STOP, &ss))
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

        Details::Service::Handle handle;
        if (handle.hSCManager == NULL)
            return false;

        if (!handle.Open(name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_PAUSE_CONTINUE))
            return false;

        if (acceptPauseCallback_ && (!acceptPauseCallback_()))
            return false;

        if (!ControlService(handle.hService, SERVICE_CONTROL_PAUSE, &ss))
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

        Details::Service::Handle handle;
        if (handle.hSCManager == NULL)
            return false;

        if (!handle.Open(name_.c_str(), SERVICE_QUERY_STATUS | SERVICE_PAUSE_CONTINUE))
            return false;

        if (!ControlService(handle.hService, SERVICE_CONTROL_CONTINUE, &ss))
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
        Details::Service::Handle handle;
        if (handle.hSCManager == NULL)
            return false;

        return handle.Open(name_.c_str(), SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS);
    }

    bool QueryServiceStatus(SERVICE_STATUS &ss) const
    {
        Details::Service::Handle handle(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_CONNECT));
        if (handle.hSCManager == NULL)
            return false;

        if (!handle.Open(name_.c_str(), SERVICE_INTERROGATE))
            return false;

        if (ControlService(handle.hService, SERVICE_CONTROL_INTERROGATE, &ss) == FALSE)
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

    ServiceT &SetAcceptStop(AcceptStopCallback Callback)
    {
        acceptStopCallback_ = Callback;
        return *this;
    }

    ServiceT &SetAcceptPause(AcceptPauseCallback Callback)
    {
        acceptPauseCallback_ = Callback;
        return *this;
    }

    const StringType &Name() const
    {
        return name_;
    }

    const StringType &DisplayName() const
    {
        return displayName_;
    }

    const StringType &Description() const
    {
        return description_;
    }

    DWORD ServiceType() const
    {
        return serviceType_;
    }

    const StringType &BinaryPathName() const
    {
        if (binaryPathName_.empty())
            binaryPathName_ = ThisProcess::ExecutablePathT<StringType>();

        return binaryPathName_;
    }

    const std::list<ServiceT> &Dependencies() const
    {
        return dependencies_;
    }

    Result<const std::list<StringType> &> RequiredPrivileges() const
    {
        Details::Service::Handle handle;
        if (!handle.Open(name_.c_str()))
            return Error();

        SharedPtr<SERVICE_REQUIRED_PRIVILEGES_INFOT<CharType>> requredPrivsInfo =
            QueryConfig2_<SERVICE_REQUIRED_PRIVILEGES_INFOT<CharType>>(handle.hService);
        if (!requredPrivsInfo)
            return Error();

        if (requredPrivsInfo->pmszRequiredPrivileges != NULL && requiredPrivileges_.empty())
            Convert::MultiSzToList<StringType>(requredPrivsInfo->pmszRequiredPrivileges, requiredPrivileges_);

        return requiredPrivileges_;
    }

    Result<std::list<ServiceT>> DependentServices(DWORD ServiceState = SERVICE_STATE_ALL) const
    {
        Details::Service::Handle handle;
        if (!handle.Open(name_.c_str(), SERVICE_ENUMERATE_DEPENDENTS))
            return Error();

        DWORD bytesNeeded = 0;
        DWORD count = 0;
        EnumDependentServicesT<CharType>(handle.hService, ServiceState, NULL, bytesNeeded, &bytesNeeded, &count);
        if (GetLastError() != ERROR_MORE_DATA)
            return Error();

        SharedPtr<ENUM_SERVICE_STATUST<CharType>> dependencies((ENUM_SERVICE_STATUST<CharType> *)new BYTE[bytesNeeded]);
        if (dependencies == NULL)
            return Error();

        if (!EnumDependentServicesT<CharType>(handle.hService, ServiceState, dependencies.get(), bytesNeeded,
                                              &bytesNeeded, &count))
            return Error();

        std::list<ServiceT> deps;
        for (DWORD i = 0; i < count; ++i)
            deps.push_back(ServiceT(dependencies.get()[i].lpServiceName));

        return deps;
    }

    Result<SharedPtr<typename SERVICE_FAILURE_ACTIONST<CharType>::Type>> FailureActions() const
    {
        Details::Service::Handle handle;
        if (!handle.Open(name_.c_str(), SERVICE_QUERY_CONFIG))
            return Error();

        SharedPtr<typename SERVICE_FAILURE_ACTIONST<CharType>::Type> failureActions =
            QueryConfig2_<typename SERVICE_FAILURE_ACTIONST<CharType>::Type>(handle.hService);
        if (!failureActions)
            return Error();

        return failureActions;
    }

    bool FailureActions(const SERVICE_FAILURE_ACTIONST<CharType> &FailureActions)
    {
        Details::Service::Handle handle;
        return FailureActions_(handle, FailureActions);
    }

    bool FailureActions(const SERVICE_FAILURE_ACTIONST<CharType> &FailureActions, SERVICE_FAILURE_ACTIONS_FLAG Flags)
    {
        Details::Service::Handle handle;

        if (!FailureActions_(handle, FailureActions))
            return false;

        return ChangeServiceConfig2T<CharType>(handle, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, (PVOID)&Flags) == TRUE;
    }
#if defined(SERVICE_CONFIG_TRIGGER_INFO)
    Result<SharedPtr<SERVICE_TRIGGER_INFO>> Trigger() const
    {
        Details::Service::Handle handle;
        if (!handle.Open(name_.c_str(), SERVICE_QUERY_CONFIG))
            return Error();

        SharedPtr<SERVICE_TRIGGER_INFO> info = QueryConfig2_<SERVICE_TRIGGER_INFO>(handle);
        if (!info)
            return Error();
        return info;
    }

    bool Trigger(const SERVICE_TRIGGER_INFO &TriggerInfo)
    {
        Details::Service::Handle handle;
        if (!handle.Open(name_.c_str(), SERVICE_CHANGE_CONFIG))
            return false;

        return ChangeServiceConfig2T<CharType>(handle, SERVICE_CONFIG_TRIGGER_INFO, (PVOID)&TriggerInfo) == TRUE;
    }
#endif
#ifdef SERVICE_CONFIG_PRESHUTDOWN_INFO
    Result<SERVICE_PRESHUTDOWN_INFO> PreshutdownTimeout() const
    {
        Details::Service::Handle handle;
        if (!handle.Open(name_.c_str(), SERVICE_QUERY_CONFIG))
            return Error();

        SharedPtr<SERVICE_PRESHUTDOWN_INFO> info = QueryConfig2_<SERVICE_PRESHUTDOWN_INFO>(handle.hService);
        if (!info)
            return Error();

        return *info;
    }

    bool PreshutdownTimeout(Duration PreshutdownTimeout)
    {
        Details::Service::Handle handle;
        if (!handle.Open(name_.c_str(), SERVICE_CHANGE_CONFIG))
            return false;

        SERVICE_PRESHUTDOWN_INFO info = {PreshutdownTimeout};
        return ChangeServiceConfig2T<CharType>(handle, SERVICE_CONFIG_PRESHUTDOWN_INFO, &info) == TRUE;
    }
#endif

  private:
    bool FailureActions_(const Details::Service::Handle &Handle,
                         const SERVICE_FAILURE_ACTIONST<CharType> &FailureActions)
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

        Details::Service::Handle handle;
        if (!handle.Open(name_.c_str(), SERVICE_CHANGE_CONFIG))
            return false;

        if (needToAcquireShutdownPrivilege)
        {
            Security::TokenPrivileges privileges(Security::SeShutdownPrivilege);
            return ChangeServiceConfig2T<CharType>(handle, SERVICE_CONFIG_FAILURE_ACTIONS, (PVOID)&FailureActions) ==
                   TRUE;
        }
        return ChangeServiceConfig2T<CharType>(handle, SERVICE_CONFIG_FAILURE_ACTIONS, (PVOID)&FailureActions) == TRUE;
    }

    template <typename T> SharedPtr<T> QueryConfig2_(SC_HANDLE hService) const
    {
        typedef SharedPtr<T> result_type;

        if (hService == NULL)
            return result_type();

        DWORD infoLevel;
        if (typeid(T) == typeid(SERVICE_DELAYED_AUTO_START_INFO))
            infoLevel = SERVICE_CONFIG_DELAYED_AUTO_START_INFO;
        else if ((typeid(T) == typeid(SERVICE_DESCRIPTIONT<CharType>)) ||
                 (typeid(T) == typeid(typename SERVICE_DESCRIPTIONT<CharType>::Type)))
            infoLevel = SERVICE_CONFIG_DESCRIPTION;
        else if ((typeid(T) == typeid(SERVICE_FAILURE_ACTIONST<CharType>)) ||
                 (typeid(T) == typeid(typename SERVICE_FAILURE_ACTIONST<CharType>::Type)))
            infoLevel = SERVICE_CONFIG_FAILURE_ACTIONS;
        else if (typeid(T) == typeid(SERVICE_FAILURE_ACTIONS_FLAG))
            infoLevel = SERVICE_CONFIG_FAILURE_ACTIONS_FLAG;
#if defined(SERVICE_CONFIG_PREFERRED_NODE)
        else if (typeid(T) == typeid(SERVICE_PREFERRED_NODE_INFO))
            infoLevel = SERVICE_CONFIG_PREFERRED_NODE;
#endif
#if defined(SERVICE_CONFIG_PRESHUTDOWN_INFO)
        else if (typeid(T) == typeid(SERVICE_PRESHUTDOWN_INFO))
            infoLevel = SERVICE_CONFIG_PRESHUTDOWN_INFO;
#endif
        else if ((typeid(T) == typeid(SERVICE_REQUIRED_PRIVILEGES_INFOT<CharType>)) ||
                 (typeid(T) == typeid(typename SERVICE_REQUIRED_PRIVILEGES_INFOT<CharType>::Type)))
            infoLevel = SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO;
        else if (typeid(T) == typeid(SERVICE_SID_INFO))
            infoLevel = SERVICE_CONFIG_SERVICE_SID_INFO;
#if defined(SERVICE_CONFIG_TRIGGER_INFO)
        else if (typeid(T) == typeid(SERVICE_TRIGGER_INFO))
            infoLevel = SERVICE_CONFIG_TRIGGER_INFO;
#endif
#if defined(SERVICE_CONFIG_LAUNCH_PROTECTED)
        else if (typeid(T) == typeid(SERVICE_LAUNCH_PROTECTED_INFO))
            infoLevel = SERVICE_CONFIG_LAUNCH_PROTECTED;
#endif
        else
            return result_type();

        DWORD bytesNeeded;
        QueryServiceConfig2T<CharType>(hService, infoLevel, NULL, 0, &bytesNeeded);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return result_type();

        result_type lpsc = result_type((T *)new BYTE[bytesNeeded]);
        if (lpsc == NULL)
            return result_type();

        return QueryServiceConfig2T<CharType>(hService, infoLevel, (LPBYTE)lpsc.get(), bytesNeeded, &bytesNeeded)
                   ? lpsc
                   : result_type();
    }

    SharedPtr<QUERY_SERVICE_CONFIGT<CharType>> QueryConfig_(SC_HANDLE hService) const
    {
        typedef SharedPtr<QUERY_SERVICE_CONFIGT<CharType>> result_type;

        if (hService == NULL)
            return result_type();

        DWORD bytesNeeded;
        QueryServiceConfigT<CharType>(hService, NULL, 0, &bytesNeeded);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return result_type();

        result_type lpsc = result_type((QUERY_SERVICE_CONFIGT<CharType> *)new BYTE[bytesNeeded]);
        if (lpsc == NULL)
            return result_type();

        return QueryServiceConfigT<CharType>(hService, lpsc.get(), bytesNeeded, &bytesNeeded) ? lpsc : result_type();
    }

  private:
    StringType name_;
    DWORD serviceType_;
    DWORD startType_;
    DWORD errorControl_;
    mutable StringType binaryPathName_;
    StringType loadOrderGroup_;
    DWORD TagId_;
    mutable std::list<ServiceT> dependencies_;
    StringType serviceStartName_;
    StringType displayName_;
    StringType description_;
    mutable std::list<StringType> requiredPrivileges_;
    AcceptStopCallback acceptStopCallback_;
    AcceptPauseCallback acceptPauseCallback_;

  public:
    template <const ServiceT<_StringType> &Service> class Instance
    {
        friend class ServiceT<String>;
        friend class ServiceT<StringW>;
        friend class ServiceT<StringT>;

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
        Instance &On(DWORD Control, OtherCallback Callback)
        {
            otherCallbackMap_[Control] = Callback;
            return *this;
        }

        Instance &OnEx(DWORD Control, OtherCallbackEx Callback)
        {
            otherCallbackExMap_[Control] = Callback;
            return *this;
        }

        Instance &OnStart(StartCallback Callback)
        {
            startCallback_ = Callback;
            return *this;
        }

        Instance &OnStartEx(StartCallbackEx Callback)
        {
            startCallbackEx_ = Callback;
            return *this;
        }

        Instance &OnStop(StopCallback Callback = NULL)
        {
            stopCallback_ = Callback;

            if (stopCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_STOP);
            else
                ClearControlsAccepted(SERVICE_ACCEPT_STOP);

            return *this;
        }

        Instance &OnPause(PauseCallback Callback = NULL)
        {
            pauseCallback_ = Callback;

            if (pauseCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
            else if (continueCallback_ == NULL)
                ClearControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);

            return *this;
        }

        Instance &OnContinue(ContinueCallback Callback = NULL)
        {
            continueCallback_ = Callback;

            if (continueCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);
            else if (pauseCallback_ == NULL)
                ClearControlsAccepted(SERVICE_ACCEPT_PAUSE_CONTINUE);

            return *this;
        }

        Instance &OnShutdown(ShutdownCallback Callback = NULL)
        {
            shutdownCallback_ = Callback;

            if (shutdownCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_SHUTDOWN);
            else
                ClearControlsAccepted(SERVICE_ACCEPT_SHUTDOWN);

            return *this;
        }

        Instance &OnParamChange(ParamChangeCallback Callback = NULL)
        {
            paramChangeCallback_ = Callback;

            if (paramChangeCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_PARAMCHANGE);
            else
                ClearControlsAccepted(SERVICE_ACCEPT_PARAMCHANGE);

            return *this;
        }

        Instance &OnNetbindAdd(NetbindAddCallback Callback = NULL)
        {
            netbindAddCallback_ = Callback;

            if (netbindAddCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);
            else if ((netbindRemoveCallback_ == NULL) && (netbindEnableCallback_ == NULL) &&
                     (netbindDisableCallback_ == NULL))
                ClearControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);

            return *this;
        }

        Instance &OnNetbindRemove(NetbindAddCallback Callback = NULL)
        {
            netbindRemoveCallback_ = Callback;

            if (netbindRemoveCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);
            else if ((netbindAddCallback_ == NULL) && (netbindEnableCallback_ == NULL) &&
                     (netbindDisableCallback_ == NULL))
                ClearControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);

            return *this;
        }

        Instance &OnNetbindEnable(NetbindAddCallback Callback = NULL)
        {
            netbindEnableCallback_ = Callback;

            if (netbindEnableCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);
            else if ((netbindAddCallback_ == NULL) && (netbindRemoveCallback_ == NULL) &&
                     (netbindDisableCallback_ == NULL))
                ClearControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);

            return *this;
        }

        Instance &OnNetbindDisable(NetbindAddCallback Callback = NULL)
        {
            netbindDisableCallback_ = Callback;

            if (netbindDisableCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);
            else if ((netbindAddCallback_ == NULL) && (netbindRemoveCallback_ == NULL) &&
                     (netbindEnableCallback_ == NULL))
                ClearControlsAccepted(SERVICE_ACCEPT_NETBINDCHANGE);

            return *this;
        }

        Instance &OnDeviceEvent(PVOID NotificationFilter = NULL, DeviceEventCallback Callback = NULL)
        {
            return OnDeviceEventT<CharType>(NotificationFilter, Callback);
        }

        template <typename T>
        Instance &OnDeviceEventT(PVOID NotificationFilter = NULL, DeviceEventCallback Callback = NULL)
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

        Instance &OnHardwareProfileChange(HardwareProfileChangeCallback Callback = NULL)
        {
            hardwareProfileChangeCallback_ = Callback;

            if (hardwareProfileChangeCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_HARDWAREPROFILECHANGE);
            else
                ClearControlsAccepted(SERVICE_ACCEPT_HARDWAREPROFILECHANGE);

            return *this;
        }

        Instance &OnPowerEvent(PowerEventCallback Callback = NULL)
        {
            powerEventCallback_ = Callback;

            if (powerEventCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_POWEREVENT);
            else
                ClearControlsAccepted(SERVICE_ACCEPT_POWEREVENT);

            return *this;
        }

        Instance &OnSessionChanage(SessionChangeCallback Callback = NULL)
        {
            sessionChangeCallback_ = Callback;

            if (sessionChangeCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_SESSIONCHANGE);
            else
                ClearControlsAccepted(SERVICE_ACCEPT_SESSIONCHANGE);

            return *this;
        }
#ifdef SERVICE_CONTROL_PRESHUTDOWN
        Instance &OnPreShutdown(Duration PreshutdownTimeout, PreShutdownCallback Callback = NULL)
        {
            Service.SetPreshutdownTimeout(PreshutdownTimeout);
            return OnPreShutdown(Callback);
        }

        Instance &OnPreShutdown(PreShutdownCallback Callback = NULL)
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
        Instance &OnTimeChange(TimeChangeCallback Callback = NULL)
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
        Instance &OnTriggerEvent(const SERVICE_TRIGGER_INFO &TriggerInfo, TriggerEventallback Callback = NULL)
        {
            Service.SetTrigger(TriggerInfo);
            return OnTriggerEvent(Callback);
        }

        Instance &OnTriggerEvent(TriggerEventallback Callback = NULL)
        {
            triggerEventCallback_ = Callback;

            if (triggerEventCallback_)
                SetControlsAccepted(SERVICE_ACCEPT_TRIGGEREVENT);
            else
                ClearControlsAccepted(SERVICE_ACCEPT_TRIGGEREVENT);

            return *this;
        }
#endif
        Instance &OnError(ErrorCallback Callback)
        {
            errorCallback_ = Callback;
            return *this;
        }

        bool SetWin32ExitCode(DWORD Win32ExitCode)
        {
            serviceStatus_.dwServiceType = Service.ServiceType();
            serviceStatus_.dwWin32ExitCode = Win32ExitCode;
            return UpdateServiceStatus_();
        }

        bool SetServiceSpecificExitCode(DWORD ServiceSpecificExitCode)
        {
            serviceStatus_.dwServiceType = Service.ServiceType();
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
            serviceStatus_.dwServiceType = Service.ServiceType();
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
                    {(CharType *)Service.Name().c_str(),
                     (typename LPSERVICE_MAIN_FUNCTIONT<CharType>::Type)ServiceMain_},
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
        Instance()
            : serviceStatusHandle_(NULL), deviceNotifyHandle_(NULL), hCleanupCompleteEvent_(NULL), hStopedEvent_(NULL),
              hWndConsoleService_(NULL), service_(Service)
        {
            ZeroMemory(&serviceStatus_, sizeof(serviceStatus_));
        }

        ~Instance()
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
        static Instance &Get()
        {
            static Instance service;
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
            CONSOLE_WND_THREAD_PARAM(Instance &Instance, HINSTANCE hInstance, PCSTR ClassName)
                : Instance(Instance), hWnd(NULL), hInstance(hInstance), ClassName(ClassName),
                  hEventWndCreatedOrFailed(NULL)
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
            Instance &Instance;
        };

        static DWORD WINAPI ConsoleWndThreadProc_(LPVOID lpThreadParameter)
        {
            CONSOLE_WND_THREAD_PARAM *param = (CONSOLE_WND_THREAD_PARAM *)lpThreadParameter;

            DWORD errorCode;
            Instance &instance = param->Instance;

            HWND hWndSvc =
                CreateWindowExA(0, param->ClassName, "ConsoleServiceWindow", 0, 0, 0, 0, 0, 0, 0, param->hInstance, 0);

            if (hWndSvc == NULL)
            {
                errorCode = GetLastError();
                instance.RaiseError_(errorCode, "Failed to CreateWindowExA");
                SetEvent(param->hEventWndCreatedOrFailed);
                return GetLastError();
            }

            if (!WTSRegisterSessionNotification(hWndSvc, NOTIFY_FOR_ALL_SESSIONS))
            {
                errorCode = GetLastError();
                DestroyWindow(hWndSvc);
                instance.RaiseError_(errorCode, "Failed to WTSRegisterSessionNotification");
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
            Instance &service = Get();

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
            Instance &instance = Get();

            instance.hCleanupCompleteEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (instance.hCleanupCompleteEvent_ == NULL)
            {
                instance.RaiseError_(GetLastError(), "Failed to CreateEventW");
                CloseHandle(instance.hStopedEvent_);
                return;
            }

            if (IsServiceMode())
            {
                instance.hStopedEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
                if (instance.hStopedEvent_ == NULL)
                {
                    instance.RaiseError_(GetLastError(), "Failed to CreateEventW");
                    return;
                }

                instance.serviceStatusHandle_ = RegisterServiceCtrlHandlerExT<CharType>(
                    (CharType *)Service.Name().c_str(), ServiceHandlerEx_, &instance);

                if (!instance.serviceStatusHandle_)
                {
                    errorCode = GetLastError();
                    CloseHandle(instance.hStopedEvent_);
                    instance.RaiseError_(GetLastError(), "Failed to RegisterServiceCtrlHandlerExW");
                    instance.Cleanup_();
                    return;
                }

                instance.SetStatus(SERVICE_START_PENDING);

                if (instance.startCallbackEx_)
                {
                    errorCode = instance.startCallbackEx_(dwNumServicesArgs, lpServiceArgVectors);
                    if (errorCode != ERROR_SUCCESS)
                    {
                        instance.RaiseError_(errorCode, "Failed to start service.");
                        instance.SetStatus(SERVICE_STOPPED, errorCode);
                        instance.Cleanup_();
                        return;
                    }
                }
                else if (instance.startCallback_)
                {
                    instance.startCallback_();
                }

                instance.SetStatus(SERVICE_RUNNING);
                WaitForSingleObject(instance.hStopedEvent_, INFINITE);

                instance.Cleanup_();
                return;
            }
#ifdef TWIN32EX_USE_SERVICE_SIMULATE_MODE
            else
            {
                instance.hStopedEvent_ = Details::Service::CreateStopEvent(instance.service_.Name());
                if (instance.hStopedEvent_ == NULL)
                {
                    instance.RaiseError_(GetLastError(), "Failed to CreateEventW");
                    return;
                }

                if (!SetConsoleCtrlHandler(ConsoleHandlerRoutine_, TRUE))
                {
                    errorCode = GetLastError();
                    instance.RaiseError_(GetLastError(), "Failed to SetConsoleCtrlHandler");
                    instance.Cleanup_();
                    if (!IsServiceMode())
                        SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                    return;
                }

                using namespace Convert::String;

                std::string className = typeid(StringType) == typeid(String) ? (PCSTR)Service.Name().c_str()
                                                                             : (PCSTR)(!Service.Name()).c_str();
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
                    instance.RaiseError_(errorCode, "Failed to RegisterClassExA");
                    instance.Cleanup_();
                    if (!IsServiceMode())
                        SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                    return;
                }

                CONSOLE_WND_THREAD_PARAM param(instance, hInstance, className.c_str());

                param.hEventWndCreatedOrFailed = CreateEvent(NULL, FALSE, FALSE, NULL);

                if (!param.hEventWndCreatedOrFailed)
                {
                    errorCode = GetLastError();
                    UnregisterClassA(className.c_str(), hInstance);
                    instance.RaiseError_(errorCode, "Failed to CreateEvent");
                    instance.Cleanup_();
                    if (!IsServiceMode())
                        SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                    return;
                }

                HANDLE hThraed = CreateThread(NULL, 0, ConsoleWndThreadProc_, &param, 0, NULL);
                if (!hThraed)
                {
                    errorCode = GetLastError();
                    UnregisterClassA(className.c_str(), hInstance);
                    instance.RaiseError_(errorCode, "Failed to WTSRegisterSessionNotification");
                    instance.Cleanup_();
                    if (!IsServiceMode())
                        SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                    return;
                }

                instance.SetStatus(SERVICE_START_PENDING);
                WaitForSingleObject(param.hEventWndCreatedOrFailed, INFINITE);

                if (param.hWnd == NULL)
                {
                    instance.Cleanup_();
                    if (!IsServiceMode())
                        SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                    return;
                }

                instance.hWndConsoleService_ = param.hWnd;
                if (instance.startCallbackEx_)
                {
                    errorCode = instance.startCallbackEx_(dwNumServicesArgs, lpServiceArgVectors);
                    if (errorCode != ERROR_SUCCESS)
                    {
                        instance.RaiseError_(errorCode, "Failed to start service.");

                        instance.SetStatus(SERVICE_STOPPED, errorCode);
                        SendMessage(instance.hWndConsoleService_, WM_DESTROY, 0, 0);
                        WaitForSingleObject(hThraed, INFINITE);
                        CloseHandle(hThraed);

                        instance.Cleanup_();
                        if (!IsServiceMode())
                            SetConsoleCtrlHandler(ConsoleHandlerRoutine_, FALSE);
                        return;
                    }
                }
                else if (instance.startCallback_)
                {
                    instance.startCallback_();
                }

                instance.SetStatus(SERVICE_RUNNING);
                ResumeThread(hThraed);
                WaitForSingleObject(instance.hStopedEvent_, INFINITE);

                SendMessage(instance.hWndConsoleService_, WM_DESTROY, 0, 0);
                WaitForSingleObject(hThraed, INFINITE);
                CloseHandle(hThraed);
                UnregisterClassA(className.c_str(), hInstance);

                instance.Cleanup_();
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

        const ServiceT<StringType> &service_;
    };

  public:
    static std::list<ServiceT> All(DWORD ServiceType = SERVICE_DRIVER | SERVICE_WIN32,
                                   DWORD ServiceState = SERVICE_STATE_ALL, Optional<StringType> GroupName = None())
    {
        Details::Service::Handle handle(OpenSCManagerT<CharType>(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE));
        if (handle.hSCManager == NULL)
            return std::list<ServiceT>();

        const CharType *groupName = GroupName.IsSome() ? GroupName.Get() : NULL;
        DWORD bytesNeeded = 0;
        DWORD count = 0;
        EnumServicesStatusExT<CharType>(handle.hSCManager, SC_ENUM_PROCESS_INFO, ServiceType, ServiceState, NULL, 0,
                                        &bytesNeeded, &count, NULL, groupName);

        ENUM_SERVICE_STATUS_PROCESST<CharType> *services =
            (ENUM_SERVICE_STATUS_PROCESST<CharType> *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytesNeeded);
        if (services == NULL)
            return std::list<ServiceT>();

        std::list<ServiceT> all;
        DWORD resumeHandle = 0;
        while (EnumServicesStatusExT<CharType>(handle.hSCManager, SC_ENUM_PROCESS_INFO, ServiceType, ServiceState,
                                               (LPBYTE)services, bytesNeeded, &bytesNeeded, &count, &resumeHandle,
                                               groupName) == FALSE)
        {
            if (GetLastError() != ERROR_MORE_DATA)
                break;

            for (DWORD i = 0; i < count; ++i)
                all.push_back(ServiceT(services[i].lpServiceName, services[0].lpDisplayName));

            if (bytesNeeded == 0)
                break;

            services = (ENUM_SERVICE_STATUS_PROCESST<CharType> *)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                                                             services, bytesNeeded);
            count = 0;
            if (services == NULL)
                return std::list<ServiceT>();
        }

        for (DWORD i = 0; i < count; ++i)
            all.push_back(ServiceT(services[i].lpServiceName, services[0].lpDisplayName));

        HeapFree(GetProcessHeap(), 0, services);
        return all;
    }

#if defined(__cpp_variadic_templates)
    template <class... InstanceType> static bool Run()
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(typename InstanceType::CharType *)(InstanceType::Get()).service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType::CharType>::Type)InstanceType::ServiceMain_}...,
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }
    template <class... InstanceType> static bool Run(InstanceType &... instance)
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(typename InstanceType::CharType *)instance.service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType::CharType>::Type)InstanceType::ServiceMain_}...,
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }
#else
    template <typename InstanceType0, typename InstanceType1> static bool Run()
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(typename InstanceType0::CharType *)(InstanceType0::Get()).service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType0::CharType>::Type)InstanceType0::ServiceMain_},
            {(typename InstanceType1::CharType *)(InstanceType1::Get()).service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType1::CharType>::Type)InstanceType1::ServiceMain_},
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }
    template <typename InstanceType0, typename InstanceType1, typename InstanceType2> static bool Run()
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(typename InstanceType0::CharType *)(InstanceType0::Get()).service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType0::CharType>::Type)InstanceType0::ServiceMain_},
            {(typename InstanceType1::CharType *)(InstanceType1::Get()).service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType1::CharType>::Type)InstanceType1::ServiceMain_},
            {(typename InstanceType2::CharType *)(InstanceType2::Get()).service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType2::CharType>::Type)InstanceType2::ServiceMain_},
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }

    template <typename InstanceType0, typename InstanceType1>
    static bool Run(InstanceType0 &Instance0, InstanceType1 &Instance1)
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(typename InstanceType0::CharType *)Instance0.service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType0::CharType>::Type)InstanceType0::ServiceMain_},
            {(typename InstanceType1::CharType *)Instance1.service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType1::CharType>::Type)InstanceType1::ServiceMain_},
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }
    template <typename InstanceType0, typename InstanceType1, typename InstanceType2>
    static bool Run(InstanceType0 &Instance0, InstanceType1 &Instance1, InstanceType2 &Instance2)
    {
        typename SERVICE_TABLE_ENTRYT<CharType>::Type DispatchTable[] = {
            {(typename InstanceType0::CharType *)Instance0.service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType0::CharType>::Type)InstanceType0::ServiceMain_},
            {(typename InstanceType1::CharType *)Instance1.service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType1::CharType>::Type)InstanceType1::ServiceMain_},
            {(typename InstanceType2::CharType *)Instance2.service_.Name().c_str(),
             (typename LPSERVICE_MAIN_FUNCTIONT<typename InstanceType2::CharType>::Type)InstanceType2::ServiceMain_},
            {NULL, NULL}};

        return StartServiceCtrlDispatcherT<CharType>(DispatchTable) == TRUE;
    }
#endif
};

typedef ServiceT<String> Service;
typedef ServiceT<StringW> ServiceW;
} // namespace System
} // namespace Win32Ex

#undef _STD_NS_

#endif // _WIN32EX_SYSTEM_SERVICE_HPP_
