//_____________________________________________________________________________
// Module to combine 2D track projections
//_____________________________________________________________________________

 
#include <iostream>
#include <exception>
//#include <_exception>
//#include "/usr/products/Glast/GLAST_EXT/rh9_gcc32/ROOT/v4.02.00/root/cint/include/_exception.h"
#include "trackProj.h"
#include "EDS/FFS.h"

//____________________________________________________________________________

void trackProj::execute(int flag, const TFC_projections *prjs,
         int &xHits, int &yHits,
         double &slopeXZ, double &slopeYZ,
         double &intXZ, double &intYZ){
//   printf("trackProj execute()");

   xHits= 0;
   yHits= 0;
   slopeXZ = 0.0;
   slopeYZ = 0.0;
   intXZ = 0.0;
   intYZ = 0.0;

   const TFC_projection *prj;

   prj = prjs->prjs;   
   int tmsk = prjs->twrMsk << 16;
   int maxCnt = prjs->maxCnt;
   int curCnt = prjs->curCnt;
//   printf("Projections: tower mask %x maxCnt %d curCnt %d\n",tmsk,maxCnt,curCnt);
   int xy = 1;
   while (tmsk) {
      int                      tower;
      const TFC_projectionDir *dir;

      tower = FFS (tmsk);     // get first bit set, this is the tower number
      tmsk  = FFS_eliminate (tmsk, tower);  // eliminate bit corresponding to tower

      dir       = prjs->dir + tower;
      prj = prjs->prjs + dir->idx;
      int        xCnt;
      int        yCnt;
      int        tCnt;


      /* Form the projection directory for this tower */
      xCnt = dir->xCnt;
      yCnt = dir->yCnt;

      tCnt      = xCnt + yCnt;
//      printf("   Tower %d xcnt %d ycnt %d tcnt %d\n",tower,xCnt,yCnt,tCnt);
   }



//Loop over the towers
   int startPrj=0;
   int maxTotalHits = 0; 
   double maxLength=0;
//   for(int tower=0;tower<16;tower++){
//      HepPoint3D point;
//      const TFC_projectionDir *dir = prjs->dir + tower;
   tmsk = prjs->twrMsk << 16;
   while (tmsk) {
      int                      tower;
      const TFC_projectionDir *dir;
      HepPoint3D point;
      tower = FFS (tmsk);     // get first bit set, this is the tower number
      tmsk  = FFS_eliminate (tmsk, tower);  // eliminate bit corresponding to tower

      dir       = prjs->dir + tower;
      prj = prjs->prjs + dir->idx;
      int        xCnt;
      int        yCnt;


      /* Form the projection directory for this tower */
      xCnt = dir->xCnt;
      yCnt = dir->yCnt;
      if(dir->xCnt>0 && dir->yCnt>0){
//         printf("tower %d xcnt %d yxnt %d\n",tower,dir->xCnt,dir->yCnt);
         //Loop over the x projections
         for(int xprj=startPrj;xprj<startPrj+dir->xCnt;xprj++){

            point=findStripPosition(tower,prjs->prjs[xprj].max,0,
            prjs->prjs[xprj].hits[prjs->prjs[xprj].max]);
            m_x[0]=point.x();
            m_xz[0]=point.z();
            point=findStripPosition(tower,prjs->prjs[xprj].max-1,0,
            prjs->prjs[xprj].hits[prjs->prjs[xprj].max-1]);
            m_x[1]=point.x();
            m_xz[1]=point.z();
//            printf("   xproj start layer %d hits %d\n",prjs->prjs[xprj].max,prjs->prjs[xprj].nhits);
            //Loop over the y projections
            for(int yprj=startPrj+dir->xCnt;yprj<startPrj+dir->yCnt+dir->xCnt;yprj++){

//                  printf("      yproj start layer %d hits %d\n",prjs->prjs[yprj].max,prjs->prjs[yprj].nhits);
                if(prjs->prjs[xprj].max==prjs->prjs[yprj].max){//if they start in the same layer
                 
                  point=findStripPosition(tower,prjs->prjs[yprj].max,
                  1,prjs->prjs[yprj].hits[prjs->prjs[yprj].max]);
                  m_y[0]=point.y();
                  m_yz[0]=point.z();
                  //printf("  findStripPosition tower %d max %d hits %d m_y[0] %f\n",
                  //                     tower,prjs->prjs[yprj].max,
                  //                      prjs->prjs[yprj].hits[0],m_y[0]);
                  point=findStripPosition(tower,prjs->prjs[yprj].max-1,
                  1,prjs->prjs[yprj].hits[prjs->prjs[yprj].max-1]);
                  m_y[1]=point.y();
                  m_yz[1]=point.z();
                  //printf("  findStripPosition tower %d max %d hits %d m_y[1] %f\n",
                  //                     tower,prjs->prjs[yprj].max-1,
                  //                      prjs->prjs[yprj].hits[1],m_y[1]);
                  unsigned char maxhits;
                  if(prjs->prjs[xprj].nhits<prjs->prjs[yprj].nhits) {//they don't need the same number
                     maxhits=prjs->prjs[xprj].nhits;              //of layers.  use the smaller number
                  } else {                                             //of hits.
                     maxhits=prjs->prjs[yprj].nhits;
                  }
                  point=findStripPosition(tower,prjs->prjs[xprj].max-(maxhits-1),
                  0,prjs->prjs[xprj].hits[prjs->prjs[xprj].max-(maxhits-1)]);
                  m_x[2]=point.x();
                  m_xz[2]=point.z();
                  point=findStripPosition(tower,prjs->prjs[yprj].max-(maxhits-1),
                  1,prjs->prjs[yprj].hits[prjs->prjs[yprj].max-(maxhits-1)]);
                  m_y[2]=point.y();
                  m_yz[2]=point.z();
                  for(int counter=0;counter<3;counter++) {
                     m_zAvg[counter]=(m_xz[counter]+m_yz[counter])/2;
                  }
                  computeAngles(m_x[1]-m_x[0], m_xz[0]-m_xz[1], m_y[1]-m_y[0],
                  m_yz[0]-m_yz[1], m_zAvg[0]-m_zAvg[1]);
                  computeSlopeInt();
                  computeLength();
                  computeExtension();
                  //Add track to TDS
                  if((prjs->prjs[xprj].nhits+prjs->prjs[yprj].nhits) > maxTotalHits){                     //longest, not the track with the most
                     maxTotalHits = prjs->prjs[xprj].nhits+prjs->prjs[yprj].nhits;
                     xHits=prjs->prjs[xprj].nhits;
                     yHits=prjs->prjs[yprj].nhits;
                     slopeXZ = m_slopeXZ;
                     slopeYZ = m_slopeYZ;
                     intXZ = m_intXZ;
                     intYZ = m_intYZ;
                  }                                                         //at a shallow angle with only 3 hits,
               }                                                           //and a short track with more than this.
               //printf("end check on max\n");
            } //printf("end yproj loop\n");
         } //printf("end xproj loop\n");
      }
      //    startPrj+=(int)prjs->curCnt[tower];//I moved this 06/14/04 - DW
      //printf("end check on proj\n");
      startPrj+=dir->yCnt+dir->xCnt;
   }
//   if (maxTotalHits > 0) 
//       printf("trackProj: found track: hitsX %d hits Y %d slopeX %f slopeY %f intX %f intY %f\n",
//               xHits,yHits,slopeXZ,slopeYZ,intXZ,intYZ);

   return;
}

