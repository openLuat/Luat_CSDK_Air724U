
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"

#include "auxmods.h"
#include "lrotable.h"
 
#include "platform_factory.h"

 int l_factory_check_calib(lua_State *L) 
{

    boolean ret = FALSE;
    
    ret = platform_factory_chkcalib();

    lua_pushboolean(L, ret);

    return 1;
}


#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

const LUA_REG_TYPE factorycore_map[] =
{ 
  { LSTRKEY( "chkcalib" ),  LFUNCVAL( l_factory_check_calib ) },

  { LNILKEY, LNILVAL }
};



LUALIB_API int luaopen_factorycore( lua_State *L )
{
    luaL_register( L, AUXLIB_FACTORY, factorycore_map );

    return 1;
}  

