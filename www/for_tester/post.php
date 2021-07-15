<?php
# 여기서 이렇게 처리하거나 client_max_body_size를 상속 받도록 해야 할거 같습니다
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo "method error";
    exit();
} else {
    $putdata = fopen("php://input", "r");
    $length =  1000000;
    while ($data = fread($putdata, 1) && ($length >= 0)) {
        $length--;
    }
    if ($length >= 0) {
        http_response_code(201);
    } else {
        http_response_code(413);
    }
    fclose($putdata);
}