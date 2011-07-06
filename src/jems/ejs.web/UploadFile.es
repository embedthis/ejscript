/**
    uploadedFile.es - Description of an uploaded file. Instances are created and stored in Request.files.
 */

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

