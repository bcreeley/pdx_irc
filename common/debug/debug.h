#ifndef _DEBUG_H
#define _DEBUG_H

#include <sys/types.h>
#include <stdint.h>

const char *resp_type_to_str(uint32_t resp);
const char *msg_type_to_str(uint8_t type);

#endif /* _DEBUG_H */