void trackProj::computeSlopeInt() {
   double dx = m_x[1] - m_x[0];
   double dxz = m_xz[1] - m_xz[0];
   double dy = m_y[1] - m_y[0];
   double dyz = m_yz[1] - m_yz[0];
   m_slopeXZ = dx / dxz;
   m_slopeYZ = dy / dyz;
   m_intXZ = m_x[1] - m_slopeXZ*m_xz[1];
   m_intYZ = m_y[1] - m_slopeYZ*m_yz[1];
}


void trackProj::computeAngles(double x_h, double x_v, double y_h, double y_v, double z_v){
    //double x_h = m_x[1]-m_x[0];
    //double x_v = m_xz[0]-m_xz[1];
    //double y_h = m_y[1]-m_y[0];
    //double y_v = m_yz[0]-m_yz[1];
    //double z_v = m_zAvg[0]-m_zAvg[1];
//    printf("computeAngles: x_h %f,  x_v %f,  y_h %f,  y_v%f ,  z_v %f \n",x_h,  x_v,  y_h,  y_v,  z_v);
  double t_h_ave = 1;
  /*
    Must do a little stretching of vectors here!
    This accounts for the difference in z distance
    between x and y layers.  To get the vectors to
    line up, we have to adjust them a bit.
    */
    if (y_v < x_v){
        y_h = x_v*(y_h/y_v);
        y_v = x_v;
        z_v = x_v;
    }
    else if (x_v < y_v){
        x_h = y_v*(x_h/x_v);
        x_v = y_v;
        z_v = y_v;
  }

  if(x_h == 0 && y_h==0){
    m_phi_rad=0;
    m_theta_rad=0;
  }
  else{
    if(x_h ==0){
      m_theta_rad=M_PI/2-atan(y_v/y_h);
      if(y_h>0)
        m_phi_rad=M_PI/2;
      else
        m_phi_rad=3*M_PI/2;
    }
    else{
      if(y_h==0){
        m_theta_rad=M_PI/2-atan(x_v/x_h);
        if(x_h>0)
          m_phi_rad=0;
        else
          m_phi_rad=M_PI;
      }
      else{
        if((x_h>0) && (y_h>0)){
          m_phi_rad=atan(y_h/x_h);
          t_h_ave=((x_h/cos(m_phi_rad)) + (y_h/sin(m_phi_rad)))/2;
          m_theta_rad = M_PI - atan(t_h_ave/z_v);
        }
        else{
          if((x_h<0) && (y_h>0)){
            m_phi_rad=M_PI/2-atan(x_h/y_h);
            t_h_ave = ( -x_h/sin(m_phi_rad - M_PI/2) + y_h/cos(m_phi_rad - M_PI/2) )/2;
            m_theta_rad = M_PI - atan(t_h_ave/z_v);
          }
          else{
            if((x_h<0) && (y_h<0)){
              m_phi_rad=3*M_PI/2 - atan(x_h/y_h);
              t_h_ave = ( (-x_h/sin(3*M_PI/2 - m_phi_rad)) + (-y_h/cos(3*M_PI/2 - m_phi_rad)) )/2;
              m_theta_rad = M_PI - atan(t_h_ave/z_v);
            }
            else{
              if ((x_h>0) && (y_h<0)){
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
  m_theta=m_theta_rad*180/M_PI;
  m_phi=m_phi_rad*180/M_PI;
}


void trackProj::computeLength(){
  double t_v=m_zAvg[0]-m_zAvg[2];
  double t_h = t_v*tan(M_PI - m_theta_rad);
  m_length=sqrt(t_v*t_v+t_h*t_h);
  m_pointHigh[0]=t_h*cos(m_phi_rad) + m_x[0];
  m_pointHigh[1]=t_h*sin(m_phi_rad) + m_y[0];
  m_pointHigh[2]=m_zAvg[2];
}

void trackProj::computeExtension(){
  const double length=1000;
  m_extendLow[0]=length*sin(M_PI-m_theta_rad)*cos(M_PI+m_phi_rad) + m_x[0];
  m_extendLow[1]=length*sin(M_PI-m_theta_rad)*sin(M_PI+m_phi_rad) + m_y[0];
  m_extendLow[2]=length*cos(M_PI-m_theta_rad) + m_zAvg[0];

  m_extendHigh[0]=length*sin(m_theta_rad)*cos(m_phi_rad)+m_x[2];
  m_extendHigh[1]=length*sin(m_theta_rad)*sin(m_phi_rad)+m_y[2];
  m_extendHigh[2]=length*cos(m_theta_rad)+m_zAvg[2];
}

HepPoint3D trackProj::findStripPosition(int tower,
                                           int layer, int view, int hits){
   double xpos = 0.0;
   double ypos = 0.0;
   double zpos = 0.0;
//      printf("     calling findStripPosition tow %d lay %d view %d hits %d\n",
//                  tower,layer,view,hits);
//
// xviews
   if (view == 0) {      
      zpos = tkr_zpos_xlayer[layer];

      xpos = TKR_K_STRIP_MM*hits
                     + tower_offset_x[tower];
      ypos = tower_offset_y[tower] + TKR_XY_WIDTH_MM/2.0;
//
// yviews
   } else {
      zpos = tkr_zpos_ylayer[layer];

      ypos = TKR_K_STRIP_MM*hits
                     + tower_offset_y[tower];
      xpos = tower_offset_x[tower] + TKR_XY_WIDTH_MM/2.0;
   }

   HepPoint3D point(xpos,ypos,zpos);

  return point;
}


void trackProj::loadGeometry() {

//
// Load geometry
   tkr_zpos_xlayer[0] = TKR_Z_XLAYER_00_MM;
   tkr_zpos_xlayer[1] = TKR_Z_XLAYER_01_MM;
   tkr_zpos_xlayer[2] = TKR_Z_XLAYER_02_MM;
   tkr_zpos_xlayer[3] = TKR_Z_XLAYER_03_MM;
   tkr_zpos_xlayer[4] = TKR_Z_XLAYER_04_MM;
   tkr_zpos_xlayer[5] = TKR_Z_XLAYER_05_MM;
   tkr_zpos_xlayer[6] = TKR_Z_XLAYER_06_MM;
   tkr_zpos_xlayer[7] = TKR_Z_XLAYER_07_MM;
   tkr_zpos_xlayer[8] = TKR_Z_XLAYER_08_MM;
   tkr_zpos_xlayer[9] = TKR_Z_XLAYER_09_MM;
   tkr_zpos_xlayer[10] = TKR_Z_XLAYER_10_MM;
   tkr_zpos_xlayer[11] = TKR_Z_XLAYER_11_MM;
   tkr_zpos_xlayer[12] = TKR_Z_XLAYER_12_MM;
   tkr_zpos_xlayer[13] = TKR_Z_XLAYER_13_MM;
   tkr_zpos_xlayer[14] = TKR_Z_XLAYER_14_MM;
   tkr_zpos_xlayer[15] = TKR_Z_XLAYER_15_MM;
   tkr_zpos_xlayer[16] = TKR_Z_XLAYER_16_MM;
   tkr_zpos_xlayer[17] = TKR_Z_XLAYER_17_MM;

   tkr_zpos_ylayer[0] = TKR_Z_YLAYER_00_MM;
   tkr_zpos_ylayer[1] = TKR_Z_YLAYER_01_MM;
   tkr_zpos_ylayer[2] = TKR_Z_YLAYER_02_MM;
   tkr_zpos_ylayer[3] = TKR_Z_YLAYER_03_MM;
   tkr_zpos_ylayer[4] = TKR_Z_YLAYER_04_MM;
   tkr_zpos_ylayer[5] = TKR_Z_YLAYER_05_MM;
   tkr_zpos_ylayer[6] = TKR_Z_YLAYER_06_MM;
   tkr_zpos_ylayer[7] = TKR_Z_YLAYER_07_MM;
   tkr_zpos_ylayer[8] = TKR_Z_YLAYER_08_MM;
   tkr_zpos_ylayer[9] = TKR_Z_YLAYER_09_MM;
   tkr_zpos_ylayer[10] = TKR_Z_YLAYER_10_MM;
   tkr_zpos_ylayer[11] = TKR_Z_YLAYER_11_MM;
   tkr_zpos_ylayer[12] = TKR_Z_YLAYER_12_MM;
   tkr_zpos_ylayer[13] = TKR_Z_YLAYER_13_MM;
   tkr_zpos_ylayer[14] = TKR_Z_YLAYER_14_MM;
   tkr_zpos_ylayer[15] = TKR_Z_YLAYER_15_MM;
   tkr_zpos_ylayer[16] = TKR_Z_YLAYER_16_MM;
   tkr_zpos_ylayer[17] = TKR_Z_YLAYER_17_MM;

   tower_offset_x[0] = TKR_X0_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[1] = TKR_X1_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[2] = TKR_X2_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[3] = TKR_X3_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[4] = TKR_X0_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[5] = TKR_X1_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[6] = TKR_X2_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[7] = TKR_X3_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[8] =  TKR_X0_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[9] =  TKR_X1_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[10] = TKR_X2_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[11] = TKR_X3_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[12] = TKR_X0_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[13] = TKR_X1_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[14] = TKR_X2_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_x[15] = TKR_X3_MID_MM  - TKR_XY_WIDTH_MM/2.0;

   tower_offset_y[0] = TKR_Y0_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[4] = TKR_Y1_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[8] = TKR_Y2_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[12] = TKR_Y3_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[1] =  TKR_Y0_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[5] =  TKR_Y1_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[9] =  TKR_Y2_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[13] =  TKR_Y3_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[2] =   TKR_Y0_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[6] =   TKR_Y1_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[10] =  TKR_Y2_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[14] =  TKR_Y3_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[3] =  TKR_Y0_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[7] =  TKR_Y1_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[11] =  TKR_Y2_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   tower_offset_y[15] =  TKR_Y3_MID_MM  - TKR_XY_WIDTH_MM/2.0;
   return;
}

