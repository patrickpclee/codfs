<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
   "http://www.w3.org/TR/html4/strict.dtd">
<HTML>
   <HEAD>
      <TITLE>NCDS File Uploader</TITLE>
        <script src="//ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>
        <script>
            $(document).ready(function() {
                $("#uploadForm").submit(function(){
                    var isFormValid = true;

                    $("#uploadForm input").each(function(){
                        if ($.trim($(this).val()).length == 0){
                            $(this).addClass("highlight");
                            isFormValid = false;
                        }
                        else{
                            $(this).removeClass("highlight");
                        }
                    });

                    if (!isFormValid) alert("Please fill in all the required fields");

                    var str = $("#path").val();
                    if(/^[a-zA-Z0-9-_.\/]*$/.test(str) == false) {
                        isFormValid = false;
                        alert('Path string contains illegal characters.');
                    }

                    return isFormValid;
                });
            });
        </script>
   </HEAD>
   <BODY>
        <form id="uploadForm" enctype="multipart/form-data" method="post" action="do_upload.php">
            <input id="file" type="file" name="upload_file" /> <br /><br />
            Target Path: <input id="path" type="text" name="path" /> <br /><br />
            <button type="submit" name="submit">Upload to NCDS</button>
        </form>
   </BODY>
</HTML>
