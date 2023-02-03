#include <coreinit/filesystem_fsa.h>
#include <coreinit/mcp.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/memory.h>
#include <mocha/mocha.h>

#include <wups.h>

#include <mbedtls/md5.h>

#include <string.h>

/**
    Mandatory plugin information.
    If not set correctly, the loader will refuse to use the plugin.
**/
WUPS_PLUGIN_NAME("c2w patcher");
WUPS_PLUGIN_DESCRIPTION("Unlocks the full potential of vWii");
WUPS_PLUGIN_VERSION("v1.1");
WUPS_PLUGIN_AUTHOR("V10lator");
WUPS_PLUGIN_LICENSE("GPL3");

#define STR_R6_SP_0 0x95029600
#define FS_ALIGN(x) ((x + 0x3F) & ~(0x3F))
#define C2W_PATH    "/code/c2w.img"

static const uint32_t emd5sum[4] = { 0x3894e852, 0xa279827e, 0xbe31930f, 0x3855773f };

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
    if(Mocha_InitLibrary() == MOCHA_RESULT_SUCCESS)
    {
        if(FSAInit() == FS_ERROR_OK)
        {
            FSAClientHandle fsa = FSAAddClient(NULL);
            if(fsa != 0)
            {
                if(Mocha_UnlockFSClientEx(fsa) == MOCHA_RESULT_SUCCESS)
                {
                    char path[FS_MAX_PATH] __attribute__((__aligned__(0x40)));
                    size_t s = strlen(p);
                    OSBlockMove(path, p, s, false);
                    OSBlockMove(path + s, C2W_PATH, strlen(C2W_PATH) + 1, false);

                    FSAFileHandle file;
                    if(FSAOpenFileEx(fsa, path, "r", (FSMode)0, FS_OPEN_FLAG_NONE, 0, &file) == FS_ERROR_OK)
                    {
                        FSACloseFile(fsa, file);
                        patch();
                    }
                }

                FSADelClient(fsa);
            }

            FSAShutdown();
        }

        Mocha_DeInitLibrary();
    }

    return real__SYSLaunchTitleByPathFromLauncher(p, unk);
}
WUPS_MUST_REPLACE(_SYSLaunchTitleByPathFromLauncher, WUPS_LOADER_LIBRARY_SYSAPP, _SYSLaunchTitleByPathFromLauncher);
