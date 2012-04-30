//_____________________________________________________________________________
// Module to combine 2D track projections
//_____________________________________________________________________________

#include "trackProj.h"

#ifdef OBF_B1_1_3
# include "EDS/FFS.h"
#else 
# include "PBI/FFS.ih"
# define  FFS_mask  FFSL_mask
# define  FFS_eliminate FFSL_eliminate
# define  FFS       FFSL
#endif

#include "EFC_DB/EFC_DB_sampler.h"
#ifdef  OBF_B3_0_0
#include "EFC/GFC_def.h"
#include "EFC/TFC_geometryDef.h"
#else
#include "EFC/../src/GFC_def.h"
#include "EFC/../src/TFC_geometryDef.h"
#endif
#include <iostream>
#include <exception>
#include <cmath>

#ifdef OBF_B3_0_0
#  include "GGF_DB/GGF_DB_data.h"
#else
#  ifdef SCons
#    include "EFC/../src/GEO_DB_data.h" // was GGF/.../src
#  else
#    include "src/GEO_DB_data.h"
#  endif
#endif

//  Temporary!   Instead of above include
//#define TKR_STRIP_PITCH_MM    0.228
//#define TKR_LADDER_GAP_MM  (2*.974 +.2)
#include "GEO_DB/GEO_DB_macros.h"
#include "EFC/TFC_prjDef.h"

typedef HepGeom::Point3D<double> HepPoint3D;
//____________________________________________________________________________
trackProj::trackProj(GFC_cfg* cfg) 
{
    const GammaCfgTkr&     tkrCfg = cfg->prms.tkr;
    const TFC_geometry*    geom   = tkrCfg.geometry;
    const TFC_geometryTkr& tkrGeo = geom->tkr;

    m_tkrGeo = &tkrGeo;
}


