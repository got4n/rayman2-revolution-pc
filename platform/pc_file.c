#include "PC.h"
#include "pc_host.h"

int32_t pc_file_find(const char *path, PcFile *file)
{
    return revo_disc_find(path, &file->sector, &file->size);
}

int32_t pc_file_read(const PcFile *file, uint32_t offset, void *buffer, uint32_t size)
{
    return revo_disc_read_bytes(file->sector, offset, size, buffer);
}
