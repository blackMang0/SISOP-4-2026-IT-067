#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

static const char *real_root = "/home/blackmang0/pratikumSisop/SISOP-4-2026-IT-067/soal_2/encrypted_storage";
static const unsigned char XOR_KEY = 0x76;

void build_path(char fpath[PATH_MAX], const char *path)
{
    snprintf(fpath, PATH_MAX, "%s%s", real_root, path);
}

void xor_buffer(char *buf, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        buf[i] ^= XOR_KEY;
    }
}

static int xmp_getattr(const char *path,
                       struct stat *stbuf,
                       struct fuse_file_info *fi)
{
    (void) fi;

    char fpath[PATH_MAX];

    build_path(fpath, path);

    int res = lstat(fpath, stbuf);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_access(const char *path, int mask)
{
    char fpath[PATH_MAX];

    build_path(fpath, path);

    int res = access(fpath, mask);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_readdir(const char *path,
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

    build_path(fpath, path);

    dp = opendir(fpath);

    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {

        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if (filler(buf, de->d_name, &st, 0, 0))
            break;
    }

    closedir(dp);

    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];

    build_path(fpath, path);

    int res = mkdir(fpath, mode);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_mknod(const char *path,
                     mode_t mode,
                     dev_t rdev)
{
    char fpath[PATH_MAX];

    build_path(fpath, path);

    int res;

    if (S_ISREG(mode)) {

        res = open(fpath,
                   O_CREAT | O_EXCL | O_WRONLY,
                   mode);

        if (res >= 0)
            res = close(res);

    } else if (S_ISFIFO(mode)) {

        res = mkfifo(fpath, mode);

    } else {

        res = mknod(fpath, mode, rdev);
    }

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_create(const char *path,
                      mode_t mode,
                      struct fuse_file_info *fi)
{
    char fpath[PATH_MAX];

    build_path(fpath, path);

    int fd = open(fpath, fi->flags, mode);

    if (fd == -1)
        return -errno;

    fi->fh = fd;

    return 0;
}

static int xmp_open(const char *path,
                    struct fuse_file_info *fi)
{
    char fpath[PATH_MAX];

    build_path(fpath, path);

    int fd = open(fpath, fi->flags);

    if (fd == -1)
        return -errno;

    fi->fh = fd;

    return 0;
}

static int xmp_read(const char *path,
                    char *buf,
                    size_t size,
                    off_t offset,
                    struct fuse_file_info *fi)
{
    (void) path;

    int fd = fi->fh;

    int res = pread(fd, buf, size, offset);

    if (res == -1)
        return -errno;

    xor_buffer(buf, res);

    return res;
}

static int xmp_write(const char *path,
                     const char *buf,
                     size_t size,
                     off_t offset,
                     struct fuse_file_info *fi)
{
    (void) path;

    int fd = fi->fh;

    char *enc = malloc(size);

    memcpy(enc, buf, size);

    xor_buffer(enc, size);

    int res = pwrite(fd, enc, size, offset);

    free(enc);

    if (res == -1)
        return -errno;

    return res;
}

static int xmp_truncate(const char *path,
                        off_t size,
                        struct fuse_file_info *fi)
{
    char fpath[PATH_MAX];

    build_path(fpath, path);

    int res;

    if (fi != NULL)
        res = ftruncate(fi->fh, size);
    else
        res = truncate(fpath, size);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_unlink(const char *path)
{
    char fpath[PATH_MAX];

    build_path(fpath, path);

    int res = unlink(fpath);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_utimens(const char *path,
                       const struct timespec tv[2],
                       struct fuse_file_info *fi)
{
    (void) fi;

    char fpath[PATH_MAX];

    build_path(fpath, path);

    int res = utimensat(0,
                        fpath,
                        tv,
                        AT_SYMLINK_NOFOLLOW);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_release(const char *path,
                       struct fuse_file_info *fi)
{
    (void) path;

    close(fi->fh);

    return 0;
}

static int xmp_rmdir(const char *path)
{
    char fpath[PATH_MAX];

    build_path(fpath, path);

    int res = rmdir(fpath);

    if (res == -1)
        return -errno;

    return 0;
}


static const struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .access = xmp_access,
    .readdir = xmp_readdir,
    .mkdir = xmp_mkdir,
    .rmdir = xmp_rmdir,
    .mknod = xmp_mknod,
    .create = xmp_create,
    .open = xmp_open,
    .read = xmp_read,
    .write = xmp_write,
    .truncate = xmp_truncate,
    .unlink = xmp_unlink,
    .utimens = xmp_utimens,
    .release = xmp_release,
};

int main(int argc, char *argv[])
{
    umask(0);

    return fuse_main(argc,
                     argv,
                     &xmp_oper,
                     NULL);
}
