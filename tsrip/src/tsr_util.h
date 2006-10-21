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
 * file: tsr_util.h
 * Author: Sven Salzwedel <sven_salzwedel@web.de>
 *
 */

char *tsr_get_filename(tsr_cfg_t *cfg, tsr_metainfo_t *metainfo, int tracknum);

void tsr_exit_error(char *file, int line, int err);

tsr_metainfo_t *tsr_metainfo_new(int size);

void tsr_metainfo_free(tsr_metainfo_t *metainfo);
