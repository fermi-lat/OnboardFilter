set tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if $status != 0 then
  set tempfile=/tmp/cmt.$$
endif
${CMTROOT}/mgr/cmt -quiet cleanup -csh -pack=OnboardFilter -version=v0 -path=/local/data/ocean/golpa/slacCO $* >${tempfile}; source ${tempfile}
/bin/rm -f ${tempfile}

