/*
 * This file is part of donky.
 *
 * donky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * donky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with donky.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getArgs(int argc, char **argv, int *version, int *help, int *debug);
void showVersion(int bye);
void showHelp(int bye);

/**
 * @brief Program entry point.
 *
 * @param argc Argument count
 * @param argv Array of arguments
 * @return Exit status
 */
int main(int argc, char **argv)
{
        int version = 0, help = 0, debug = 0;
        getArgs(argc, argv, &version, &help, &debug);

        if (version)
                showVersion(1);
        else if (help)
                showHelp(1);
        else if (debug)
                printf("Look at all this sweet debuggin'.\n");
        else if (argc > 1)
                showHelp(1);

        return 0;
}

/** 
 * @brief Parse arguments.
 * 
 * @param argc Argument count
 * @param *argv Array of arguments
 * @param version Show version?
 * @param help Show help output?
 * @param debug Enable debugging?
 */
void getArgs(int argc, char **argv, int *version, int *help, int *debug)
{
        int i;

        for (i = 1; i < argc; i++)
        {
                if (argv[i][0] == '-')
                {
                        if (!strcmp(argv[i], "--version"))
                        {
                                *version = 1;
                                return;
                        }

                        else if (!strcmp(argv[i], "--help"))
                        {
                                *help = 1;
                                return;
                        }

                        else if (!strcmp(argv[i], "--debug"))
                                *debug = 1;
                }
        }
}

/** 
 * @brief Print version
 * 
 * @param bye Exit program if 1
 */
void showVersion(int bye)
{
        printf("donky 0.00\n");

        if (bye)
                exit(0);
}

/** 
 * @brief Print help
 * 
 * @param bye Exit program if 1
 */
void showHelp(int bye)
{
        showVersion(0);

        printf( "\nOptions:\n"
                        "  --version    Show donky's current version.\n"
                        "  --help       Show this message.\n"
                        "  --debug      Enable debugging output.\n");

        if (bye)
                exit(0);
}
