/* Header file by Dwayne Litzenberger
 * for gprintf.c */
#ifndef GPRINTF_H
#define GPRINTF_H
extern int general_printf(int (*output_function)(void *, int),
                          void *output_pointer, const char *control_string,
                          const int *argument_pointer);
#endif
