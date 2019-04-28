//
// Created by maxim on 3/9/19.
//

#ifndef OIL_OILERROR_H
#define OIL_OILERROR_H

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define MAX_ERRORF_LEN 1000

void oilPrintError(void);
char* oilGetError(void);
void oilClearError(void);
void oilPushError(char* error);
void oilPushErrorf(const char* fmt, ...);

#endif //OIL_OILERROR_H
