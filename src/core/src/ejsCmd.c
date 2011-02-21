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
static EjsObj *cmd_constructor(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    cmd->error = ejs->nullValue;
    if (argc >= 1) {
        cmd_start(ejs, cmd, argc, argv);
    }
    return 0;
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
    int     pid, signal;

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
    if (kill(pid, signal) < 0) {
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
    ejsAddObserver(ejs, &cmd->emitter, argv[0], argv[1]);
    return 0;
}


/**
    function get off(name, listener: Function): Void
 */
static EjsObj *cmd_off(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    ejsRemoveObserver(ejs, cmd->emitter, argv[0], argv[1]);
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


/**
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
 */
static EjsObj *cmd_read(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
#if MOB
    EjsByteArray    *buffer;
    ssize           offset, count;
    int             i;

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
    if (getInput(ejs, ap, 1) <= 0) {
        /* eof */
        return (EjsObj*) ejs->nullValue;
    }
    count = min(availableBytes(ap), count);
    for (i = 0; i < count; i++) {
        buffer->value[offset++] = ap->value[ap->readPosition++];
    }
    buffer->writePosition += count;
    if (ap->emitter && availableBytes(ap) && !ejs->exception) {
        ejsSendEvent(ejs, ap->emitter, "writable", NULL, (EjsObj*) ap);
    }
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) count);
#endif
    return 0;
}


/*
    Read a UTF-8 string from the array. Read data from the read position up to the write position but not more 
    than count characters.

    function readString(count: Number = -1): String
 */
static EjsObj *cmd_readString(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
#if MOB
    EjsObj  *result;
    ssize   count;

    count = (argc == 1) ? ejsGetInt(ejs, argv[0]) : -1;

    if (count < 0) {
        if (getInput(ejs, ap, 1) < 0) {
            return (EjsObj*) ejs->nullValue;
        }
        count = availableBytes(ap);

    } else if (getInput(ejs, ap, count) < 0) {
        return (EjsObj*) ejs->nullValue;
    }
    count = min(count, availableBytes(ap));
    //  MOB - UNICODE ENCODING
    result = (EjsObj*) ejsCreateStringFromMulti(ejs, (cchar*) &ap->value[ap->readPosition], count);
    adjustReadPosition(ap, count);
    return result;
#endif
    return 0;
}


static void createArgs(EjsCmd *cmd, int *pargc, char ***pargv, char ***penv)
{
    //  MOB - UNICODE
    *pargc = cmd->argc;
    *pargv = cmd->argv;
    *penv = cmd->env;
}


/**
    function start(cmdline: String = null, options: Object = {}): Void
 */
static EjsObj *cmd_start(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    char    *command, **av, **env, *err;
    int     rc, flags, ac;

    if (argc >= 1) {
        cmd->command = argv[0];
    }
    if (argc >= 2) {
        cmd->options = argv[1];
    }
    if (cmd->command == ejs->nullValue) {
        ejsThrowStateError(ejs, "Missing command line");
    } else {
        if (ejsIsString(ejs, cmd->command)) {
            command = ejsToMulti(ejs, argv[0]);
            if (mprMakeArgv(NULL, command, &cmd->argc, &cmd->argv) < 0 || cmd->argv == 0) {
                ejsThrowArgError(ejs, "Can't parse command line");
                return 0;
            }
        }
        createArgs(cmd, &ac, &av, &env);

        if ((cmd->mc = mprCreateCmd(ejs->dispatcher)) == 0) {
            return 0;
        }
        flags = MPR_CMD_IN | MPR_CMD_OUT | MPR_CMD_ERR | MPR_CMD_ASYNC;

        //  MOB - env m
        if ((rc = mprStartCmd(cmd->mc, ac, av, env, 0)) < 0) {
            if (rc == MPR_ERR_CANT_ACCESS) {
                err = mprAsprintf("Can't access command %s", cmd->argv[0]);
            } else if (MPR_ERR_CANT_OPEN) {
                err = mprAsprintf("Can't open standard I/O for command %s", cmd->argv[0]);
            } else if (rc == MPR_ERR_CANT_CREATE) {
                err = mprAsprintf("Can't create process for %s", cmd->argv[0]);
            } else {
                err = "";
            }
            //  MORE here see mprRunCmdV
            ejsThrowError(ejs, "Command failed: %s\n\nError %d, %s\n\nError Output: %s", cmd->command, rc, err);
mprDestroyCmd(cmd->mc);
            cmd->mc = 0;
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
    if (mprWaitForCmd(cmd->mc, MPR_TIMEOUT_STOP_TASK) < 0) {
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
static EjsObj *cmd_timeout(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
#if FUTURE
    //  MOB - who does anything with this timeout?  MprCmd does not have this feature
    return ejsCreateNumber(ejs, cmd->timeout);
#else
    return (EjsObj*) ejs->zeroValue;
#endif
}


/**
    function set timeout(msec: Number): Void
 */
static EjsObj *cmd_set_timeout(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
#if FUTURE
    cmd->timeout = ejsGetInt(ejs, argv[0]);
#endif
    return 0;
}


/**
    function wait(timeout: Number = -1): Boolean
 */
static EjsObj *cmd_wait(Ejs *ejs, EjsCmd *cmd, int argc, EjsObj **argv)
{
    int     timeout;

    timeout = argc > 0 ? ejsGetInt(ejs, argv[0]) : -1;

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

    mprMakeArgv(NULL, ejsToMulti(ejs, argv[0]), &argCount, &argVector);
    execv(argVector[0], argVector);
#endif
    ejsThrowStateError(ejs, "Can't exec %@", ejsToString(ejs, argv[0]));
    return 0;
}
#endif /* OLD && UNUSED */


/************************************ Factory *********************************/

void ejsConfigureCmdType(Ejs *ejs)
{
#if DEBUG_IDE
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
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, cmd_constructor);
    ejsBindMethod(ejs, type, ES_Cmd_kill, cmd_kill);

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
