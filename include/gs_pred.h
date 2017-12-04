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

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <gecko-commons/gecko-commons.h>

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

enum gs_comp_type_e {
    CT_LESS, CT_LESSEQ, CT_EQUALS, CT_GREATEREQ, CT_GREATER
};

typedef struct gs_expr_not_t_e {
    struct gs_expr_t *expr;
} gs_expr_not_t_e;

typedef struct gs_expr_const_t {
    bool value;
} gs_expr_const_t;

struct gs_tuplet_field_t;

typedef struct gs_expr_var_t {
    enum gs_comp_type_e comp;
    struct gs_tuplet_field_t *lhs;
    struct gs_tuplet_field_t *rhs;
} gs_expr_var_t;

enum gs_expr_type_e {
    ET_NOT,
    ET_CONST,
    ET_VAR
};

typedef struct gs_expr_t {
    enum gs_expr_type_e type;
    struct gs_tuplet_field_t *field;
    void *expr;
} gs_expr_t;

typedef struct gs_pred_tree_t {
    struct gs_pred_tree_t *or, *and; /*<! alternatives and mandatories related to this predicate */
    gs_expr_t *expr; /*<! expression of this predicate */
} gs_pred_tree_t;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

gs_pred_tree_t *pred_tree_create(gs_expr_t *expr);
gs_pred_tree_t *pred_tree_and(gs_pred_tree_t *subj, gs_pred_tree_t *other);
gs_pred_tree_t *pred_tree_or(gs_pred_tree_t *subj, gs_pred_tree_t *other);
bool pred_tree_eval(gs_pred_tree_t *tree);
gs_expr_t *pred_expr_create_var(enum gs_expr_type_e type, struct gs_tuplet_field_t *field_lhs, struct gs_tuplet_field_t *field_rhs);
gs_expr_t *pred_expr_create_const(enum gs_expr_type_e type, struct gs_tuplet_field_t *field, const void *value);
void pred_expr_bind(gs_expr_t *expr, const void *value);
void pred_expr_bind2(gs_expr_t *expr, const void *value_lhs, const void *value_rhs);
bool pred_eval(gs_expr_t *expr);
gs_expr_t *pred_expr_create_not(gs_expr_t *other);

