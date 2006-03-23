/**
 * @class FilterTracks
 * @brief Algorithm to compute tracks from the Filter's projections
 * @author Navid Golpayeagani - golpa@milkyway.gsfc.nasa.gov
 * @author David Wren - dnwren@milkyway.gsfc.nasa.gov
 * $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/OnboardFilter/FilterTracks.h,v 1.11 2005/09/22 15:38:53 hughes Exp $
 */

#ifndef _FILTER_TRACKS_H_
#define _FILTER_TRACKS_H_

#include "OnboardFilter/FilterStatus.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Property.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/MonteCarlo/McParticle.h"
#include "Event/TopLevel/MCEvent.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "facilities/Util.h" // for expandEnvVar

#include <string>
#include <vector>
#include <fstream>
#include <cmath>

class FilterTracks : public Algorithm{
 public:
  FilterTracks(const std::string &name, ISvcLocator *pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

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
  double GetMCAngles(double track_theta_rad, double track_phi_rad,
                     double &theta_rad_mc,   double &phi_rad_mc);
  /**
   * Find out the position of a strip after converting Filter's strip
   * numbering into numbering consistent with getStripPosition()
   */
  HepPoint3D findStripPosition(ITkrGeometrySvc *tkrGeoSvc, int tower,
                               int layer, int view, double stripId);

  /**
   * Compute the multiple scattering angles
   */
  StatusCode MultipleScattering();
  /**
   * Write out the OnboardFilter hits
   */
  StatusCode WriteHits();
  /**
   * Experiment with different track selection methods
   */
  StatusCode TrackSelect();
  /**
   * Experiment with a Hough Transform method
   */
  StatusCode HoughTransform();
  void InitHoughMatrix();
  void SetUpHoughVectors();
  StatusCode BuildHoughZLayerIntercepts();
  void ComputeHoughRhoAndMatrix(double x, double y, double z, int view);
  void IncrementHoughMatrix(double rho_upperbound, int view,
                            double &rho_grid_unit, double &theta_grid_unit);
  void FindHoughMaxima(double rho_grid_unit, double theta_grid_unit);
  void FindHoughLayerIntersections();
  StatusCode ComputeHoughAngles();

  const static int m_grid_divisor = 1;
  const static int m_theta_numelements = 180/1;
  const static int m_rho_numelements = m_theta_numelements;
  const static int m_rho_grid = m_rho_numelements/m_grid_divisor; //can be anything <= m_rho_numelements
  const static int m_theta_grid = m_theta_numelements/m_grid_divisor; //can be anything <= m_theta_numelements
  int m_HX[m_rho_grid*2][m_theta_grid];
  int m_HY[m_rho_grid*2][m_theta_grid];

  double m_rho_vectorX[m_rho_numelements],
         m_rho_vectorY[m_rho_numelements],
         m_theta_vector[m_theta_numelements],
         m_cos_vector[m_theta_numelements],
         m_sin_vector[m_theta_numelements];
  double m_ZLayerInterceptsX[18];
  double m_ZLayerInterceptsY[18];
  double m_XviewIntersections[18];
  double m_YviewIntersections[18];
  double m_final_rhoX, m_final_rhoY, m_final_thetaX, m_final_thetaY;
  int m_rho_indexX, m_rho_indexY, m_theta_indexX, m_theta_indexY;
  double m_XviewZDist, m_YviewZDist, m_XviewXDist, m_YviewYDist, m_ZAvgDist;
  double m_XviewPrelimSlope, m_YviewPrelimSlope;

  std::vector<double> m_x;
  std::vector<double> m_y;
  std::vector<double> m_xz;
  std::vector<double> m_yz;
  std::vector<double> m_zAvg;
  std::vector<double> m_extendLow;
  std::vector<double> m_extendHigh;
  std::vector<double> m_pointHigh;
  double m_phi, m_phi_rad;
  double m_theta, m_theta_rad;
  double m_length;

  int m_run;
  int m_usenumhits;
  int m_writehits;
  int m_scattering;
  int m_trackselect;
  int m_hough;
  int m_zenith;

  std::ofstream m_outfile;

  //StringProperty m_hitsfilename;
  //const char *m_hitsfilename;
};

#endif
