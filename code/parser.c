#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "utils/argparse.h"

#define DEFAULT_FILEIN "test.txt"

static const char *const usages[] = {
    "./archive [options] [[--] args]",
    "./archive [options]",
    NULL,
};

static void error(const char* message) {
    printf("Error: %s.\n", message);
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

/*
 * Determines output filename and returns it,
 * based on the input file name and whether it's being archived or unarchived
 */
static char* determine_out_file(Data* data) {
    if (data->isArchiving == true) {
        return add_suffix(data->fileIn, ".par");
    }
    if (ends_with(data->fileIn, ".par")) {
        return replace_suffix(data->fileIn, ".uar");
    }
    return add_suffix(data->fileIn, ".uar");
}
/*
 * Determines whether the input file is to be archived or unarchived,
 * based on the input file name,
 * when neither "-u" nor "-a" were specified
 */
static bool determine_is_archiving(int argc, char *argv[]) {
    if (argc == 0) {
        return true;
    } else if (argc == 1) {
        if (ends_with(argv[0], ".par")) {
            return false; /* Unarchiving */
        }
        return true;
    }
    if (ends_with(argv[0], ".par") && ends_with(argv[1], ".par")) {
        return true;
    } else if (ends_with(argv[0], ".par")) {
        return false;
    }
    return true;
}

void parse_user_input(int argc, char *argv[], Data* data) {
    bool isArchiving = 0;
    bool isUnarchiving = 0;
    char* algorithm = NULL;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_BOOLEAN('a', NULL, &isArchiving, "archive", NULL, 0, 0),
        OPT_BOOLEAN('u', NULL, &isUnarchiving, "unarchive", NULL, 0, 0),
        OPT_STRING(0, "algorithm", &algorithm, "algorithm type", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\npar - pocket archiver. A simple data compression cli program, which supports a set of compression algorithms.", 
                                 "\nAlgorithm types\n    huffman (*)\n    adaptive-huffman\n\nArgs: [[--] [input file] [output file]]\n  or: [[--] [input file]]\nEmpty args sets input file name to default.");
    argc = argparse_parse(&argparse, argc, (const char**) argv);

    /* Both -u and -a are specified */
    if (isArchiving != 0 && isUnarchiving != 0) {
        argparse_help_cb(&argparse, options);
    } else if (isArchiving == 0 && isUnarchiving == 0) {
        data->isArchiving = determine_is_archiving(argc, argv);
    } else if (isArchiving != 0) {
        data->isArchiving = true;
    } else if (isUnarchiving != 0) {
        data->isArchiving = false;
    }

    if (algorithm == NULL) {
        data->algorithmType = ALG_HUFFMAN;
    } else {
        data->algorithmType = str_to_algorithm_type(algorithm);
    }

    if (argc == 0) {
        data->fileIn = DEFAULT_FILEIN;
        data->fileOut = determine_out_file(data);
    } else if (argc == 1) {
        data->fileIn = argv[0];
        data->fileOut = determine_out_file(data);
    } else {
        data->fileIn = argv[0];
        data->fileOut = argv[1];
    }
}