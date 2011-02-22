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
    function Cmd(cmdline, options)
 */
static EjsCmd *cmd_constructor(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    cmd->error = ejs->nullValue;
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


#if UNUSED
/**
    function get cmdline(): String
 */
static EjsObj *cmd_cmdline(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    return cmd->command;
}


/**
    function set cmdline(value: String): Void
 */
static EjsObj *cmd_cmdline(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    cmd->command = argv[0];
    return 0;
}
#endif


/**
    function get error(): Stream
 */
static EjsObj *cmd_error(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    int     fd;

    if (cmd->error == 0) {
        if ((fd = mprGetCmdFd(cmd->mc, MPR_CMD_STDOUT)) < 0) {
            //  MOB
        }
        if ((cmd->error = ejsCreateFileFromFd(ejs, fd, "error", O_RDONLY)) == 0) {
            //  MOB
        }
    }
    return (EjsObj*) cmd->error;
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
    mprCloseCmdFd(cmd->mc, MPR_CMD_STDIN);
    return 0;
}


/**
    function flush(dir: Number = Stream.BOTH): Stream
 */
static EjsObj *cmd_flush(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    //  MOB
    return 0;
}


/**
    static function kill(pid: Number, signal: Number = 2): Boolean
 */
static EjsObj *cmd_kill(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    int     rc, pid, signal;

    signal = SIGINT;

    if (argc == 0) {
        if (cmd->mc && cmd->mc->pid) {
            pid = cmd->mc->pid;
        } else {
            ejsThrowStateError(ejs, "No process to kill");
        }
    } else if (argc >= 1) {
        pid = ejsGetInt(ejs, argv[0]);
    } else if (argc >= 2) {
        signal = ejsGetInt(ejs, argv[0]);
    }
#if BLD_WIN_LIKE
{
    HANDLE	handle;
	handle = OpenProcess(DELETE, 0, pid);
    rc = TerminateProcess(handle, signal) == 0;
}
#elif VXWORKS
    // CRASH    rc = taskDelete(cmd->pid);
    rc = taskDelete(pid);
#else
    rc = kill(pid, signal);
#endif
    if (rc < 0) {
        if (cmd->throw) {
            ejsThrowIOError(ejs, "Can't kill %d with signal %d, errno %d", pid, signal, errno);
        }
        return ejs->falseValue;
    }
    return ejs->trueValue;
}


/**
    function get on(name, listener: Function): Void
 */
static EjsObj *cmd_on(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    //  MOB - check what "this" is set to in callback function
    cmd->async = 1;
    ejsAddObserver(ejs, &cmd->emitter, argv[0], argv[1]);

#if FUTURE
    if (cmd->mc && conn->readq->count > 0) {
        ejsSendEvent(ejs, hp->emitter, "readable", NULL, (EjsObj*) hp);
    }
    if (!conn->writeComplete && !conn->error && HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE &&
            conn->writeq->ioCount == 0) {
        ejsSendEvent(ejs, hp->emitter, "writable", NULL, (EjsObj*) hp);
    }
#endif
    return 0;
}


/**
    function get off(name, listener: Function): Void
 */
static EjsObj *cmd_off(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    ejsRemoveObserver(ejs, cmd->emitter, argv[0], argv[1]);
    //  MOB -- who to know to turn async off?
    return 0;
}


/**
    function pid(): Number
 */
static EjsObj *cmd_pid(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    if (cmd->mc == 0) {
        ejsThrowStateError(ejs, "No active command");
        return 0;
    }
    return (EjsObj*) ejsCreateNumber(ejs, cmd->mc->pid);
}


#if UNUSED
/*  
    Read the required number of bytes into the response content buffer. Count < 0 means transfer the entire content.
    Returns the number of bytes read.
 */ 
static ssize readCmdData(Ejs *ejs, EjsCmd *cmd, ssize count)
{
    MprBuf      *buf;
    ssize       space, nbytes;

    if (cmd->mc == 0) {
        ejsThrowStateError(ejs, "No active command");
        return 0;
    }
    if (cmd->stdoutBuf == 0) {
        cmd->stdoutBuf = mprCreateBuf(MPR_BUFSIZE, -1);
    }
    buf = cmd->stdoutBuf;
    mprResetBufIfEmpty(buf);
    while (count < 0 || mprGetBufLength(buf) < count) {
        if (mprGetBufSpace(buf) < MPR_BUFSIZE) {
            if (mprGrowBuf(buf, MPR_BUFSIZE) < 0) {
                ejsThrowMemoryError(ejs);
                return -1;
            }
        }
        space = mprGetBufSpace(buf);
        if ((nbytes = mprReadCmd(cmd->mc, MPR_CMD_STDOUT, mprGetBufEnd(buf), space)) < 0) {
            ejsThrowIOError(ejs, "Can't read required data");
            return MPR_ERR_CANT_READ;
        }
        mprAdjustBufEnd(buf, nbytes);
        if (nbytes == 0) {
            break;
        }
    }
    if (count < 0) {
        return mprGetBufLength(buf);
    }
    return min(count, mprGetBufLength(buf));
}
#endif


/**
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
 */
static EjsObj *cmd_read(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
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
    if (nbytes == 0) {
        if (mprWaitForCmd(cmd->mc, cmd->timeout) < 0) {
            ejsThrowStateError(ejs, "Command timed out");
            return 0;
        }
        nbytes = mprGetBufLength(cmd->stdoutBuf);
    }
    count = min(count, nbytes);
    ejsCopyToByteArray(ejs, buffer, buffer->writePosition, (char*) mprGetBufStart(cmd->stdoutBuf), count);
    ejsSetByteArrayPositions(ejs, buffer, -1, buffer->writePosition + count);
    mprAdjustBufStart(cmd->stdoutBuf, count);
    return (EjsObj*) ejsCreateNumber(ejs, count);
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
    if (nbytes == 0) {
        if (mprWaitForCmd(cmd->mc, cmd->timeout) < 0) {
            ejsThrowStateError(ejs, "Command timed out");
            return 0;
        }
        nbytes = mprGetBufLength(cmd->stdoutBuf);
    }
    count = min(count, nbytes);
    result = ejsCreateStringFromBytes(ejs, mprGetBufStart(cmd->stdoutBuf), count);
    mprAdjustBufEnd(cmd->stdoutBuf, count);
    mprResetBufIfEmpty(cmd->stdoutBuf);
    return result;
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
            if (value == ejs->trueValue) {
                flags |= MPR_CMD_DETACH;
            }
        }
        if ((value = ejsGetPropertyByName(ejs, cmd->options, EN("dir"))) != 0) {
            path = ejsToPath(ejs, value);
            if (path) {
                mprSetCmdDir(cmd->mc, path->value);
            }
        }
        if ((value = ejsGetPropertyByName(ejs, cmd->options, EN("exception"))) != 0) {
            if (value == ejs->trueValue) {
                cmd->throw = 1;
            }
        }
    }
    if (cmd->async) {
        flags |= MPR_CMD_ASYNC;
    }
    return flags;
}


