/*  mcchunktools
 *  Copyright (C) 2013  Toon Schoenmakers
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

#ifndef _MCCHUNK_H
#define _MCCHUNK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "nbt.h"

#define SECTOR_BYTES 4096
#define SECTOR_INTS 1024

typedef struct {
  uint32_t offsets[SECTOR_INTS];
  uint32_t timestamps[SECTOR_INTS];
  char* filename;
} regionfile;

/** Used to open a region file simply pass
 * the filename into it
 */
regionfile* open_regionfile(char* filename);

/** Used to free the region structure
 */
void free_region(regionfile* region);

/** Count the amount of chunks available in this
 * region structure
 */
size_t count_chunks(regionfile* region);

/** Loop through all the available chunks in our regionfile
 * don't do some odd for loop yourself calling get_chunk as
 * this function is simply more efficient, it keeps the file
 * open for example. (And it's just easier..)
 * Also don't keep node around, as it is freed automatically.
 */
void for_each_chunk(regionfile* region, void *function(nbt_node* node));

/** Check if this region structure contains the
 * chunk located at x: cx y: cy returns 1 if it
 * is in the region structure, 0 otherwise
 * ingame you can get the chunk coordinates from
 * the debug screen (F3). It'll look something like
 * this:
 * x: -20.64464 (-21) // c: -2 (11)
 * y: 120.500 (feet pos, 122.120 eyes pos)
 * z: 36.39552 (36) // c: 2 (4)
 *
 * The x, y & z coordinates here are your players location
 * the number just behind "c:" is the chunk number. So in
 * this case it would be x: -2, z: 2.
 */
int region_contains_chunk(regionfile* region, int32_t cx, int32_t cz);

/** Returns the nbt structure for this particular chunk
 * @see region_contains_chunk @see chunk_from_coord
 */
nbt_node* get_raw_chunk(regionfile* region, int32_t cx, int32_t cz);

/** Calculate in which chunk this coordinate is
 */
#define chunk_from_coord(i) (i/16)

/** Determine in which region file the chunk cx, cz
 * is located. @see region_contains_chunk @see chunk_from_coord
 * Don't forget to free the output!
 */
char* determine_region_file(int32_t cx, int32_t cz);

/** Initialize the block names database
 * you must call this once before using get_block_name
 */
void initblockdb();

/** Get the human readable name for block_id with data
 * if the block simply doesn't exist it'll return NULL
 */
char* get_block_name(uint8_t block_id, uint8_t data);

#define CHUNK_WIDTH 16
#define CHUNK_LENGTH 16
#define CHUNK_HEIGHT 256

typedef struct {
  int32_t x;
  int32_t z;
  int8_t blocks[CHUNK_WIDTH][CHUNK_LENGTH][CHUNK_HEIGHT];
  int8_t data[CHUNK_WIDTH][CHUNK_LENGTH][CHUNK_HEIGHT];
  int8_t biomes[CHUNK_WIDTH][CHUNK_LENGTH];
} chunk;

/** Get a chunk structure for the chunk located at cx, cz in
 * region. Blocks in this structure are accessed using the
 * blocks and data arrays, it would look like the following:
 * c->blocks[x][z][y];
 */
chunk* get_chunk(regionfile* region, int32_t cx, int32_t cz);

/** Simple macro to free a chunk structure, might be changed in
 * the future if we actually alloc stuff internally in the chunk
 * structure.
 */
#define free_chunk(c) free(c);

#endif //_MCCHUNK_H