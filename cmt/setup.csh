# echo "Setting OnlineFilter v0 in /local/data/ocean/golpa/slacCO"

setenv CMTROOT /software/glast/tools/CMT/v1r12p20021129
source ${CMTROOT}/mgr/setup.csh

set tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if $status != 0 then
  set tempfile=/tmp/cmt.$$
endif
${CMTROOT}/mgr/cmt -quiet setup -csh -pack=OnlineFilter -version=v0 -path=/local/data/ocean/golpa/slacCO $* >${tempfile}; source ${tempfile}
/bin/rm -f ${tempfile}

