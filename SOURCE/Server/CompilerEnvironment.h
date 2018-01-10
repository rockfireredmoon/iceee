/*
 *This file is part of TAWD.
 *
 * TAWD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAWD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAWD.  If not, see <http://www.gnu.org/licenses/
 */

#pragma once
#ifndef COMPILERENVIRONMENT_H
#define COMPILERENVIRONMENT_H

/* Default location to load local configuration files. The contents of LOCALCONFIGDIR/ServerConfig.txt
 * determines the locations of the rest of the data files.
 */
#ifndef LOCALCONFIGDIR
#define LOCALCONFIGDIR "Local"
#endif

/* Detect various platforms */
#if (defined(_WIN32) || defined(_WIN64))
#define WINDOWS_PLATFORM 1
#else
#undef WINDOWS_PLATFORM
#endif

#endif //COMPILERENVIRONMENT_H
