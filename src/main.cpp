#include <coreinit/cache.h>
#include <coreinit/filesystem_fsa.h>
#include <coreinit/mcp.h>
#include <coreinit/memdefaultheap.h>
#include <mocha/mocha.h>

#include <wups.h>

#include <mbedtls/md5.h>

#include <stdio.h>
#include <string.h>

/**
    Mandatory plugin information.
    If not set correctly, the loader will refuse to use the plugin.
**/
WUPS_PLUGIN_NAME("c2w patcher");
WUPS_PLUGIN_DESCRIPTION("Unlocks the full potential of vWii");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("V10lator");
WUPS_PLUGIN_LICENSE("GPL3");

#define STR_R6_SP_0 0x95029600
#define FS_ALIGN(x) ((x + 0x3F) & ~(0x3F))

static const uint8_t emd5sum[16] = { 0x38, 0x94, 0xe8, 0x52, 0xa2, 0x79, 0x82, 0x7e, 0xbe, 0x31, 0x93, 0x0f, 0x38, 0x55, 0x77, 0x3f };

static bool isPatched(FSAClientHandle handle, FSAFileHandle file)
{
    FSAStat stat;
    if(FSAGetStatFile(handle, file, &stat) != FS_ERROR_OK)
        return false;

    void *buf = MEMAllocFromDefaultHeapEx(FS_ALIGN(stat.size), 0x40);
    if(buf == NULL)
        return false;

    bool ret = false;
    int r = FSAReadFile(handle, buf, stat.size, 1, file, 0);
    if(r == 1)
    {
        uint8_t md5sum[16];
        mbedtls_md5((const unsigned char *)buf, stat.size, md5sum);
        ret = true;
        for(int i = 0; i < 16; ++i)
        {
            if(md5sum[i] != emd5sum[i])
            {
                ret = false;
                break;
            }
        }
    }

    MEMFreeToDefaultHeap(buf);
    return ret;
}

DECL_FUNCTION(int32_t, _SYSLaunchTitleByPathFromLauncher, const char *p, int unk) // Gets used!
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
                    strcpy(path, p);
                    strcat(path, "/code/c2w.img");

                    FSAFileHandle file;
                    FSError e = FSAOpenFileEx(fsa, path, "r", (FSMode)0, FS_OPEN_FLAG_NONE, 0, &file);
                    if(e == FS_ERROR_OK)
                    {
                        bool patched = isPatched(fsa, file);
                        FSACloseFile(fsa, file);
                        if(patched)
                        {
                            // move filename argument to new position in full path
                            Mocha_IOSUKernelWrite32(0x050089B4, STR_R6_SP_0);

                            // change dynamically generated title path to wii vc title path
                            Mocha_IOSUKernelWrite32(0x05008A54, 0x05062038); // /%s/code/%s
                            Mocha_IOSUKernelWrite32(0x05008A58, 0x05074A18); // wii vc path
                        }
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

