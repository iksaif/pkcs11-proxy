/*
 * dlfcn-win32
 * Copyright (c) 2007 Ramiro Polla
 *
 * dlfcn-win32 is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * dlfcn-win32 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlfcn-win32; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef __MINGW32__

#include <windows.h>
#include <stdio.h>

static char dlerror_buf[256];
static int dlerror_set = 0;

static void set_error(void)
{
	DWORD err = GetLastError();
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		       NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		       dlerror_buf, sizeof(dlerror_buf), NULL);
	dlerror_set = 1;
}

void *dlopen(const char *file, int mode)
{
	HMODULE h;
	(void)mode;

	if (file == NULL)
		h = GetModuleHandle(NULL);
	else
		h = LoadLibraryA(file);

	if (!h)
		set_error();

	return (void *)h;
}

int dlclose(void *handle)
{
	if (FreeLibrary((HMODULE)handle))
		return 0;
	set_error();
	return -1;
}

void *dlsym(void *handle, const char *name)
{
	FARPROC fp;

	fp = GetProcAddress((HMODULE)handle, name);
	if (!fp)
		set_error();

	return (void *)(intptr_t)fp;
}

char *dlerror(void)
{
	if (dlerror_set) {
		dlerror_set = 0;
		return dlerror_buf;
	}
	return NULL;
}

#endif /* __MINGW32__ */
