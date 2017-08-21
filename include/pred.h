/*
 * Copyright (C) 2017 Marcus Pinnecke
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdinc.h>

// ---------------------------------------------------------------------------------------------------------------------
// T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

enum comp_type {
    CT_LESS, CT_LESSEQ, CT_EQUALS, CT_GREATEREQ, CT_GREATER
};

typedef struct expr_not_t {
    struct expr_t *expr;
} expr_not_t;

typedef struct expr_const_t {
    bool value;
} expr_const_t;

struct tuplet_field_t;

typedef struct expr_var_t {
    enum comp_type comp;
    struct tuplet_field_t *lhs;
    struct tuplet_field_t *rhs;
} expr_var_t;

enum expr_type {
    ET_NOT,
    ET_CONST,
    ET_VAR
};

typedef struct expr_t {
    enum expr_type type;
    struct tuplet_field_t *field;
    void *expr;
} expr_t;

typedef struct pred_tree_t {
    struct pred_tree_t *or, *and; /*<! alternatives and mandatories related to this predicate */
    expr_t *expr; /*<! expression of this predicate */
} pred_tree_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

pred_tree_t *gs_pred_tree_create(expr_t *expr);

pred_tree_t *gs_pred_tree_and(pred_tree_t *subj, pred_tree_t *other);

pred_tree_t *gs_pred_tree_or(pred_tree_t *subj, pred_tree_t *other);

bool gs_pred_tree_eval(pred_tree_t *tree);

expr_t *gs_pred_expr_create_var(enum expr_type type, struct tuplet_field_t *field_lhs, struct tuplet_field_t *field_rhs);

expr_t *gs_pred_expr_create_const(enum expr_type type, struct tuplet_field_t *field, const void *value);

void gs_pred_expr_bind(expr_t *expr, const void *value);

void gs_pred_expr_bind2(expr_t *expr, const void *value_lhs, const void *value_rhs);

bool gs_pred_eval(expr_t *expr);

expr_t *gs_pred_expr_create_not(expr_t *other);

