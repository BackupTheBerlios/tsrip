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
 * file: tsr_cfg.c
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <cdda_interface.h>
#include <cdda_paranoia.h>

#include "tsr_types.h"
#include "tsr_cfg.h"
#include "tsr_util.h"

/*
 * Set if we should lowercase the files and dirs.
 *
 */
int tsr_cfg_set_lowercase(tsr_cfg_t *cfg, char *val)
{
	if (!strcmp(val, "off"))
	{
		cfg->lowercase = 0;
	}
	else if (!strcmp(val, "on"))
	{
		cfg->lowercase = 1;
	}
	else
	{
		return 0;
	}
	
	return 1;
}

/*
 * Set if we should strip spaces in files and dirs.
 *
 */
int tsr_cfg_set_stripspaces(tsr_cfg_t *cfg, char *val)
{
	if (!strcmp(val, "off"))
	{
		cfg->stripspaces = 0;
	}
	else if (!strcmp(val, "on"))
	{
		cfg->stripspaces = 1;
	}
	else
	{
		return 0;
	}
	
	return 1;
}

/*
 * Set paranoia mode, only these predefined values are allowed..
 *
 */
int tsr_cfg_set_paranoiamode(tsr_cfg_t *cfg, char *val)
{
	if (!strcmp(val, "off"))
	{
		cfg->paranoiamode = PARANOIA_MODE_DISABLE;
	}
	else if (!strcmp(val, "full"))
	{
		cfg->paranoiamode = PARANOIA_MODE_FULL;
	}
	else if (!strcmp(val, "verify"))
	{
		cfg->paranoiamode = PARANOIA_MODE_VERIFY;
	}
	else if (!strcmp(val, "fragment"))
	{
		cfg->paranoiamode = PARANOIA_MODE_FRAGMENT;
	}
	else if (!strcmp(val, "overlap"))
	{
		cfg->paranoiamode = PARANOIA_MODE_OVERLAP;
	}
	else if (!strcmp(val, "scratch"))
	{
		cfg->paranoiamode = PARANOIA_MODE_SCRATCH;
	}
	else if (!strcmp(val, "repair"))
	{
		cfg->paranoiamode = PARANOIA_MODE_REPAIR;
	}
	else if (!strcmp(val, "neverskip"))
	{
		cfg->paranoiamode = PARANOIA_MODE_NEVERSKIP;
	}
	else
	{
		return 0;
	}

	return 1;
}

/*
 * Set vorbis quality.
 *
 */
int tsr_cfg_set_vorbisqualiy(tsr_cfg_t *cfg, char *val)
{
	int iquality;
	float fquality;

	iquality = atoi(val);

	if (iquality > 0 && iquality <= 10)
	{
		fquality = 0.1f * iquality;
		cfg->vorbisquality = fquality;

		return 1;
	}

	return 0;
}

/*
 * Set if we should ask for multiple cd album.
 *
 */
int tsr_cfg_set_multidisc(tsr_cfg_t *cfg, char *val)
{
	if (!strcmp(val, "on"))
	{
		cfg->multidisc = 1;
	}
	else if (!strcmp(val, "off"))
	{
		cfg->multidisc = 0;
	}
	else
	{
		return 0;
	}

	return 1;
}


/*
 * Load default configuration.
 *
 */
void tsr_cfg_defaults(tsr_cfg_t *cfg)
{
	cfg->musicdir = strdup(CFG_MUSICDIR);
	cfg->device = strdup(CFG_DEVICE);
	cfg->paranoiamode = PARANOIA_MODE_REPAIR;
	cfg->vorbisquality = 0.4;
	cfg->multidisc = 0;
	cfg->stripspaces = 0;
	cfg->lowercase = 0;
	cfg->enctype = CFG_TYPE_VORBIS;
}

/*
 * Get value out of a line in the cfg file.
 *
 */
int tsr_cfg_getval(char *line, char **val)
{
	char *esign, *nsign;

	/* check for equality sign */
	esign = strchr(line, '=');
	nsign = strchr(line, '\n');

	if (esign == NULL || nsign == NULL || nsign - esign < 2)
	{
		return 0;
	}

	*esign = '\0';
	*nsign = '\0';
	*val = esign + 1;

	return 1;
}

/*
 * Parse and set an option from a string.
 *
 */
int tsr_cfg_setopt(tsr_cfg_t *cfg, char *line)
{
	char *val = 0;

	/* skip empty lines and comments */
	if (*line == '#' || *line == '\0' || *line == '\n')
		return 1;

	if (!tsr_cfg_getval(line, &val))
		return 0;

	if (!strcmp(line, "musicdir"))
	{
		cfg->musicdir = strdup(val);
	}
	else if (!strcmp(line, "device"))
	{
		cfg->device = strdup(val);
	}
	else if (!strcmp(line, "paranoiamode"))
	{
		return tsr_cfg_set_paranoiamode(cfg, val);
	}
	else if (!strcmp(line, "vorbisquality"))
	{
		return tsr_cfg_set_vorbisqualiy(cfg, val);
	}
	else if (!strcmp(line, "multidisc"))
	{
		return tsr_cfg_set_multidisc(cfg, val);
	}
	else if (!strcmp(line, "stripspaces"))
	{
		return tsr_cfg_set_stripspaces(cfg, val);
	}
	else if (!strcmp(line, "lowercase"))
	{
		return tsr_cfg_set_lowercase(cfg, val);
	}
	else
	{
		return 0;
	}

	return 1;
}

/*
 * Load configuratoin from user config file.
 *
 */
void tsr_cfg_load_usercfg(tsr_cfg_t *cfg)
{
	char *home, *line;
	int lineno = 0;
	size_t len;

	line = 0;
	home = getenv("HOME");

	if (!home)
	{
		fprintf(stderr, "Cant't get home directory.\n");
		exit(1);
	}

	asprintf(&cfg->cfg_file, "%s/%s", home, CFG_FILE);
	cfg->cfg_fp = fopen(cfg->cfg_file, "r");

	if (errno == ENOENT)
	{
		return;
	}

	if (!cfg->cfg_fp)
	{
		perror("Can't load user config");
		exit(1);
	}

	while ((getline(&line, &len, cfg->cfg_fp)) != -1)
	{
		lineno++;

		/* TODO: print error information (bad value etc) */
		if (!tsr_cfg_setopt(cfg, line))
		{
			fprintf(stderr, "Invalid config file at line %i.\n", lineno);
			exit(1);
		}

		if (line != NULL)
		{
			free(line);
		}

		line = NULL;
	}

	fclose(cfg->cfg_fp);
}

/*
 * Initialize the cfg data and return a tsr_cfg_t struct.
 *
 */
tsr_cfg_t *tsr_cfg_init()
{
	tsr_cfg_t *cfg;

	cfg = (tsr_cfg_t *) malloc(sizeof(tsr_cfg_t));

	if (!cfg)
	{
		tsr_exit_error(__FILE__, __LINE__, errno);
	}

	tsr_cfg_defaults(cfg);
	tsr_cfg_load_usercfg(cfg);

	return cfg;
}
