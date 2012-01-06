	/*
    ejsCmd.c -- Cmd class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forwards *********************************/

static EjsObj *cmd_start(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv);

/************************************ Methods *********************************/
/*
    function Cmd(cmdline: Object, options: Object)
 */
static EjsCmd *cmd_constructor(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    cmd->stdoutBuf = mprCreateBuf(MPR_BUFSIZE, -1);
    cmd->stderrBuf = mprCreateBuf(MPR_BUFSIZE, -1);
    cmd->ejs = ejs;
    cmd->timeout = -1;
    if (argc >= 1) {
        cmd_start(ejs, cmd, argc, argv);
    }
    return cmd;
}


/**
    function close(): Void
 */
static EjsObj *cmd_close(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    if (cmd->mc) {
        mprDestroyCmd(cmd->mc);
        cmd->mc = 0;
    }
    return 0;
}


/**
    function get errorStream(): Stream
 */
static EjsByteArray *cmd_errorStream(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    if (cmd->error == 0) {
        cmd->error = ejsCreateByteArray(ejs, -1);
    }
    return cmd->error;
}


/**
    function get env(): Object
 */
static EjsObj *cmd_env(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    return cmd->env;
}


/**
    function set env(values: Object): Void
 */
static EjsObj *cmd_set_env(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    cmd->env = argv[0];
    return 0;
}


/**
    function finalize(): Void
 */
static EjsObj *cmd_finalize(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    if (cmd->mc == 0) {
        ejsThrowStateError(ejs, "No active command");
        return 0;
    }
    mprFinalizeCmd(cmd->mc);
    return 0;
}


/**
    function flush(dir: Number = Stream.BOTH): Void
 */
static EjsObj *cmd_flush(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    /* Nothing to do */
    return 0;
}


/**
    static function kill(pid: Number, signal: Number = 2): Boolean
 */
static EjsObj *cmd_kill(Ejs *ejs, EjsAny *unused, int argc, EjsObj **argv)
{
    int     rc, pid, signal;

#if BLD_UNIX_LIKE
    signal = SIGINT;
#else
    signal = 2;
#endif
    if (argc >= 2) {
        signal = ejsGetInt(ejs, argv[1]);
    }
    pid = ejsGetInt(ejs, argv[0]);
    if (pid == 0) {
        ejsThrowStateError(ejs, "No process to kill");
        return 0;
    }
#if BLD_WIN_LIKE
{
    HANDLE	handle;
	handle = OpenProcess(PROCESS_TERMINATE, 0, pid);
    if (handle == 0) {
        ejsThrowIOError(ejs, "Can't find process ID %d", pid);
        return 0;
    }
    rc = TerminateProcess(handle, signal) == 0;
}
#elif VXWORKS
    rc = taskDelete(pid);
#else
    rc = kill(pid, signal);
#endif
    if (rc < 0) {
        ejsThrowIOError(ejs, "Can't kill %d with signal %d, errno %d", pid, signal, errno);
        return ESV(false);
    }
    return ESV(true);
}


/**
    function get on(name, observer: Function): Void
 */
static EjsObj *cmd_on(Ejs *ejs, EjsCmd *cmd, int argc, EjsAny **argv)
{
    EjsFunction     *observer;

    observer = (EjsFunction*) argv[1];
    if (observer->boundThis == 0 || observer->boundThis == ejs->global) {
        observer->boundThis = cmd;
    }
    ejsAddObserver(ejs, &cmd->emitter, argv[0], observer);
    if (!cmd->async) {
        cmd->async = 1;
    }
    return 0;
}


/**
    function get off(name, observer: Function): Void
 */
static EjsObj *cmd_off(Ejs *ejs, EjsCmd *cmd, int argc, EjsAny **argv)
{
    ejsRemoveObserver(ejs, cmd->emitter, argv[0], argv[1]);
    return 0;
}


/**
    function pid(): Number
 */
static EjsNumber *cmd_pid(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    if (cmd->mc == 0 || cmd->mc->pid == 0) {
        return ESV(zero);
    }
    return ejsCreateNumber(ejs, cmd->mc->pid);
}


/**
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
 */
