#ifndef __INFRA_UTILS_COPY_H__
#define __INFRA_UTILS_COPY_H__

#include "lua_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Copy value from \p src at \p idx and push to \p dst.
 * @param dst   Destination stack.
 * @param src   Source stack.
 * @param idx   The value need to copy at index.
 * @return      0 if success, otherwise error code.
 */
API_LOCAL int infra_copy(lua_State* dst, lua_State* src, int idx);

#ifdef __cplusplus
}
#endif
#endif
