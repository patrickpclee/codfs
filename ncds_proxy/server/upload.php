<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
   "http://www.w3.org/TR/html4/strict.dtd">
<HTML>
   <HEAD>
        <link rel="stylesheet" href="plupload/js/jquery.plupload.queue/css/jquery.plupload.queue.css" type="text/css" media="screen" />
      <TITLE>NCDS File Uploader</TITLE>
        <script src="//ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>
        <script type="text/javascript" src="plupload/js/plupload.js"></script>
        <script type="text/javascript" src="plupload/js/plupload.html4.js"></script>
        <script type="text/javascript" src="plupload/js/plupload.html5.js"></script>
        <script type="text/javascript" src="plupload/js/jquery.plupload.queue/jquery.plupload.queue.js"></script>
        <script>
            $(document).ready(function() {

                $("#pathForm").submit(function(){
                    var isFormValid = true;

                    var str = $.trim($("#path").val());
                    if (str.length == 0) {
                        isFormValid = false;
                    }
                    if(/^[a-zA-Z0-9-_.\/]*$/.test(str) == false) {
                        isFormValid = false;
                    }

                    if (!isFormValid) {
                        alert ("Invalid path");
                        return false;
                    }

                    $("#pathForm :input").attr('disabled', true);

                    $("#uploader").pluploadQueue({
                        // General settings
                        runtimes : 'html5,html4',
                            url : 'do_upload.php',
                            multipart_params: {'path': str}
                    });

                    return false;
                });
            });
        </script>
   </HEAD>
   <BODY>

        <form id="pathForm" action="">
            Destination Folder on NCDS: <input id="path" type="text" name="path" /> 
            <button type="submit" name="submit">Set Folder Path</button>
        </form>
        
        <br /><br />

        <div id="uploader"></div>
   </BODY>
</HTML>
