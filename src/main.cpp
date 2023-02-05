#include <coreinit/filesystem_fsa.h>
#include <coreinit/memory.h>
#include <mocha/mocha.h>

#include <wups.h>

#include <string.h>

/**
    Mandatory plugin information.
    If not set correctly, the loader will refuse to use the plugin.
**/
WUPS_PLUGIN_NAME("c2w patcher");
WUPS_PLUGIN_DESCRIPTION("This redirects cafe2wii to be loaded from any Wii VC title you boot up");
WUPS_PLUGIN_VERSION("v1.3");
WUPS_PLUGIN_AUTHOR("V10lator");
WUPS_PLUGIN_LICENSE("GPL3");

#define STR_R6_SP_0 0x95029600
#define C2W_PATH    "/code/c2w.img"
#define MLC_PATH    "/vol/storage_mlc01/"

static void patch()
{
    // move filename argument to new position in full path
    Mocha_IOSUKernelWrite32(0x050089B4, STR_R6_SP_0);

    // change dynamically generated title path to wii vc title path
    Mocha_IOSUKernelWrite32(0x05008A54, 0x05062038); // /%s/code/%s
    Mocha_IOSUKernelWrite32(0x05008A58, 0x05074A18); // wii vc path
}

DECL_FUNCTION(int32_t, _SYSLaunchTitleByPathFromLauncher, const char *p, int unk)
{
    size_t s = strlen(p);
    if(s < FS_MAX_PATH - strlen(C2W_PATH) && strncmp(MLC_PATH, p, strlen(MLC_PATH)) == 0)
    {
        if(Mocha_InitLibrary() == MOCHA_RESULT_SUCCESS)
        {
            if(FSAInit() == FS_ERROR_OK)
            {
                FSAClientHandle fsa = FSAAddClient(NULL);
                if(fsa != 0)
                {
                    if(Mocha_UnlockFSClientEx(fsa) == MOCHA_RESULT_SUCCESS)
                    {
                        char path[s + (strlen(C2W_PATH) + 1)] __attribute__((__aligned__(0x40)));
                        OSBlockMove(path, p, s, false);
                        OSBlockMove(path + s, C2W_PATH, strlen(C2W_PATH) + 1, false);

                        FSAStat stat;
                        if(FSAGetStat(fsa, path, &stat) == FS_ERROR_OK)
                            patch();
                    }

                    FSADelClient(fsa);
                }

                FSAShutdown();
            }

            Mocha_DeInitLibrary();
        }
    }

    return real__SYSLaunchTitleByPathFromLauncher(p, unk);
}
WUPS_MUST_REPLACE(_SYSLaunchTitleByPathFromLauncher, WUPS_LOADER_LIBRARY_SYSAPP, _SYSLaunchTitleByPathFromLauncher);
