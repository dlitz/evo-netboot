#ifndef SERIAL_H
#define SERIAL_H

extern void serial_putc(unsigned char c);
extern void serial_outstr(const char *s);
extern void serial_init(void);

#endif /* SERIAL_H */