static EjsNumber *cmd_read(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    EjsByteArray    *buffer;
    ssize           offset, count, nbytes;

    mprAssert(1 <= argc && argc <= 3);

    buffer = (EjsByteArray*) argv[0];
    offset = (argc == 2) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc == 3) ? ejsGetInt(ejs, argv[2]) : (int) buffer->length;

    if (count < 0) {
        count = buffer->length;
    }
    if (offset < 0) {
        offset = buffer->writePosition;
    } else if (offset >= buffer->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad read offset value");
        return 0;
    } else {
        buffer->readPosition = 0;
        buffer->writePosition = 0;
    }
    count = buffer->length - buffer->writePosition;
    if (count <= 0) {
        if (ejsGrowByteArray(ejs, buffer, MPR_BUFSIZE) < 0) {
            return 0;
        }
        count = buffer->length - buffer->writePosition;
    }
    nbytes = mprGetBufLength(cmd->stdoutBuf);
    if (nbytes == 0 && !cmd->async && cmd->mc) {
        if (mprWaitForCmd(cmd->mc, cmd->timeout) < 0) {
            ejsThrowStateError(ejs, "Command timed out");
            return 0;
        }
        nbytes = mprGetBufLength(cmd->stdoutBuf);
    }
    count = min(count, nbytes);
    //  MOB - should use RC Value (== count)
    ejsCopyToByteArray(ejs, buffer, buffer->writePosition, (char*) mprGetBufStart(cmd->stdoutBuf), count);
    ejsSetByteArrayPositions(ejs, buffer, -1, buffer->writePosition + count);
    mprAdjustBufStart(cmd->stdoutBuf, count);
    return ejsCreateNumber(ejs, (MprNumber) count);
}


/*
    Read a UTF-8 string from the array. Read data from the read position up to the write position but not more 
    than count characters.

    function readString(count: Number = -1): String
 */
static EjsString *cmd_readString(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    EjsString   *result;
    ssize       nbytes, count;
    
    mprAssert(0 <= argc && argc <= 1);
    
    count = (argc >= 1) ? ejsGetInt(ejs, argv[0]) : -1;
    if (count < 0) {
        count = MAXSSIZE;
    }
    nbytes = mprGetBufLength(cmd->stdoutBuf);
    if (nbytes == 0 && cmd->mc) {
        if (mprWaitForCmd(cmd->mc, cmd->timeout) < 0) {
            ejsThrowStateError(ejs, "Command timed out");
            return 0;
        }
        nbytes = mprGetBufLength(cmd->stdoutBuf);
    }
    count = min(count, nbytes);
    result = ejsCreateStringFromBytes(ejs, mprGetBufStart(cmd->stdoutBuf), count);
    mprAdjustBufStart(cmd->stdoutBuf, count);
    mprResetBufIfEmpty(cmd->stdoutBuf);
    return result;
}


static ssize cmdIOCallback(MprCmd *mc, int channel, void *data)
{
    EjsCmd          *cmd;
    EjsByteArray    *ba;
    MprBuf          *buf;
    ssize           len, space;

    /*
        Note: stdin, stdout and stderr are named from the client's perspective
     */
    cmd = data;
    buf = 0;

    switch (channel) {
    case MPR_CMD_STDIN:
        ejsSendEvent(cmd->ejs, cmd->emitter, "writable", NULL, cmd);
        mprEnableCmdEvents(mc, channel);
        return 0;
    case MPR_CMD_STDOUT:
        buf = cmd->stdoutBuf;
        break;
    case MPR_CMD_STDERR:
        buf = cmd->stderrBuf;
        break;
    default:
        /* Child death */
        return 0;
    }
    /*
        Read and aggregate the result into a single string
     */
    mprResetBufIfEmpty(buf);
    space = mprGetBufSpace(buf);
    if (space < (MPR_BUFSIZE / 4)) {
        if (mprGrowBuf(buf, MPR_BUFSIZE) < 0) {
            mprCloseCmdFd(mc, channel);
            return 0;
        }
        space = mprGetBufSpace(buf);
    }
    mprAssert(mc->files[channel].fd >= 0);
    len = mprReadCmd(mc, channel, mprGetBufEnd(buf), space);
    if (len <= 0) {
        if (len == 0 || (len < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK))) {
            mprCloseCmdFd(mc, channel);
        }
    } else {
        mprAdjustBufEnd(buf, len);
    }
    if (len > 0) {
        mprEnableCmdEvents(mc, channel);
    } else if (len < 0) {
        len = 0;
    }
    if (channel == MPR_CMD_STDERR) {
        if (cmd->error == 0) {
            cmd->error = ejsCreateByteArray(cmd->ejs, -1);
        }
        ba = cmd->error;
        //  MOB - should use RC Value (== count)
        ejsCopyToByteArray(cmd->ejs, ba, ba->writePosition, mprGetBufStart(buf), len);
        ba->writePosition += len;
        mprAdjustBufStart(buf, len);
        mprResetBufIfEmpty(buf);
    }
    if (cmd->async) {
        if (channel == MPR_CMD_STDOUT) {
            ejsSendEvent(cmd->ejs, cmd->emitter, "readable", NULL, cmd);
        }
        if (channel == MPR_CMD_STDERR) {
            if (len > 0) {
                ejsSendEvent(cmd->ejs, cmd->emitter, "error", NULL, cmd);
            } else {
                ejsSendEvent(cmd->ejs, cmd->emitter, "complete", NULL, cmd);
            }
        }
    }
    return len;
}


