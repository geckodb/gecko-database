// An implementation of the linear hash table data structure
// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <containers/dictionaries/hash_tables/linear_hash_table.h>
#include <require.h>
#include <msg.h>

// ---------------------------------------------------------------------------------------------------------------------
// N O N - P U B L I C   M E T H O D   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void this_destruct(void *self);
void this_to_string(void *self, FILE *out);

// ---------------------------------------------------------------------------------------------------------------------
// B A S E   P  R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

bool this_clear(void *self);
bool this_is_empty(const void *self);
bool this_contains_values(const void *self, size_t num_values, const void *values);
bool this_contains_keys(const void *self, size_t num_keys, const void *keys);
const void *this_get(const void *self, const void *key);
vector_t *this_gets(const void *self, size_t num_keys, const void *keys);
bool this_remove(void *self, size_t num_keys, const void *keys);
bool this_put(void *self, const void *key, const void *value);
size_t this_num_elements(void *self);

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void linear_hash_table_construct(linear_hash_table_t *table, hash_code_fn_t hash_code_fn,
                                 hash_fn_t hash_fn, size_t key_size, size_t elem_size)
{
    require_nonnull(table);
    require_nonnull(hash_code_fn);
    require_nonnull(hash_fn);

    hash_table_create(&table->protected.member.base, hash_code_fn, hash_fn, key_size, elem_size,
                      this_clear, this_is_empty, this_contains_values, this_contains_keys, this_get,
                      this_gets, this_remove, this_put, this_num_elements);
    hash_table_override_to_string(&table->protected.member.base, this_to_string);

    table->public.methods.destruct = this_destruct;

    // TODO: allocate resources
}

// ---------------------------------------------------------------------------------------------------------------------
// B A S E   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

bool this_clear(void *self)
{
    printf("clear linear hash table...\n");
    return true;
}

bool this_is_empty(const void *self)
{
    printf("is empty linear hash table...\n");
    return true;
}

bool this_contains_values(const void *self, size_t num_values, const void *values)
{
    printf("contains_values linear hash table...\n");
    return true;
}

bool this_contains_keys(const void *self, size_t num_keys, const void *keys)
{
    printf("contains_keys linear hash table...\n");
    return true;
}

const void *this_get(const void *self, const void *key)
{
    printf("get linear hash table...\n");
    return NULL;
}

vector_t *this_gets(const void *self, size_t num_keys, const void *keys)
{
    printf("gets linear hash table...\n");
    return NULL;
}

bool this_remove(void *self, size_t num_keys, const void *keys)
{
    printf("remove linear hash table...\n");
    return NULL;
}

bool this_put(void *self, const void *key, const void *value)
{
    printf("put linear hash table...\n");
    return NULL;
}

size_t this_num_elements(void *self)
{
    printf("num_elements linear hash table...\n");
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// N O N - P U B L I C   M E T H O D   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void this_destruct(void *self)
{
    require_nonnull(self);
//    linear_hash_table_t *table = (linear_hash_table_t *) self;
    // TODO: free resources
}

void this_to_string(void *self, FILE *out)
{
    require_nonnull(self);
    require_nonnull(out);
    fprintf(out, "linear_hash_table(adr=%p)", self);
}