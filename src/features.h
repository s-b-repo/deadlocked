#ifndef FEATURES_H
#define FEATURES_H

#include "memory.h"
#include "offsets.h"

bool find_offsets(ProcessHandle *process, Offsets *offsets);
void run(ProcessHandle *process);

#endif
