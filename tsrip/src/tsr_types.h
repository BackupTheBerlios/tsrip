/*
 * This file is part of tsrip.
 * 
 * tsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * tsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with tsrip; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * file: tsr_types.h
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

#include <stdio.h>
#include <vorbis/vorbisenc.h>

typedef struct _tsr_trackinfo_t
{
	char *title;
	char *artist;
} tsr_trackinfo_t;

typedef struct _tsr_metainfo_t
{
	char *album;
	int numtracks;
	int discnum;
	int ismultiple;
	tsr_trackinfo_t **trackinfos;
} tsr_metainfo_t;

typedef struct _tsr_trackfile_t
{
	FILE *file_s;
	char *filename;

	vorbis_info vinfo;
	vorbis_comment vcomment;
	vorbis_dsp_state vdsp_state;
	vorbis_block vblock;

	ogg_stream_state ostream;
} tsr_trackfile_t;
