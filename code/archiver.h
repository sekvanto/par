#ifndef ARCHIVER_H
#define ARCHIVER_H

#include "common.h"
#include "data.h"

int archive(Data* data);
int unarchive(Data* data);
int archiveError(const char* message);

#endif