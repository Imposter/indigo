/*
*   This file is part of the Indigo library.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "Utility/Console.hpp"
#include "Platform.hpp"
#if !defined(OS_WIN)
#error "Unsupported platform!"
#endif

#include <Windows.h>
#include <string>

namespace indigo
{
	bool Console::mAllocated;

	void Console::Show(std::string title)
	{
		if (!mAllocated)
		{
			AllocConsole();
			AttachConsole(GetCurrentProcessId());

			FILE *stream;
			freopen_s(&stream, "CONIN$", "r", stdin);
			freopen_s(&stream, "CONOUT$", "w", stdout);
			freopen_s(&stream, "CONOUT$", "w", stderr);

			mAllocated = true;
		}

		SetConsoleTitleA(title.c_str());

		ShowWindow(GetConsoleWindow(), SW_SHOW);
	}

	void Console::Hide()
	{
		ShowWindow(GetConsoleWindow(), SW_HIDE);
		FreeConsole();
	}
}
