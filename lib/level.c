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

int write_level(level* lvl, char* into) {
  if (lvl == NULL)
  	return -1;
  if (into == NULL)
  	into = lvl->filename;
  nbt_node* node = nbt_parse_path(lvl->filename);
  if (node == NULL)
  	return -2;

  nbt_node* tmp = NULL;

#define UPDATE_NBT_STR(nbt_path, from) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_STRING) { \
  	free(tmp->payload.tag_string); \
  	tmp->payload.tag_string = strdup(lvl->from); \
  } else { goto error; };

#define UPDATE_NBT_LONG(nbt_path, from) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_LONG) { \
  	tmp->payload.tag_long = (long) lvl->from; \
  } else { goto error; };

#define UPDATE_NBT_BOOL(nbt_path, from) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_BYTE) { \
  	tmp->payload.tag_byte = lvl->from; \
  } else { goto error; };

#define UPDATE_NBT_INT(nbt_path, from) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_INT) { \
  	tmp->payload.tag_int = lvl->from; \
  } else { goto error; };

#define UPDATE_NBT_GAMERULE(nbt_path, from) \
  tmp = nbt_find_by_path(node, nbt_path); \
  if (tmp && tmp->type == TAG_STRING) { \
  	free(tmp->payload.tag_string); \
  	if (lvl->gamerules.from == 1) { \
  	  tmp->payload.tag_string = strdup("true"); \
  	} else { \
  	  tmp->payload.tag_string = strdup("false"); \
  	}; \
  } else { goto error; };

  UPDATE_NBT_STR(".Data.LevelName", levelname);
  UPDATE_NBT_LONG(".Data.RandomSeed", seed);
  UPDATE_NBT_LONG(".Data.LastPlayed", last_played);
  UPDATE_NBT_BOOL(".Data.allowCommands", allowCommands);
  UPDATE_NBT_BOOL(".Data.hardcore", hardcore);
  UPDATE_NBT_LONG(".Data.Time", time);
  UPDATE_NBT_LONG(".Data.DayTime", daytime);
  UPDATE_NBT_INT(".Data.SpawnX", spawn.x);
  UPDATE_NBT_INT(".Data.SpawnY", spawn.y);
  UPDATE_NBT_INT(".Data.SpawnZ", spawn.z);
  UPDATE_NBT_BOOL(".Data.raining", raining);
  UPDATE_NBT_BOOL(".Data.thundering", thundering);
  UPDATE_NBT_INT(".Data.rainTime", rainTime);
  UPDATE_NBT_INT(".Data.thunderTime", thunderTime);
  UPDATE_NBT_GAMERULE(".Data.GameRules.commandBlockOutput", commandBlockOutput);
  UPDATE_NBT_GAMERULE(".Data.GameRules.doDaylightCycle", doDaylightCycle);
  UPDATE_NBT_GAMERULE(".Data.GameRules.doFireTick", doFireTick);
  UPDATE_NBT_GAMERULE(".Data.GameRules.doMobLoot", doMobLoot);
  UPDATE_NBT_GAMERULE(".Data.GameRules.doMobSpawning", doMobSpawning);
  UPDATE_NBT_GAMERULE(".Data.GameRules.doTileDrops", doTileDrops);
  UPDATE_NBT_GAMERULE(".Data.GameRules.keepInventory", keepInventory);
  UPDATE_NBT_GAMERULE(".Data.GameRules.mobGriefing", mobGriefing);
  UPDATE_NBT_GAMERULE(".Data.GameRules.naturalRegeneration", naturalRegeneration);

#undef UPDATE_NBT_STR
#undef UPDATE_NBT_LONG
#undef UPDATE_NBT_ULONG
#undef UPDATE_NBT_INT
#undef UPDATE_NBT_BOOL
#undef UPDATE_NBT_GAMERULE

  FILE* f = fopen(into, "wb");
  if (f && nbt_dump_file(node, f, STRAT_GZIP) == NBT_OK) {
  	fclose(f);
  	nbt_free(node);
  	return 0;
  }
  fclose(f);
error:
  nbt_free(node);
  return -3;
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