#ifndef EFC_H
#define EFC_H


/* ---------------------------------------------------------------------- *//*!

  \file   EFC.h
  \brief  Provides a wrapper in which to run a filter as an EDS callback
          routine.
  \author JJRussell - russell@slac.stanford.edu
   
\verbatim

   CVS $Id: EFC.h,v 1.6 2007/10/10 23:40:42 russell Exp $
\endverbatim


  \par
                                                                          */
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *\
 * 
 * HISTORY
 * -------
 *
 * DATE     WHO WHAT
 * -------- --- ---------------------------------------------------------
 * 10.02.07 jjr Renamed EFC_definitionC -> EFC_definition along with there
 *              corresponding constructor. Eliminated all references to
 *              original definitions.
 * 06.22.07 jjr Added EFC_samplerConditionsDefine
 * 03.07.06 jjr Added an interface to fetch and set values in the EFC
 *              control structure without exposing the actual structure.
 * 03.14.05 jjr Changed name to EFC and clearly identified its function
 *              as a wrapper for an EDS_fw callback routine.
 * 02.15.05 jjr Created
 *
\* ---------------------------------------------------------------------- */



#include "EFC/EFC_fate.h"
#include "EFC/EFA_def.h"
#include "EDS/EDS_fw.h"
#include "EDS/EBF_siv.h"
#include "EDS_DB/EDS_DB_schema.h"



/* ====================================================================== */
/* Forward References                                                     */
/* ---------------------------------------------------------------------- */
#ifndef CMX_DOXYGEN


#ifndef         EBF_PKT_TD
#define         EBF_PKT_TD
typedef struct _EBF_pkt                EBF_pkt;
#endif

#ifndef         EDS_FWIXB_TD
#define         EDS_FWIXB_TD
typedef struct _EDS_fwIxb              EDS_fwIxb;
#endif


#ifndef         EFC_TD
#define         EFC_TD
typedef struct _EFC                    EFC;
#endif

#ifndef         EFC_SAMPLER_TD
#define         EFC_SAMPLER_TD
typedef struct _EFC_sampler            EFC_sampler;
#endif

#ifndef         EFC_DB_SCHEMA_TD
#define         EFC_DB_SCHEMA_TD
typedef struct _EFC_DB_Schema          EFC_DB_Schema;
#endif

#ifndef         EFC_DB_SCHEMAFILTER_TD
#define         EFC_DB_SCHEMAFILTER_TD
typedef struct _EFC_DB_SchemaFilter    EFC_DB_SchemaFilter;
#endif

#ifndef         EFC_DB_SAMPLER_TD
#define         EFC_DB_SAMPLER_TD
typedef struct _EFC_DB_sampler         EFC_DB_sampler;
#endif

#ifndef         EDS_FW_TD
#define         EDS_FW_TD
typedef struct _EDS_fw                 EDS_fw;
#endif

#ifndef         EDS_CFGINFO_TD
#define         EDS_CFGINFO_TD
typedef struct _EDS_cfgInfo    EDS_cfgInfo;
#endif

#endif
/* ====================================================================== */



/* ====================================================================== */
/* Typedefs                                                               */
/* ---------------------------------------------------------------------- *//*!

  \typedef void *(*EFC_servicesConstruct)(void              *ctl,
					  const void *parameters)

  \brief   Constructs the configuration parameters block
  \return  Status

  \param  ctl       The target filter's control block
  \param parameters The defining parameters
									  */
/* ---------------------------------------------------------------------- */
typedef void *(*EFC_servicesConstruct)(void              *ctl,
				       const void *parameters);
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \typedef EFC_fate (*EFC_servicesFilter)(void             *ctl,
                                          int          pktBytes,
					  EBF_pkt          *pkt,
					  EBF_siv           siv,
					  EDS_fwIxb        *ixb,
					  unsigned int   vetoes,
					  void          *result)
  \brief EFC filter routine function signature
  \return The filtering fate
 
  \param  ctl       The target filter's control block
  \param  pktBytes  Number of bytes in the EBF packet
  \param  pkt       The EBF packet
  \param  siv       The EBF state information vector
  \param  EDS_fwIxb The framework's information exchange block
  \param  vetoes    The list of active vetoes
  \param  result    The result vector to fill

									  */
