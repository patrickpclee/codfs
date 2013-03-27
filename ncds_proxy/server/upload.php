<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
   "http://www.w3.org/TR/html4/strict.dtd">
<HTML>
   <HEAD>
      <TITLE>NCDS File Uploader</TITLE>
   </HEAD>
   <BODY>
        <form enctype="multipart/form-data" method="post" action="do_upload.php">
            <input type="file" name="upload_file" /> <br />
            Target Path (time if not specified): <input type="text" name="path" /> <br />
            <button type="submit" name="submit">Submit</button>
        </form>
   </BODY>
</HTML>