//  MOB - unique name
static void cmdCallback(MprCmd *mc, int channel, void *data)
{
    EjsCmd      *cmd;
    MprBuf      *buf;
    ssize       len, space;

    /*
        Note: stdin, stdout and stderr are named from the client's perspective
     */
    cmd = data;
    buf = 0;
    switch (channel) {
    case MPR_CMD_STDIN:
        return;
    case MPR_CMD_STDOUT:
        buf = cmd->stdoutBuf;
        break;
    case MPR_CMD_STDERR:
        buf = cmd->stderrBuf;
        break;
    }
    /*
        Read and aggregate the result into a single string
     */
    space = mprGetBufSpace(buf);
    if (space < (MPR_BUFSIZE / 4)) {
        if (mprGrowBuf(buf, MPR_BUFSIZE) < 0) {
            mprCloseCmdFd(mc, channel);
            return;
        }
        space = mprGetBufSpace(buf);
    }
    len = mprReadCmd(mc, channel, mprGetBufEnd(buf), space);
    if (len <= 0) {
        if (len == 0 || (len < 0 && !(errno == EAGAIN || EWOULDBLOCK))) {
            if (channel == MPR_CMD_STDOUT && mc->flags & MPR_CMD_ERR) {
                /*
                    Now that stdout is complete, enable stderr to receive an EOF or any error output.
                    This is serialized to eliminate both stdin and stdout events on different threads at the same time.
                    Do before closing as the stderr event may come on another thread and we want to ensure avoid locking.
                 */
                mprCloseCmdFd(mc, channel);
                mprEnableCmdEvents(mc, MPR_CMD_STDERR);
            } else {
                mprCloseCmdFd(mc, channel);
            }
            return;
        }
    } else {
        mprAdjustBufEnd(buf, len);
    }
    mprEnableCmdEvents(mc, channel);
}


