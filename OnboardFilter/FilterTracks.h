/**
 * @class FilterTracks
 * @brief Algorithm to compute tracks from the Filter's projections
 * @author Navid Golpayeagani - golpa@milkyway.gsfc.nasa.gov
 * @author David Wren - dnwren@milkyway.gsfc.nasa.gov
 * $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/OnboardFilter/FilterTracks.h,v 1.6 2004/08/13 22:08:57 golpa Exp $
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

  int m_usenumhits;
  int m_writehits;
  int m_scattering;
  int m_trackselect;

  std::ofstream m_outfile;

  //StringProperty m_hitsfilename;
  //const char *m_hitsfilename;
};

#endif
