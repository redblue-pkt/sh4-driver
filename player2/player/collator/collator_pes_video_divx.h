/************************************************************************
Copyright (C) 2006 STMicroelectronics. All Rights Reserved.

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

Source file name : collator_pes_video_divx.h
Author : Chris

Definition of the base collator pes class implementation for player 2.

Date Modification Name
---- ------------ --------
11-Jul-07 Created from existing collator_pes_video.h Chris

************************************************************************/

#ifndef H_COLLATOR_PES_VIDEO_DIVX
#define H_COLLATOR_PES_VIDEO_DIVX

// /////////////////////////////////////////////////////////////////////
//
// Include any component headers

#include "collator_pes_video.h"

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined structures
//

// /////////////////////////////////////////////////////////////////////////
//
// The class definition
//

class Collator_PesVideoDivx_c : public Collator_PesVideo_c
{
	private:

		bool IgnoreCodes;
		unsigned char Version;

	protected:

	public:

		Collator_PesVideoDivx_c();

		CollatorStatus_t Reset(void);

		CollatorStatus_t Input(PlayerInputDescriptor_t *Input,
				       unsigned int DataLength,
				       void *Data,
				       bool NonBlocking = false,
				       unsigned int *DataLengthRemaining = NULL);
};

#endif // H_COLLATOR_PES_VIDEO_DIVX