/**
    function start(cmdline: String = null, options: Object = {}): Void
 */
static EjsObj *cmd_start(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    EjsArray    *ap;
    char        *command, *err;
    int         rc, flags, i;

    if (argc >= 1) {
        cmd->command = argv[0];
    }
    if (argc >= 2) {
        cmd->options = argv[1];
    }
    if (cmd->command == ejs->nullValue) {
        ejsThrowStateError(ejs, "Missing command line");
        return 0;
    }
    if ((cmd->mc = mprCreateCmd(ejs->dispatcher)) == 0) {
        return 0;
    }
    mprSetCmdCallback(cmd->mc, cmdCallback, cmd);
    if (cmd->stdoutBuf == 0) {
        cmd->stdoutBuf = mprCreateBuf(MPR_BUFSIZE, -1);
    }
    if (cmd->stderrBuf == 0) {
        cmd->stderrBuf = mprCreateBuf(MPR_BUFSIZE, -1);
    }
    if (ejsIsArray(ejs, cmd->command)) {
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
        cmd->command = ejsToString(ejs, cmd->command);
        command = ejsToMulti(ejs, argv[0]);
        if (mprMakeArgv(command, &cmd->argc, &cmd->argv, 0) < 0 || cmd->argv == 0) {
            ejsThrowArgError(ejs, "Can't parse command line");
            return 0;
        }
    }
    flags = parseOptions(ejs, cmd);

    if ((rc = mprStartCmd(cmd->mc, cmd->argc, cmd->argv, cmd->env, flags)) < 0) {
        if (rc == MPR_ERR_CANT_ACCESS) {
            err = mprAsprintf("Can't access command %s", cmd->argv[0]);
        } else if (MPR_ERR_CANT_OPEN) {
            err = mprAsprintf("Can't open standard I/O for command %s", cmd->argv[0]);
        } else if (rc == MPR_ERR_CANT_CREATE) {
            err = mprAsprintf("Can't create process for %s", cmd->argv[0]);
        } else {
            err = "";
        }
        ejsThrowError(ejs, "Command failed: %s\n\nError %d, %s\n\nError Output: %s", cmd->command, rc, err);
        return 0;
    }
    if (!(flags & MPR_CMD_DETACH)) {
        if (mprWaitForCmd(cmd->mc, -1) < 0) {
            //  MOB - more diagnostics - why cant wait
            ejsThrowStateError(ejs, "Timeout waiting for command to complete: %s", cmd->command);
        }
    }
    return 0;
}


/**
    function get status(): Number
 */
