#ifndef   CDM_PUBDEFS_H
#define   CDM_PUBDEFS_H

/*------------------------------------------------------------------------*//*!
\file  CDM_pubdefs.h
\brief Public include file for CDM package.

\verbatim
CVS $Id: CDM_pubdefs.h,v 1.1 2008/05/16 18:23:56 usher Exp $
\endverbatim
*//*-------------------------------------------------------------------------*/

#include "CAB/CAB_asBuiltPub.h"

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions ==========================================================*/

/*! \def CDM_K_NONE       \brief No such objects                             */
#define  CDM_K_NONE       (0xffff)

/* === Forward references ================================================== */

typedef struct _CDM_Database CDM_Database;

/* === Callback prototypes ==================================================*/

/*------------------------------------------------------------------------*//*!
\typedef CDM_cb_ScanDatabase
\brief   Signature for an CDM database scanning callback routine
*//*-------------------------------------------------------------------------*/
typedef unsigned int
    CDM_cb_ScanDatabase               /* Database scanner callback routine   */
    (
        CDM_Database             *cdb,/* CDM database instance               */
        void                     *prm /* User callback parameter             */
    );

/* === Function prototypes ==================================================*/

extern unsigned int
    CDM_countInstance                 /* Count instances of database schema  */
    (
        unsigned short         schema /* Database schema ID                  */
    );

extern CMX_asBuiltIdentity *
    CDM_findCAB                       /* Find CMX as built block pointer     */
    (
        CDM_Database             *cdb /* Database handle                     */
    );

extern CDM_Database *
    CDM_findDatabase                  /* Find database by schema/instance    */
    (
        unsigned short         schema,/* Database schema ID                  */
        unsigned short       instance /* Database instance ID                */
    );

extern const void *
    CDM_getData                       /* Get database data pointer           */
    (
        CDM_Database             *cdb /* Database handle                     */
    );

extern short
    CDM_getInstance                   /* Get database instance numbers       */
    (
        CDM_Database             *cdb /* Database handle                     */
    );

extern unsigned int
    CDM_getKey                        /* Get database file key               */
    (
        CDM_Database             *cdb /* Database handle                     */
    );

extern short
    CDM_getSchema                     /* Get database schema identity        */
    (
        CDM_Database             *cdb /* Database handle                     */
    );

extern short
    CDM_getVersion                    /* Get database version                */
    (
        CDM_Database             *cdb /* Database handle                     */
    );

extern void
    CDM_insertDatabase                /* Insert database into database list  */
    (
        CDM_Database             *newCDM /* Database handle                     */
    );

extern CDM_Database *
    CDM_loadDatabase                  /* Load a database                     */
    (
        const char               *fil,/* Filename of database to load        */
        unsigned int              opt /* Options (none currently defined)    */
    );

extern void
    CDM_removeDatabase                /* Remove database from database list  */
    (
        CDM_Database             *old /* Database handle                     */
    );

extern unsigned int
    CDM_scanDatabase                  /* Scan the database instance list     */
    (
        CDM_cb_ScanDatabase      *rtn,/* User callback routine               */
        void                     *prm /* User callback routine parameter     */
    );

extern unsigned int
    CDM_unloadDatabase                /* Unload a database                   */
    (
        CDM_Database             *cdb /* Database handle                     */
    );

#ifdef __cplusplus
}
#endif

#endif    /* CDM_PUBDEFS_H */