static int parseOptions(Ejs *ejs, EjsCmd *cmd)
{
    EjsObj      *value;
    EjsPath     *path;
    int         flags;

    cmd->throw = 0;    
    flags = MPR_CMD_IN | MPR_CMD_OUT | MPR_CMD_ERR;
    if (cmd->options) {
        if ((value = ejsGetPropertyByName(ejs, cmd->options, EN("detach"))) != 0) {
            if (value == ESV(true)) {
                flags |= MPR_CMD_DETACH;
            }
        }
        if ((value = ejsGetPropertyByName(ejs, cmd->options, EN("dir"))) != 0) {
            path = ejsToPath(ejs, value);
            if (path && cmd->mc) {
                mprSetCmdDir(cmd->mc, path->value);
            }
        }
        if ((value = ejsGetPropertyByName(ejs, cmd->options, EN("exception"))) != 0) {
            if (value == ESV(true)) {
                cmd->throw = 1;
            }
        }
        if ((value = ejsGetPropertyByName(ejs, cmd->options, EN("timeout"))) != 0) {
            cmd->timeout = ejsGetInt(ejs, value);
        }
    }
    return flags;
}


static bool setCmdArgs(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    EjsArray    *ap;
    int         i;

    if (ejsIs(ejs, cmd->command, Array)) {
        ap = (EjsArray*) cmd->command;
        if ((cmd->argv = mprAlloc(sizeof(void*) * (ap->length + 1))) == 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
        for (i = 0; i < ap->length; i++) {
            cmd->argv[i] = ejsToMulti(ejs, ejsToString(ejs, ejsGetProperty(ejs, cmd->command, i)));
        }
        cmd->argv[i] = 0;
        cmd->argc = ap->length;

    } else {
        cmd->command = ejsToMulti(ejs, cmd->command);
        if ((cmd->argc = mprMakeArgv(cmd->command, &cmd->argv, 0)) < 0 || cmd->argv == 0) {
            ejsThrowArgError(ejs, "Can't parse command line");
            return 0;
        }
    }
    return 1;
}


/**
    function start(cmdline: Object, options: Object = null): Void
 */
static EjsObj *cmd_start(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    EjsName     qname;
    char        *err, **env;
    int         rc, flags, len, i, status;

    cmd->command = argv[0];
    cmd->options = (argc >=2 ) ? argv[1] : 0;

    if (cmd->command == ESV(null)) {
        ejsThrowStateError(ejs, "Missing command line");
        return 0;
    }
    if ((cmd->mc = mprCreateCmd(ejs->dispatcher)) == 0) {
        return 0;
    }
    mprSetCmdCallback(cmd->mc, cmdIOCallback, cmd);
    mprFlushBuf(cmd->stdoutBuf);
    mprFlushBuf(cmd->stderrBuf);
    if (!setCmdArgs(ejs, cmd, argc, argv)) {
        return 0;
    }
    if (cmd->env) {
        len = ejsGetLength(ejs, cmd->env);
        if ((env = mprAlloc(sizeof(void*) * (len + 1))) == 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
        for (i = 0; i < len; i++) {
            qname = ejsGetPropertyName(ejs, cmd->env, i);
            env[i] = sfmt("%s=%s", qname.name->value, 
                ejsToMulti(ejs, ejsToString(ejs, ejsGetProperty(ejs, cmd->env, i))));
        }
        env[i] = 0;
    } else {
        env = 0;
    }
    flags = parseOptions(ejs, cmd);

    if ((rc = mprStartCmd(cmd->mc, cmd->argc, cmd->argv, env, flags)) < 0) {
        if (rc == MPR_ERR_CANT_ACCESS) {
            err = "Can't access command";
        } else if (MPR_ERR_CANT_OPEN) {
            err = "Can't open standard I/O for command";
        } else if (rc == MPR_ERR_CANT_CREATE) {
            err = "Can't create process";
        } else {
            err = "";
        }
        ejsThrowError(ejs, "Command failed: %s\nError Output: %s", cmd->argv[0], err);
        return 0;
    }
    if (!(flags & MPR_CMD_DETACH)) {
        mprAssert(cmd->mc);
        mprFinalizeCmd(cmd->mc);
        if (mprWaitForCmd(cmd->mc, cmd->timeout) < 0) {
            ejsThrowStateError(ejs, "Timeout %d msec waiting for command to complete: %s", cmd->timeout, cmd->argv[0]);
            return 0;
        }
        mprAssert(cmd->mc);
        if (cmd->throw) {
            status = mprGetCmdExitStatus(cmd->mc);
            if (status != 0) {
                ejsThrowIOError(ejs, "Command failed status %d, %@", status, ejsToString(ejs, cmd->error));
            }
        }
    }
    return 0;
}


/**
    function get status(): Number
 */
static EjsNumber *cmd_status(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    if (cmd->mc == 0) {
        ejsThrowStateError(ejs, "No active command");
        return 0;
    }
    if (mprWaitForCmd(cmd->mc, cmd->timeout) < 0) {
        ejsThrowStateError(ejs, "Command still active");
    }
    return ejsCreateNumber(ejs, mprGetCmdExitStatus(cmd->mc));
}


/**
    function stop(signal: Number = 2): Boolean
 */
static EjsObj *cmd_stop(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    int     signal;

    signal = (argc >= 1) ? ejsGetInt(ejs, argv[0]) : SIGINT;

    if (cmd->mc == 0) {
        ejsThrowStateError(ejs, "No active command");
        return 0;
    }
    mprFinalizeCmd(cmd->mc);
    if (!mprIsCmdRunning(cmd->mc)) {
        return ESV(true);
    }
    if (mprStopCmd(cmd->mc, signal) < 0) {
        return ESV(false);
    }
    return ESV(true);
}


/**
    function get timeout(): Number
 */
static EjsNumber *cmd_timeout(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) cmd->timeout);
}


