/**
    uploadedFile.es - Description of an uploaded file. Instances are created and stored in Request.files.
 */

# Config.WEB
module ejs.web {

    /** 
        Instances of UploadFile are created for each uploaded file.
        The UploadFile instances are stored in the request.files property.
        Users should not create instances of UploadFile manually.
        @spec ejs
        @stability evolving
     */
    class UploadFile {

        use default namespace public

        /** 
            Name of the uploaded file given by the client
         */
        var clientFilename: String

        /** 
            Mime type of the encoded data
         */
        var contentType: String

        /** 
            Name of the uploaded file. This is a temporary file in the upload directory.
         */
        var filename: Path

        /** 
            HTML input ID for the upload file element
         */
        var name: String
 
        /** 
            Size of the uploaded file in bytes
         */
        var size: Number
    }
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
