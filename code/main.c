/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>

#include "common.h"
#include "parser.h"

int main(int argc, char const *argv[])
{
    char *fileIn, *fileOut;
    bool isArchiving;
    parse_user_input(argc, (char**) argv, &fileIn, &fileOut, &isArchiving);

#ifdef VERBOSE
    printf("Parsed input: fileIn = %s, fileOut = %s, isArchiving = %d\n", 
            fileIn, fileOut, isArchiving);
#endif

    return 0;
}