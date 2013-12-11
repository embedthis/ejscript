/*
    ejsZlib.c -- Zlib compression 

    Copyright (c) All Rights Reserved. See details at the end of the file.

 */
/********************************** Includes **********************************/

#include    "ejs.h"

#if BIT_EJS_ZLIB
#include    "zlib.h"
#include    "ejs.zlib.slots.h"

#define     ZBUFSIZE (16 * 1024)

/************************************ Code ************************************/
/*
    compress(src: Path, dest: Path = null)
 */
static EjsObj *zlib_compress(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprFile     *in;
    gzFile      out;
    cchar       *src, *dest;
    uchar       inbuf[BIT_MAX_BUFFER];
    ssize       nbytes;

    src = ((EjsPath*) argv[0])->value;
    dest = (argc >= 2) ? ejsToMulti(ejs, argv[1]) : 0;
    if (!dest) {
        dest = sjoin(src, ".gz", NULL);
    }
    if ((in = mprOpenFile(src, O_RDONLY | O_BINARY, 0)) == 0) {
        ejsThrowIOError(ejs, "Cannot open from %s", src);
        return 0;
    }
    if ((out = gzopen(dest, "wb")) == 0) {
        ejsThrowIOError(ejs, "Cannot open destination %s", dest);
        return 0;
    }
    while (1) {
        if ((nbytes = mprReadFile(in, inbuf, sizeof(inbuf))) < 0) {
            ejsThrowIOError(ejs, "Cannot read from %s", src);
            return 0;
        } else if (nbytes == 0) {
            break;
        }
        if (gzwrite(out, inbuf, (int) nbytes) != nbytes) {
            ejsThrowIOError(ejs, "Cannot write to %s", dest);
            return 0;
        }
    }
    mprCloseFile(in);
    gzclose(out);
    return 0;
}


/*
    uncompress(src: Path, dest: Path = null)
 */
static EjsObj *zlib_uncompress(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprFile     *out;
    gzFile      in;
    cchar       *src, *dest;
    uchar       inbuf[BIT_MAX_BUFFER];
    ssize       nbytes;

    src = ((EjsPath*) argv[0])->value;
    dest = (argc >= 2) ? ejsToMulti(ejs, argv[1]) : 0;
    if (!dest) {
        dest = strim(src, ".gz", MPR_TRIM_END);
    }
    if ((in = gzopen(src, "rb")) == 0) {
        ejsThrowIOError(ejs, "Cannot open from %s", src);
        return 0;
    }
    if ((out = mprOpenFile(dest, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644)) == 0) {
        ejsThrowIOError(ejs, "Cannot open destination %s", dest);
        return 0;
    }
    while (1) {
        if ((nbytes = gzread(in, inbuf, sizeof(inbuf))) < 0) {
            ejsThrowIOError(ejs, "Cannot read from %s", src);
            return 0;
        } else if (nbytes == 0) {
            break;
        }
        if (mprWriteFile(out, inbuf, (int) nbytes) != nbytes) {
            ejsThrowIOError(ejs, "Cannot write to %s", dest);
            return 0;
        }
    }
    mprCloseFile(out);
    gzclose(in);
    return 0;
}


/*
    compressBytes(data: ByteArray): ByteArray
 */
static EjsObj *zlib_compressBytes(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsByteArray    *in, *out;
    z_stream        zs;
    char            outbuf[ZBUFSIZE];
    ssize           nbytes;
    int             level, flush;

    in = (EjsByteArray*) argv[0];
    if ((out = ejsCreateByteArray(ejs, in->size)) == 0) {
        return 0;
    }
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    level = Z_DEFAULT_COMPRESSION;
    deflateInit(&zs, level);
    zs.avail_in = (int) ejsGetByteArrayAvailableData(in);
    zs.next_in = (uchar*) &in->value[in->readPosition];
    do {
        flush = (zs.avail_in == 0) ? Z_FINISH : Z_NO_FLUSH;
        do {
            zs.avail_out = ZBUFSIZE;
            zs.next_out = (uchar*) outbuf;
            deflate(&zs, flush);
            nbytes = ZBUFSIZE - zs.avail_out;
            if (ejsCopyToByteArray(ejs, out, -1, outbuf, nbytes) != nbytes) {
                ejsThrowIOError(ejs, "Cannot copy to byte array");
                deflateEnd(&zs);
                return 0;
            }
            out->writePosition += nbytes;
        } while (zs.avail_out == 0);
        assert(zs.avail_in == 0);
    } while (flush != Z_FINISH);
    deflateEnd(&zs);
    return (EjsObj*) out;
}


/*
    uncompressBytes(data: ByteArray): ByteArray
 */
static EjsObj *zlib_uncompressBytes(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsByteArray    *in, *out;
    z_stream        zs;
    uchar           outbuf[ZBUFSIZE];
    ssize           nbytes, size;
    int             rc;

    in = (EjsByteArray*) argv[0];
    if ((out = ejsCreateByteArray(ejs, in->size)) == 0) {
        return 0;
    }
    if ((size = (int) ejsGetByteArrayAvailableData(in)) == 0) {
        return (EjsObj*) out;
    }
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = 0;
    rc = inflateInit(&zs);
    zs.next_in = &in->value[in->readPosition];
    zs.avail_in = (int) size;

    do {
        if (zs.avail_in == 0) {
            break;
        }
        do {
            zs.avail_out = ZBUFSIZE;
            zs.next_out = (uchar*) outbuf;
            if ((rc = inflate(&zs, Z_NO_FLUSH)) == Z_NEED_DICT) {
                inflateEnd(&zs);
                return 0;
            } else if (rc == Z_DATA_ERROR || rc == Z_MEM_ERROR) {
                inflateEnd(&zs);
                return 0;
            } else {
                nbytes = ZBUFSIZE - zs.avail_out;
            }
            if (ejsCopyToByteArray(ejs, out, -1, (char*) outbuf, nbytes) != nbytes) {
                ejsThrowIOError(ejs, "Cannot copy to byte array");
                inflateEnd(&zs);
                return 0;
            }
            out->writePosition += nbytes;
        } while (zs.avail_out == 0);
        assert(zs.avail_in == 0);
    } while (rc != Z_STREAM_END);

    deflateEnd(&zs);
    return (EjsObj*) out;
}


