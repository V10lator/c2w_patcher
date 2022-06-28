#define FW_TID 0x0000000000000000

/*
 * ---------------------------------------------
 * Don't edit anything below this
 * ---------------------------------------------
 */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <coreinit/title.h>
#include <coreinit/ios.h>
#include <coreinit/cache.h>
#include <coreinit/mcp.h>
#include <coreinit/thread.h>
#include <iosuhax.h>
#include <sysapp/launch.h>
#include <wut_structsize.h>

#define crypto_phys(addr)       (addr - 0x04000000 + 0x08280000)
#define mcp_phys(addr)          (addr - 0x05000000 + 0x081C0000)
#define mcp_rodata_phys(addr)   (addr - 0x05060000 + 0x08220000)
#define acp_phys(addr)          (addr - 0xE0000000 + 0x12900000)

#define MOV_R0_0            0xE3A00000
#define BX_LR               0xE12FFF1E
#define MOV_R0_0_BX_LR      0x20004770
#define MOV_R0_0_MOV_R0_0   0x20002000
#define STR_R6_SP_0         0x95029600

int main()
{
    if(IOSUHAX_Open(NULL) >= 0)
    {
        // fix 10 minute timeout that crashes MCP after 10 minutes of booting
        IOSUHAX_kern_write32(mcp_phys(0x05022474), 0xFFFFFFFF); // NEW_TIMEOUT

        // patch cached cert check
        IOSUHAX_kern_write32(mcp_phys(0x05054D6C), MOV_R0_0);
        IOSUHAX_kern_write32(mcp_phys(0x05054D70), BX_LR);

        // patch cert verification
        IOSUHAX_kern_write32(mcp_phys(0x05052A90), MOV_R0_0);
        IOSUHAX_kern_write32(mcp_phys(0x05052A94), BX_LR);

        // patch MCP authentication check
        IOSUHAX_kern_write32(mcp_phys(0x05014CAC), MOV_R0_0_BX_LR);

        // patch IOSC_VerifyPubkeySign to always succeed
        IOSUHAX_kern_write32(mcp_phys(0x05052C44), MOV_R0_0);
        IOSUHAX_kern_write32(mcp_phys(0x05052C48), BX_LR);

        // patch OS launch sig check
        IOSUHAX_kern_write32(mcp_phys(0x0500A818), MOV_R0_0_MOV_R0_0);

        // move filename argument to new position in full path
        IOSUHAX_kern_write32(mcp_phys(0x050089B4), STR_R6_SP_0);

        // change dynamically generated title path to wii vc title path
        IOSUHAX_kern_write32(mcp_phys(0x05008A54), 0x05062038); // /%s/code/%s
        IOSUHAX_kern_write32(mcp_phys(0x05008A58), 0x05074A18); // wii vc path

        // allow custom bootLogoTex and bootMovie.h264
        IOSUHAX_kern_write32(mcp_phys(0xE0030D68), MOV_R0_0);
        IOSUHAX_kern_write32(mcp_phys(0xE0030D34), MOV_R0_0);

        // allow any region title launch
        IOSUHAX_kern_write32(acp_phys(0xE0030498), MOV_R0_0);

        // nop out memcmp hash checks
        IOSUHAX_kern_write32(crypto_phys(0x040017E0), MOV_R0_0);
        IOSUHAX_kern_write32(crypto_phys(0x040019C4), MOV_R0_0);
        IOSUHAX_kern_write32(crypto_phys(0x04001BB0), MOV_R0_0);
        IOSUHAX_kern_write32(crypto_phys(0x04001D40), MOV_R0_0);

        IOSUHAX_kern_write32(0x1555500, 0);

        IOSUHAX_Close();
        _SYSLaunchTitleWithStdArgsInNoSplash(FW_TID, NULL);
    }
    else
        SYSLaunchMenu();

    return 0;
}
