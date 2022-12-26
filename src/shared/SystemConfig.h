/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2017 MaNGOSOne <https://github.com/mangos/one>
 * Copyright (C) 2017 Hellfire <https://hellfire-core.github.io/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

// THIS FILE IS DEPRECATED

#ifndef _SYSTEMCONFIG_H
#define _SYSTEMCONFIG_H

#include "Platform/Define.h"
#include "revision.h"


#define _PACKAGENAME "HellfireCore "
#define _CODENAME "YUME"

#if MANGOS_ENDIAN == MANGOS_BIGENDIAN
# define _ENDIAN_STRING "big-endian"
#else
# define _ENDIAN_STRING "little-endian"
#endif

#define _FULLVERSION _PACKAGENAME "Rev: "  REVISION_HASH " Build Time: " REVISION_DATE

#define DEFAULT_PLAYER_LIMIT 100
#define DEFAULT_WORLDSERVER_PORT 8085                       //8129
#define DEFAULT_REALMSERVER_PORT 3724
#define DEFAULT_SOCKET_SELECT_TIME 10000

// The path to config files
#ifndef SYSCONFDIR
# define SYSCONFDIR ""
#endif

#define _MANGOS_CORE_CONFIG "mangosd.conf"
#define _MANGOS_REALM_CONFIG "realmd.conf"

// Format is YYYYMMDDRR where RR is the change in the conf file
// for that day.
#ifndef _MANGOS_CORE_CONFVER
# define _MANGOS_CORE_CONFVER 2015020801
#endif //_MANGOS_CORE_CONFVER

// Format is YYYYMMDDRR where RR is the change in the conf file
// for that day.
#ifndef _REALMDCONFVERSION
# define _REALMDCONFVERSION 2014021201
#endif

#endif

