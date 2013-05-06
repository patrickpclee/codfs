<?php

$backup_location = "physics@ncds1:/disk1/physics_backup";

require_once('uploaded.class.php');
require_once('KLogger.php');

$log = new KLogger ( "log.txt" , KLogger::DEBUG );

if (isset($_POST['submit'])) {
    if (!empty ($_POST['path'])) {
        $path = str_replace('/', '@', $_POST['path']);
    } else {
        $path = $string = str_replace(' ', '_', microtime());
    }

    $log->LogInfo("Upload: $path");

    $file = new Uploaded('upload_file');
    $fileid = $file->uploadToNCDS ($path);

    $log->LogInfo("Uploaded to NCDS Path: $path FileID: $fileid");

    if ($file->makeBackup($path, $backup_location)) {
        print "Replication Successful <br />";
        $log->LogInfo("Backup Successful: $path Location: $backup_location");
    } else {
        $log->LogError("Backup Error: $path Location: $backup_location");
    }

    if ($fileid) {
        $file->saveMappingToDb();
        $log->LogInfo("Mapping Saved: $path FileId: $fileid");
    } else {
        print "Upload Failed <br />";
    }
}
?>