/**
    function set timeout(msec: Number): Void
 */
static EjsObj *cmd_set_timeout(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    cmd->timeout = ejsGetInt(ejs, argv[0]);
    return 0;
}


/**
    function wait(timeout: Number = -1): Boolean
 */
static EjsObj *cmd_wait(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    MprTime     timeout;

    timeout = argc > 0 ? ejsGetInt(ejs, argv[0]) : cmd->timeout;
    if (cmd->mc == 0) {
        ejsThrowStateError(ejs, "No active command");
        return 0;
    }
    /* NOTE: mprWaitForCmd will auto-finalize */
    if (mprWaitForCmd(cmd->mc, timeout) < 0) {
        return ESV(false);
    }
    return ESV(true);
}


/**
    function write(...data): Number
 */
static EjsNumber *cmd_write(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *bp;
    EjsString       *sp;
    EjsObj          *vp;
    ssize           len, wrote;
    int             i;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Array));

    /*
        Unwrap nested arrays
     */
    args = (EjsArray*) argv[0];
    while (ejsIs(ejs, args, Array) && args->length == 1) {
        vp = ejsGetProperty(ejs, args, 0);
        if (!ejsIs(ejs, vp, Array)) {
            break;
        }
        args = (EjsArray*) vp;
    }
    wrote = 0;
    for (i = 0; i < args->length; i++) {
        if ((vp = ejsGetProperty(ejs, args, i)) == 0) {
            continue;
        }
        if (ejsIs(ejs, vp, ByteArray)) {
            bp = (EjsByteArray*) vp;
            len = bp->writePosition - bp->readPosition;
            wrote += mprWriteCmd(cmd->mc, MPR_CMD_STDIN, (char*) &bp->value[bp->readPosition], len);
        } else {
            sp = (EjsString*) ejsToString(ejs, vp);
            wrote += mprWriteCmd(cmd->mc, MPR_CMD_STDIN, sp->value, sp->length);
        }
    }
    return ejsCreateNumber(ejs, (MprNumber) wrote);
}


