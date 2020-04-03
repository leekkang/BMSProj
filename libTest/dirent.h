#pragma once
// reference : http://www.two-sdg.demon.co.uk/curbralan/code/dirent/dirent.html
#ifndef DIRENT_INCLUDED
#define DIRENT_INCLUDED

#include <errno.h>
#include <io.h> /* _findfirst and _findnext set errno iff they return -1 */
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <string>

/*

	Declaration of POSIX directory browsing functions and types for Win32.

	Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
	History: Created March 1997. Updated June 2003.
	Rights:  See end of file.

*/

//#pragma warning(disable: 4996)
constexpr auto CURRENT_FOLDER_SYM = L".";
constexpr auto PREV_FOLDER_SYM = L"..";

extern "C" {
	typedef ptrdiff_t handle_type; /* C99's intptr_t not sufficiently portable */
	struct DIR {
		handle_type          handle; /* -1 for failed rewind */
		struct _wfinddata_t  info;
		wchar_t				 name[260] = {0,};  /* null-terminated char string */
	};

	DIR* OpenDir(const std::wstring& name) noexcept;
	int CloseDir(DIR* dir) noexcept;
	wchar_t* ReadDir(DIR* dir) noexcept;

}

#endif
/*

	Copyright Kevlin Henney, 1997, 2003, 2012. All rights reserved.

	Permission to use, copy, modify, and distribute this software and its
	documentation for any purpose is hereby granted without fee, provided
	that this copyright and permissions notice appear in all copies and
	derivatives.

	This software is supplied "as is" without express or implied warranty.

	But that said, if there are any problems please get in touch.

*/