/* ---------------------------------------------------------------------- */
typedef EFC_fate (*EFC_servicesFilter)(void             *ctl,
				       int          pktBytes,
				       EBF_pkt          *pkt,
				       EBF_siv           siv,
				       EDS_fwIxb        *ixb,
				       unsigned int   vetoes,
				       void          *result);
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \typedef void *(*EFC_servicesCfgSet )(void        *ctl,
                                        void *parameters)

  \brief   Constructs the configuration parameters block
  \return  Status

  \param ctl        The target filter's control block
  \param parameters The defining parameters

									  */
/* ---------------------------------------------------------------------- */
typedef void *(*EFC_servicesCfgSet)(void                 *cfg_ctl,
				    const void        *parameters);
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \typedef   void (*EFC_servicesDestruct)(void *ctl)
  \brief     Destroys this filter's context

  \param ctl The target filter's control block
									  */
/* ---------------------------------------------------------------------- */
typedef void (*EFC_servicesDestruct)(void  *ctl);
/* ---------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *//*!

  \typedef unsigned int (*EFC_servicesReport)(void              *prm, 
					      const void        *cfg,
					      const EFC_sampler *sampler,
					      int                list);
  \brief   Configuration reporting callback routine
  \return  The bit list of the remaining ones to print, 0 terminates it

  \param     prm User context parameter
  \param     cfg The configuration to report on
  \param sampler The sampler structure that is paired with this configuration
  \param    list The bit list of what is remaining, by implication, this
                 is 0 when done
									  */
/* ---------------------------------------------------------------------- */
typedef unsigned int (*EFC_servicesReport)(void              *prm, 
					   const void        *cfg,
					   const EFC_sampler *sampler,
					   int                which);
/* ====================================================================== */



  

  


/* --------------------------------------------------------------------- *//*!

  \struct _EFC_services
  \brief   The defining services of a standard filter
									  *//*!
  \typedef EFC_services
  \brief   Typedef for struct _EFC_services
									  */
/* ---------------------------------------------------------------------- */
struct _EFC_services
{
  EFC_servicesConstruct construct; /*!< The filter's configuration 
			                constructor                       */
  EFC_servicesFilter       filter; /*!< The filter routine                */
  EFC_servicesCfgSet      cfg_set; /*!< Set a new configuration           */
  EFC_servicesDestruct   destruct; /*!< The destruct routine              */
  EFC_servicesReport       report; /*!< Optional Configuration report     */
};
/* ---------------------------------------------------------------------- */
#ifndef         EFC_SERVICES_TD
#define         EFC_SERVICES_TD
typedef struct _EFC_services     EFC_services;
#endif
/* ---------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \struct _EFC_sizes
  \brief   The defining sizes the filters control block, configuration 
           block and result vector
									  *//*!
  \typedef EFC_sizes
  \brief   Typedef for struct _EFC_sizes
									  */
/* ---------------------------------------------------------------------- */
struct _EFC_sizes
{
  int    ctl; /*!< Size, in bytes, of the filter's control block          */
  int    cfg; /*!< Size, in bytes, of one filter's parameter block        */
  int result; /*!< Size, in bytes, of the filter's result vector          */
};
/* ---------------------------------------------------------------------- */
#ifndef         EFC_SIZES_TD
#define         EFC_SIZES_TD
typedef struct _EFC_sizes    EFC_sizes;
#endif
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//*!

  \struct _EFC_classes
  \brief   Breaks the status word into classes of bits
                                                                          *//*!
  \typedef EFC_classes
  \brief   Typedef for struct _EFC_classes
                                                                          */
/* ---------------------------------------------------------------------- */
struct _EFC_classes
{
    unsigned int    all;  /*!< The OR of veto and pass set                */
    unsigned int vetoes;  /*!< The set of vetoes                          */
    unsigned int passes;  /*!< The set of passes                          */
    unsigned int   info;  /*!< The set of informational bits              */
};
/* ---------------------------------------------------------------------- */
#ifndef         EFC_CLASSES_TD
#define         EFC_CLASSES_TD
typedef struct _EFC_classes    EFC_classes;
#endif
/* ---------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \struct _EFC_definition
  \brief   The defining values of a standard filter
									  *//*!
  \typedef EFC_definition
  \brief   Typedef for struct _EFC_defintion

   The original definition of an EFC_definition did not contain that 
   information. In order to stay backwards compatible, an augmented
   version, EFC_definitionC, along with a matching constructor, 
   EFC_constructC was added. With EFC_2.0.0, the qualifying C was dropped
   and all references to the original version were eliminated.
									  */
