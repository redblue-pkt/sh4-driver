/************************************************************************
Copyright (C) 2003 STMicroelectronics. All Rights Reserved.

This file is part of the Player2 Library.

Player2 is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by the
Free Software Foundation.

Player2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with player2; see the file COPYING. If not, write to the Free Software
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

The Player2 Library may alternatively be licensed under a proprietary
license from ST.

Source file name : player_factory.h - built in factory registration
Author : Julian

Date Modification Name
---- ------------ --------
19-Feb-07 Created Julian

************************************************************************/

#ifndef H_PLAYER_FACTORY
#define H_PLAYER_FACTORY

/* Debug printing macros */
#ifndef ENABLE_FACTORY_DEBUG
#define ENABLE_FACTORY_DEBUG 0
#endif

#define FACTORY_DEBUG(fmt, args...) ((void) (ENABLE_FACTORY_DEBUG && \
					     (report(severity_note, "Player factory:%s: " fmt, __FUNCTION__, ##args), 0)))

/* Output trace information off the critical path */
#define FACTORY_TRACE(fmt, args...) (report(severity_note, "Player factory:%s: " fmt, __FUNCTION__, ##args))
/* Output errors, should never be output in 'normal' operation */
#define FACTORY_ERROR(fmt, args...) (report(severity_error, "Player factory:%s: " fmt, __FUNCTION__, ##args))

/// Player wrapper class responsible for system instanciation.
HavanaStatus_t RegisterBuiltInFactories(class HavanaPlayer_c *HavanaPlayer);

#endif
