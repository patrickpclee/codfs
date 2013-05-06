<?php

if (empty($_GET['fileid'])){
    return false;
} else{
    $fileid = $_GET['fileid'];
}

try{
    $db = new PDO('sqlite:filelist.sqlite3');
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $db->exec("DELETE FROM FILELIST WHERE FILEID = $fileid;");
    print 'File deleted from DB <br />';
    $db = null;
} catch(PDOException $e) {
    print 'Exception : '.$e->getMessage();
}

?>
