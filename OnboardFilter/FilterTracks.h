/**
 * @class FilterTracks
 * @brief Algorithm to compute tracks from the Filter's projections
 * @author Navid Golpayeagani - golpa@milkyway.gsfc.nasa.gov
 * @author David Wren - dnwren@milkyway.gsfc.nasa.gov
 * $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/OnboardFilter/FilterTracks.h,v 1.3 2004/07/12 17:22:23 golpa Exp $
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
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include <string>
#include <vector>

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
    void computeAngles();
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
    double m_pi;

	int m_usenumhits;
};

#endif
