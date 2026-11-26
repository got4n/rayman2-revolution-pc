#ifndef SNDCMD_INV_H
#define SNDCMD_INV_H

typedef int (*CmdHandler_inv)(int *out, int outsz, void *in, int insz);

typedef struct {
    int tag;
    int x4;
    int count;
    CmdHandler_inv *cmds;
} tdstCmdModule_inv;

#endif