void trackProj::execute(const TFC_prjs* prjs, 
                        int&            xHits, 
                        int&            yHits, 
                        double&         slopeXZ, 
                        double&         slopeYZ,
                        double&         intXZ, 
                        double&         intYZ)
{
    // Initialize output in case of failure
    xHits   = 0;
    yHits   = 0;
    slopeXZ = 0.0;
    slopeYZ = 0.0;
    intXZ   = 0.0;
    intYZ   = 0.0;

    int startPrj     = 0;
    int maxTotalHits = 0; 

    unsigned int tmsk = prjs->twrMsk << 16;

    while (tmsk) 
    {
      int               tower = FFS(tmsk);  // tower = FFSL(tmsk);
        const TFC_prjDir* dir   = prjs->dir + tower;
        const TFC_prj*    prj   = prjs->prjs + dir->idx;
        int               xCnt  = dir->xCnt;
        int               yCnt  = dir->yCnt;

        // eliminate bit corresponding to tower
        tmsk  = FFS_eliminate (tmsk, tower);  // FFSL_eliminate (tmsk, tower);

        /* Form the projection directory for this tower */
        if (xCnt > 0 && yCnt > 0)
        {
            //Loop over the x projections
            for(int xPrjIdx=startPrj; xPrjIdx<startPrj+xCnt; xPrjIdx++)
            {
                const TFC_prj& xPrj     = prjs->prjs[xPrjIdx];
                unsigned int   xLyrMask = xPrj.layers;
                int            xLayer   = xPrj.max;            // Highest hit
                const TFC_hit& xHit     = xPrj.hits[xLayer];
                
                HepPoint3D point = findStripPosition(xHit.tower, xLayer, 0, xHit.strip);
                
                m_x[0]  = point.x();
                m_xz[0] = point.z();

                // Next layer with a valid hit
                int nxtXLayer = xLayer;
                while(!(xLyrMask & (1 << --nxtXLayer)) && nxtXLayer > 0);

                const TFC_hit& nxtXHit = xPrj.hits[nxtXLayer];

                point = findStripPosition(nxtXHit.tower, nxtXLayer, 0, nxtXHit.strip);

                m_x[1]  = point.x();
                m_xz[1] = point.z();

                //Loop over the y projections
                for(int yPrjIdx = startPrj+xCnt; yPrjIdx < startPrj+yCnt+xCnt; yPrjIdx++)
                {
                    const TFC_prj& yPrj     = prjs->prjs[yPrjIdx];
                    unsigned int   yLyrMask = yPrj.layers;
                    int            yLayer   = yPrj.max;            // Highest hit
                    const TFC_hit& yHit     = yPrj.hits[yLayer];

                    // Condition that both projections begin in the same layer
                    // Change this (12/18/07 TU) to allow starting in "near" layer AND
                    // that the proposed track be longer than the current track
//                    if( xLayer == yLayer && xPrj.nhits + yPrj.nhits > maxTotalHits)
                    if( std::abs(xLayer - yLayer) < 2 && xPrj.nhits + yPrj.nhits > maxTotalHits)
                    {
                        point = findStripPosition(yHit.tower, yLayer, 1, yHit.strip);

                        m_y[0]  = point.y();
                        m_yz[0] = point.z();

                        // Next layer with a valid hit
                        int nxtYLayer = yLayer;
                        while(!(yLyrMask & (1 << --nxtYLayer)) && nxtYLayer > 0);

                        const TFC_hit& nxtYHit = yPrj.hits[nxtYLayer];

                        point = findStripPosition(nxtYHit.tower, nxtYLayer, 1, nxtYHit.strip);

                        m_y[1]  = point.y();
                        m_yz[1] = point.z();

                        // Try to set to common first valid hit
                        // Note that this doesn't (yet) check that both projections have a valid hit in
                        // this layer...
                        int firstHit = xPrj.min;
                        if (xPrj.min < yPrj.min) firstHit = yPrj.min; 

                        const TFC_hit& botXHit = xPrj.hits[firstHit];

                        point = findStripPosition(botXHit.tower, firstHit, 0, botXHit.strip);

                        m_x[2]  = point.x();
                        m_xz[2] = point.z();

                        const TFC_hit& botYHit = yPrj.hits[firstHit];

                        point = findStripPosition(botYHit.tower, firstHit, 1, botYHit.strip);

                        m_y[2]  = point.y();
                        m_yz[2] = point.z();

                        for(int counter=0;counter<3;counter++) 
                        {
                            m_zAvg[counter] = (m_xz[counter] + m_yz[counter]) / 2;
                        }

                        computeAngles(m_x[1]-m_x[0], m_xz[0]-m_xz[1], m_y[1]-m_y[0],
                            m_yz[0]-m_yz[1], m_zAvg[0]-m_zAvg[1]);

                        computeSlopeInt();
                        computeLength();
                        computeExtension();

                        //Add track to TDS
                        if((xPrj.nhits + yPrj.nhits) > maxTotalHits)
                        {                     //longest, not the track with the most
                            maxTotalHits = xPrj.nhits + yPrj.nhits;
                            xHits        = xPrj.nhits;
                            yHits        = yPrj.nhits;
                            slopeXZ      = m_slopeXZ;
                            slopeYZ      = m_slopeYZ;
                            intXZ        = m_intXZ;
                            intYZ        = m_intYZ;
                        }                                                   //at a shallow angle with only 3 hits,
                    }                                                       //and a short track with more than this.
                    //printf("end check on max\n");
                } //printf("end yproj loop\n");
            } //printf("end xproj loop\n");
        }
        //    startPrj+=(int)prjs->curCnt[tower];//I moved this 06/14/04 - DW
        //printf("end check on proj\n");
        startPrj+=yCnt + xCnt;
    }
//   if (maxTotalHits > 0) 
//       printf("trackProj: found track: hitsX %d hits Y %d slopeX %f slopeY %f intX %f intY %f\n",
//               xHits,yHits,slopeXZ,slopeYZ,intXZ,intYZ);

    return;
}

void trackProj::computeSlopeInt() 
{
    double dx  = m_x[1] - m_x[0];
    double dxz = m_xz[1] - m_xz[0];
    double dy  = m_y[1] - m_y[0];
    double dyz = m_yz[1] - m_yz[0];
    
    m_slopeXZ = dx / dxz;
    m_slopeYZ = dy / dyz;
    m_intXZ   = m_x[1] - m_slopeXZ*m_xz[1];
    m_intYZ   = m_y[1] - m_slopeYZ*m_yz[1];
}


