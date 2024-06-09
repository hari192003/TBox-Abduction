/**************************************************************/
/* ********************************************************** */
/* *                                                        * */
/* *        DFG LEXER STRING TABLE                          * */
/* *                                                        * */
/* *  $Module: DFG_STRING_TABLE_H                           * */ 
/* *                                                        * */
/* *  Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001      * */
/* *  MPI fuer Informatik                                   * */
/* *                                                        * */
/* *  This program is free software; you can redistribute   * */
/* *  it and/or modify it under the terms of the FreeBSD    * */
/* *  Licence.                                              * */
/* *                                                        * */
/* *  This program is distributed in the hope that it will  * */
/* *  be useful, but WITHOUT ANY WARRANTY; without even     * */
/* *  the implied warranty of MERCHANTABILITY or FITNESS    * */
/* *  FOR A PARTICULAR PURPOSE.  See the LICENCE file       * */ 
/* *  for more details.                                     * */
/* *                                                        * */
/* *                                                        * */
/* $Revision: 1.0 $                                         * */
/* $State: Exp $                                            * */
/* $Date: 2016/03/23 16:54:37 $                             * */
/* $Author: weidenb $                                       * */
/* *                                                        * */
/* *             Contact:                                   * */
/* *             Christoph Weidenbach                       * */
/* *             MPI fuer Informatik                        * */
/* *             Stuhlsatzenhausweg 85                      * */
/* *             66123 Saarbruecken                         * */
/* *             Email: spass@mpi-inf.mpg.de                * */
/* *             Germany                                    * */
/* *                                                        * */
/* ********************************************************** */
/**************************************************************/

#include "dfg_string_table.h"
#include "dfg_util.h"

#define DEBUG_HASH 0

#define BLOCK_SIZE (1024 * 1024)

/**
 * Internal state of the string table.
 */
static struct {
	string_t* table;    /* The hash table of strings. */
	size_t    capacity; /* The capacity of the string hash table.  Must be a power of 2. */
	size_t    size;     /* The number of used slots of the string hash table. */
	char*     buf;
	size_t    gap;
} strings;

/**
 * Calculate a hash value of the string s.
 *
 * @return The hash for the string s.
 */
static u4 string_hash(char const* const s)
{
#if 0
	size_t hash = 0;
	for (char const* i = s; *i != '\0'; ++i) {
#	if 1
		hash = (hash + (unsigned char)*i) * 17;
#	else
		hash = (hash ^ (unsigned char)*i) * 31;
#endif
	}
#else
	u4 hash = 2166136261U;
	for (char const* i = s; *i != '\0'; ++i) {
#if 1
		// FNV-1a
		hash = (hash ^ (unsigned char)*i) * 16777619U;
#else
		// FNV-1
		hash = hash * 16777619U ^ (unsigned char)*i;
#endif
	}
#endif
	return hash;
}

string_t const* string_table_insert(char const* const s, size_t const size, int const kind)
{
	/* The string table uses quadratic probing to resolve collisions. */

	/* Double the size of the hash table and rehash all entries of it is two
	 * thirds full. */
	if (strings.size * 3 >= strings.capacity * 2) {
		size_t    const new_cap = strings.capacity * 2;
		string_t* const new_tab = MALLOCNZ(string_t, new_cap);
#if DEBUG_HASH
		size_t max_coll  = 0;
		size_t sum_coll  = 0;
		size_t n_strings = 0;
#endif
		for (string_t* i = strings.table, *const end = i + strings.capacity; i != end; ++i) {
			if (i->text) {
				for (u4 k = i->hash, step = 0;; k += ++step) {
					string_t* const entry = &new_tab[k & (new_cap - 1)];
					if (!entry->text) {
						*entry = *i;
#if DEBUG_HASH
						sum_coll  += step;
						n_strings += 1;
						if (max_coll < step)
							max_coll = step;
#endif
						break;
					}
				}
			}
		}
#if DEBUG_HASH
		fprintf(stderr, "---> rehash #strings %7zu max %2zu avg %f\n", n_strings, max_coll, (double)sum_coll / n_strings);
#endif
		free(strings.table);
		strings.capacity = new_cap;
		strings.table    = new_tab;
	}

	u4 const hash = string_hash(s);
	for (u4 i = hash, step = 0;; i += ++step) {
		string_t* const entry = &strings.table[i & (strings.capacity - 1)];
		if (!entry->text) {
			/* The string is not in the string table.
			 * Enter it into the table and return the new entry. */
			if (strings.gap < size) {
				strings.gap = size > BLOCK_SIZE ? size : BLOCK_SIZE;
				strings.buf = MALLOCN(char, strings.gap);
			}
			entry->text  = memcpy(strings.buf, s, size);
			strings.buf += size;
			strings.gap -= size;
			entry->kind  = kind;
			entry->hash  = hash;
			++strings.size;
			return entry;
		} else if (entry->hash == hash && streq(entry->text, s)) {
			/* The string is already in the string table.
			 * Return the existing entry. */
			return entry;
		}
	}
}

void string_table_init(void)
{
	strings.size     =   0;
	strings.capacity = 256;
	strings.table    = MALLOCNZ(string_t, strings.capacity);
	strings.buf      =   0;
	strings.gap      =   0;
}
