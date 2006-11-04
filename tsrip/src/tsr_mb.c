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
 * file: tsr_mb.c
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <musicbrainz/mb_c.h>

/*
 * Initialize musicbrainz, set utf, set device etc.
 *
 */
musicbrainz_t tsr_mb_init(char *device)
{
	musicbrainz_t mb_o;
	
	mb_o = mb_New();
	mb_SetDevice(mb_o, device);
	mb_UseUTF8(mb_o, 1);

	return mb_o;
}

/* 
 * Get number of albums found for cd in drive.
 *
 */
int tsr_mb_numalbums(musicbrainz_t mb_o)
{
	int c = 0;

	if (mb_Query(mb_o, MBQ_GetCDInfo))
	{
		c = mb_GetResultInt(mb_o, MBE_GetNumAlbums);
	}

	return (c > 0) ? c : 0;
}

/* 
 * Get number of tracks for the given album number.
 *
 */
int tsr_mb_album_numtracks(musicbrainz_t mb_o, int numalbum)
{
	int tracks;

	mb_Select1(mb_o, MBS_SelectAlbum, numalbum);
	tracks = mb_GetResultInt(mb_o, MBE_AlbumGetNumTracks);

	return tracks;
}

/*
 * Get name for the given album number.
 *
 */
char *tsr_mb_album_name(musicbrainz_t mb_o, int numalbum)
{
	char buf[256];

	mb_Select1(mb_o, MBS_SelectAlbum, numalbum);
	mb_GetResultData(mb_o, MBE_AlbumGetAlbumName, buf, 256);
	buf[255] = '\0';

	return strdup(buf);
}

/*
 * Get artist for the given album and track number.
 *
 */
char *tsr_mb_track_artist(musicbrainz_t mb_o, int numalbum, int numtrack)
{	
	char buf[256];
	int len;

	mb_Select1(mb_o, MBS_SelectAlbum, numalbum);
	mb_GetResultData1(mb_o, MBE_AlbumGetArtistName, buf, 256, numtrack);
	buf[255] = '\0';

	return strdup(buf);

}

/* 
 * Check if the album identifyed by numalbum is a multiartist album.
 *
 */
int tsr_mb_album_ismultiple(musicbrainz_t mb_o, int numalbum)
{
	int i;
	char *artist;
	char *partist = NULL;

	for (i = 1; i <= tsr_mb_album_numtracks(mb_o, numalbum); i++)
	{
		artist = tsr_mb_track_artist(mb_o, numalbum, i);

		if (!partist)
		{
			partist = strdup(artist);
		}
		else
		{
			if (!strcmp(partist, artist))
			{
				free(artist);
			}
			else
			{
				free(partist);
				free(artist);

				return 1;
			}
		}
	}
	
	free(partist);
	
	return 0;
}

/* 
 * Get track name for the given album and track number.
 *
 */
char *tsr_mb_track_title(musicbrainz_t mb_o, int numalbum, int numtrack)
{
	char buf[256];
	int len;

	mb_Select1(mb_o, MBS_SelectAlbum, numalbum);
	mb_GetResultData1(mb_o, MBE_AlbumGetTrackName, buf, 256, numtrack);
	buf[255] = '\0';

	return strdup(buf);
}
