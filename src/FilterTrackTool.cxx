/**  @file FilterTrackTool.cxx
    @brief implementation of class FilterTrackTool
    
  $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/src/FilterTrackTool.cxx,v 1.10 2011/08/16 01:27:57 jrb Exp $  
*/

#include "IFilterTool.h"

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/GaudiException.h" 
#include "GaudiKernel/IDataProviderSvc.h"

#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "OnboardFilterTds/ObfFilterTrack.h"

#include "facilities/Util.h"

// Interface to EDS package here
#include "ObfInterface.h"

// FSW includes go here
#include "EFC/EFC_edsFw.h"
#include "EDS/EBF_dir.h"
#include "EDS/EBF_cid.h"
#include "EDS/EBF_tkr.h"
#include "EDS/EDR_cal.h"
#include "EDS/EDR_tkrUnpack.h"
#include "EDS/FFS.h"
#include "GFC_DB/GAMMA_DB_instance.h"
#include "EFC_DB/EFC_DB_sampler.h"
#include "EFC/../src/GFC_def.h"
#include "EFC/../src/TFC_geometryDef.h"
#include "src/GEO_DB_data.h"

// obf Track definitions
#include "FSWHeaders/TFC_prjDef.h"
#include "EFC/EFC_edsFw.h"

#include "FSWHeaders/EFC.h"

// HepPoint3D
#include "CLHEP/Geometry/Point3D.h"
typedef HepGeom::Point3D<double> HepPoint3D;

// Useful stuff! 
#include <map>
#include <stdexcept>
#include <sstream>
#include <stdexcept>

/** @class FilterTrackTool
    @brief Manages the Gamma Filter
    @author Tracy Usher
*/
class FilterTrackTool : public AlgTool, virtual public IFilterTool
{
public:

    // Standard Gaudi Tool constructor
    FilterTrackTool(const std::string& type, const std::string& name, const IInterface* parent);

    // After building it, destroy it
    ~FilterTrackTool();

    /// @brief Intialization of the tool
    StatusCode initialize();

    /// @brief Finalize method for the tool
    StatusCode finalize();

    // Set Mode for a given filter
    void setMode(unsigned int mode) {return;}

    // This defines the method called for end of event processing
    virtual void eoeProcessing(EDS_fwIxb* ixb);

    // This for end of run processing
    virtual void eorProcessing();

    // Dump out the running configuration
    void dumpConfiguration();

private:

    // Local class def here
    // Define an internal class for temporarily handling results
    class GRBP_prjs
    {
    public:
        unsigned char  cnt;
        unsigned char  n[2];
        unsigned char  rsvd;
        const TFC_prj* prjs[2];
    };

    // Local functions here
    // This classifies the projections for the next step
    // Taken from GRBP_server.c in the GRBP package
    unsigned int projections_classify(TFC_prjs  *projections); //,
                                //      EBF_dir           *dir,
                                //      EDR_tkr           *tkr);

    // Direction cosine for a given projection
    // Taken from GRBP_server.c in the GRBP package
    int          dcos_prepare(const TFC_prj *prjs[2],
                              int                cnt,
                              unsigned int     scale);

    // This finds the "best" projection
    // Taken from GRBP_prjsSelect in the GRBP package
    int          prjsSelect(GRBP_prjs*     grbp_prjs,
                            unsigned int      lyrMsk,
                            const TFC_prjList *lists);

    // This initializes the projection lists...
    void         prjList_init (TFC_prjList lists[2][16]);

    // Find out the position of a strip 
    HepPoint3D findStripPosition(int tower, int layer, int view, int stripHit);

    //****** This section for defining JO parameters
    // This is somewhat useless but if set will be passed to the CDM utility to print info
    //BooleanProperty   m_towerHits;

    // Local geometry variables
    unsigned int      m_strip_pitch;  /*!< Tracker strip pitch, in mm            */
    unsigned int      m_dxy_scale;    /*!< XY scale factor                       */
    unsigned int      m_dz_scale;     /*!< Z  scale factor                       */

