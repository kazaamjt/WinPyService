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

HANDLE WORKER_PAUSE = INVALID_HANDLE_VALUE;
HANDLE WORKER_CONTINUE = INVALID_HANDLE_VALUE;

HANDLE STOP_EVENT = INVALID_HANDLE_VALUE;
HANDLE PAUSE_EVENT = INVALID_HANDLE_VALUE;
HANDLE CONTINUE_EVENT = INVALID_HANDLE_VALUE;

void WINAPI serviceMain();
void WINAPI controlHandler(DWORD);
void setAcceptedControls(bool);
void setState(DWORD);

void startup();
void set_startup();
void on_startup();

typedef void (*simple_worker_callback) ();
void setSimpleWorker(simple_worker_callback);
simple_worker_callback simple_worker;

//=============================================================

void c_init(const char* name, bool canPauseContinue) {
	NAME = const_cast<char*>(name);
	ENABLE_PAUSE_CONTINUE = canPauseContinue;
}

void setSimpleWorker(simple_worker_callback callback) {
	simple_worker = callback;
}

bool registerService() {
	SERVICE_TABLE_ENTRY serviceTable[] = {
		{ NAME, (LPSERVICE_MAIN_FUNCTION)serviceMain },
		{ NULL, NULL }};

	if (StartServiceCtrlDispatcher(serviceTable) == FALSE) {
		DWORD serviceDispatchError = GetLastError();
		if (serviceDispatchError == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
			simple_worker(); // replace with onFailure();
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

	simple_worker();

	// exit();
}

void WINAPI controlHandler(DWORD CtrlCode) {
	switch (CtrlCode) {
		case SERVICE_CONTROL_STOP: {
			if (STATUS.dwCurrentState != SERVICE_RUNNING) { break; }
			// control_stop();
		} break;

		case SERVICE_CONTROL_SHUTDOWN: {
			if (STATUS.dwCurrentState != SERVICE_RUNNING) { break; }
			// control_stop();
		} break;

		default: {

		} break;
	}
}

void startup() {
	ZeroMemory(&STATUS, sizeof(STATUS));
	STATUS.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	setAcceptedControls(false);
	STATUS.dwServiceSpecificExitCode = 0;
	setState(SERVICE_START_PENDING);

	// on_startup();

	STOP_EVENT = CreateEvent(NULL, TRUE, FALSE, NULL);
	PAUSE_EVENT = CreateEvent(NULL, TRUE, FALSE, NULL);
	CONTINUE_EVENT = CreateEvent(NULL, TRUE, FALSE, NULL);

	setAcceptedControls(true);

	setState(SERVICE_RUNNING);
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
