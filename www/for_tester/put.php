<?php
$path_parts = pathinfo($_SERVER['REQUEST_URI']);
$upload_dir = $_SERVER['DOCUMENT_ROOT'] . $path_parts['dirname'];
$upload_filename = $path_parts['basename'];
$upload_filepath = $_SERVER['DOCUMENT_ROOT'] . $_SERVER['REQUEST_URI'];

if ($_SERVER['REQUEST_METHOD'] !== 'PUT') {
    http_response_code(405);
    echo "method error";
    exit();
} else {
    if (is_dir($upload_dir) == false && is_file($upload_dir) == true) {
        http_response_code(400);
        echo "wrong uri";
        exit();
    } else if (is_dir($upload_dir) == false && is_file($upload_dir) == false) {
        mkdir($upload_dir);
    }
    http_response_code(201);
    $putdata = fopen("php://input", "r");
    $fp = fopen($upload_filepath, "w");
    while ($data = fread($putdata, 1024)) {
        fwrite($fp, $data);
    }
    fclose($fp);
    fclose($putdata);
}