    //****** This section contains various useful member variables
    /// Pointer to the Gaudi data provider service
    IDataProviderSvc* m_dataSvc;

    // Geometry database from FSW
    const TFC_geometryTkr* m_tkrGeo;
};

//static ToolFactory<FilterTrackTool> s_factory;
//const IToolFactory& FilterTrackToolFactory = s_factory;
DECLARE_TOOL_FACTORY(FilterTrackTool);
//------------------------------------------------------------------------

FilterTrackTool::FilterTrackTool(const std::string& type, 
                                 const std::string& name, 
                                 const IInterface* parent) :
                                 AlgTool(type, name, parent)
{
    //Declare the additional interface
    declareInterface<IFilterTool>(this);

    // declare properties with setProperties calls
    //declareProperty("FillTowerHits",   m_towerHits = true);

    m_strip_pitch = 228; // From TKR_STRIP_PITCH = TKR_STRIP_PITCH_MM * 1000 + 0.5
    m_dz_scale    = 2 * 2048;
    m_dxy_scale   = m_strip_pitch * 2 * 2048;

    return;
}
//------------------------------------------------------------------------
FilterTrackTool::~FilterTrackTool()
{
}

StatusCode FilterTrackTool::initialize()
{
    StatusCode sc   = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());

    // Set the properties
    setProperties();

    //Locate and store a pointer to the data service
    if( (sc = service("EventDataSvc", m_dataSvc)).isFailure() ) 
    {
        log << MSG::ERROR << "Service [EventDataSvc] not found" << endreq;
        return sc;
    }

    try
    {
        // Get ObfInterface pointer
        ObfInterface* obf = ObfInterface::instance();

        // Set up data members
        GFC* cfgParms = reinterpret_cast<GFC*>(obf->getFilterPrm(GAMMA_DB_SCHEMA,EFC_OBJECT_K_FILTER_PRM));

        // If there is no parameter block then there must be an error
        if (!cfgParms)
        {
            throw std::runtime_error("Filter Schema GAMMA_DB_SCHEMA not registered, can not use FilterTrackTool");
        }

        const GammaCfgTkr&     tkrCfg = cfgParms->cfg->prms.tkr;
        const TFC_geometry*    geom   = tkrCfg.geometry;

        m_tkrGeo = &geom->tkr;

        // Register this as an output routine
        obf->setEovOutputCallBack(this);
    }
    catch(ObfInterface::ObfException& obfException)
    {
        log << MSG::ERROR << obfException.m_what << endreq;
        return StatusCode::FAILURE;
    }

    return sc;
}

StatusCode FilterTrackTool::finalize ()
{
    StatusCode  status = StatusCode::SUCCESS;
    
    return status;
}

// This routine for dumping to log file the configuration being run
void FilterTrackTool::dumpConfiguration()
{
    MsgStream log(msgSvc(), name());

    return;
}

