#!/usr/bin/env bash

##### control textpressocentral installation #####

TPC_HOME=/usr/local/textpresso
CGI_BIN=/usr/lib/cgi-bin
WT_HOME=/usr/local

function uninstall() {
    rm -rf $TPC_HOME
    rm -rf $CGI_BIN/tc
}

function activate() {
    if [[ -e /etc/wt/wt_config.xml ]]
    then
        mv /etc/wt/wt_config.xml /etc/wt/orig_wt_config.xml_orig
    fi
    mkdir -p $CGI_BIN/tc
    if [[ $1 == "debug" ]]
    then
        cp $TPC_HOME/conf/wt_config_debug.xml /etc/wt/
    else
        cp $TPC_HOME/conf/wt_config.xml /etc/wt/
        cp $TPC_HOME/conf/fastcgi.conf /etc/apache2/mods-available
        ln -s /usr/local/bin/tpc $CGI_BIN/tc/
    fi
    find $TPC_HOME/resources/resources-tpc/ -maxdepth 1 -mindepth 1 | xargs -I {} ln -s {} $TPC_HOME/resources/resources-web
    find $WT_HOME/share/Wt/resources/ -maxdepth 1 -mindepth 1 | xargs -I {} ln -s {} $TPC_HOME/resources/resources-web/
    ln -s $TPC_HOME/resources/resources-web $CGI_BIN/tc/resources
    chmod -R 775 $TPC_HOME
    chmod -R 777 $TPC_HOME/useruploads
    mkdir -p $TPC_HOME/downloads
    chmod -R 777 $TPC_HOME/downloads
    ln -s $TPC_HOME/downloads $CGI_BIN/tc/
}

function set_literature_dir() {
    literature_dir=$1
    rmdir /usr/local/textpresso/luceneindex
    ln -s $literature_dir/luceneindex $TPC_HOME/luceneindex
    rmdir $TPC_HOME/tpcas
    ln -s $literature_dir/tpcas-2 $TPC_HOME/tpcas
    mkdir -p $CGI_BIN/tc
    ln -s $TPC_HOME/tpcas $CGI_BIN/tc/images
}

case "$1" in
    uninstall)
       uninstall
       ;;
    set_literature_dir)
       set_literature_dir $2
       ;;
    activate)
       activate $2
       ;;
    *)
       echo "Usage: $0 {uninstall|set_literature_dir <dir_location>|activate [debug]}"
esac

exit 0
