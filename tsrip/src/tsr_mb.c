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

#define lineno(x) rlineno(x)
#define rlineno(x) #x

/* Initialize musicbrainz, set utf, set device etc. */
musicbrainz_t
tsr_mb_init(char *device)
{
	musicbrainz_t mb_o;
	
	mb_o = mb_New();
	mb_SetDevice(mb_o, device);
	mb_UseUTF8(mb_o, 1);

	return mb_o;
}

/* Get number of albums found for cd in drive */
int
tsr_mb_numalbums(musicbrainz_t mb_o)
{
	int c = 0;

	if(mb_Query(mb_o, MBQ_GetCDInfo))
		c = mb_GetResultInt(mb_o, MBE_GetNumAlbums);

	return (c > 0) ? c : 0;
}

/* Get number of tracks for the given album number */
int
tsr_mb_album_numtracks(musicbrainz_t mb_o, int numalbum)
{
	int tracks;

	mb_Select1(mb_o, MBS_SelectAlbum, numalbum);
	tracks = mb_GetResultInt(mb_o, MBE_AlbumGetNumTracks);

	return tracks;
}

/* Get name for the given album number */
char *
tsr_mb_album_name(musicbrainz_t mb_o, int numalbum)
{
	char buf[256];
	char *album;
	int len;

	mb_Select1(mb_o, MBS_SelectAlbum, numalbum);
	mb_GetResultData(mb_o, MBE_AlbumGetAlbumName, buf, 256);

	buf[255] = 0;
	len = strlen(buf) + 1;

	album = (char *) malloc(len * sizeof(char));
	if(!album)
	{
		perror(__FILE__":"lineno(__LINE__));
		exit(1);
	}

	strncpy(album, buf, len);

	return album;
}

/* Get artist for the given album and track number */
char *
tsr_mb_track_artist(musicbrainz_t mb_o, int numalbum, int numtrack)
{	
	char buf[256];
	char *artist;
	int len;

	mb_Select1(mb_o, MBS_SelectAlbum, numalbum);
	mb_GetResultData1(mb_o, MBE_AlbumGetArtistName, buf, 256, numtrack);
	
	buf[255] = 0;
	len = strlen(buf) + 1;

	artist = (char *) malloc(len * sizeof(char));
	if(!artist)
	{
		perror(__FILE__":"lineno(__LINE__));
		exit(1);
	}

	strncpy(artist, buf, len);

	return artist;

}

/* Check if the album identifyed by numalbum is a multiartist album */
int
tsr_mb_album_ismultiple(musicbrainz_t mb_o, int numalbum)
{
	int i;
	char *artist;
	char *partist = 0;

	for(i = 1; i <= tsr_mb_album_numtracks(mb_o, numalbum); i++)
	{
		artist = tsr_mb_track_artist(mb_o, numalbum, i);
		if(!partist)
			partist = artist;
		else
		{
			if(!strcmp(partist, artist))
			{
				free(partist);
				partist = artist;
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

/* Get track name for the given album and track number */
char *
tsr_mb_track_title(musicbrainz_t mb_o, int numalbum, int numtrack)
{
	char buffer[256];
	char *title;
	int len;

	mb_Select1(mb_o, MBS_SelectAlbum, numalbum);
	mb_GetResultData1(mb_o, MBE_AlbumGetTrackName, buffer, 256, numtrack);

	buffer[255] = 0;
	len = strlen(buffer) + 1;

	title = (char *) malloc(len * sizeof(char));
	if(!title)
	{
		perror(__FILE__":"lineno(__LINE__));
		exit(1);
	}

	strncpy(title, buffer, len);

	return title;
}
