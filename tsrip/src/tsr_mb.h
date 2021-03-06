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
 * file: tsr_mb.h
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

#include <musicbrainz/mb_c.h>

musicbrainz_t tsr_mb_init(char *device);

int tsr_mb_numalbums(musicbrainz_t mb_o);

int tsr_mb_album_numtracks(musicbrainz_t mb_o, int numalbum);

int tsr_mb_album_ismultiple(musicbrainz_t mb_o, int numalbum);

char *tsr_mb_album_name(musicbrainz_t mb_o, int numalbum);
	
char *tsr_mb_track_artist(musicbrainz_t mb_o, int numalbum, int numtrack);

char *tsr_mb_track_title(musicbrainz_t mb_o, int numalbum, int numtrack);