// This defines the method called for end of event processing
void FilterTrackTool::eoeProcessing(EDS_fwIxb* ixb)
{
    // Create output class
    OnboardFilterTds::ObfFilterTrack*  filterTrack = new OnboardFilterTds::ObfFilterTrack();
    m_dataSvc->registerObject("/Event/Filter/ObfFilterTrack", filterTrack);

    // Get the projections 
    TFC_prjs *projections = (TFC_prjs *)ixb->blk.ptrs[EFC_EDS_FW_OBJ_K_TFC_PRJS];

    // use the trackProj class to do the real work here... but only if data...
    if (projections->curCnt > 0) 
    {
        //
        // The following code was modelled on that in the function grb_process which
        // can be found in the GRBP_server.c module in the GRBP package. 
        //
        unsigned int   topLayerMask = projections_classify (projections); //, dir, tkr);

        if ( (topLayerMask & 0xffff0000) & (topLayerMask << 16) )
        {
            GRBP_prjs grbp_prjs[2];

            /*
            | Need to compute
            |    dx/dz
            |    dy/dz
            */
        
            /* Find the X best projections */
            prjsSelect (&grbp_prjs[0], 
                        topLayerMask & 0xffff0000, 
                        projections->top[0]);        

            int nx  = grbp_prjs[0].cnt;
            int dxi = dcos_prepare (grbp_prjs[0].prjs, nx, m_dxy_scale);

            // In addition, find the X intercept
            const TFC_prj* xPrj     = grbp_prjs[0].prjs[0];
            unsigned int   xLyrMask = xPrj->layers;
            int            xLayer   = xPrj->max;            // Highest hit
            const TFC_hit& xHit     = xPrj->hits[xLayer];
            int            nxHits   = xPrj->nhits;
                
            HepPoint3D point = findStripPosition(xHit.tower, xLayer, 0, xHit.strip);

            float xInt = point.x();
            float zx   = point.z();

            /* Find the Y best projections */
            prjsSelect (&grbp_prjs[1],
                        topLayerMask <<  16, 
                        projections->top[1]);
            int ny  = grbp_prjs[1].cnt;
            int dyi = dcos_prepare (grbp_prjs[1].prjs, ny, m_dxy_scale);
            int dzi = m_dz_scale;

            // In addition, find the Y intercept
            const TFC_prj* yPrj     = grbp_prjs[1].prjs[0];
            unsigned int   yLyrMask = yPrj->layers;
            int            yLayer   = yPrj->max;            // Highest hit
            const TFC_hit& yHit     = yPrj->hits[xLayer];
            int            nyHits   = yPrj->nhits;
                
            point = findStripPosition(yHit.tower, yLayer, 1, yHit.strip);

            float yInt     = point.y();
            float zy       = point.z();

            float trkSlpXZ = static_cast<double>(dxi) / static_cast<double>(dzi);
            float trkSlpYZ = static_cast<double>(dyi) / static_cast<double>(dzi);


            // Ok, initialize the TDS class
            filterTrack->initialize(nxHits, nyHits, xInt, yInt, zx, trkSlpXZ, trkSlpYZ);
        }
    }

    return;
}

// This for end of run processing
void FilterTrackTool::eorProcessing()
{
    MsgStream log(msgSvc(), name());

    return;
}

unsigned int FilterTrackTool::projections_classify(TFC_prjs  *projections)
{
    int                          idx = 0;
    TFC_prjDir                *pdirs = projections->dir;
    TFC_prj                     *prj = projections->prjs;
    TFC_prj                  *prjmax = prj + sizeof (projections->prjs) / sizeof (projections->prjs[0]);
    unsigned int         topLayerMsk = 0;
    unsigned int                tmsk = 0;

    prjList_init (projections->top);

    tmsk = projections->twrMsk << 16;
    while (tmsk)
    {
        int                   tower;
        TFC_prjDir            *pdir;
        int                     beg;
        int                     end;

        tower = FFS (tmsk);
        tmsk  = FFS_eliminate (tmsk, tower);


        /* Get the projection directory for this tower */
        pdir  = pdirs + tower;


        /* Compute beginning and end X projection */
        beg  = pdir->idx;
        end  = beg + pdir->xCnt;


        /* Accumulate X list */
        for (idx = beg; idx < end; idx++)
        {
            int top_layer = 17 - prj[idx].top.layer;
            TFC__prjListInsert (&projections->top[0][top_layer], 
                                &prj[idx].topNode);
            topLayerMsk |= FFS_mask (top_layer); 
        }


        /* Y projections start where the X left off */
        end += pdir->yCnt;
        for (; idx < end; idx++)
        {
            int top_layer = 17 - prj[idx].top.layer;
            TFC__prjListInsert (&projections->top[1][top_layer],
                                &prj[idx].topNode);
            topLayerMsk |= FFS_mask (top_layer) >> 16;
        }

    }


    /* Find the top layer and only consider tracks within two */
    {
        unsigned int tmp;
        int        layer;
        tmp   = topLayerMsk | (topLayerMsk << 16);
        layer = FFS      (tmp);
        layer = FFS_mask (layer);
        tmp   = (layer) | (layer >> 1) | (layer >> 2);
        topLayerMsk &= (tmp | tmp >> 16);
    }


    return topLayerMsk;
}

