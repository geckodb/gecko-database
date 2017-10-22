// Several pre-defined functions related to hash operations
// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
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

#include <gs_hash.h>

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------------------------
// I D E N T I T Y   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_identity_size_t(const void *_, size_t key_size, const void *key)
{
    assert (key_size == sizeof(size_t) && (key != NULL));
    return *((size_t *)key);
}

// A nice tutorial on several hash functions can be found here:
// http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// The following hash function strongly based on the description and code given in the website above

// ---------------------------------------------------------------------------------------------------------------------
// P E R F E C T   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_perfect_size_t(const void *capture, size_t key_size, const void *key)
{
    panic("Not implemented '%s'. Use 'capture' argument to capture information regarding the construction of keys.",
          "gs_hash_code_perfect_size_t");
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// A D D I T I V E   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_additive(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 0;
    for (size_t i = 0; i < key_size; i++) {
        hash += ((unsigned char* )key)[i];
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// X O R   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_xor(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 0;
    for (size_t i = 0; i < key_size; i++) {
        hash ^= ((unsigned char* )key)[i];
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// R O T A T I O N   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_rot(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 0;
    for (size_t i = 0; i < key_size; i++) {
        hash ^= (hash << 4) ^ (hash >> 28) ^ ((unsigned char* )key)[i];
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// B E R N S T E I N   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_bernstein(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 0;
    for (size_t i = 0; i < key_size; i++) {
        hash ^= 33 * hash + ((unsigned char* )key)[i];
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// M O D I F I E D   B E R N S T E I N   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_bernstein2(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 0;
    for (size_t i = 0; i < key_size; i++) {
        hash ^= 33 * hash ^ ((unsigned char* )key)[i];
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// S H I F T - A D D - X O R   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_sax(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 0;
    for (size_t i = 0; i < key_size; i++) {
        hash ^= (hash << 5) + (hash >> 2) + ((unsigned char* )key)[i];
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// F N V   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_fnv(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 2166136261;
    for (size_t i = 0; i < key_size; i++) {
        hash = (hash * 16777619) ^ ((unsigned char* )key)[i];
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// O N E - A T - A - T I M E   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_oat(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 0;
    for (size_t i = 0; i < key_size; i++) {
        hash += ((unsigned char* )key)[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// J S W   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t RAND_NUM[UCHAR_MAX];

__attribute__((constructor))
void init_rand_num()
{
    for (unsigned i = 0; i < UCHAR_MAX; i++)
        RAND_NUM[i] = rand();
}

size_t gs_hash_code_jsw(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 16777551;
    for (size_t i = 0; i < key_size; i++) {
        hash = (hash << 1 | hash >> 31) ^ RAND_NUM[((unsigned char* )key)[i]];
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// E L F   H A S H
// ---------------------------------------------------------------------------------------------------------------------

size_t gs_hash_code_elf(const void *_, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    size_t hash = 0, g;
    for (size_t i = 0; i < key_size; i++) {
        hash = (hash << 4) + ((unsigned char* )key)[i];
        if ((g = hash & 0xf0000000L) != 0) {
            hash ^= g >> 24;
        }
        hash &= ~g;
    }
    return hash;
}

// ---------------------------------------------------------------------------------------------------------------------
// J E N K I N S   H A S H
// ---------------------------------------------------------------------------------------------------------------------

#define mix(a,b,c) \
{ \
    a -= b; a -= c; a ^= (c >> 13); \
    b -= c; b -= a; b ^= (a << 8); \
    c -= a; c -= b; c ^= (b >> 13); \
    a -= b; a -= c; a ^= (c >> 12); \
    b -= c; b -= a; b ^= (a << 16); \
    c -= a; c -= b; c ^= (b >> 5); \
    a -= b; a -= c; a ^= (c >> 3); \
    b -= c; b -= a; b ^= (a << 10); \
    c -= a; c -= b; c ^= (b >> 15); \
}

size_t gs_hash_code_jen(const void *capture, size_t key_size, const void *key)
{
    assert ((key != NULL) && (key_size > 0));

    unsigned a, b;
    unsigned c = (capture != NULL ? ((gs_hash_code_jen_args_t *) capture)->initval : 0);
    unsigned char *k = (unsigned char *) key;

    a = b = 0x9e3779b9;

    while (key_size >= 12) {
        a += (k[0] + ((unsigned)k[1] << 8) + ((unsigned)k[2] << 16) + ((unsigned)k[3] << 24));
        b += (k[4] + ((unsigned)k[5] << 8) + ((unsigned)k[6] << 16) + ((unsigned)k[7] << 24));
        c += (k[8] + ((unsigned)k[9] << 8) + ((unsigned)k[10] << 16) + ((unsigned)k[11] << 24));
        mix(a, b, c);
        k += 12;
        key_size -= 12;
    }

    c += key_size;

    switch (key_size) {
        case 11: c += ((unsigned)k[10] << 24);
        case 10: c += ((unsigned)k[9] << 16);
        case 9: c += ((unsigned)k[8] << 8);
        case 8: b += ((unsigned)k[7] << 24);
        case 7: b += ((unsigned)k[6] << 16);
        case 6: b += ((unsigned)k[5] << 8);
        case 5: b += k[4];
        case 4: a += ((unsigned)k[3] << 24);
        case 3: a += ((unsigned)k[2] << 16);
        case 2: a += ((unsigned)k[1] << 8);
        case 1: a += k[0];
    }
    mix(a, b, c);
    return c;
}