#include <stdio.h>
#include <time.h>
#include <string>
#include <filesystem>

#include <Windows.h>
#include <Psapi.h>


std::wstring get_current_timestamp()
{
	const time_t raw_time = time(nullptr);
	tm local_time;
	localtime_s(&local_time, &raw_time);

	std::wstring buffer;
	buffer.resize(100);
	wcsftime(&buffer[0], buffer.size(), L"[%H:%M:%S]", &local_time);
	return buffer;
}

std::wstring get_process_filename(HWND window)
{
	DWORD process_id;
	GetWindowThreadProcessId(window, &process_id);
	const HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, process_id);
	std::wstring filename;
	filename.resize(1024);
	DWORD buffer_size = filename.size();
	if (QueryFullProcessImageNameW(process, 0, &filename[0], &buffer_size) == 0)
	{
		return L"(unknown filename)";
	}

	//QueryFullProcessImageNameW returns full path (C:\Windows\system32\cmd.exe), this strips the path.
	const std::tr2::sys::wpath path(filename);
	return path.filename();
}

std::wstring get_window_title(HWND window)
{
	std::wstring window_title;
	window_title.resize(1024);
	if (GetWindowTextW(window, &window_title[0], window_title.size()) == 0)
	{
		return L"(unknown title)";
	}
	return window_title;
}

int main(int, char**)
{
	HWND previous_foreground_window = nullptr;
	for (;;)
	{
		HWND foreground_window = GetForegroundWindow();
		if (foreground_window != nullptr && foreground_window != previous_foreground_window)
		{
			//Foreground window changed.
			std::wstring filename(get_process_filename(foreground_window));
			std::wstring window_title(get_window_title(foreground_window));
			std::wstring timestamp(get_current_timestamp());
			// Format: [HH:MM:SS] name.exe | Window Title
			wprintf(L"%s %s | %s\n", timestamp.c_str(), filename.c_str(), window_title.c_str());
			previous_foreground_window = foreground_window;
		}

		Sleep(100);
	}
}