void trackProj::computeAngles(double x_h, double x_v, double y_h, double y_v, double z_v)
{
    double t_h_ave = 1;
    /*
        Must do a little stretching of vectors here!
        This accounts for the difference in z distance
        between x and y layers.  To get the vectors to
        line up, we have to adjust them a bit.
    */
    if (y_v < x_v)
    {
        y_h = x_v*(y_h/y_v);
        y_v = x_v;
        z_v = x_v;
    }
    else if (x_v < y_v)
    {
        x_h = y_v*(x_h/x_v);
        x_v = y_v;
        z_v = y_v;
    }

    if(x_h == 0 && y_h==0)
    {
        m_phi_rad=0;
        m_theta_rad=0;
    }
    else
    {
        if(x_h ==0)
        {
            m_theta_rad=M_PI/2-atan(y_v/y_h);
            if(y_h>0)
                m_phi_rad=M_PI/2;
            else
                m_phi_rad=3*M_PI/2;
        }
        else
        {
            if(y_h==0)
            {
                m_theta_rad=M_PI/2-atan(x_v/x_h);
                if(x_h>0)
                    m_phi_rad=0;
                else
                    m_phi_rad=M_PI;
            }
            else
            {
                if((x_h>0) && (y_h>0))
                {
                    m_phi_rad=atan(y_h/x_h);
                    t_h_ave=((x_h/cos(m_phi_rad)) + (y_h/sin(m_phi_rad)))/2;
                    m_theta_rad = M_PI - atan(t_h_ave/z_v);
                }
                else
                {
                    if((x_h<0) && (y_h>0))
                    {
                        m_phi_rad=M_PI/2-atan(x_h/y_h);
                        t_h_ave = ( -x_h/sin(m_phi_rad - M_PI/2) + y_h/cos(m_phi_rad - M_PI/2) )/2;
                        m_theta_rad = M_PI - atan(t_h_ave/z_v);
                    }
                    else
                    {
                        if((x_h<0) && (y_h<0))
                        {
                            m_phi_rad=3*M_PI/2 - atan(x_h/y_h);
                            t_h_ave = ( (-x_h/sin(3*M_PI/2 - m_phi_rad)) + (-y_h/cos(3*M_PI/2 - m_phi_rad)) )/2;
                            m_theta_rad = M_PI - atan(t_h_ave/z_v);
                        }
                        else
                        {
                            if ((x_h>0) && (y_h<0))
                            {
                                m_phi_rad=2*M_PI-atan(-y_h/x_h);
                                t_h_ave = ((x_h/cos(2*M_PI-m_phi_rad)) + (-y_h/sin(2*M_PI-m_phi_rad)))/2;
                                m_theta_rad = M_PI - atan(t_h_ave/z_v);
                            }
                            else
                                m_phi_rad=3*M_PI/2-atan(y_h/x_h);
                        }
                    }
                }
            }
        }
    }

    m_theta = m_theta_rad * 180 / M_PI;
    m_phi   = m_phi_rad   * 180 / M_PI;
}


void trackProj::computeLength()
{
    double t_v = m_zAvg[0] - m_zAvg[2];
    double t_h = t_v*tan(M_PI - m_theta_rad);
  
    m_length       = sqrt(t_v*t_v+t_h*t_h);
    m_pointHigh[0] = t_h * cos(m_phi_rad) + m_x[0];
    m_pointHigh[1] = t_h * sin(m_phi_rad) + m_y[0];
    m_pointHigh[2] = m_zAvg[2];
}

void trackProj::computeExtension()
{
    const double length = 1000;

    m_extendLow[0]  = length*sin(M_PI-m_theta_rad) * cos(M_PI+m_phi_rad) + m_x[0];
    m_extendLow[1]  = length*sin(M_PI-m_theta_rad) * sin(M_PI+m_phi_rad) + m_y[0];
    m_extendLow[2]  = length*cos(M_PI-m_theta_rad) + m_zAvg[0];

    m_extendHigh[0] = length*sin(m_theta_rad) * cos(m_phi_rad)+m_x[2];
    m_extendHigh[1] = length*sin(m_theta_rad) * sin(m_phi_rad)+m_y[2];
    m_extendHigh[2] = length*cos(m_theta_rad) + m_zAvg[2];
}

HepPoint3D trackProj::findStripPosition(int tower, int layer, int view, int hits)
{
    double xpos = 0.0;
    double ypos = 0.0;
    double zpos = 0.0;

    // xviews
    if (view == 0) 
    {      
        zpos = (float)(m_tkrGeo->xy[0].z.positions[layer]) / TFC_Z_ABS_SCALE_FACTOR;
        xpos = (hits + m_tkrGeo->xy[0].offsets[tower]) * TKR_STRIP_PITCH_MM;
        ypos = m_tkrGeo->xy[1].offsets[tower] * TKR_STRIP_PITCH_MM + TKR_XY_WIDTH_MM/2.0;

    // yviews
    } 
    else 
    {
        zpos = (float)(m_tkrGeo->xy[1].z.positions[layer]) / TFC_Z_ABS_SCALE_FACTOR;
        xpos = m_tkrGeo->xy[0].offsets[tower] * TKR_STRIP_PITCH_MM + TKR_XY_WIDTH_MM/2.0;
        ypos = (hits + m_tkrGeo->xy[1].offsets[tower]) * TKR_STRIP_PITCH_MM;
    }

    HepPoint3D point(xpos,ypos,zpos);

   return point;
}
