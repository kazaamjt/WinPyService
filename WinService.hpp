#pragma once
#include <string>
#include <Windows.h>

using std::string;

class WindowsService {
private:
	string name;
	LPSTR Wname;
	bool canPauseContinue;

public:
	WindowsService(string, bool);
};

WindowsService::WindowsService(string _name, bool _canPauseContinue):
	name(_name), canPauseContinue(_canPauseContinue)
	{
	Wname = const_cast<char*>(name.c_str());
}
