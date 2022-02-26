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
#include <stdlib.h>

#include "common.h"
#include "data.h"
#include "parser.h"
#include "stats.h"

int main(int argc, char const *argv[])
{
    Data data;
    initData(&data);
    parse_user_input(argc, (char**) argv, &data);

#ifdef VERBOSE
    printf("Parsed input: fileIn = %s, fileOut = %s, isArchiving = %d, algorithmType = %d\n", 
            data.fileIn, data.fileOut, data.isArchiving, data.algorithmType);
#endif

    int success;
    if (data.isArchiving)
        success = archive(&data);//TODO
    else
        success = unarchive(&data);//TODO
    if (success != 0) {
        printf("Unsuccessful operation.");
        exit(success);
    }
    output_stats(&data);
    return 0;
}