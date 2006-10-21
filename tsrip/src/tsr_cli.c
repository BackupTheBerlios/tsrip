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
 * file: tsr_cli.c
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <cdda_interface.h>
#include <cdda_paranoia.h>

#include "config.h"
#include "tsr_cfg.h"
#include "tsr_track.h"
#include "tsr_mb.h"
#include "tsr_util.h"

/*
 * Print version.
 *
 */
void tsr_cli_print_version()
{
	printf(PACKAGE " " VERSION "\n");
}

/*
 * Print usage.
 *
 */
void tsr_cli_print_usage()
{
	printf("Usage: " PACKAGE " [options]\n");
	printf("	-m --multidisc			Ask for disc number\n"
	       "	-d --device <device>		Use device <device>\n"
	       "	-p --paranoiamode <mode>	Paranoia mode to use\n"
	       "	-s --stripspaces		Replace spaces by underscores\n"
	       "	-l --lowercase			Lowercase ASCII chars in path\n"
	       "	   --musicdir <dir>		Directory where files should be saved\n"
	       "	   --vorbisquality <1-10>	The vorbis quality to use\n"
	       "	-u --usage			Print usage information\n"
	       "	-v --version			Print version\n"
	       "	-h --help			Print help\n");
}

/*
 * Print version, usage and help.
 *
 */
void tsr_cli_print_help()
{
	tsr_cli_print_version();
	tsr_cli_print_usage();
}


/*
 * Print the head (album, artist) of the given metainfo.
 *
 */
void tsr_cli_print_metainfo_head(tsr_metainfo_t *metainfo)
{

	printf("Album \"%s\" ", metainfo->album);

	if (!metainfo->ismultiple)
	{
		printf("by %s\n", metainfo->trackinfos[0]->artist);
	}
	else
	{
		printf("by various Artists\n");
	}
}

/*
 * Shows the given metainfo about an album.
 *
 */
void tsr_cli_print_metainfo(tsr_metainfo_t *metainfo)
{
	int i;

	printf(" ----\n");
	tsr_cli_print_metainfo_head(metainfo);

	for (i = 0; i < metainfo->numtracks; i++)
	{
		printf("%i: %s", i + 1, metainfo->trackinfos[i]->title);

		if (metainfo->ismultiple)
			printf(", by %s", metainfo->trackinfos[i]->artist);

		printf("\n");
	}

	printf(" ----\n");
}

/*
 * Callback needed by paranoia_read(), urgs ...
 *
 */
void cb(long a, int b)
{
}

/*
 * Function to get a line from stdin and automatically strip the newline at end.
 *
 */
char *tsr_cli_read_str()
{
	char *newline, *input = NULL;
	int read;
	size_t len;

	read = getline(&input, &len, stdin);

	if (read == -1)
	{
		tsr_exit_error(__FILE__, __LINE__, 1);
	}
	
	newline = strchr(input, '\n');

	if (newline != NULL)
	{
		*newline = '\0';
	}
	else
	{
		*input = '\0';
	}

	return input;
}

/*
 * Get metainfo for a single album, identifyed by number.
 *
 */
tsr_metainfo_t *tsr_cli_metainfo_mb_bynum(musicbrainz_t mb_o, int numalbum)
{
	int i;
	tsr_metainfo_t *metainfo;

	i = tsr_mb_album_numtracks(mb_o, numalbum);
	metainfo = tsr_metainfo_new(i);
	metainfo->numtracks = i;
	metainfo->album = tsr_mb_album_name(mb_o, numalbum);
	metainfo->ismultiple = tsr_mb_album_ismultiple(mb_o, numalbum);

	for (i = 0; i < metainfo->numtracks; i++)
	{
		metainfo->trackinfos[i]->title = tsr_mb_track_title(mb_o,
				numalbum, i + 1);
		metainfo->trackinfos[i]->artist = tsr_mb_track_artist(mb_o,
				numalbum, i + 1);
	}

	return metainfo;
}

