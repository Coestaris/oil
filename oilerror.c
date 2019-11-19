//
// Created by maxim on 3/9/19.
//

#include "oilerror.h"

char* error;

void oilPrintError()
{
   char* __err = oilGetError();
   if (__err == NULL) puts("no errors");
   else puts(__err);
   oilClearError();
}

void oilClearError(void)
{
   free(error);
   error = NULL;
}

char* oilGetError()
{
   return error;
}

void oilPushError(char* new_error)
{
   if (error == NULL)
   {
      error = strdup(new_error);

   }
   else
   {

      error = realloc(error, strlen(error) + strlen(new_error) + 1);
      error = strcat(error, new_error);

   }
}

void oilPushErrorf(const char* fmt, ...)
{
   va_list list;
   va_start(list, fmt);

   char newError[MAX_ERRORF_LEN];
   vsnprintf(newError, MAX_ERRORF_LEN, fmt, list);

   oilPushError(newError);
}