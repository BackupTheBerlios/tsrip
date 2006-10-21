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
#include "tsr_cfg.h"
#include "tsr_util.h"

/*
 * Replace chars which aren't usable for pathnames.
 *
 */
void tsr_replace_badchars(char *str)
{
	char *badchar;

	while ((badchar = strchr(str, '/')))
	{
		*badchar = '|';
	}
}

/*
 * Lowercase the given string.
 *
 */
void tsr_lowercase(char *str)
{
	while (*str != '\0')
	{
		if (*str >= 'A' && *str <= 'Z')
		{
			*str += 32;
		}
		str++;
	}
}

/*
 * Replace spaces with underscores in the given string.
 *
 */
void tsr_replace_spaces(char *str)
{
	char *space;

	while ((space = strchr(str, ' ')) != NULL)
	{
		*space = '_';
	}
}

/*
 * Prepare string for file/dir creation, depending on config.
 *
 */
void tsr_preparefile(tsr_cfg_t *cfg, char *str)
{
	tsr_replace_badchars(str);

	if (cfg->stripspaces)
	{
		tsr_replace_spaces(str);
	}

	if (cfg->lowercase)
	{
		tsr_lowercase(str);
	}
}

/*
 * Create filename and needed directorys.
 *
 */
char *tsr_get_filename(tsr_cfg_t *cfg, tsr_metainfo_t *metainfo, int tracknum)
{
	char *path = NULL;
	char *artist;
	char *album;
	char *title;
	char *home;
	struct stat s;
	mode_t mode;
	char *badchar;

	if (metainfo->ismultiple)
	{
		artist = "Various";
	}
	else
	{
		artist = strdup(metainfo->trackinfos[tracknum]->artist);
	}

	mode = 0755;

	if (*cfg->musicdir == '~')
	{
		char *buf;

		home = getenv("HOME");
		asprintf(&path, "%s%s", home, cfg->musicdir + 1);
		free(home);
	}
	else
	{
		path = strdup(cfg->musicdir);
	}

	if (lstat(path, &s) == -1 && errno != EEXIST)
	{
		perror("Failed to access music directory");
		exit(EXIT_FAILURE);
	}
	

	tsr_preparefile(cfg, artist);
	asprintf(&path, "%s/%s", path, artist);

	if (mkdir(path, mode) == -1 && errno != EEXIST)
	{
		perror("Failed to create artist directory");
		exit(EXIT_FAILURE);
	}

	album = strdup(metainfo->album);
	tsr_preparefile(cfg, album);
	asprintf(&path, "%s/%s", path, album);

	if (mkdir(path, mode) == -1 && errno != EEXIST)
	{
		perror("Failed to create album directory");
		exit(EXIT_FAILURE);
	}

	title = strdup(metainfo->trackinfos[tracknum]->title);
	tsr_preparefile(cfg, title);
	asprintf(&path, "%s/%s.ogg", path, title);
	free(title);
	free(artist);
	free(album);

	return path;
}

void tsr_exit_error(char *file, int line, int err)
{
	printf("File %s:line %i: %s", file, line, strerror(errno));
	exit(EXIT_FAILURE);
}

/*
 * Create a new instance of an metainfo structure.
 *
 */
tsr_metainfo_t *tsr_metainfo_new(int size)
{
	int i;
	tsr_metainfo_t *metainfo;

	metainfo = (tsr_metainfo_t *) malloc(sizeof(tsr_metainfo_t));

	if (metainfo == NULL)
	{
		tsr_exit_error(__FILE__, __LINE__, errno);
	}

	metainfo->trackinfos = (tsr_trackinfo_t **) malloc(size * sizeof(tsr_metainfo_t));

	if (metainfo->trackinfos == NULL)
	{
		tsr_exit_error(__FILE__, __LINE__, errno);
	}

	for (i = 0; i < size; i++)
	{
		metainfo->trackinfos[i] = (tsr_trackinfo_t *) malloc(sizeof(tsr_trackinfo_t));

		if (metainfo->trackinfos[i] == NULL)
		{
			tsr_exit_error(__FILE__, __LINE__, errno);
		}
	}

	return metainfo;
}

/*
 * Free metainfo structure.
 *
 */
void tsr_metainfo_free(tsr_metainfo_t *metainfo)
{
	int i;

	for (i = 0; i < metainfo->numtracks; i++)
	{
		free(metainfo->trackinfos[i]);
	}

	free(metainfo->trackinfos);
	free(metainfo->album);
	free(metainfo);
}
