#!/usr/bin/env bash
#

function start() {
    a2enmod cgid
    service apache2 start
    tail -f /var/log/apache2/error.log
}

function make_tpc() {
    if [ ! -e /usr/local/bin/tpc ]
    then
	if [ -d /data/textpresso/textpressocentral ]
	then
	    cd /data/textpresso/textpressocentral
	    rm -rf build; mkdir build; cd build
	    cmake -DCMAKE_BUILD_TYPE=Release ..
	    make -j 10 && make install && make clean
	    /usr/local/bin/tpctl.sh activate
	    /usr/local/bin/tpctl.sh set_literature_dir /data/textpresso
	    cd /data/textpresso/tpctools
	    rm -rf build; mkdir build; cd build 
	    cmake -DCMAKE_BUILD_TYPE=Release ..;
	    make -j 10 && make install
	fi
    fi
}

function set_postgres() {
    service postgresql start
    sudo -u postgres createuser -s root
    createuser textpresso
    createuser "www-data"
    createdb "www-data"
    if [ -e /data/textpresso/postgres/www-data.tar.gz ]
    then
	gunzip /data/textpresso/postgres/www-data.tar.gz
	pg_restore -d "www-data" /data/textpresso/postgres/www-data.tar
	gzip /data/textpresso/postgres/www-data.tar
    fi
}

set_postgres
case "$1" in
    default)
	make_tpc
	start
       ;;
    dev)
	start
	;;
    stopped)
	tail -f /var/log/apache2/error.log
	;;
    *)
       echo "Usage: $0 {default|dev|stopped}"
esac
exit 0
