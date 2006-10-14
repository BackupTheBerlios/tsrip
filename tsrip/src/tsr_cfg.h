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
 * file: tsr_cfg.h
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

typedef struct _tsr_cfg_t
{
	char *cfg_file;
	FILE *cfg_fp;
	char *musicdir;
	char *device;
	int paranoia_mode;
	float vorbis_quality;
	int multidisc;
} tsr_cfg_t;

int
tsr_cfg_set_paranoiamode(tsr_cfg_t *cfg, char *val);

int
tsr_cfg_set_vorbisqualiy(tsr_cfg_t *cfg, char *val);

tsr_cfg_t *
tsr_cfg_init();
