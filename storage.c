#include "specs.h"
#include "storage.h"
#include "blocks.h"
#include "inode.h"
#include "slist.h"
#include "bitmap.h"

int inode_for_path(const char *path)
{
    
}

void storage_init(const char *path)
{
    blocks_init(path);

    blocks_clear();

    int inum = alloc_inode();
    printf("INUM: %d\n", inum);
    inode_t *nodep = get_inode(inum);
    print_inode(nodep);

    for (int i = 0; i < BLOCK_SIZE * 3; i++)
    {
        grow_inode(nodep, 1);
    }

    print_inode(nodep);
    //print_inode(get_inode(1));
    //print_inode(get_inode(2));
}

void storage_deinit(void)
{
    blocks_free();
}

int storage_stat(const char *path, struct stat *st)
{

}

int storage_read(const char *path, char *buf, size_t size, off_t offset)
{

}

int storage_write(const char *path, const char *buf, size_t size, off_t offset)
{
    
}

int storage_truncate(const char *path, off_t size)
{
    
}

int storage_mknod(const char *path, int mode)
{
    
}

int storage_unlink(const char *path)
{
    
}

int storage_link(const char *from, const char *to)
{
    
}

int storage_rename(const char *from, const char *to)
{
    
}

int storage_set_time(const char *path, const struct timespec ts[2])
{
    
}

slist_t *storage_list(const char *path)
{
    
}
