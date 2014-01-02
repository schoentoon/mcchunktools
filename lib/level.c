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

#include "mcchunk.h"

#include <string.h>

level* open_level(char* filename) {
  nbt_node* node = nbt_parse_path(filename);
  if (node == NULL)
  	return NULL;
  level* lvl = malloc(sizeof(level));
  bzero(lvl, sizeof(level));

  lvl->filename = strdup(filename);
  nbt_node* tmp = NULL;

#define FROM_NBT_STR(nbt_path, into) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_STRING) { \
  	lvl->into = strdup(tmp->payload.tag_string); \
  } else { goto error; };

#define FROM_NBT_LONG(nbt_path, into) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_LONG) { \
  	lvl->into = tmp->payload.tag_long; \
  } else { goto error; };

#define FROM_NBT_ULONG(nbt_path, into) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_LONG && tmp->payload.tag_long >= 0) { \
  	lvl->into = (unsigned) tmp->payload.tag_long; \
  } else { goto error; };

#define FROM_NBT_BOOL(nbt_path, into) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_BYTE && (tmp->payload.tag_byte == 0 || tmp->payload.tag_byte == 1)) { \
  	lvl->into = (unsigned) tmp->payload.tag_byte; \
  } else { goto error; };

#define FROM_NBT_INT(nbt_path, into) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_INT) { \
  	lvl->into = tmp->payload.tag_int; \
  } else { goto error; };

/** What smartass decides these should be true/false strings? God damn it..
 */
#define FROM_NBT_GAMERULE(nbt_path, into) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_STRING) { \
    if (strcmp(tmp->payload.tag_string, "true") == 0) { \
  	  lvl->gamerules.into = 1; \
  	}; \
  } else { goto error; };

  FROM_NBT_STR(".Data.LevelName", levelname);
  FROM_NBT_LONG(".Data.RandomSeed", seed);
  FROM_NBT_ULONG(".Data.LastPlayed", last_played);
  FROM_NBT_BOOL(".Data.allowCommands", allowCommands);
  FROM_NBT_BOOL(".Data.hardcore", hardcore);
  FROM_NBT_ULONG(".Data.Time", time);
  FROM_NBT_ULONG(".Data.DayTime", daytime);
  FROM_NBT_INT(".Data.SpawnX", spawn.x);
  FROM_NBT_INT(".Data.SpawnY", spawn.y);
  FROM_NBT_INT(".Data.SpawnZ", spawn.z);
  FROM_NBT_BOOL(".Data.raining", raining);
  FROM_NBT_BOOL(".Data.thundering", thundering);
  FROM_NBT_INT(".Data.rainTime", rainTime);
  FROM_NBT_INT(".Data.thunderTime", thunderTime);
  FROM_NBT_GAMERULE(".Data.GameRules.commandBlockOutput", commandBlockOutput);
  FROM_NBT_GAMERULE(".Data.GameRules.doDaylightCycle", doDaylightCycle);
  FROM_NBT_GAMERULE(".Data.GameRules.doFireTick", doFireTick);
  FROM_NBT_GAMERULE(".Data.GameRules.doMobLoot", doMobLoot);
  FROM_NBT_GAMERULE(".Data.GameRules.doMobSpawning", doMobSpawning);
  FROM_NBT_GAMERULE(".Data.GameRules.doTileDrops", doTileDrops);
  FROM_NBT_GAMERULE(".Data.GameRules.keepInventory", keepInventory);
  FROM_NBT_GAMERULE(".Data.GameRules.mobGriefing", mobGriefing);
  FROM_NBT_GAMERULE(".Data.GameRules.naturalRegeneration", naturalRegeneration);

#undef FROM_NBT_STR
#undef FROM_NBT_LONG
#undef FROM_NBT_ULONG
#undef FROM_NBT_INT
#undef FROM_NBT_BOOL
#undef FROM_NBT_GAMERULE

  nbt_free(node);
  return lvl;
error:
  nbt_free(node);
  free_level(lvl);
  return NULL;
}

void free_level(level* lvl) {
  if (lvl) {
	if (lvl->filename)
      free(lvl->filename);
	if (lvl->levelname)
	  free(lvl->levelname);
	free(lvl);
  }
}