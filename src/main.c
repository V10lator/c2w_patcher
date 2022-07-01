#include <stdbool.h>
#include <stdint.h>

#include <coreinit/mcp.h>
#include <coreinit/title.h>
#include <iosuhax.h>
#include <nn/acp/title.h>
#include <sysapp/launch.h>
#include <sysapp/title.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

#include <padscore/kpad.h>
#include <coreinit/memdefaultheap.h>

#define MOV_R0_0            0xE3A00000
#define BX_LR               0xE12FFF1E
#define MOV_R0_0_BX_LR      0x20004770
#define MOV_R0_0_MOV_R0_0   0x20002000
#define STR_R6_SP_0         0x95029600

#define IO_ALIGN(x)  ((x + 0x3F) & ~(0x3F))

int main()
{
    WHBProcInit();
    WHBLogUdpInit();
    if(WHBProcIsRunning())
    {
        if(IOSUHAX_Open(NULL) >= 0)
        {
            // patch OS launch sig check
            WHBLogPrint("Patch 1...");
            IOSUHAX_kern_write32(0x0500A818, MOV_R0_0_MOV_R0_0);

            // move filename argument to new position in full path
            WHBLogPrint("Patch 2...");
            IOSUHAX_kern_write32(0x050089B4, STR_R6_SP_0);

            // change dynamically generated title path to wii vc title path
            WHBLogPrint("Patch 3...");
            IOSUHAX_kern_write32(0x05008A54, 0x05062038); // /%s/code/%s
            IOSUHAX_kern_write32(0x05008A58, 0x05074A18); // wii vc path

            // nop out memcmp hash checks
            WHBLogPrint("Patch 4...");
            IOSUHAX_kern_write32(0x040017E0, MOV_R0_0);
            IOSUHAX_kern_write32(0x040019C4, MOV_R0_0);
            IOSUHAX_kern_write32(0x04001BB0, MOV_R0_0);
            IOSUHAX_kern_write32(0x04001D40, MOV_R0_0);

            WHBLogPrint("Patch 5...");
            IOSUHAX_kern_write32(0x01555500, 0);

            WHBLogPrint("Closing iosuhax...");
            IOSUHAX_Close();
        }
        else
            WHBLogPrint("Error opening iosuhax!");

        SYSLaunchMenu();
        while(WHBProcIsRunning())
            WHBLogPrint("Proc loop...");
    }
    else
        WHBLogPrint("Proc error!");

    WHBLogPrint("Returning...");
    WHBLogUdpDeinit();
//    WHBProcShutdown(); // TODO: Re-enable after HBL
    return 0;
}
