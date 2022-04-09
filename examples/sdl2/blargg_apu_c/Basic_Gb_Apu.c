
// Gb_Snd_Emu 0.1.4. http://www.slack.net/~ant/libs/

#include "Basic_Gb_Apu.h"
#include "Gb_Apu.h"

/* Copyright (C) 2003-2005 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

gb_time_t const frame_length = 70224;
static Stereo_Buffer buf;

blargg_err_t set_sample_rate(long rate)
{
	apu.output( buf.center(), buf.left(), buf.right() );
	buf.clock_rate( 4194304 );
	return buf.set_sample_rate( rate );
}

