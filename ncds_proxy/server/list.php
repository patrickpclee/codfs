<?php

try{
    $db = new PDO('sqlite:filelist.sqlite3');
    $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $result = $db->query("SELECT PATH, FILEID FROM FILELIST;");
} catch(PDOException $e) {
    print 'Exception : '.$e->getMessage();
}

if (isset($_GET['python'])) {

    print str_pad("Path", 50)."File ID\n";
    foreach ($result as $row) {
        $path = str_replace('@', '/', $row['PATH']);
        $fileid = $row['FILEID'];
        print str_pad($path, 50)."$fileid\n";
    }

} else {

    print "Files for download:<br />\n";

    print "<table width='100%' border='1'>\n";
    print "<tr>\n";
    print "<td>Path</td>\n";
    print "<td>File ID</td>\n";
    print "</tr>\n";


    foreach ($result as $row) {
        $path = str_replace('@', '/', $row['PATH']);
        $fileid = $row['FILEID'];
        print "<tr>\n";
        print "<td>".$path."</td>\n";
        print "<td>".$fileid."</td>\n";
        print "<td><a href='do_download.php?fileid=$fileid'>Download</a></td>\n";
        print "</tr>\n";
    }

    print "</table>\n";
}

?>
