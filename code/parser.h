#ifndef PARSER_H
#define PARSER_H

#include "common.h"

void parse_user_input(int argc, char *argv[], 
                      char** fileIn, char** fileOut,
                      bool* isArchiving);

#endif