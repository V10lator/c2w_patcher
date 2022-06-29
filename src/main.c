#define FW_TID 0x000500025484F87E

/*
 * ---------------------------------------------
 * Don't edit anything below this
 * ---------------------------------------------
 */

#include <iosuhax.h>
#include <sysapp/launch.h>
#include <whb/log.h>
#include <whb/log_udp.h>

#define MOV_R0_0            0xE3A00000
#define BX_LR               0xE12FFF1E
#define MOV_R0_0_BX_LR      0x20004770
#define MOV_R0_0_MOV_R0_0   0x20002000
#define STR_R6_SP_0         0x95029600

int main()
{
    WHBLogUdpInit();
    WHBLogPrint("Opening iosuhax...");
    if(IOSUHAX_Open(NULL) >= 0)
    {
        WHBLogPrint("Patch 1...");
        // move filename argument to new position in full path
        IOSUHAX_kern_write32(0x050089B4, STR_R6_SP_0);

        WHBLogPrint("Patch 2...");
        // change dynamically generated title path to wii vc title path
        IOSUHAX_kern_write32(0x05008A54, 0x05062038); // /%s/code/%s
        IOSUHAX_kern_write32(0x05008A58, 0x05074A18); // wii vc path

        WHBLogPrint("Closing iosuhax...");
        IOSUHAX_Close();
//        _SYSLaunchTitleWithStdArgsInNoSplash(FW_TID, NULL);
        SYSLaunchMenu();
    }
    else
        SYSLaunchMenu();

    WHBLogPrint("Returning...");
    WHBLogUdpDeinit();
    return 0;
}
