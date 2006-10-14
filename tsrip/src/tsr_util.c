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
 * file: tsr_util.c
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "tsr_types.h"
#include "tsr_util.h"

/*
 * Create filename and needed directorys.
 *
 */
char *
tsr_get_filename(char *musicdir, tsr_metainfo_t *metainfo, int tracknum)
{
	char *music_path;
	char *artist_path;
	char *album_path;
	char *artist;
	char *fname;
	char *home;
	struct stat s;
	mode_t mode;
	int new_mpath = 0; /* FIXME: hack */

	if(metainfo->ismultiple)
		artist = "Various";
	else
		artist = metainfo->trackinfos[tracknum]->artist;

	mode = 0755;

	if(*musicdir == '~')
	{
		char *buf;

		home = getenv("HOME");
		asprintf(&music_path,"%s%s", home, (musicdir + 1));
		new_mpath = 1;

		free(home);
	}
	else
		music_path = musicdir;

	if(lstat(music_path, &s) == -1 && errno != EEXIST)
	{
		perror("Failed to access music directory");
		exit(1);
	}
	
	asprintf(&artist_path, "%s/%s", music_path, artist);

	if(mkdir(artist_path, mode) == -1 && errno != EEXIST)
	{
		perror("Failed to create artist directory");
		exit(1);
	}

	asprintf(&album_path, "%s/%s", artist_path, metainfo->album);
	if(mkdir(album_path, mode) == -1 && errno != EEXIST)
	{
		perror("Failed to create album directory");
		exit(1);
	}

	asprintf(&fname, "%s/%s.ogg", album_path, metainfo->trackinfos[tracknum]->title);

	if(new_mpath)
		free(music_path);
	free(artist_path);
	free(album_path);

	return fname;
}

void
tsr_copystr(char **dest, char *src)
{
	int len;

	len = strlen(src) + 1;
	*dest = (char *) malloc(len * sizeof(char));
	if(!*dest)
	{
		perror(__FILE__":"LINENO(__LINE__));
		exit(1);
	}
	strncpy(*dest, src, len);
}

/*
 * Free metainfo structure.
 *
 */
void
tsr_metainfo_free(tsr_metainfo_t *metainfo)
{
	int i;

	for(i = 0; i < metainfo->numtracks; i++)
		free(metainfo->trackinfos[i]);

	free(metainfo->trackinfos);
	free(metainfo->album);
	free(metainfo);
}
