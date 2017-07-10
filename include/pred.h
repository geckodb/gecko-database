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

typedef struct EXPR_NOT {
    struct EXPR *expr;
} EXPR_NOT;

typedef struct EXPR_CONST {
    bool value;
} EXPR_CONST;

struct FIELD;

typedef struct EXPR_VAR {
    enum comp_type comp;
    struct FIELD *lhs;
    struct FIELD *rhs;
} EXPR_VAR;

enum expr_type {
    ET_NOT,
    ET_CONST,
    ET_VAR
};

typedef struct EXPR {
    enum expr_type type;
    struct FIELD *field;
    void *expr;
} EXPR;

typedef struct PRED_TREE {
    struct PRED_TREE *or, *and; /*<! alternatives and mandatories related to this predicate */
    EXPR *expr; /*<! expression of this predicate */
} PRED_TREE;

// ---------------------------------------------------------------------------------------------------------------------
// I N T E R F A C E   D E C L A R A T I O N
// ---------------------------------------------------------------------------------------------------------------------

PRED_TREE *gs_pred_tree_create(EXPR *expr);

PRED_TREE *gs_pred_tree_and(PRED_TREE *subj, PRED_TREE *other);

PRED_TREE *gs_pred_tree_or(PRED_TREE *subj, PRED_TREE *other);

bool gs_pred_tree_eval(PRED_TREE *tree);

EXPR *gs_pred_expr_create_var(enum expr_type type, struct FIELD *field_lhs, struct FIELD *field_rhs);

EXPR *gs_pred_expr_create_const(enum expr_type type, struct FIELD *field, const void *value);

void gs_pred_expr_bind(EXPR *expr, const void *value);

void gs_pred_expr_bind2(EXPR *expr, const void *value_lhs, const void *value_rhs);

bool gs_pred_eval(EXPR *expr);

EXPR *gs_pred_expr_create_not(EXPR *other);

