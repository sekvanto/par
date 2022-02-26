#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

#define DEFAULT_FILEIN "test.txt"

#define ARCHIVE "-a"
#define UNARCHIVE "-u"

static void error(const char* message) {
    printf("%s", message);
    exit(FAILURE);
}

static bool ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return false;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return false;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

static bool strequal(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

/*
 * Adds filename extension, e.g. "file.txt" -> "file.txt.uar"
 */
static char* add_suffix(const char *str, const char *suffix) {
    int strLen = strlen(str);
    int suffixLen = strlen(suffix);

    char* subtext = malloc(strLen + suffixLen + 1);
    strncpy(subtext, str, strLen);
    strncpy(subtext + strLen, suffix, suffixLen);
    
    subtext[strLen + suffixLen] = '\0';
    return subtext;
} 

/* 
 * Replaces filename extension, e.g. "file.txt" -> "file.par" 
 */
static char* replace_suffix(const char *str, const char *suffix) {
    char* lastDot = strrchr(str, '.');
    int strLen = lastDot - str;
    int suffixLen = strlen(suffix);

    char* subtext = malloc(strLen + suffixLen + 1);
    strncpy(subtext, str, strLen);
    strncpy(subtext + strLen, suffix, suffixLen);
    
    subtext[strLen + suffixLen] = '\0';
    return subtext;
}

static void char_array_copy(char **dst, char **src, int count) {
    for(; count > 0; count--) {
        *dst++ = *src++;
    }
}

/*
 * Takes arguments where only input filename is specified.
 * Offset == 1 if options specified
 * Determines output filename and returns it
 */
static char* determine_out_file(int argc, char *argv[],
                                int offset, bool* isArchiving) {
    /* Archiving in these cases */
    if (strequal(argv[0], ARCHIVE) || (!ends_with(argv[0], ".par") && offset == 0)) {
        *isArchiving = true;
        return add_suffix(argv[offset], ".par");
    }

    /* Unarchiving otherwise */
    *isArchiving = false;

    if (ends_with(argv[offset], ".par")) {
        return replace_suffix(argv[offset], ".uar");
    } else {
        return add_suffix(argv[offset], ".uar");
    }
}

/*
 * Takes arguments in argv[], based on which determines
 * the names of input/output file and whether archiving or
 * unarchiving will happen
 */
void parse_user_input(int argc, char *argv[], Data* data) {
    argc--;

    /* Offset == 1 if options specified */
    int offset = 0;
    if (argc > 0 && (strequal(argv[1], ARCHIVE) || strequal(argv[1], UNARCHIVE))) {
        offset = 1;
    }

    /* No in/out filenames specified */
    if (argc - offset == 0) {
        data->fileIn = DEFAULT_FILEIN;
        char* newArgs[argc + 1]; /* Adding fileIn to the end of args[] */
        char_array_copy(newArgs, argv + 1, argc);
        newArgs[argc] = data->fileIn;
        data->fileOut = determine_out_file(argc + 1, newArgs, offset, &data->isArchiving);
    }

    /* Only input filename specified */
    else if (argc - offset == 1) {
        data->fileIn = argv[offset + 1];
        data->fileOut = determine_out_file(argc, argv + 1, offset, &data->isArchiving);
    }

    /* Both filenames specified */
    else {
        data->fileIn = argv[offset + 1];
        data->fileOut = argv[offset + 2];
        if (strequal(data->fileIn, data->fileOut)) {
            error("Error: name can't be the same for input and output files\n");
        }
        data->isArchiving = strequal(argv[1], ARCHIVE) || (!ends_with(argv[offset + 1], ".par") && offset == 0);
    }
}