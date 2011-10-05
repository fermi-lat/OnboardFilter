#ifndef GrbFindTrack_h
#define GrbFindTrack_h

// obf Track definitions
#ifdef OBF_B1_1_3
#include "FSWHeaders/TFC_prjDef.h"
#endif
#ifdef OBF_B3_0_0
#include "EFC/TFC_prjDef.h"
#endif

#include "EFC/EFC_edsFw.h"

// HepPoint3D
#include "CLHEP/Geometry/Point3D.h"

class GrbTrack
{
public:
    GrbTrack() : m_valid(false), m_xPrj(0), m_yPrj(0), m_dxzi(0), m_dyzi(0), m_dzi(0) {};
    GrbTrack(const TFC_prj* xPrj, const TFC_prj* yPrj, int dxzi, int dyzi, int dzi) :
             m_valid(true), m_xPrj(xPrj), m_yPrj(yPrj), m_dxzi(dxzi), m_dyzi(dyzi), m_dzi(dzi)
             {};
    ~GrbTrack() {};

    GrbTrack(const GrbTrack& track)
    {
        m_valid = track.m_valid;
        m_xPrj  = track.m_xPrj;
        m_yPrj  = track.m_yPrj;
        m_dxzi  = track.m_dxzi;
        m_dyzi  = track.m_dyzi;
        m_dzi   = track.m_dzi;
    }

    bool           valid()          {return m_valid;}
    const TFC_prj* getProjectionX() {return m_xPrj; }
    const TFC_prj* getProjectionY() {return m_yPrj; }
    int            get_dxzi()       {return m_dxzi; }
    int            get_dyzi()       {return m_dyzi; }
    int            get_dzi()        {return m_dzi;  }

private:
    bool           m_valid;
    const TFC_prj* m_xPrj;
    const TFC_prj* m_yPrj;
    int            m_dxzi;
    int            m_dyzi;
    int            m_dzi;
};

// Forward declaration
typedef struct _GFC_cfg         GFC_cfg;
typedef struct _TFC_geometryTkr TFC_geometryTkr;

class GrbFindTrack 
{ 
public:
    GrbFindTrack(GFC_cfg* cfg);
    ~GrbFindTrack();

//    GrbTrack findTrack(EDS_fwIxb* ixb);
    GrbTrack findTrack(TFC_prjs* prjs);

private:
    // Define an internal class for temporarily handling results
    class GRBP_prjs
    {
    public:
        unsigned char  cnt;
        unsigned char  n[2];
        unsigned char  rsvd;
        const TFC_prj* prjs[2];
    };

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
    HepGeom::Point3D<double> findStripPosition(int tower, int layer, int view, int stripHit);
    
    // data members
    unsigned int m_strip_pitch;  /*!< Tracker strip pitch, in mm            */
    unsigned int   m_dxy_scale;  /*!< XY scale factor                       */
    unsigned int    m_dz_scale;  /*!< Z  scale factor                       */

    // Geometry database from FSW
    const TFC_geometryTkr* m_tkrGeo;
};

#endif