static EjsObj *cmd_status(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    int     status;

    if (cmd->mc == 0) {
        ejsThrowStateError(ejs, "No active command");
        return 0;
    }
    if (mprWaitForCmd(cmd->mc, cmd->timeout) < 0) {
        ejsThrowStateError(ejs, "Command still active");
    }
    if (mprGetCmdExitStatus(cmd->mc, &status) < 0) {
        ejsThrowStateError(ejs, "Status not ready to collect");
        return 0;
    }
    return (EjsObj*) ejsCreateNumber(ejs, status);
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
    if (mprStopCmd(cmd->mc, signal) < 0) {
        ejsThrowIOError(ejs, "Can't kill %d with signal %d, errno %d", cmd->mc->pid, signal, errno);
        return ejs->falseValue;
    }
    return ejs->trueValue;
}




/**
    function get timeout(): Number
 */
static EjsNumber *cmd_timeout(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, cmd->timeout);
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
    int     timeout;

    timeout = argc > 0 ? ejsGetInt(ejs, argv[0]) : cmd->timeout;
    if (cmd->mc == 0) {
        ejsThrowStateError(ejs, "No active command");
        return 0;
    }
    if (mprWaitForCmd(cmd->mc, timeout) < 0) {
        ejsThrowStateError(ejs, "Can't wait for command");
    }
    return 0;
}


/**
    function write(...data): Number
 */
static EjsObj *cmd_write(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *bp;
    EjsString       *sp;
    EjsDate         *dp;
    EjsNumber       *np;
    EjsObj          *vp;
    char            bvalue;
    ssize           len;
    int             i, wrote;

    mprAssert(argc == 1 && ejsIsArray(ejs, argv[0]));

    /*
        Unwrap nested arrays
     */
    args = (EjsArray*) argv[0];
    while (args && ejsIsArray(ejs, args) && args->length == 1) {
        vp = ejsGetProperty(ejs, args, 0);
        if (!ejsIsArray(ejs, vp)) {
            break;
        }
        args = (EjsArray*) vp;
    }
    wrote = 0;

    for (i = 0; i < args->length; i++) {
        if ((vp = ejsGetProperty(ejs, args, i)) == 0) {
            continue;
        }
        switch (TYPE(vp)->id) {
        case ES_Boolean:
            bvalue = ejsGetBoolean(ejs, vp);
            wrote = mprWriteCmd(cmd->mc, MPR_CMD_STDIN, &bvalue, 1);
            break;

        case ES_Date:
            //  MOB - is this really the best way to encode. 
            dp = (EjsDate*) vp;
            wrote = mprWriteCmd(cmd->mc, MPR_CMD_STDIN, (char*) &dp->value, sizeof(MprNumber));
            break;

        case ES_Number:
            np = (EjsNumber*) vp;
            wrote = mprWriteCmd(cmd->mc, MPR_CMD_STDIN, (char*) &np->value, sizeof(MprNumber));
            break;

        default:
            vp = (EjsObj*) ejsToString(ejs, vp);
            /* Fall through */

        case ES_String:
            sp = (EjsString*) vp;
            //  MOB - encoding
            //  MOB - what about blocking
            wrote = mprWriteCmd(cmd->mc, MPR_CMD_STDIN, sp->value, sp->length);
            break;


        case ES_ByteArray:
            bp = (EjsByteArray*) vp;
            //  MOB - should this update the byte array source?
            len = ejsGetByteArrayAvailable(bp);
            wrote = mprWriteCmd(cmd->mc, MPR_CMD_STDIN, (char*) &bp->value[bp->readPosition], len);
            break;
        }
    }
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) wrote);
}


#if OLD && UNUSED
/*
    function run(cmd: String): String
 */
static EjsObj *cmd_run(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprCmd      *cmd;
    char        *cmdline;
    char        *err, *output;
    int         status;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

    cmd = mprCreateCmd(ejs->dispatcher);
    cmdline = ejsToMulti(ejs, argv[0]);
    mprHold(cmdline);
    status = mprRunCmd(cmd, cmdline, &output, &err, 0);
    if (status) {
        ejsThrowError(ejs, "Command failed: %s\n\nExit status: %d\n\nError Output: \n%s\nPrevious Output: \n%s\n", 
            cmdline, status, err, output);
        mprDestroyCmd(cmd);
        mprRelease(cmdline);
        return 0;
    }
    mprDestroyCmd(cmd);
    mprRelease(cmdline);
    return (EjsObj*) ejsCreateStringFromAsc(ejs, output);
}


