tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if test ! $? = 0 ; then tempfile=/tmp/cmt.$$; fi
${CMTROOT}/mgr/cmt -quiet cleanup -sh -pack=OnboardFilter -version=v0 -path=/local/data/ocean/golpa/slacCO $* >${tempfile}; . ${tempfile}
/bin/rm -f ${tempfile}

