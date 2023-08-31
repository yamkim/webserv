#!/bin/bash

curr_directory=$(pwd)
conf_directory=$curr_directory/conf
root_directory=$curr_directory/www/html
php_bin_path=$curr_directory/php-bin/php-cgi

python_bin_path=/usr/bin/python3

escaped_root_path=${root_directory//\//\\/}
escaped_php_bin_path=${php_bin_path//\//\\/}
escaped_python_bin_path=${python_bin_path//\//\\/}

sed "s/\$ROOT_PATH/$escaped_root_path/g 
     s/\$PHP_BIN_PATH/$escaped_php_bin_path/g;
     s/\$PYTHON_BIN_PATH/$escaped_python_bin_path/g; " \
     "$conf_directory/default.conf.template" > "default.conf"
