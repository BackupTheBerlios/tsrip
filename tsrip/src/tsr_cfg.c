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
#include "tsr_util.h"
#include "tsr_cfg.h"

#define CFG_FILE ".tsriprc"
#define CFG_MUSICDIR "~/music"
#define CFG_DEVICE "/dev/cdrom"

/*
 * Set paranoia mode, only these predefined values are allowed..
 *
 */
int
tsr_cfg_set_paranoiamode(tsr_cfg_t *cfg, char *val)
{
	if(strcmp(val, "off"))
		cfg->paranoia_mode = PARANOIA_MODE_DISABLE;
	else if(strcmp(val, "full"))
		cfg->paranoia_mode = PARANOIA_MODE_FULL;
	else if(strcmp(val, "verify"))
		cfg->paranoia_mode = PARANOIA_MODE_VERIFY;
	else if(strcmp(val, "fragment"))
		cfg->paranoia_mode = PARANOIA_MODE_FRAGMENT;
	else if(strcmp(val, "overlap"))
		cfg->paranoia_mode = PARANOIA_MODE_OVERLAP;
	else if(strcmp(val, "scratch"))
		cfg->paranoia_mode = PARANOIA_MODE_SCRATCH;
	else if(strcmp(val, "repair"))
		cfg->paranoia_mode = PARANOIA_MODE_REPAIR;
	else if(strcmp(val, "neverskip"))
		cfg->paranoia_mode = PARANOIA_MODE_NEVERSKIP;
	else
		return 0;

	return 1;
}

/*
 * Set vorbis quality.
 *
 */
int
tsr_cfg_set_vorbisqualiy(tsr_cfg_t *cfg, char *val)
{
	int iquality;
	float fquality;

	iquality = atoi(val);
	if(iquality > 0 && iquality <= 10)
	{
		fquality = 0.1f * iquality;
		cfg->vorbis_quality = fquality;
		return 1;
	}

	return 0;
}

/*
 * Set if we should ask for multiple cd album.
 *
 */
int
tsr_cfg_set_multidisc(tsr_cfg_t *cfg, char *val)
{
	if(!strcmp(val, "on"))
		cfg->multidisc = 1;
	else if(!strcmp(val, "off"))
		cfg->multidisc = 0;
	else
		return 0;

	return 1;
}


/*
 * Load default configuration.
 *
 */
void
tsr_cfg_defaults(tsr_cfg_t *cfg)
{
	tsr_copystr(&cfg->musicdir, CFG_MUSICDIR);
	tsr_copystr(&cfg->device, CFG_DEVICE);
	cfg->paranoia_mode = PARANOIA_MODE_REPAIR;
	cfg->vorbis_quality = 0.4;
	cfg->multidisc = 0;
}

/*
 * Get value out of a line in the cfg file.
 *
 */
int
tsr_cfg_getval(char *line, char **val)
{
	char *esign, *nsign;

	/* check for equality sign */
	esign = strchr(line, '=');
	nsign = strchr(line, '\n');

	if(!esign || !nsign || nsign - esign < 2)
		return 0;

	*esign = 0;
	*nsign = 0;
	*val = esign + 1;

	return 1;
}

/*
 * Parse and set an option from a string.
 *
 */
int 
tsr_cfg_setopt(tsr_cfg_t *cfg, char *line)
{
	char *val = 0;

	/* skip empty lines and comments */
	if(*line == '#' || *line == 0 || *line == '\n')
		return 1;

	if(!tsr_cfg_getval(line, &val))
		return 0;

	if(!strcmp(line, "musicdir"))
		tsr_copystr(&cfg->musicdir, val);
	else if(!strcmp(line, "device"))
		tsr_copystr(&cfg->device, val);
	else if(!strcmp(line, "paranoiamode"))
		return tsr_cfg_set_paranoiamode(cfg, val);
	else if(!strcmp(line, "vorbisquality"))
		return tsr_cfg_set_vorbisqualiy(cfg, val);
	else if(!strcmp(line, "multidisc"))
		return tsr_cfg_set_multidisc(cfg, val);
	else
		return 0;

	return 1;
}

/*
 * Load configuratoin from user config file.
 *
 */
void
tsr_cfg_load_usercfg(tsr_cfg_t *cfg)
{
	char *home, *line;
	int lineno = 0;
	size_t len;

	line = 0;
	home = getenv("HOME");
	if(!home)
	{
		fprintf(stderr, "Cant't get home directory.\n");
		exit(1);
	}

	asprintf(&cfg->cfg_file, "%s/%s", home, CFG_FILE);
	cfg->cfg_fp = fopen(cfg->cfg_file, "r");

	if(errno == ENOENT)
		return;

	if(!cfg->cfg_fp)
	{
		perror("Can't load user config");
		exit(1);
	}

	while((getline(&line, &len, cfg->cfg_fp)) != -1)
	{
		lineno++;
		if(!tsr_cfg_setopt(cfg, line))
		{
			fprintf(stderr, "Invalid config file at line %i.\n", lineno);
			exit(1);
		}

		if(line)
			free(line);
		line = 0;
	}

	fclose(cfg->cfg_fp);
}

tsr_cfg_t *
tsr_cfg_init()
{
	tsr_cfg_t *cfg;

	cfg = (tsr_cfg_t *) malloc(sizeof(tsr_cfg_t));
	if(!cfg)
	{
		perror(__FILE__":"LINENO(__LINE__));
		exit(1);
	}

	tsr_cfg_defaults(cfg);
	tsr_cfg_load_usercfg(cfg);

	return cfg;
}
