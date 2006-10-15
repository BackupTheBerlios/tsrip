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
 * Replace chars which aren't usable for pathnames.
 *
 */
void
tsr_replace_badchars(char *str)
{
	char *badchar;

	while((badchar = strchr(str, '/')))
		*badchar = '|';
}

/*
 * Create filename and needed directorys.
 *
 */
char *
tsr_get_filename(char *musicdir, tsr_metainfo_t *metainfo, int tracknum)
{
	char *path = 0;
	char *artist;
	char *album;
	char *title;
	char *home;
	struct stat s;
	mode_t mode;
	char *badchar;

	if(metainfo->ismultiple)
		artist = "Various";
	else
		tsr_copystr(&artist, metainfo->trackinfos[tracknum]->artist);

	mode = 0755;

	if(*musicdir == '~')
	{
		char *buf;

		home = getenv("HOME");
		asprintf(&path,"%s%s", home, (musicdir + 1));

		free(home);
	}
	else
		tsr_copystr(&path, musicdir);

	if(lstat(path, &s) == -1 && errno != EEXIST)
	{
		perror("Failed to access music directory");
		exit(1);
	}
	
	tsr_replace_badchars(artist);
	asprintf(&path, "%s/%s", path, artist);

	if(mkdir(path, mode) == -1 && errno != EEXIST)
	{
		perror("Failed to create artist directory");
		exit(1);
	}

	tsr_copystr(&album, metainfo->album); 
	tsr_replace_badchars(album);
	asprintf(&path, "%s/%s", path, album);

	if(mkdir(path, mode) == -1 && errno != EEXIST)
	{
		perror("Failed to create album directory");
		exit(1);
	}

	tsr_copystr(&title, metainfo->trackinfos[tracknum]->title);
	tsr_replace_badchars(title);
	asprintf(&path, "%s/%s.ogg", path, title);

	free(title);
	free(artist);
	free(album);

	return path;
}

/*
 * Copy a string from src to dest.
 *
 */
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
