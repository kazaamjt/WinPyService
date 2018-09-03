/*
	Copyright (c) 2018 Yaron Vanhulst

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

	See LICENSE.MD for more info.
*/

#pragma once

#include <iostream>
#include <Windows.h>

//====== init =================================================
LPSTR NAME;
bool ENABLE_PAUSE_CONTINUE;

SERVICE_STATUS STATUS = { 0 };
SERVICE_STATUS_HANDLE STATUS_HANDLE = NULL;

HANDLE WORKER_PAUSED = INVALID_HANDLE_VALUE;
HANDLE WORKER_CONTINUED = INVALID_HANDLE_VALUE;

HANDLE STOP_EVENT = INVALID_HANDLE_VALUE;
HANDLE PAUSE_EVENT = INVALID_HANDLE_VALUE;
HANDLE CONTINUE_EVENT = INVALID_HANDLE_VALUE;

void WINAPI serviceMain();
void WINAPI controlHandler(DWORD);
void setAcceptedControls(bool);
void setState(DWORD);

typedef void (*simple_worker_callback) ();
void setSimpleWorker(simple_worker_callback);
simple_worker_callback simpleWorker;
void SimpleServiceWrapper();

typedef void (*simple_callback) ();

void startup();
void set_startup_callback(simple_callback);
simple_callback startup_callback;

void controlStop();
void controlPause();
void controlContinue();
void controlStopOnPause();

void confirmPause() { setState(SERVICE_PAUSED); }
void confirmContinue();

void exit();

//=============================================================

void c_init(const char* name, bool canPauseContinue) {
	NAME = const_cast<char*>(name);
	ENABLE_PAUSE_CONTINUE = canPauseContinue;
}

void setSimpleWorker(simple_worker_callback callback) {
	simpleWorker = callback;
}

bool registerService() {
	SERVICE_TABLE_ENTRY serviceTable[] = {
		{ NAME, (LPSERVICE_MAIN_FUNCTION)serviceMain },
		{ NULL, NULL }};

	if (StartServiceCtrlDispatcher(serviceTable) == FALSE) {
		DWORD serviceDispatchError = GetLastError();
		if (serviceDispatchError == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
			simpleWorker(); // replace with onFailure();
		} else {
			return false;
		}
	}

	return true;
}

void WINAPI serviceMain() {
	// Register service control handler with the SCM
	STATUS_HANDLE = RegisterServiceCtrlHandler(NAME, controlHandler);

	if (STATUS_HANDLE == NULL) {
		return;
	}

	startup();

	SimpleServiceWrapper();

	exit();
}

void WINAPI controlHandler(DWORD CtrlCode) {
	switch (CtrlCode) {
		case SERVICE_CONTROL_STOP: {
			if (STATUS.dwCurrentState == SERVICE_PAUSED) {
				controlStopOnPause();
				break;
			}

			if (STATUS.dwCurrentState != SERVICE_RUNNING) { break; }
			controlStop();
		} break;

		case SERVICE_CONTROL_PAUSE:{
			if (STATUS.dwCurrentState != SERVICE_RUNNING) { break; }
			controlPause();
		} break;

		case SERVICE_CONTROL_CONTINUE: {
			if (STATUS.dwCurrentState != SERVICE_PAUSED) { break; }
			controlContinue();
		}break;

		case SERVICE_CONTROL_SHUTDOWN: {
			if (STATUS.dwCurrentState != SERVICE_RUNNING) { break; }
			controlStop();
		} break;

		case SERVICE_CONTROL_INTERROGATE: { // Deprecated, but you never know... let's just handle it.
			SetServiceStatus(STATUS_HANDLE, &STATUS);
		} break;

		default: { } break;
	}
}

void startup() {
	ZeroMemory(&STATUS, sizeof(STATUS));
	STATUS.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	setAcceptedControls(false);
	STATUS.dwServiceSpecificExitCode = 0;
	setState(SERVICE_START_PENDING);

	// startup_callback();

	STOP_EVENT = CreateEvent(NULL, TRUE, FALSE, NULL);
	PAUSE_EVENT = CreateEvent(NULL, TRUE, FALSE, NULL);
	CONTINUE_EVENT = CreateEvent(NULL, TRUE, FALSE, NULL);

	setAcceptedControls(true);

	setState(SERVICE_RUNNING);
}

void SimpleServiceWrapper() {
	while (WaitForSingleObject(STOP_EVENT, 0) != WAIT_OBJECT_0) {
		// Pause on pauseEvent
		if (WaitForSingleObject(PAUSE_EVENT, 0) != WAIT_OBJECT_0) {
			simpleWorker();

		} else {
			confirmPause();
			// Wait for continue to be thrown
			if (WaitForSingleObject(CONTINUE_EVENT, 0) != WAIT_OBJECT_0) {
				
			} else {
				confirmContinue();
			}
		}
	}
}

void controlStop() {
	setState(SERVICE_STOP_PENDING);
	setAcceptedControls(false);
	SetEvent(STOP_EVENT);
}

void controlPause() {
	setState(SERVICE_PAUSE_PENDING);
	SetEvent(PAUSE_EVENT);
}

void controlContinue() {
	setState(SERVICE_CONTINUE_PENDING);
	setAcceptedControls(false);
	ResetEvent(PAUSE_EVENT);
	SetEvent(CONTINUE_EVENT);
}

void confirmContinue() {
	setAcceptedControls(true);
	ResetEvent(CONTINUE_EVENT);
	setState(SERVICE_RUNNING);
}

void controlStopOnPause() {
	setAcceptedControls(false);
	setState(SERVICE_STOP_PENDING);
	SetEvent(STOP_EVENT);
	SetEvent(CONTINUE_EVENT);
}

void exit() {
	// exitCallback();
	CloseHandle(STOP_EVENT);
	CloseHandle(PAUSE_EVENT);
	CloseHandle(CONTINUE_EVENT);
	CloseHandle(WORKER_PAUSED);
	CloseHandle(WORKER_CONTINUED);
	setState(SERVICE_STOPPED);
}

void setAcceptedControls(bool on) {
	if (on) {
		STATUS.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
		if (ENABLE_PAUSE_CONTINUE) {
			STATUS.dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
		}
	} else {
		STATUS.dwControlsAccepted = 0;
	}

	if (SetServiceStatus(STATUS_HANDLE, &STATUS) == FALSE) {
		// log failure
	}
}

void setState(DWORD state) {
	STATUS.dwCurrentState = state;
	STATUS.dwWin32ExitCode = 0;
	STATUS.dwCheckPoint = 0;
	STATUS.dwWaitHint = 0;

	if (SetServiceStatus(STATUS_HANDLE, &STATUS) == FALSE) {
		// log failure
	}
}
