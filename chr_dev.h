//
// Created by vladi on 23 Jun 2023.
//

#ifndef CHR_DEV_CHR_DEV_H
#define CHR_DEV_CHR_DEV_H

#include <linux/compiler.h>

__must_check int register_device(void);
void unregister_device(void);

#endif //CHR_DEV_CHR_DEV_H
