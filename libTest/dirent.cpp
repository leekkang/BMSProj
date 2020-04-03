#include "pch.h"
#include "dirent.h"
/*

	Implementation of POSIX directory browsing functions and types for Win32.

	Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
	History: Created March 1997. Updated June 2003 and July 2012.
	Rights:  See end of file.

*/

extern "C" {
	DIR* OpenDir(const std::wstring& name) noexcept {
		DIR* dir = 0;
		size_t base_length = name.size();
		if (base_length != 0) {
			/* search pattern must end with suitable wildcard */
			const wchar_t *all = wcschr(L"/\\", name[base_length - 1]) ? L"*" : L"/*";
			dir = new DIR;
			wcscpy_s(dir->name, name.data());
			wcscat_s(dir->name, all);
			if ((dir->handle = (handle_type)_wfindfirst(dir->name, &dir->info)) == -1) {
				delete dir; dir = 0;
			}
		} else {
			errno = EINVAL;
		}

		return dir;
	}

	int CloseDir(DIR* dir) noexcept {
		int result = -1;

		if (dir) {
			if (dir->handle != -1) {
				result = _findclose(dir->handle);
			}
			delete dir;
		}

		if (result == -1) { /* map all errors to EBADF */
			errno = EBADF;
		}

		return result;
	}

	wchar_t* ReadDir(DIR *dir) noexcept {
		wchar_t* result = nullptr;

		if (dir && dir->handle != -1) {
			if (_wfindnext(dir->handle, &dir->info) != -1) {
				result = dir->info.name;
			}
		} else {
			errno = EBADF;
		}

		return result;
	}
}
/*

	Copyright Kevlin Henney, 1997, 2003, 2012. All rights reserved.

	Permission to use, copy, modify, and distribute this software and its
	documentation for any purpose is hereby granted without fee, provided
	that this copyright and permissions notice appear in all copies and
	derivatives.

	This software is supplied "as is" without express or implied warranty.

	But that said, if there are any problems please get in touch.

*/