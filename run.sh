#!/usr/bin/env bash
#

function start() {
    mkdir /usr/wt; touch /usr/wt/socket
    chmod 777 -R /usr/wt;
    service lighttpd start
    tail -f /var/log/postgresql/postgresql-9.5-main.log
}

function make_tpc() {
    if [ ! -e /usr/local/bin/tpc ]
    then
	if [ -d /usr/textpresso/textpressocentral ]
	then
	    cd /usr/textpresso/textpressocentral
	    #rm -rf build; mkdir build; cd build
	    rm -rf cmake-build-debug; mkdir cmake-build-debug; cd cmake-build-debug
	    cmake -DCMAKE_BUILD_TYPE=Release ..
	    make -j 10 && make install && make clean
	    /usr/local/bin/tpctl.sh activate
	    /usr/local/bin/tpctl.sh set_literature_dir /data/textpresso
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