/*
    compressString(data: String): String
 */
static EjsString *zlib_compressString(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString       *in;
    MprBuf          *out;
    z_stream        zs;
    uchar           outbuf[ZBUFSIZE];
    ssize           size, nbytes;
    int             level, flush;

    in = (EjsString*) argv[0];
    if ((size = in->length) == 0) {
        return ESV(empty);
    }
    if ((out = mprCreateBuf(in->length, 0)) == 0) {
        return 0;
    }
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    level = Z_DEFAULT_COMPRESSION;
    deflateInit(&zs, level);
    zs.next_in = (uchar*) in->value;
    zs.avail_in = (int) size;
    do {
        flush = (zs.avail_in == 0) ? Z_FINISH : Z_NO_FLUSH;
        do {
            zs.avail_out = ZBUFSIZE;
            zs.next_out = outbuf;
            deflate(&zs, flush);
            nbytes = ZBUFSIZE - zs.avail_out;
            if (mprPutBlockToBuf(out, (char*) outbuf, nbytes) != nbytes) {
                ejsThrowIOError(ejs, "Cannot copy to output buffer");
                deflateEnd(&zs);
                return 0;
            }
        } while (zs.avail_out == 0);
        assert(zs.avail_in == 0);
    } while (flush != Z_FINISH);

    deflateEnd(&zs);
    return ejsCreateStringFromBytes(ejs, mprGetBufStart(out), mprGetBufLength(out));
}


/*
    uncompressString(data: String): String
 */
static EjsString *zlib_uncompressString(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString       *in;
    MprBuf          *out;
    z_stream        zs;
    uchar           outbuf[ZBUFSIZE];
    ssize           nbytes, size;
    int             rc;

    in = (EjsString*) argv[0];
    if ((out = mprCreateBuf(ZBUFSIZE, -1)) == 0) {
        return 0;
    }
    if ((size = in->length) == 0) {
        return ESV(empty);
    }
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = 0;
    rc = inflateInit(&zs);
    zs.next_in = (uchar*) in->value;
    zs.avail_in = (int) size;

    do {
        if (zs.avail_in == 0) {
            break;
        }
        do {
            zs.avail_out = ZBUFSIZE;
            zs.next_out = outbuf;
            if ((rc = inflate(&zs, Z_NO_FLUSH)) == Z_NEED_DICT) {
                inflateEnd(&zs);
                return 0;
            } else if (rc == Z_DATA_ERROR || rc == Z_MEM_ERROR) {
                inflateEnd(&zs);
                return 0;
            } else {
                nbytes = ZBUFSIZE - zs.avail_out;
            }
            if (mprPutBlockToBuf(out, (char*) outbuf, nbytes) != nbytes) {
                ejsThrowIOError(ejs, "Cannot copy to byte array");
                inflateEnd(&zs);
                return 0;
            }
        } while (zs.avail_out == 0);
        assert(zs.avail_in == 0);
    } while (rc != Z_STREAM_END);

    deflateEnd(&zs);
    return ejsCreateStringFromBytes(ejs, mprGetBufStart(out), mprGetBufLength(out));
}

/*********************************** Factory *******************************/

#if UNUSED
static int manageZlib(EjsZlib *db, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManagePot(db, flags);

    } else if (flags & MPR_MANAGE_FREE) {
        if (db->sdb) {
            zlibClose(db->ejs, db, 0, 0);
        }
    }
    return 0;
}
#endif


static int configureZlibTypes(Ejs *ejs)
{
    EjsType     *type;
    
    if ((type = ejsFinalizeScriptType(ejs, N("ejs.zlib", "Zlib"), 0, NULL, 0)) == 0) {
        return 0;
    }
    ejsBindMethod(ejs, type, ES_ejs_zlib_Zlib_compress, zlib_compress);
    ejsBindMethod(ejs, type, ES_ejs_zlib_Zlib_uncompress, zlib_uncompress);
    ejsBindMethod(ejs, type, ES_ejs_zlib_Zlib_compressBytes, zlib_compressBytes);
    ejsBindMethod(ejs, type, ES_ejs_zlib_Zlib_uncompressBytes, zlib_uncompressBytes);
    ejsBindMethod(ejs, type, ES_ejs_zlib_Zlib_compressString, zlib_compressString);
    ejsBindMethod(ejs, type, ES_ejs_zlib_Zlib_uncompressString, zlib_uncompressString);
    return 0;
}


/*
    Module load entry point. This must be idempotent as it will be called for each new interpreter created.
 */
PUBLIC int ejs_zlib_Init(Ejs *ejs, MprModule *mp)
{
    return ejsAddNativeModule(ejs, "ejs.zlib", configureZlibTypes, _ES_CHECKSUM_ejs_zlib, EJS_LOADER_ETERNAL);
}
#endif /* BIT_EJS_ZLIB */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