/*
    function runx(cmd: String): Void
 */
static EjsObj *cmd_runx(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprCmd      *cmd;
    char        *err;
    int         status;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

    cmd = mprCreateCmd(ejs->dispatcher);
    status = mprRunCmd(cmd, ejsToMulti(ejs, argv[0]), NULL, &err, 0);
    if (status) {
        ejsThrowError(ejs, "Can't run command: %@\nDetails: %s", ejsToString(ejs, argv[0]), err);
    }
    mprDestroyCmd(cmd);
    return 0;
}


//  TODO - refactor and rename
/*
    function daemon(cmd: String): Number
 */
static EjsObj *cmd_daemon(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprCmd      *cmd;
    int         status, pid;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

    cmd = mprCreateCmd(ejs->dispatcher);
    status = mprRunCmd(cmd, ejsToMulti(ejs, argv[0]), NULL, NULL, MPR_CMD_DETACH);
    if (status) {
        ejsThrowError(ejs, "Can't run command: %@", ejsToString(ejs, argv[0]));
    }
    pid = cmd->pid;
    mprDestroyCmd(cmd);
    return (EjsObj*) ejsCreateNumber(ejs, pid);
}


//  TODO - refactor and rename
/*
    function exec(cmd: String): Void
 */
static EjsObj *cmd_exec(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
#if BLD_UNIX_LIKE
    char    **argVector;
    int     argCount;

    mprMakeArgv(ejsToMulti(ejs, argv[0]), &argCount, &argVector, 0);
    execv(argVector[0], argVector);
#endif
    ejsThrowStateError(ejs, "Can't exec %@", ejsToString(ejs, argv[0]));
    return 0;
}
#endif /* OLD && UNUSED */


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
#if DEBUG_IDE || 1
    EjsType     *type;
    EjsPot      *prototype;

#if UNUSED
    EjsType     *type;
    EjsHelpers  *helpers;

    type = ejs->cmdType = ejsConfigureNativeType(ejs, N("ejs", "Cmd"), sizeof(EjsCmd), 
        (MprManager) manageCmd, EJS_POT_HELPERS);
    prototype = type->prototype;

    helpers = &type->helpers;
    helpers->cast = (EjsCastHelper) castByteArrayVar;
    helpers->clone = (EjsCloneHelper) cloneByteArrayVar;
    helpers->deleteProperty = (EjsDeletePropertyHelper) deleteByteArrayProperty;
    helpers->getProperty = (EjsGetPropertyHelper) getByteArrayProperty;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) getByteArrayPropertyCount;
    helpers->invokeOperator = (EjsInvokeOperatorHelper) invokeByteArrayOperator;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupByteArrayProperty;
    helpers->setProperty = (EjsSetPropertyHelper) setByteArrayProperty;
#endif

    if ((type = ejsGetTypeByName(ejs, N("ejs", "Cmd"))) == 0) {
        mprError("Can't find Cmd type");
        return;
    }
    type->instanceSize = sizeof(EjsCmd);
    type->manager = (MprManager) manageEjsCmd;
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, cmd_constructor);
    ejsBindMethod(ejs, type, ES_Cmd_kill, cmd_kill);
#if FUTURE
    ejsBindMethod(ejs, type, ES_Cmd_run, cmd_run);
    ejsBindMethod(ejs, type, ES_Cmd_exec, cmd_exec);
#endif

    ejsBindMethod(ejs, prototype, ES_Cmd_close, cmd_close);
    ejsBindAccess(ejs, prototype, ES_Cmd_error, cmd_error, 0);
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
#endif
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
