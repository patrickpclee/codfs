<?php

if (empty($_GET['fileid'])){
    return false;
} else{
    $fileid = $_GET['fileid'];
}

// log file
$client_no = rand();
$logfile = uniqid(rand(), true) . '.log';
$outfile = uniqid(rand(), true) . '.out';
system ("cd ../bin; ./CLIENT -i $client_no -a download -f $fileid -t $outfile > $logfile 2>&1");
send_file("../bin/$outfile", $fileid);
unlink("../bin/$outfile");
unlink("../bin/$logfile");

function send_file($file, $fileid) {
    if (file_exists($file)) {
        header('Content-Description: File Transfer');
        header('Content-Type: application/octet-stream');
        header("Content-Disposition: attachment; filename=$fileid");
        header('Content-Transfer-Encoding: binary');
        header('Expires: 0');
        header('Cache-Control: must-revalidate');
        header('Pragma: public');
        header('Content-Length: ' . filesize($file));
        ob_clean();
        flush();
        readfile($file);
    }
}

?>
