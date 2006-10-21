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
 * file: tsr_track.c
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cdda_interface.h>
#include <vorbis/vorbisenc.h>

#include "tsr_types.h"

/* 
 * Initialize file, ogg stuff etc. 
 *
 */
tsr_trackfile_t *tsr_trackfile_init(int tracknum, char *filename, tsr_metainfo_t *metainfo)
{
	tsr_trackfile_t *trackfile;
	tsr_trackinfo_t *trackinfo;
	ogg_page opage;
	ogg_packet oheader;
	ogg_packet oheader_comm;
	ogg_packet oheader_code;
	char *stracknum;
	char *sdiscnum;

	trackinfo = metainfo->trackinfos[tracknum];
	trackfile = (tsr_trackfile_t *) malloc(sizeof(tsr_trackfile_t));
	trackfile->file_s = fopen(filename, "w");

	if(!trackfile->file_s)
	{
		perror("tsr_track_init: fopen");
		exit(1);
	}
	
	trackfile->filename = filename;
	vorbis_info_init(&trackfile->vinfo);
	vorbis_encode_init_vbr(&trackfile->vinfo, 2, 44100, 0.4);
	vorbis_comment_init(&trackfile->vcomment);
	vorbis_analysis_init(&trackfile->vdsp_state, &trackfile->vinfo);
	vorbis_block_init(&trackfile->vdsp_state, &trackfile->vblock);
	ogg_stream_init(&(trackfile->ostream), tracknum);

	vorbis_comment_add_tag(&trackfile->vcomment, "ENCODER", "tsrip");
	vorbis_comment_add_tag(&trackfile->vcomment, "TITLE", trackinfo->title);
	vorbis_comment_add_tag(&trackfile->vcomment, "ARTIST", trackinfo->artist);
	vorbis_comment_add_tag(&trackfile->vcomment, "ALBUM", metainfo->album);

	if(metainfo->discnum > 0)
	{
		asprintf(&sdiscnum, "%i", metainfo->discnum);
		vorbis_comment_add_tag(&trackfile->vcomment, "DISC", sdiscnum);
		free(sdiscnum);
	}

	asprintf(&stracknum, "%i", tracknum + 1);
	vorbis_comment_add_tag(&trackfile->vcomment, "TRACKNUMBER", stracknum);
	free(stracknum);

	vorbis_analysis_headerout(&trackfile->vdsp_state, &trackfile->vcomment,
			&oheader, &oheader_comm, &oheader_code);
	ogg_stream_packetin(&trackfile->ostream, &oheader);
	ogg_stream_packetin(&trackfile->ostream, &oheader_comm);
	ogg_stream_packetin(&trackfile->ostream, &oheader_code);

	/* finish ogg block */
	while(1)
	{
		int result = ogg_stream_flush(&trackfile->ostream, &opage);
		
		if(!result)
			break;

		fwrite(opage.header, 1, opage.header_len, trackfile->file_s);
		fwrite(opage.body, 1, opage.body_len, trackfile->file_s);
	}

	return trackfile;
}

/* 
 * Write next ogg blocks and finish unfinished ones.
 *
 */
void tsr_trackfile_encode_handle_blocks(tsr_trackfile_t *trackfile)
{
	ogg_packet opackage;
	ogg_page opage;

	while(vorbis_analysis_blockout(&trackfile->vdsp_state, &trackfile->vblock) == 1)
	{
		vorbis_analysis(&trackfile->vblock, 0);
		vorbis_bitrate_addblock(&trackfile->vblock);

		while(vorbis_bitrate_flushpacket(&trackfile->vdsp_state, &opackage))
		{
			ogg_stream_packetin(&trackfile->ostream, &opackage);

			while(1)
			{
				int result = ogg_stream_pageout(&trackfile->ostream, &opage);
				
				if(!result)
					break;

				fwrite(opage.header, 1, opage.header_len, trackfile->file_s);
				fwrite(opage.body, 1, opage.body_len, trackfile->file_s);
			}
		}
	}
}

/* 
 * Encode next buffer. 
 *
 */
void tsr_trackfile_encode_next(tsr_trackfile_t *trackfile, char *read_buffer)
{
	float **encode_buffer;
	int i;

	encode_buffer = vorbis_analysis_buffer(&trackfile->vdsp_state, CD_FRAMESAMPLES);
	
	for(i = 0; i < CD_FRAMESAMPLES; i++)
	{
		encode_buffer[0][i] = ((read_buffer[i * 4 + 1] << 8)
				| (0x00ff & (int)read_buffer[i * 4])) / 32768.f;
		encode_buffer[1][i] = ((read_buffer[i * 4 + 3] << 8)
				| (0x00ff & (int)read_buffer[i * 4 + 2])) / 32768.f;
	}

	vorbis_analysis_wrote(&trackfile->vdsp_state, i);
	tsr_trackfile_encode_handle_blocks(trackfile);
}

/* 
 * OMG, something went wrong ...
 *
 */
void tsr_trackfile_fail(tsr_trackfile_t *trackfile)
{
	fclose(trackfile->file_s);
	unlink(trackfile->filename);
}

/* 
 * Finish file. 
 *
 */
void tsr_trackfile_finish(tsr_trackfile_t *trackfile)
{
	vorbis_analysis_wrote(&trackfile->vdsp_state, 0);
	tsr_trackfile_encode_handle_blocks(trackfile);
	ogg_stream_clear(&trackfile->ostream);
	vorbis_block_clear(&trackfile->vblock);
	vorbis_dsp_clear(&trackfile->vdsp_state);
	vorbis_comment_clear(&trackfile->vcomment);
	vorbis_info_clear(&trackfile->vinfo);

	fclose(trackfile->file_s);
	free(trackfile);
}
