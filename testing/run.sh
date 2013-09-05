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
	echo "Starting bucket: $DIR" &> $LOGFILE
	make clean &> $LOGFILE;
	make init &> $LOGFILE;
	make run &> $LOGFILE;
	make verify &> $LOGFILE;
	RESULT=$?;
	if [ "$RESULT" -eq "0" ]; then
		echo "$DIR: PASSED";
	else
		echo "$DIR: FAILED";
	fi;
done