/*
 * This function is dedicated to edit the artist on a normal album with one
 * artist. This function is very special, because the artist is stored for
 * each track in the metainfo, even if the album only has one artist.
 *
 */
void tsr_cli_artist_edit(tsr_metainfo_t *metainfo)
{
	char *artist;
	int i;

	artist = tsr_cli_read_str();
	
	for (i = 0; i < metainfo->numtracks; i++)
	{
		free(metainfo->trackinfos[i]->artist);
		metainfo->trackinfos[i]->artist = strdup(artist);
	}

	free(artist);
}

/*
 * print musicbrainz metainfo, ask for edit and return metainfo, or null.
 * 
 */
tsr_metainfo_t *tsr_cli_metainfo_mb_finish(tsr_metainfo_t *metainfo)
{
	char *input;
	char *selection;
	int track, read;

	while (1)
	{
		tsr_cli_print_metainfo(metainfo);
		printf("Use this information (Y/n) or do you want to edit (a)lbum");

		if (!metainfo->ismultiple)
		{
			printf(", a(r)tist");
		}

		printf(" or a track#? ");
		input = tsr_cli_read_str();
		track = atoi(input);

		if (*input == '\0' || *input == 'y')
		{
			free(input);

			return metainfo;
		}
		else if (*input == 'a')
		{
			printf("Enter new album name: ");
			metainfo->album = tsr_cli_read_str();
		}
		else if (*input == 'r' && !metainfo->ismultiple)
		{
			printf("Enter new artist name: ");
			tsr_cli_artist_edit(metainfo);
		}
		else if (track > 0 && track <= metainfo->numtracks)
		{
			printf("Enter new title for track %i: ", track--);
			metainfo->trackinfos[track]->title = tsr_cli_read_str();

			if (metainfo->ismultiple)
			{
				printf("Enter new artist for track %i: ", track);
				metainfo->trackinfos[track]->artist = tsr_cli_read_str();
			}

		}
		else if (*input == 'n')
		{
			free(input);

			return NULL;
		}

		free(input);
	}

	return NULL;
}

/*
 * Get meta information from musicbrainz database.
 *
 */
tsr_metainfo_t *tsr_cli_metainfo_mb(musicbrainz_t mb_o, int numalbums)
{
	char *input;
	size_t len;
	int album, i, read;
	tsr_metainfo_t *metainfo;
	tsr_metainfo_t **metainfos;

	if (numalbums == 1)
	{
		metainfo = tsr_cli_metainfo_mb_bynum(mb_o, 1);
		return tsr_cli_metainfo_mb_finish(metainfo);
	}

	metainfos = (tsr_metainfo_t **) malloc(numalbums * sizeof(tsr_metainfo_t *));

	if (metainfos == 0)
	{
		tsr_exit_error(__FILE__, __LINE__, errno);
	}

	for (i = 0; i < numalbums; i++)
	{
		metainfos[i] = tsr_cli_metainfo_mb_bynum(mb_o, i + 1);
	}

	while (1)
	{
		for (i = 0; i < numalbums; i++)
		{
			printf("%i: ", i + 1);
			tsr_cli_print_metainfo_head(metainfos[i]);
		}

		printf("Select a number to view info or press Enter to leave: ");
		input = tsr_cli_read_str();

		if (*input == '\0')
		{
			for (i = 0; i < numalbums; i++)
			{
				tsr_metainfo_free(metainfos[i]);
			}

			free(metainfos);
			free(input);

			return NULL;
		}
		
		album = atoi(input);

		if (album > 0 && album <= numalbums)
		{
			metainfo = tsr_cli_metainfo_mb_finish(metainfos[album - 1]);

			if (metainfo == NULL)
			{
				free(input);
				continue;
			}

			for (i = 0; i < numalbums; i++)
			{
				if (i != album - 1)
				{
					tsr_metainfo_free(metainfos[i]);
				}
			}

			free(metainfos);
			free(input);

			return metainfo;
		}

		free(input);
	}

	return NULL;
}

