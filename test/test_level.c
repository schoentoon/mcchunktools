/*  mcchunktools
 *  Copyright (C) 2013-2014  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __cplusplus
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include "mcchunk.h"

#include "insist.h"

int main(int argc, char** argv) {
	level* lvl = open_level("testdata/level.dat");
	insist(lvl != NULL, "open_level returned NULL anyway..");
	lvl->hardcore = 1;
	lvl->gamerules.naturalRegeneration = 0;
	int ret = write_level(lvl, NULL);
	insist(ret == 0, "write_level returned %d instead of 0", ret);
	free_level(lvl);
	lvl = open_level("testdata/level.dat");
	insist(lvl != NULL, "open_level returned NULL anyway..");
	insist(lvl->hardcore == 1, "lvl->hardcore wasn't correctly set to 1? :/");
	free_level(lvl);
	return 0;
}