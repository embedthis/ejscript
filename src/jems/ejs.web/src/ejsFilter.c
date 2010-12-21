/*
    ejsFilter.c - Transfer chunk endociding filter.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "http.h"

#if 0
/********************************** Forwards **********************************/

static void incomingFilterData(HttpQueue *q, HttpPacket *packet);
static bool matchFilter(HttpConn *conn, HttpStage *handler);
static void openFilter(HttpQueue *q);
static void outgoingFilterService(HttpQueue *q);
static void setFilterPrefix(HttpQueue *q, HttpPacket *packet);

#define availableBytes(ap)  (((EjsByteArray*) ap)->writePosition - ((EjsByteArray*) ap)->readPosition)

typedef EjsFilter {
    EjsObj          obj;
    HttpQueue       q;
    EjsFunction     *put;
} struct EjsFilter;

/*********************************** Code *************************************/

static void openFilter(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;

    conn = q->conn;
    rec = conn->receiver;

    filter->put = ejsGetPropertyByName(ejs, filter, ejsName(&qname, "", "put"), filter, 1, &data);
    if (filter->put == 0 || ejs->exception) {
        return ejsThrow();
    }
}


static void incomingFilterData(HttpQueue *q, HttpPacket *packet)
{
}


static void outgoingFilterPut(HttpQueue *q)
{
    ejsRunFunction(ejs, ser, filter, data);
}


static void outgoingFilterService(HttpQueue *q)
{
    HttpConn        *conn;
    HttpPacket      *packet;
    HttpTransmitter *trans;

    conn = q->conn;
    trans = conn->transmitter;

    //  MOB -- want packets joined.

    filter = (EjsFilter*) q->pair->queueData;
    ejsRunFunction(ejs, ser, filter, data);

    if () {
    }

    if (trans->chunkSize <= 0 || trans->altBody) {
        httpDefaultOutgoingServiceStage(q);
    } else {
        for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
            if (!(packet->flags & HTTP_PACKET_HEADER)) {
                httpJoinPackets(q, trans->chunkSize);
                if (httpGetPacketLength(packet) > trans->chunkSize) {
                    httpResizePacket(q, packet, trans->chunkSize);
                }
            }
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                httpPutBackPacket(q, packet);
                return;
            }
            if (!(packet->flags & HTTP_PACKET_HEADER)) {
                setFilterPrefix(q, packet);
            }
            httpSendPacketToNext(q, packet);
        }
    }
}

/*************************************************** Methods ************************************************/

/*
    function write(data): Number
 */
static EjsObj *filter_write(Ejs *ejs, EjsObj *filter, int argc, EjsObj *argv)
{
    EjsObj      *data, *written;
    HttpQueue   *q;
    int         available, room;

    data = argv[0];
    mprAssert(ejsIsByteArray(data);
    q = filter->q;

    available = availableBytes(data);
    written = ejsRunFunction(ejs, filter->put, filter, 1, &data);
    if (ejs->exception) {
        return 0;
    }
    if (ejsGetInt(ejs, written) < available) {
        /*
           Could not write all available data. Assume the downstream queue is full. Disable this queue and 
           mark the downstream queue as full and service it immediately if not disabled.
        */
        httpDisbleQueue(q);
        next->flags |= HTTP_QUEUE_FULL;
        if (!(next->flags & HTTP_QUEUE_DISABLED)) {
            httpScheduleQueue(next);
        }
    }
}


static int createEjsHttpFilter(Ejs *ejs)
{
    HttpStage     *stage;

    mprAssert(ejs->http);

    stage = httpCreateFilter(ejs->http, "ejsFilter", HTTP_STAGE_ALL);
    if (stage == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    ejs->http->ejsFilter = stage;
    stage->open = openFilter; 
    stage->outgoingData = outgoingFilterData; 
    stage->outgoingService = outgoingFilterService; 
    stage->incomingData = incomingFilterData; 
    stage->incomingService = incomingFilterService; 
    return 0;
}


void ejsConfigureFilterType(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;
    EjsObj          *prototype;

    type = ejs->requestType = ejsConfigureNativeType(ejs, "ejs.web", "Filter", sizeof(EjsFilter));

    helpers = &type->helpers;
    helpers->mark = (EjsMarkHelper) markFilter;
    helpers->clone = (EjsCloneHelper) ejsCloneFilter;
    helpers->destroy = (EjsDestroyHelper) destroyFilter;
    helpers->getProperty = (EjsGetPropertyHelper) getFilterProperty;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) getFilterPropertyCount;
    helpers->getPropertyName = (EjsGetPropertyNameHelper) getFilterPropertyName;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupFilterProperty;
    helpers->setProperty = (EjsSetPropertyHelper) setFilterProperty;

    createEjsHttpFilter(ejs);

    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_write, (EjsProc) filter_write);
/*
    ejsBindAccess(ejs, prototype, ES_ejs_web_Filter_async, (EjsProc) filter_async, (EjsProc) filter_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_close, (EjsProc) filter_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_destroySession, (EjsProc) filter_destroySession);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_dontFinalize, (EjsProc) filter_dontFinalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_finalize, (EjsProc) filter_finalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_flush, (EjsProc) filter_flush);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_header, (EjsProc) filter_header);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_limits, (EjsProc) filter_limits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_off, (EjsProc) filter_off);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_read, (EjsProc) filter_read);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_sendFile, (EjsProc) filter_sendFile);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_setLimits, (EjsProc) filter_setLimits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_setHeader, (EjsProc) filter_setHeader);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_trace, (EjsProc) filter_trace);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Filter_write, (EjsProc) filter_write);
*/
}

#endif /* 0 */

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
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
    vim: sw=8 ts=8 expandtab

    @end
 */
