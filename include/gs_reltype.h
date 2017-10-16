#pragma once

typedef enum {
    GS_REL_SYS_NESTED,
    GS_REL_SYS_ARRAY,
    GS_REL_USER
} gs_reltype_e;

const char *gs_reltype_str(gs_reltype_e reltype);