#ifndef LOGINFODEF_H
#define LOGINFODEF_H


/* ---------------------------------------------------------------------- *//*!

   \file  LogInforDef.h
   \brief Structure of the onboard filter TDS
   \author R. Hughes

\verbatim
    CVS $Id
\endverbatim
                                                                          */
/* ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

 
  typedef struct loginfo{
      int tower;
      int layer;
      int column;

      int valN;
      int rangeN;
      int eN;
      int pedN;
      int gainN;
      int shiftN;

      int valP;
      int rangeP;
      int eP;
      int pedP;
      int gainP;
      int shiftP;
   } LogInfo;
   

#ifdef __cplusplus
}
#endif


#endif