/*
 * Get meta info from user by asking for input.
 *
 */
tsr_metainfo_t *tsr_cli_metainfo_input(int numtracks)
{
	int i;
	char *input;
	char *artist;
	tsr_metainfo_t *metainfo;

	metainfo = tsr_metainfo_new(numtracks);
	metainfo->numtracks = numtracks;
	metainfo->album = NULL;
	printf("Is this a multi-artist album? (y/N) ");
	input = tsr_cli_read_str();
	metainfo->ismultiple = (*input == 'y') ? 1 : 0;
	free(input);

	if (!metainfo->ismultiple)
	{
		printf("Album artist: ");
		artist = tsr_cli_read_str();
	}

	printf("Album: ");
	metainfo->album = tsr_cli_read_str();

	for (i = 0; i < numtracks; i++)
	{
		metainfo->trackinfos[i]->title = NULL;
		metainfo->trackinfos[i]->artist = NULL;
		printf("Title for Track %i: ", i + 1);
		metainfo->trackinfos[i]->title = tsr_cli_read_str();

		if (!metainfo->ismultiple)
		{
			metainfo->trackinfos[i]->artist = strdup(artist);
		}
		else
		{
			printf("Artist for Track %i: ", i + 1);
			metainfo->trackinfos[i]->artist = tsr_cli_read_str();
		}
	}

	free(artist);

	return metainfo;
}

/*
 * Get meta info, choose which way is ok ...
 *
 */
tsr_metainfo_t *tsr_cli_metainfo(musicbrainz_t mb_o, int numtracks)
{
	int numalbums;
	char *input = NULL;
	size_t read;
	tsr_metainfo_t *metainfo = NULL;

	printf("Querying musicbrainz database...");
	fflush(stdout);
	numalbums = tsr_mb_numalbums(mb_o);

	if (numalbums)
	{
		printf("\n");
		metainfo = tsr_cli_metainfo_mb(mb_o, numalbums);
	}
	else
	{
		printf(" Nothing found.\n");
	}

	while (metainfo == NULL)
	{
		printf("Do you want to (i)nput or (q)uit? ");
		input = tsr_cli_read_str();

		if (*input == 'q')
			break;

		if (*input == 'i')
		{
			metainfo = tsr_cli_metainfo_input(numtracks);
			break;
		}
	}

	if (input != NULL)
		free(input);

	return metainfo;
}

/*
 * Encode the specified track.
 *
 */
void tsr_cli_encode_track(int tracknum, tsr_metainfo_t *metainfo, cdrom_drive
		*drive, cdrom_paranoia *paranoia, tsr_cfg_t *cfg)
{
	char *filename, *read_buffer;
	int rtrack;
	long fsec, lsec, cursor, p, pp;
	tsr_trackfile_t *trackfile;

	filename = tsr_get_filename(cfg, metainfo, tracknum);
	rtrack = tracknum + 1;
	fsec = cdda_track_firstsector(drive, rtrack);
	lsec = cdda_track_lastsector(drive, rtrack);
	paranoia_seek(paranoia, fsec, SEEK_SET);
	trackfile = tsr_trackfile_init(tracknum, filename, metainfo);
	cursor = fsec;
	pp = 0;

	while (cursor <= lsec)
	{
		read_buffer = (char *) paranoia_read(paranoia, cb);

		if (!read_buffer)
		{
			tsr_trackfile_fail(trackfile);
			printf("\n");
			fflush(stdout);
			fprintf(stderr, "\nError reading Track %i\n", rtrack);
			exit(1);
		}

		tsr_trackfile_encode_next(trackfile, read_buffer);
		p = (cursor - fsec) * 100 / (lsec - fsec);

		if (p != pp)
		{
			printf("\rEncoding Track No. %02i/%02i... %3li%%", rtrack,
					metainfo->numtracks, p);
			fflush(stdout);
		}

		pp = p;
		cursor++;
	}

	tsr_trackfile_finish(trackfile);
}

