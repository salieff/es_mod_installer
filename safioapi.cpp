#include "safadapter.h"

#include <QFileInfo>
#include <QDir>


static voidpf minizip_saf_open(voidpf, const char *filename, int mode)
{
    const char* mode_fopen = NULL;
    int fd = -1;

    QString fullDir = QFileInfo(filename).dir().path();
    QString fullFname = QFileInfo(filename).fileName();

    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ)
    {
        mode_fopen = "rb";
        fd = SafAdapter::OpenFile(fullDir, fullFname, "r");
    }
    else if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        mode_fopen = "r+b";
        fd = SafAdapter::OpenFile(fullDir, fullFname, "rw");
    }
    else if (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        mode_fopen = "wb";
        fd = SafAdapter::CreateFile(fullDir, fullFname, "wt");
    }

    if ((fd >= 0) && (filename!=NULL) && (mode_fopen != NULL))
        return fdopen(fd, mode_fopen);

    return nullptr;
}

static uLong minizip_saf_read(voidpf, voidpf stream, void* buf, uLong size)
{
    return fread(buf, 1, size, (FILE *)stream);
}

static uLong minizip_saf_write(voidpf, voidpf stream, const void* buf, uLong size)
{
    return fwrite(buf, 1, size, (FILE *)stream);
}

static int minizip_saf_close(voidpf, voidpf stream)
{
    return fclose((FILE *)stream);
}

static int minizip_saf_testerror(voidpf, voidpf stream)
{
    return ferror((FILE *)stream);
}

static long minizip_saf_tell(voidpf, voidpf stream)
{
    return ftell((FILE *)stream);
}

static long minizip_saf_seek(voidpf, voidpf stream, uLong offset, int origin)
{
    int fseek_origin=0;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        break;

    case ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        break;

    case ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        break;

    default:
        return -1;
    }

    if (fseek((FILE *)stream, offset, fseek_origin) != 0)
        return -1;

    return 0;
}


zlib_filefunc_def SafAdapter::MiniZipFileAPI {
    .zopen_file = minizip_saf_open,
    .zread_file = minizip_saf_read,
    .zwrite_file = minizip_saf_write,
    .ztell_file = minizip_saf_tell,
    .zseek_file = minizip_saf_seek,
    .zclose_file = minizip_saf_close,
    .zerror_file = minizip_saf_testerror,
    .opaque = nullptr
};