/* ---------------------------------------------------------------------- */
struct _EFC_definition
{
    EFC_services services; /*!< The standard set of services              */
    EFC_sizes       sizes; /*!< The sizes of the various blocks           */
    EFC_classes   classes; /*!< The status classes                        */
};
/* ---------------------------------------------------------------------- */
#ifndef         EFC_DEFINITION_TD
#define         EFC_DEFINITION_TD
typedef struct _EFC_definition    EFC_definition;
#endif
/* ---------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *//*!

  \enum   _EFC_OBJECT_K
  \brief   Enumeration of the objects
                                                                          *//*!
  \typedef EFC_OBJECT_K
  \brief   Typedef for enum _EFC_OBJECT_K

   This enumerates the objects that can be accessed and set via the 
   EFC_objectGet and EFC_objectSet routines.

   These routines allow access to the private EFC structure and is meant
   to be used mainly in a debugging environment. However, there may be
   uses for this outside of that arena, so it was decided to exposed this
   as a general interface. With that said, one should realize that this
   facility should be used with caution.

   The one very legitimate use is to get a pointer to the filter specific
   parameter. This would be called immediately after EFC_construct 
   completes. Fetching this value will all the user to complete 
   initialization of the structure. This was omitted from the first 
   implementation by oversight and by the fact that the early filters did
   not need it.
                                                                          */
/* ---------------------------------------------------------------------- */
typedef enum _EFC_OBJECT_K
{
    EFC_OBJECT_K_MODE        = 0, /*!< The configuration mode             */
    EFC_OBJECT_K_FILTER_RTN  = 1, /*!< The filter routine                 */
    EFC_OBJECT_K_FILTER_PRM  = 2, /*!< The filter parameter               */
    EFC_OBJECT_K_CFGSET_RTN  = 3, /*!< The configuration set routine      */
    EFC_OBJECT_K_REPORT_RTN  = 4, /*!< The report routine                 */
    EFC_OBJECT_K_SAMPLER     = 5, /*!< The sampling parameters            */
    EFC_OBJECT_K_EFA         = 6, /*!< The event analysis structure       */
    EFC_OBJECT_K_ID          = 7, /*!< The id of this event filter/handler*/
}
EFC_OBJECT_K;
/* ---------------------------------------------------------------------- */



#ifdef __cplusplus
extern "C" {
#endif


/* ====================================================================== */
/* Prototypes                                                             */
/* ---------------------------------------------------------------------- */

    extern int   EFC_sizeof         (const EFC_DB_SchemaFilter *schema,
                                     void                         *prm,
                                     const EFC_sizes            *sizes);

    extern const EDS_fwHandlerServicesX1
               *EFC_construct       (EFC                          *efc,
                                     unsigned int           handler_id,
				     const EFC_DB_SchemaFilter *schema,
                                     unsigned int           schema_key,
				     void                         *prm,
				     EDS_fw                     *edsFw,
				     const EFC_definition  *definition,
				     unsigned int              objects,
				     unsigned int                needs);

    extern unsigned int 
                EFC_objectsAdd      (EDS_fw                        *fw,
				     unsigned int              objects);

    extern unsigned int 
                EFC_objectsGet      (const EFC                    *efc);

    extern unsigned int 
                EFC_needsGet        (const EFC                    *efc);

    extern  void 
               *EFC_get             (const EFC                    *efc, 
                                     EFC_OBJECT_K               object);

    extern int  EFC_set             (EFC                          *efc,
                                     EFC_OBJECT_K object, void  *value);

   extern int   EFC_filter          (EFC                          *efc,
				     unsigned int             pktBytes,
				     EBF_pkt                      *pkt,
				     EBF_siv                       siv,
				     EDS_fwIxb                    *ixb,
                                     int                            id);

   extern const EFC_DB_Schema
               *EFC_lookup          (unsigned short int      schema_id,
				     unsigned short int    instance_id);


   extern int   EFC_modeAssociate   (EFC                          *efc,
				     int                          mode,
				     int                 configuration);

   extern int   EFC_modeSelect      (EFC                          *efc,
				     int                          mode,
                                     EDS_cfgInfo                 *info);

   extern int   EFC_start           (EFC                          *efc,
                                     unsigned int               reason,
                                     unsigned int               run_id,
                                     unsigned int           start_time,
                                     unsigned int                 mode);

   extern void  EFC_report          (EFC                          *efc,
				     unsigned int                 list);

   extern int   EFC_teardown        (EFC                          *efc);

/* ====================================================================== */

#ifdef __cplusplus
}
#endif

#endif