/*
 * Handle command line arguments.
 * 
 */
void tsr_cli_handle_args(int argc, char **argv, tsr_cfg_t *cfg)
{
	int option, loption;
	struct option lopts[] =
	{
		{"multidisc", 0, 0, 'm'},
		{"device", 1, 0, 'd'},
		{"stripspaces", 0, 0, 's'},
		{"lowercase", 0, 0, 'l'},
		{"paranoiamode", 1, 0, 'p'}, 
		{"musicdir", 1, 0, 0},
		{"vorbisquality", 1, 0, 0},
		{"usage", 0, 0, 'u'},
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 'v'},
		{0, 0, 0, 0}
	};

	while ((option = getopt_long(argc, argv, "md:slp:uvh", lopts, &loption)) != -1)
	{
		switch (option)
		{
			case 0:
				if (!strcmp(lopts[loption].name, "musicdir"))
				{
					cfg->musicdir = strdup(optarg);
				}
				else if (!strcmp(lopts[loption].name, "vorbisquality"))
				{
					tsr_cfg_set_vorbisqualiy(cfg, optarg);
				}
				break;
			case 'm':
				cfg->multidisc = 1;
				break;
			case 'd':
				cfg->device = strdup(optarg);
				break;
			case 's':
				cfg->stripspaces = 1;
				break;
			case 'l':
				cfg->lowercase = 1;
				break;
			case 'p':
				tsr_cfg_set_paranoiamode(cfg, optarg);
				break;
			case 'v':
				tsr_cli_print_version();
				exit(EXIT_SUCCESS);
			case 'u':
				tsr_cli_print_usage();
				exit(EXIT_SUCCESS);
			case 'h':
				tsr_cli_print_help();
				exit(EXIT_SUCCESS);
		}
	}
}

/*
 * Main program.
 *
 */
int main(int argc, char **argv)
{
	int i;
	cdrom_drive *drive = NULL;
	cdrom_paranoia *paranoia;
	musicbrainz_t *mb_o;
	tsr_metainfo_t *metainfo;
	char *discinput;
	size_t read;
	tsr_cfg_t *cfg;

	cfg = tsr_cfg_init();
	tsr_cli_handle_args(argc, argv, cfg);
	printf("Initializing device... ");
	fflush(stdout);
	drive = (cfg->device) ?
		cdda_identify(cfg->device, CDDA_MESSAGE_FORGETIT, 0) : 
		cdda_find_a_cdrom(CDDA_MESSAGE_FORGETIT, 0);
		
	if (drive == NULL || cdda_open(drive))
	{
		char *device;

		device = (!drive) ? cfg->device : drive->ioctl_device_name;
		fprintf(stderr, "Can't open cdrom drive %s.\n", device);
		return EXIT_FAILURE;
	}

	printf("%s\n", drive->ioctl_device_name);
	mb_o = tsr_mb_init(drive->ioctl_device_name);
	metainfo = tsr_cli_metainfo(mb_o, drive->tracks);

	if (metainfo == NULL)
		return EXIT_SUCCESS;

	if (cfg->multidisc)
	{
		printf("Enter disc number (leave blank if there is only one): ");
		discinput = tsr_cli_read_str();
		metainfo->discnum = atoi(discinput);
		free(discinput);
	}
	else
	{
		metainfo->discnum = 0;
	}

	paranoia = paranoia_init(drive);
	paranoia_modeset(paranoia, cfg->paranoiamode);
	
	for (i = 0; i < metainfo->numtracks; i++)
		tsr_cli_encode_track(i, metainfo, drive, paranoia, cfg);

	tsr_metainfo_free(metainfo);
	printf("\nEncoded all tracks.\n");

	return EXIT_SUCCESS;
}
