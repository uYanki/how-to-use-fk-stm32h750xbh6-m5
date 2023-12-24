#ifndef __SHELL_PORT_H__
#define __SHELL_PORT_H__

#include "./core.h"

void     shell_setup(void);
void     shell_task(void);
shell_t* shell_get(void);

#endif
