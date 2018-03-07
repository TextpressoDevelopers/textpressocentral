#
# Build:
#	cd /home/mueller/docker/images/textpressocentral-dev
#	docker build -t textpressocentral-dev .
#
FROM ubuntu-tpc
RUN apt-get update
RUN apt-get install -y apache2 libfcgi-dev libapache2-mod-fastcgi sudo
COPY run.sh /root/run.sh
EXPOSE 80
CMD ["default"]
ENTRYPOINT ["/root/run.sh"]
#
# To be done manually:
#    	- make sure there are no sym. links in tpcas or luceneindex that the container
#	  cannot reach. Do rsync luceneindex and tpcas directory with --copy-links option
#	  to a tmp directory if necessary. Make sure all postgres, tpcas and luceneindices
#	  on the host are owned by root
#       - set default literature permissions
#
#
# Run:
#    1) Put tpcas, luceneindex, postgres, textpressocentral and tpctools directory in a data
#       directory on the host file system. If that's not possible, map the five
#       directories separately.
#    2) Put a pg dump of www-data as www-data.tar.gz in the postgres directory.
#
#       docker run --name tpc-dev -d -p <host port>:80 \
#	-v <data directory on host>:/data textpressocentral-dev {default|dev|stopped}
#
#	or if you want to split up sub-directories of /data on different drives, map:
#
#       docker run --name tpc-dev -d -p <host port>:80 \
#	-v <luceneindex dir on host>:/data/luceneindex \
#       -v <tpcas dir on host>:/data/tpcas \
#       -v <postgres dir on host>:/data/postgres \
#	-v <textpressocentral git dir on host>:/data/textpressocentral
#	-v <tpctools git dir on host>:/data/tpctools textpressocentral-dev {default|dev|stopped}
#
