#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "sdcard/sdcard.h"

DSTATUS disk_status(BYTE pdrv)
{
  return 0;
}

DSTATUS disk_initialize(BYTE pdrv)
{
  // Initialization could also be done here
  // instead of in kernel_main()
  return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
  int32_t ret = sdTransferBlocks((uint64_t)sector * FF_MIN_SS, count, buff, 0);
  return (ret == 0 ? RES_OK : RES_ERROR);
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
  int32_t ret = sdTransferBlocks((uint64_t)sector * FF_MIN_SS, count, (BYTE *)buff, 1);
  return (ret == 0 ? RES_OK : RES_ERROR);
}

DWORD get_fattime()
{
  return (40 << 25) | (1 << 21) | (1 << 16);
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  if (cmd == CTRL_SYNC) return RES_OK;
  return RES_OK;
}
