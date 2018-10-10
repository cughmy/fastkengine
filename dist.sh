#! /bin/sh

if [ $# -ge 1 ]; then
  base_path=$1
else
  base_path=$(pwd)
fi

uname=$(uname)
if [ "$uname" = 'Darwin' ]; then
  html_path=/usr/local/html
else
  html_path=/usr/html
fi

cp -R $base_path/src/nginx-module/template /etc/fken/
cp -R $base_path/conf/unix /etc/fken/

cp $base_path/src/nginx-module/template/index.html $html_path/
perl -pi -e 's/\$\{question\}//g' $html_path/index.html
perl -pi -e 's/\$\{answer\}//g' $html_path/index.html
perl -pi -e 's/\$\{display_answer\}/none/g' $html_path/index.html

