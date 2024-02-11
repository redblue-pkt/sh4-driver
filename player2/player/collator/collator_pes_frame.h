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

Source file name : collator_pes_frame.h
Author : Julian

Definition of the wmv collator pes class implementation for player 2.

Date Modification Name
---- ------------ --------
11-Sep-07 Created from existing collator_pes_audio_mpeg.h Julian

************************************************************************/

#ifndef H_COLLATOR_PES_FRAME
#define H_COLLATOR_PES_FRAME

// /////////////////////////////////////////////////////////////////////
//
// Include any component headers

#include "collator_pes.h"

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined structures
//

// /////////////////////////////////////////////////////////////////////////
//
// The class definition
//

class Collator_PesFrame_c : public Collator_Pes_c
{
	private:

		int RemainingDataLength;
		int FrameSize;

	protected:

	public:
		Collator_PesFrame_c();

		CollatorStatus_t Input(PlayerInputDescriptor_t *Input,
				       unsigned int DataLength,
				       void *Data,
				       bool NonBlocking = false,
				       unsigned int *DataLengthRemaining = NULL);

		CollatorStatus_t InternalFrameFlush(bool FlushedByStreamTerminate);
		CollatorStatus_t InternalFrameFlush(void);

		CollatorStatus_t Reset(void);
};

#endif /* H_COLLATOR_PES_FRAME */

