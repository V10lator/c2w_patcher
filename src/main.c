#define FW_TID 0x00050000103562FF

/*
 * ---------------------------------------------
 * Don't edit anything below this
 * ---------------------------------------------
 */

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

#include "jailbreak.h"

#define MOV_R0_0            0xE3A00000
#define BX_LR               0xE12FFF1E
#define MOV_R0_0_BX_LR      0x20004770
#define MOV_R0_0_MOV_R0_0   0x20002000
#define STR_R6_SP_0         0x95029600

#define IO_ALIGN(x)  ((x + 0x3F) & ~(0x3F))

// REed function declarations
extern int32_t CMPTAcctClearInternalState();
extern int CMPTAcctSetDrcCtrlEnabled(bool param_1);
extern int CMPTAcctSetEula(bool param_1);
extern int32_t CMPTAcctSetPcConf(int32_t *param_1);
extern int32_t CMPTAcctSetProfile(int param_1);
extern int CMPTExLaunch(void);
extern int32_t CMPTExPrepareLaunch(uint32_t param_1, int32_t *param_2, uint32_t param_3);
extern int32_t CMPTExSetWorkBuffer(uint32_t param_1, uint32_t param_2);
extern void CMPTInitSystem(int32_t param_1, int32_t param_2);
extern int32_t CMPTIsClean(uint32_t *param_1);
extern int CMPTSetDirty();

// Fixed CMTP functions
typedef enum
{
    CMPT_SCREEN_TYPE_TV = 1,
    CMPT_SCREEN_TYPE_DRC,
    CMPT_SCREEN_TYPE_BOTH,
} CmptScreenType;
extern int32_t CMPTLaunchTitle(void* dataBuffer, uint32_t bufferSize, uint64_t titleId);

// Functions copied from nn/cmpt/cmpt.h
extern int32_t CMPTGetDataSize(uint32_t* outSize);
extern int32_t CMPTAcctSetScreenType(CmptScreenType type);
extern int32_t CMPTCheckScreenState();

int main()
{
    uint64_t tid = OSGetTitleID();
    if((tid & 0xFFFFFFFFFFFFF0FF) == 0x000500101004A000)
    {
        WHBProcInit();
        if(WHBProcIsRunning())
        {
            jailbreak();
            tid &= 0xFFFFFFFFFFFF0FFF;
            tid |= 0x000000000000E000;
            _SYSLaunchTitleWithStdArgsInNoSplash(tid, NULL);
            while(WHBProcIsRunning())
                ;
        }
        return 0;
    }

//    WHBProcInit();
    WHBLogUdpInit();
//    if(WHBProcIsRunning())
//    {
        if(IOSUHAX_Open(NULL) >= 0)
        {
/*            // patch OS launch sig check
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
*/
            WHBLogPrint("Closing iosuhax...");
            IOSUHAX_Close();

            KPADInit();
//            CMPTAcctClearInternalState();

            // Try to find a screen type that works
            CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_BOTH);
            if(CMPTCheckScreenState() < 0)
            {
                CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_DRC);
                if(CMPTCheckScreenState() < 0)
                    CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_TV);
            }

            CMPTAcctSetDrcCtrlEnabled(true);
//            CMPTAcctSetEula(false);

            uint32_t dataSize;
            CMPTGetDataSize(&dataSize);
            void *dataBuffer = MEMAllocFromDefaultHeapEx(IO_ALIGN(dataSize), 0x40);
            if(dataBuffer)
            {
//                WHBProcStopRunning();
                CMPTLaunchTitle(dataBuffer, dataSize, FW_TID);
//                _SYSLaunchTitleWithStdArgsInNoSplash(FW_TID, NULL);
//                _SYSLaunchTitleFromLauncher(FW_TID);
                MEMFreeToDefaultHeap(dataBuffer);
            }
            else
            {
                WHBLogPrint("Error allocating buffer!");
//                SYSLaunchMenu();
            }
        }
        else
        {
            WHBLogPrint("Error opening iosuhax!");
//            SYSLaunchMenu();
        }

//        while(WHBProcIsRunning())
//            WHBLogPrint("Proc loop...");
//    }
//    else
//        WHBLogPrint("Proc error!");

    WHBLogPrint("Returning...");
    WHBLogUdpDeinit();
//    WHBProcShutdown(); // TODO: Re-enable after HBL
    return 0;
}
