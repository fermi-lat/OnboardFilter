# echo "Setting OnboardFilter v0 in /local/data/ocean/golpa/slacCO"

CMTROOT=/software/glast/tools/CMT/v1r12p20021129; export CMTROOT
. ${CMTROOT}/mgr/setup.sh

tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if test ! $? = 0 ; then tempfile=/tmp/cmt.$$; fi
${CMTROOT}/mgr/cmt -quiet setup -sh -pack=OnboardFilter -version=v0 -path=/local/data/ocean/golpa/slacCO $* >${tempfile}; . ${tempfile}
/bin/rm -f ${tempfile}