int FilterTrackTool::dcos_prepare(const TFC_prj *prjs[2],
                           int                cnt,
                           unsigned int     scale)
{
    int dcos;

    dcos  = scale * prjs[0]->top.dxy;
    dcos /= prjs[0]->top.dz;

    /*
    if (cnt == 2) 
    {
        dcos  += scale * prjs[1]->top.dxy / prjs[1]->top.dz;
        dcos  /= 2;
    }
    */


    return dcos;
}

int FilterTrackTool::prjsSelect(GRBP_prjs     *grbp_prjs,
                         unsigned int      lyrMsk,
                         const TFC_prjList *lists)
{
    //
    // This code is copied directly from the module GRBP_prjsSelect.c in the GRBP package
    // Tracy Usher 1/8/08
    //
    int             cnt = 0;
    int              n0 = 0;
    int              n1 = 0;
    const TFC_prj* prj0 = 0;
    const TFC_prj* prj1 = 0;


    //printf ("LyrMsk = %8.8x\n", lyrMsk);
    while (lyrMsk)
    {
        int layer;
        const TFC_prjList *list;
        const TFC_prjNode *node;

        layer   = FFS (lyrMsk);
        lyrMsk  = FFS_eliminate (lyrMsk, layer);
        list    = lists + layer;
        node    = TFC__prjListFirst (list);
        //printf ("Layer = %u\n", layer + 2);

        do
        {
            const TFC_prj *prj = TFC__prjFromTopNode (node);
            int         nrange = prj->top.layer - prj->bot.layer;

            /*
            printf ("Twr Top:Bot %x:%u %x:%u nrange:n0 %u %u:%u\n", 
                    prj->top.tower,
                    prj->top.layer,
                    prj->bot.tower,
                    prj->bot.layer,
                    nrange, n0, n1);
            */
            if (nrange >= n0) 
            {
                n1 = n0;      prj1 = prj0;
                n0 = nrange;  prj0 = prj;
            }
            else if (nrange >= n1)
            {
                n1 = nrange; prj1 = prj;
            }
        }
        while ((node = TFC__prjListNext (list, node)));

    }

    grbp_prjs->cnt     = cnt = n1==0 ? 1 : 2;
    grbp_prjs->n[0]    = n0;
    grbp_prjs->n[1]    = n1;
    grbp_prjs->prjs[0] = prj0;
    grbp_prjs->prjs[1] = prj1;

    return cnt;
}

void FilterTrackTool::prjList_init (TFC_prjList lists[2][16])
{
    int idx;
    for (idx = 0; idx < 16; idx++)
    {
        TFC__prjListInit (&lists[0][idx]);
        TFC__prjListInit (&lists[1][idx]);
    }

    return;
}


HepPoint3D FilterTrackTool::findStripPosition(int tower, int layer, int view, int stripHit)
{
    double xpos = 0.0;
    double ypos = 0.0;
    double zpos = 0.0;

    // xviews
    if (view == 0) 
    {      
        zpos = (float)(m_tkrGeo->xy[0].z.positions[layer]) / TFC_Z_ABS_SCALE_FACTOR;
        xpos = (stripHit + m_tkrGeo->xy[0].offsets[tower]) * TKR_STRIP_PITCH_MM;
        ypos = m_tkrGeo->xy[1].offsets[tower] * TKR_STRIP_PITCH_MM + TKR_XY_WIDTH_MM/2.0;

    // yviews
    } 
    else 
    {
        zpos = (float)(m_tkrGeo->xy[1].z.positions[layer]) / TFC_Z_ABS_SCALE_FACTOR;
        xpos = m_tkrGeo->xy[0].offsets[tower] * TKR_STRIP_PITCH_MM + TKR_XY_WIDTH_MM/2.0;
        ypos = (stripHit + m_tkrGeo->xy[1].offsets[tower]) * TKR_STRIP_PITCH_MM;
    }

    HepPoint3D point(xpos,ypos,zpos);

   return point;
}
