#ifndef NEW_SECRET_H
#define NEW_SECRET_H
#include <dialogs.h>

/* Return codes */
#define ND_OK 1
#define ND_CANCELLED 2
#define ND_MEM 2
#define ND_ERROR 3

/* sets up the dialog handle */
int run_new_secret_dialog (HANDLE manifest_handle, int wide_format);
int run_open_secret_dialog (HANDLE manifest_handle, int wide_format);
#endif
