#include <gs_reltype.h>
#include <error.h>

const char *gs_reltype_str(gs_reltype_e reltype)
{
    switch (reltype) {
        case GS_REL_SYS_NESTED:
            return "(nested)";
        case GS_REL_SYS_ARRAY:
            return "(array)";
        case GS_REL_USER:
            return "(user-def)";
        default: panic("Unknown relation type '%d'", reltype);
    }
}