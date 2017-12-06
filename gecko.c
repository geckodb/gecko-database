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

#include <gecko-commons/gecko-commons.h>

#include <gs_info.h>
#include <gs_system.h>

#include <records/gs_value.h>

// curl -i -G -d "key=val" -d "abs=[1,2,3,4]" http://localhost:36895/api/test

gs_system_t *system_instance;

void setup_config(int argc, char **argv);

int main(int argc, char* argv[])
{
    setup_config(argc, argv);

    gs_system_create(&system_instance, startup_config.port);
    gs_system_start(system_instance);
    gs_system_cleanup(system_instance);

    return EXIT_SUCCESS;
}

 void setup_config(int argc, char **argv)
{
    startup_config.port = 35497;

    argp_parse (&argp, argc, argv, 0, 0, &startup_config);
}

