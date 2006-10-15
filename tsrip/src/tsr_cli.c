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
void
tsr_cli_print_version()
{
	printf(PACKAGE " " VERSION "\n");
}

/*
 * Print usage.
 *
 */
void
tsr_cli_print_usage()
{
	printf("Usage: " PACKAGE " [options]\n");
	printf("	-m --multidisc			Ask for disc number\n"
	       "	-d --device <device>		Use device <device>\n"
	       "	-p --paranoiamode <mode>	Paranoia mode to use\n"
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
void
tsr_cli_print_help()
{
	tsr_cli_print_version();
	tsr_cli_print_usage();
}


/* 
 * Strip newline and set it as end of the string.
 *
 */
void
tsr_cli_stripnewline(char *str)
{
	char *nline;
	nline = strchr(str, '\n');
	if(nline != 0)
		*nline = 0;
}

/*
 * Print the head (album, artist) of the given metainfo.
 *
 */
void
tsr_cli_print_metainfo_head(tsr_metainfo_t *metainfo)
{

	printf("Album \"%s\" ", metainfo->album);
	if(!metainfo->ismultiple)
		printf("by %s\n", metainfo->trackinfos[0]->artist);
	else
		printf("by various Artists\n");
}

/*
 * Shows the given metainfo about an album.
 *
 */
void
tsr_cli_print_metainfo(tsr_metainfo_t *metainfo)
{
	int i;

	printf(" ----\n");
	tsr_cli_print_metainfo_head(metainfo);

	for(i = 0; i < metainfo->numtracks; i++)
	{
		printf("%i: %s", i + 1, metainfo->trackinfos[i]->title);
		if(metainfo->ismultiple)
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
{}

/*
 * Get metainfo for a single album, identifyed by number.
 *
 */
tsr_metainfo_t *
tsr_cli_metainfo_mb_bynum(musicbrainz_t mb_o, int numalbum)
{
	int i;
	tsr_metainfo_t *metainfo;

	metainfo = (tsr_metainfo_t *) malloc(sizeof(tsr_metainfo_t));
	if(!metainfo)
	{
		perror(__FILE__":"LINENO(__LINE__));
		exit(1);
	}

	metainfo->numtracks = tsr_mb_album_numtracks(mb_o, numalbum);
	metainfo->album = tsr_mb_album_name(mb_o, numalbum);
	metainfo->ismultiple = tsr_mb_album_ismultiple(mb_o, numalbum);

	metainfo->trackinfos = (tsr_trackinfo_t **) malloc(
			metainfo->numtracks * sizeof(tsr_trackinfo_t *));
	if(!metainfo->trackinfos)
	{
		perror(__FILE__":"LINENO(__LINE__));
		exit(1);
	}

	for(i = 0; i < metainfo->numtracks; i++)
	{
		metainfo->trackinfos[i] = (tsr_trackinfo_t *)
			malloc(sizeof(tsr_trackinfo_t));
		if(!metainfo->trackinfos[i])
		{
			perror(__FILE__":"LINENO(__LINE__));
			exit(1);
		}

		metainfo->trackinfos[i]->title = tsr_mb_track_title(mb_o,
				numalbum, i + 1);
		metainfo->trackinfos[i]->artist = tsr_mb_track_artist(mb_o,
				numalbum, i + 1);
	}

	return metainfo;
}

/*
 * Get a new input string and replace it with the old one, given as param.
 *
 */
void
tsr_cli_input_edit(char **str)
{
	size_t read;
	char *input = 0;


	getline(&input, &read, stdin);
	if(*input == '\n')
	{
		free(input);
		return;
	}

	tsr_cli_stripnewline(input);
	free(*str);
	*str = input;
}

/*
 * This function is dedicated to edit the artist on a normal album with one
 * artist. This function is very special, because the artist is stored for
 * each track in the metainfo, even if the album only has one artist.
 *
 */
void tsr_cli_artist_edit(tsr_metainfo_t *metainfo) { char *artist = 0; size_t
	len; int alen, i;

	getline(&artist, &len, stdin);
	if(*artist == '\n')
	{
		free(artist);
		return;
	} 

	tsr_cli_stripnewline(artist);
	alen = strlen(artist) + 1;

	for(i = 0; i < metainfo->numtracks; i++)
	{
		char *tartist;
		tsr_copystr(&tartist, artist);
		free(metainfo->trackinfos[i]->artist);
		metainfo->trackinfos[i]->artist = tartist;
	}
	free(artist);
}

/*
 * print musicbrainz metainfo, ask for edit and return metainfo, or null.
 * 
 */
tsr_metainfo_t *
tsr_cli_metainfo_mb_finish(tsr_metainfo_t *metainfo)
{
	char *selection = 0;
	size_t len;
	int track;

	while(1)
	{
		tsr_cli_print_metainfo(metainfo);

		printf("Use this information (Y/n) or do you want to edit (a)lbum");
		if(!metainfo->ismultiple)
			printf(", a(r)tist");
		printf(" or a track#? ");
		getline(&selection, &len, stdin);
		track = atoi(selection);

		if(*selection == 'y' || *selection == '\n')
		{
			free(selection);
			return metainfo;
		}
		else if(*selection == 'a')
		{
			printf("Enter new album name: ");
			tsr_cli_input_edit(&metainfo->album);
		}
		else if(*selection == 'r' && !metainfo->ismultiple)
		{
			printf("Enter new artist name: ");
			tsr_cli_artist_edit(metainfo);
		}
		else if(track > 0 && track <= metainfo->numtracks)
		{
			printf("Enter new title for track %i: ", track--);
			tsr_cli_input_edit(&metainfo->trackinfos[track]->title);

			if(metainfo->ismultiple)
			{
				printf("Enter new artist for track %i: ", track);
				tsr_cli_input_edit(&metainfo->trackinfos[track]->artist);
			}

		}
		else if(*selection == 'n')
		{
			free(selection);
			return 0;
		}

		free(selection);
		selection = 0;
	}

	return 0;
}

/*
 * Get meta information from musicbrainz database.
 *
 */
tsr_metainfo_t *
tsr_cli_metainfo_mb(musicbrainz_t mb_o, int numalbums)
{
	char *selection = 0;
	size_t len;
	int album, i;
	tsr_metainfo_t *metainfo;
	tsr_metainfo_t **metainfos;

	if(numalbums == 1)
	{
		metainfo = tsr_cli_metainfo_mb_bynum(mb_o, 1);
		return tsr_cli_metainfo_mb_finish(metainfo);
	}

	metainfos = (tsr_metainfo_t **) malloc(numalbums * sizeof(tsr_metainfo_t *));
	if(!metainfos)
	{
		perror(__FILE__":"LINENO(__LINE__));
		exit(1);
	}

	for(i = 0; i < numalbums; i++)
		metainfos[i] = tsr_cli_metainfo_mb_bynum(mb_o, i + 1);

	while(1)
	{
		for(i = 0; i < numalbums; i++)
		{
			printf("%i: ", i + 1);
			tsr_cli_print_metainfo_head(metainfos[i]);
		}

		printf("Select a number to view info or press Enter to leave: ");
		getline(&selection, &len, stdin);
		if(*selection == '\n')
		{
			for(i = 0; i < numalbums; i++)
				tsr_metainfo_free(metainfos[i]);
			free(metainfos);
			free(selection);
			return 0;
		}
		
		album = atoi(selection);
		if(album > 0 && album <= numalbums && (metainfo =
					tsr_cli_metainfo_mb_finish(metainfos[album-1])))
		{
			for(i = 0; i < numalbums; i++)
			{
				if(i != album - 1)
					tsr_metainfo_free(metainfos[i]);
			}
			free(metainfos);
			return metainfo;
		}
	}

	return 0;
}

/*
 * Get meta info from user by asking for input.
 *
 */
tsr_metainfo_t *
tsr_cli_metainfo_input(int numtracks)
{
	int i;
	char *artist = 0, *selection = 0;
	size_t a_read, t_read;

	tsr_metainfo_t *metainfo;

	metainfo = (tsr_metainfo_t *) malloc(sizeof(tsr_metainfo_t));
	if(!metainfo)
	{
		perror(__FILE__":"LINENO(__LINE__));
		exit(1);
	}

	metainfo->numtracks = numtracks;
	metainfo->album = 0;
	
	printf("Is this a multi-artist album? (y/N) ");
	getline(&selection, &a_read, stdin);
	metainfo->ismultiple = (*selection == 'y') ? 1 : 0;
	free(selection);

	if(!metainfo->ismultiple)
	{
		printf("Album artist: ");
		getline(&artist, &a_read, stdin);
		tsr_cli_stripnewline(artist);
	}


	printf("Album: ");
	getline(&metainfo->album, &t_read, stdin);
	tsr_cli_stripnewline(metainfo->album);

	metainfo->trackinfos = (tsr_trackinfo_t **) malloc(numtracks * sizeof(tsr_trackinfo_t *));
	if(!metainfo->trackinfos)
	{
		perror(__FILE__":"LINENO(__LINE__));
		exit(1);
	}

	for(i = 0; i < numtracks; i++)
	{
		tsr_trackinfo_t *trackinfo;

		trackinfo = (tsr_trackinfo_t *) malloc(sizeof(tsr_trackinfo_t));
		if(!trackinfo)
		{
			perror(__FILE__":"LINENO(__LINE__));
			exit(1);
		}

		trackinfo->title = 0;
		trackinfo->artist = 0;

		printf("Title for Track %i: ", i + 1);
		getline(&trackinfo->title, &t_read, stdin);
		tsr_cli_stripnewline(trackinfo->title);

		if(!metainfo->ismultiple)
			tsr_copystr(&trackinfo->artist, artist);
		else
		{
			printf("Artist for Track %i: ", i + 1);
			getline(&trackinfo->artist, &a_read, stdin);
			tsr_cli_stripnewline(trackinfo->artist);
		}

		metainfo->trackinfos[i] = trackinfo;
	}

	free(artist);
	return metainfo;
}

/*
 * Get meta info, choose which way is ok ...
 *
 */
tsr_metainfo_t *
tsr_cli_metainfo(musicbrainz_t mb_o, int numtracks)
{
	int numalbums;
	char *input = 0;
	size_t read;
	tsr_metainfo_t *metainfo = 0;

	printf("Querying musicbrainz database...");
	fflush(stdout);
	numalbums = tsr_mb_numalbums(mb_o);

	if(numalbums)
	{
		printf("\n");
		metainfo = tsr_cli_metainfo_mb(mb_o, numalbums);
	}
	else
		printf(" Nothing found.\n");

	while(!metainfo)
	{
		printf("Do you want to (i)nput or (q)uit? ");
		getline(&input, &read, stdin);

		if(*input == 'q')
			break;

		if(*input == 'i')
		{
			metainfo = tsr_cli_metainfo_input(numtracks);
			break;
		}
	}

	free(input);
	return metainfo;
}

/*
 * Encode the specified track.
 *
 */
void
tsr_cli_encode_track(int tracknum, char *musicdir, tsr_metainfo_t *metainfo,
		cdrom_drive *drive, cdrom_paranoia *paranoia)
{
	char *filename, *read_buffer;
	int rtrack;
	long fsec, lsec, cursor, p, pp;
	tsr_trackfile_t *trackfile;

	filename = tsr_get_filename(musicdir, metainfo, tracknum);
	rtrack = tracknum + 1;

	fsec = cdda_track_firstsector(drive, rtrack);
	lsec = cdda_track_lastsector(drive, rtrack);

	paranoia_seek(paranoia, fsec, SEEK_SET);

	trackfile = tsr_trackfile_init(tracknum, filename, metainfo);

	cursor = fsec;
	pp = 0;

	while(cursor <= lsec)
	{
		read_buffer = (char *) paranoia_read(paranoia, cb);

		if(!read_buffer)
		{
			tsr_trackfile_fail(trackfile);
			printf("\n");
			fflush(stdout);
			fprintf(stderr, "\nError reading Track %i\n", rtrack);
			exit(1);
		}

		tsr_trackfile_encode_next(trackfile, read_buffer);

		p = (cursor - fsec) * 100 / (lsec - fsec);

		if(p != pp)
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
void
tsr_cli_handle_args(int argc, char **argv, tsr_cfg_t *cfg)
{
	int option, loption;

	struct option lopts[] =
	{
		{"multidisc", 0, 0, 'm'},
		{"device", 1, 0, 'd'},
		{"paranoiamode", 1, 0, 'p'}, 
		{"musicdir", 1, 0, 0},
		{"vorbisquality", 1, 0, 0},
		{"usage", 0, 0, 'u'},
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 'v'},
		{0, 0, 0, 0}
	};

	while((option = getopt_long(argc, argv, "md:p:uvh", lopts, &loption)) != -1)
	{
		switch(option)
		{
			case 0:
				if(!strcmp(lopts[loption].name, "musicdir"))
					tsr_copystr(&cfg->musicdir, optarg);
				else if(!strcmp(lopts[loption].name, "vorbisquality"))
					tsr_cfg_set_vorbisqualiy(cfg, optarg);
				break;
			case 'm':
				cfg->multidisc = 1;
				break;
			case 'd':
				tsr_copystr(&cfg->device, optarg);
				break;
			case 'p':
				tsr_cfg_set_paranoiamode(cfg, optarg);
			case 'v':
				tsr_cli_print_version();
				exit(0);
			case 'u':
				tsr_cli_print_usage();
				exit(0);
			case 'h':
				tsr_cli_print_help();
				exit(0);
		}
	}
}

/*
 * Main program.
 *
 */
int
main(int argc, char **argv)
{
	int i;
	cdrom_drive *drive = 0;
	cdrom_paranoia *paranoia;
	musicbrainz_t *mb_o;
	tsr_metainfo_t *metainfo;
	char *discinput = 0;
	size_t read;
	tsr_cfg_t *cfg;

	cfg = tsr_cfg_init();

	tsr_cli_handle_args(argc, argv, cfg);

	printf("Initializing device... ");
	fflush(stdout);

	drive = (cfg->device) ?
		cdda_identify(cfg->device, CDDA_MESSAGE_FORGETIT, 0) : 
		cdda_find_a_cdrom(CDDA_MESSAGE_FORGETIT, 0);
		
	if(!drive || cdda_open(drive))
	{
		char *device;

		device = (!drive) ? cfg->device : drive->cdda_device_name;
			
		fprintf(stderr, "Can't open cdrom drive %s.\n", device);
		exit(1);
	}

	printf("%s\n", drive->cdda_device_name);

	mb_o = tsr_mb_init(drive->cdda_device_name);
	metainfo = tsr_cli_metainfo(mb_o, drive->tracks);

	if(!metainfo)
		exit(0);

	if(cfg->multidisc)
	{
		printf("Enter disc number (leave blank if there is only one): ");
		getline(&discinput, &read, stdin);

		metainfo->discnum = atoi(discinput);
		free(discinput);
	}
	else
		metainfo->discnum = 0;

	paranoia = paranoia_init(drive);
	paranoia_modeset(paranoia, cfg->paranoia_mode);
	
	for(i = 0; i < metainfo->numtracks; i++)
		tsr_cli_encode_track(i, cfg->musicdir, metainfo, drive, paranoia);

	tsr_metainfo_free(metainfo);
	printf("\nEncoded all tracks.\n");

	return 0;
}
