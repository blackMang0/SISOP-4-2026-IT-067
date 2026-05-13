#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

static const char *source_dir = "/home/blackmang0/pratikumSisop/SISOP-4-2026-IT-067/soal_1/amba_files";


// ========================================
// Mengubah path virtual menjadi path asli
// ========================================
static void build_path(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, source_dir);

    if (strcmp(path, "/") != 0)
    {
        strcat(fpath, path);
    }
}


// ========================================
// GET ATTRIBUTE
// ========================================
static int kenz_getattr(const char *path,
                        struct stat *stbuf,
                        struct fuse_file_info *fi)
{
    (void) fi;

    int res;
    char fpath[PATH_MAX];

    memset(stbuf, 0, sizeof(struct stat));

    // File virtual tujuan.txt
    if (strcmp(path, "/tujuan.txt") == 0)
    {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;

        char combined[1024];

strcpy(combined, "Tujuan Mas Amba: ");

for (int i = 1; i <= 7; i++)
{
    char temp_path[PATH_MAX];

    snprintf(temp_path,
             sizeof(temp_path),
             "%s/%d.txt",
             source_dir,
             i);

    FILE *fp = fopen(temp_path, "r");

    if (fp != NULL)
    {
        char temp[1024];

        while (fgets(temp, sizeof(temp), fp) != NULL)
        {
            char *ptr = strstr(temp, "KOORD:");

            if (ptr != NULL)
            {
                ptr += strlen("KOORD:");

                ptr[strcspn(ptr, "\n")] = '\0';

                while (*ptr == ' ')
                {
                    ptr++;
                }

                strcat(combined, ptr);
            }
        }

        fclose(fp);
    }
}

strcat(combined, "\n");

stbuf->st_size = strlen(combined);

        return 0;
    }

    build_path(fpath, path);

    res = lstat(fpath, stbuf);

    if (res == -1)
    {
        return -errno;
    }

    return 0;
}


// ========================================
// READ DIRECTORY
// ========================================
static int kenz_readdir(const char *path,
                        void *buf,
                        fuse_fill_dir_t filler,
                        off_t offset,
                        struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags)
{
    (void) offset;
    (void) fi;
    (void) flags;

    DIR *dp;
    struct dirent *de;

    char fpath[PATH_MAX];

    if (strcmp(path, "/") != 0)
    {
        return -ENOENT;
    }

    build_path(fpath, path);

    dp = opendir(fpath);

    if (dp == NULL)
    {
        return -errno;
    }

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    while ((de = readdir(dp)) != NULL)
    {
        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        filler(buf, de->d_name, &st, 0, 0);
    }

    closedir(dp);

    // Tambahkan file virtual
    filler(buf, "tujuan.txt", NULL, 0, 0);

    return 0;
}


// ========================================
// OPEN FILE
// ========================================
static int kenz_open(const char *path,
                     struct fuse_file_info *fi)
{
    // tujuan.txt virtual
    if (strcmp(path, "/tujuan.txt") == 0)
    {
        return 0;
    }

    int res;

    char fpath[PATH_MAX];

    build_path(fpath, path);

    res = open(fpath, fi->flags);

    if (res == -1)
    {
        return -errno;
    }

    close(res);

    return 0;
}


// ========================================
// READ FILE
// ========================================
static int kenz_read(const char *path,
                     char *buf,
                     size_t size,
                     off_t offset,
                     struct fuse_file_info *fi)
{
    (void) fi;

    // ====================================
    // FILE VIRTUAL tujuan.txt
    // ====================================
    if (strcmp(path, "/tujuan.txt") == 0)
{
    char *combined = malloc(20000);

    if (combined == NULL)
    {
        return -ENOMEM;
    }

    // Prefix sesuai soal
    strcpy(combined, "Tujuan Mas Amba: ");

    char temp[1024];

    // Gabungkan 1.txt sampai 7.txt
    for (int i = 1; i <= 7; i++)
    {
        char temp_path[PATH_MAX];

        snprintf(temp_path,
                 sizeof(temp_path),
                 "%s/%d.txt",
                 source_dir,
                 i);

        FILE *fp = fopen(temp_path, "r");

        if (fp != NULL)
        {
            while (fgets(temp, sizeof(temp), fp) != NULL)
{
    char *ptr = strstr(temp, "KOORD:");

    if (ptr != NULL)
    {
        // Geser setelah "KOORD:"
        ptr += strlen("KOORD:");

        // Hapus newline
        ptr[strcspn(ptr, "\n")] = '\0';

        // Hapus spasi di depan
        while (*ptr == ' ')
        {
            ptr++;
        }

        // Gabungkan langsung TANPA spasi tambahan
        strcat(combined, ptr);
    }
}

            fclose(fp);
        }
    }

    // Tambahkan tepat satu newline
    strcat(combined, "\n");

    size_t len = strlen(combined);

    if (offset < len)
    {
        if (offset + size > len)
        {
            size = len - offset;
        }

        memcpy(buf, combined + offset, size);
    }
    else
    {
        size = 0;
    }

    free(combined);

    return size;
}
    // ====================================
    // FILE ASLI PASSTHROUGH
    // ====================================
    int fd;
    int res;

    char fpath[PATH_MAX];

    build_path(fpath, path);

    fd = open(fpath, O_RDONLY);

    if (fd == -1)
    {
        return -errno;
    }

    res = pread(fd, buf, size, offset);

    if (res == -1)
    {
        res = -errno;
    }

    close(fd);

    return res;
}


// ========================================
// OPERATIONS
// ========================================
static const struct fuse_operations kenz_oper =
{
    .getattr = kenz_getattr,
    .readdir = kenz_readdir,
    .open = kenz_open,
    .read = kenz_read,
};


// ========================================
// MAIN
// ========================================
int main(int argc, char *argv[])
{
    umask(0);

    return fuse_main(argc, argv, &kenz_oper, NULL);
}
