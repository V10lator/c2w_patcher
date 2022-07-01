#include <stdbool.h>

#include <jailbreak.h>

#include <coreinit/cache.h>
#include <coreinit/ios.h>
#include <coreinit/mcp.h>

#include <wut_structsize.h>

// According to https://stackoverflow.com/questions/11130109/c-struct-size-alignment the aligned attribute alignes both, start addy and size
typedef struct WUT_PACKED __attribute__ ((aligned(0x40)))
{
	uint32_t cmd;
	uint32_t tgt;
	uint32_t fs;
	uint32_t fo;
	char path[0x100];
} LOAD_REQUEST;
WUT_CHECK_OFFSET(LOAD_REQUEST, 0x00, cmd);
WUT_CHECK_OFFSET(LOAD_REQUEST, 0x04, tgt);
WUT_CHECK_OFFSET(LOAD_REQUEST, 0x08, fs);
WUT_CHECK_OFFSET(LOAD_REQUEST, 0x0C, fo);
WUT_CHECK_OFFSET(LOAD_REQUEST, 0x10, path);
WUT_CHECK_SIZE(LOAD_REQUEST, 0x140); // Would be 0x110 without the alignment

void jailbreak()
{
	int mcpHandle = MCP_Open();
	if(mcpHandle == 0)
		return;

	LOAD_REQUEST request =
	{
		.cmd = 0xFC,
		.tgt = 0,
		.fs = 0,
		.fo = 0,
		.path = "wiiu/apps/sign_c2w_patcher.rpx",
	};

	DCFlushRange(&request, sizeof(LOAD_REQUEST));
	IOS_Ioctl(mcpHandle, 100, &request, sizeof(LOAD_REQUEST), NULL, 0);
	MCP_Close(mcpHandle);
}
