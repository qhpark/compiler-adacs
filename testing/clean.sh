PATH=$PATH:/software/.admin/bins/bin:/usr/ccs/bin:/usr/bin:/usr/ucb:/usr/openwin/bin:/usr/dt/bin
WORKDIR=`pwd`
BUCKET=""
LOGFILE="$WORKDIR/log.txt"
rm -f $LOGFILE;

for DIR in `ls`; do
	if [ -d "$DIR" ] && [ "$DIR" != "CVS" ]; then
		BUCKET="$BUCKET $DIR";
	fi
done

for DIR in $BUCKET; do
	cd $WORKDIR;
	cd $DIR;
	make clean >> /dev/null;
done