//  MOB - compare with System.exec
/*
    function exec(cmd: Object): Void
 */
static EjsObj *cmd_exec(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
#if BLD_UNIX_LIKE
    char    **argVector, *path;

#if FUTURE
    for (i = 3; i < MPR_MAX_FILE; i++) {
        close(i);
    }
#endif
    if (argc == 0) {
        path = MPR->argv[0];
        if (!mprIsPathAbs(path)) {
            path = mprGetAppPath();
        }
        execv(path, MPR->argv);
    } else {
        mprMakeArgv(ejsToMulti(ejs, argv[0]), &argVector, 0);
        execv(argVector[0], argVector);
    }
#endif
    ejsThrowStateError(ejs, "Can't exec %@", ejsToString(ejs, argv[0]));
    return 0;
}

/************************************ Factory *********************************/

static void manageEjsCmd(EjsCmd *cmd, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManagePot(cmd, flags);
        mprMark(cmd->emitter);
        mprMark(cmd->mc);
        mprMark(cmd->stdoutBuf);
        mprMark(cmd->stderrBuf);
        mprMark(cmd->command);
        mprMark(cmd->env);
        mprMark(cmd->options);
        mprMark(cmd->error);
        mprMark(cmd->argv);

    } else {
        if (cmd->mc) {
            mprDestroyCmd(cmd->mc);
            cmd->mc = 0;
        }
    }
}


void ejsConfigureCmdType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Cmd"), sizeof(EjsCmd), manageEjsCmd,
            EJS_TYPE_POT | EJS_TYPE_MUTABLE_INSTANCES)) == 0) {
        return;
    }
    ejsBindConstructor(ejs, type, cmd_constructor);
    ejsBindMethod(ejs, type, ES_Cmd_kill, cmd_kill);
    ejsBindMethod(ejs, type, ES_Cmd_exec, cmd_exec);

    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_Cmd_close, cmd_close);
    ejsBindAccess(ejs, prototype, ES_Cmd_errorStream, cmd_errorStream, 0);
    ejsBindAccess(ejs, prototype, ES_Cmd_env, cmd_env, cmd_set_env);
    ejsBindMethod(ejs, prototype, ES_Cmd_finalize, cmd_finalize);
    ejsBindMethod(ejs, prototype, ES_Cmd_flush, cmd_flush);
    ejsBindMethod(ejs, prototype, ES_Cmd_on, cmd_on);
    ejsBindMethod(ejs, prototype, ES_Cmd_off, cmd_off);
    ejsBindAccess(ejs, prototype, ES_Cmd_pid, cmd_pid, 0);
    ejsBindMethod(ejs, prototype, ES_Cmd_read, cmd_read);
    ejsBindMethod(ejs, prototype, ES_Cmd_readString, cmd_readString);
    ejsBindMethod(ejs, prototype, ES_Cmd_start, cmd_start);
    ejsBindAccess(ejs, prototype, ES_Cmd_status, cmd_status, 0);
    ejsBindMethod(ejs, prototype, ES_Cmd_stop, cmd_stop);
    ejsBindAccess(ejs, prototype, ES_Cmd_timeout, cmd_timeout, cmd_set_timeout);

    ejsBindMethod(ejs, prototype, ES_Cmd_wait, cmd_wait);
    ejsBindMethod(ejs, prototype, ES_Cmd_write, cmd_write);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
