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
 * file: tsr_track.h
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

tsr_trackfile_t *
tsr_trackfile_init(int tracknum, char *filename, tsr_metainfo_t *metainfo);
	
void
tsr_trackfile_encode_next(tsr_trackfile_t *track, char *read_buffer);

void
tsr_trackfile_fail(tsr_trackfile_t *trackfile);

void
tsr_trackfile_finish(tsr_trackfile_t *track);
