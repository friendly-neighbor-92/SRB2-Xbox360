#include "../../srb2/src/dummy/i_net.c"

/* NONET target: do not attempt the platform TCP backend. */
boolean I_InitTcpNetwork(void)
{
    return false;
}
