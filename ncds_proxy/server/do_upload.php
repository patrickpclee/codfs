<?php

$backup_location = "physics@ncds1:/disk1/physics";

require_once('uploaded.class.php');
if (isset($_POST['submit'])) {
    if (!empty ($_POST['path'])) {
        $path = str_replace('/', '@', $_POST['path']);
    } else {
        $path = $string = str_replace(' ', '_', microtime());
    }
    $file = new Uploaded('upload_file');
    $fileid = $file->uploadToNCDS ($path);

    if ($file->makeBackup($path, $backup_location)) {
        print "Replication Successful <br />";
    }

    if ($fileid) {
        $file->saveMappingToDb();
    } else {
        print "Upload Failed <br />";
    }
}
?>
