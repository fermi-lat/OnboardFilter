#ifndef TRACKPROJ
#define TRACKPROJ

#ifdef OBF_B1_1_3
#include "FSWHeaders/TFC_prjDef.h"
#endif
#if defined(OBF_B3_0_0) || defined(OBF_B3_1_0)
#include "EFC/TFC_prjDef.h"
#endif

#include "CLHEP/Geometry/Transform3D.h"
#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Vector/Rotation.h"

#include "TLorentzVector.h"

#include <string>
#include <vector>
#include <fstream>
#include <cmath>

typedef struct _GFC_cfg         GFC_cfg;
typedef struct _TFC_geometryTkr TFC_geometryTkr;
typedef struct _TFC_prjs        TFC_prjs;

class trackProj 
{ 
public:
    trackProj(GFC_cfg* cfg);
    ~trackProj() {}
   
    void execute(const TFC_prjs *prjs, int &xHits, int &yHits, double &xzSlope, double &yzSlope,
                 double &xzInt, double &yzInt);
 
private:
    /**
    * Compute Angles for a given track
    */
    void computeAngles(double x_h, double x_v, double y_h, double y_v, double z_v);
    /**
    * Compute the Length of a track
    */
    void computeLength();
    /**
    * Compute the extensions to the tracks
    */
    void computeExtension();
  
    void computeSlopeInt();
    /**
    * Find out the position of a strip after converting Filter's strip
    * numbering into numbering consistent with getStripPosition()
    */
    HepGeom::Point3D<double> findStripPosition(int tower, int layer, int view, int max);

    double m_ZLayerInterceptsX[18];
    double m_ZLayerInterceptsY[18];
    double m_XviewIntersections[18];
    double m_YviewIntersections[18];
    double m_final_rhoX, m_final_rhoY, m_final_thetaX, m_final_thetaY;
    int    m_rho_indexX, m_rho_indexY, m_theta_indexX, m_theta_indexY;
    double m_XviewZDist, m_YviewZDist, m_XviewXDist, m_YviewYDist, m_ZAvgDist;
    double m_XviewPrelimSlope, m_YviewPrelimSlope;

    double m_x[50];
    double m_y[50];
    double m_xz[50];
    double m_yz[50];
    double m_zAvg[50];
    double m_extendLow[50];
    double m_extendHigh[50];
    double m_pointHigh[50];
    double m_phi, m_phi_rad;
    double m_theta, m_theta_rad;
    double m_length;

    double m_slopeXZ;
    double m_slopeYZ;
    double m_intXZ;
    double m_intYZ;
   
    int m_run;
    int m_usenumhits;
    int m_writehits;
    int m_scattering;
    int m_trackselect;
    int m_hough;
    int m_zenith;
    //
    // Geometry
    const TFC_geometryTkr* m_tkrGeo;
};

#endif

