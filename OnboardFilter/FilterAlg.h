/**
 * @class FilterAlg
 * @brief Algorithm that reproduces OnboardFilter logic
 * @author David Wren - dnwren@milkyway.gsfc.nasa.gov
 * $Header: /nfs/slac/g/glast/ground/cvs/OnboardFilter/FilterAlg.h,v 1.0 2004/04/05
 */

#ifndef _FILTER_ALG_H_
#define _FILTER_ALG_H_

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

#include "Event/Digi/AcdDigi.h"

#include <string>

#include "OnboardFilter/FilterStatus.h"
#include "OnboardFilter/FilterAlgTds.h"

#include "DFC/TFC_geos.h"//need access to the Filter Geometry!
#include "DFC/TFC_geometryPrint.h"
#include "DFC/TFC_geoIds.h"
#include "TFC_geometryDef.h"
#include "FilterAlg_acd.h"

extern const struct _TFC_geometry *TFC_Geos[];

using namespace OnboardFilterTds;

class FilterAlg : public Algorithm{
public:
   FilterAlg(const std::string &name, ISvcLocator *pSvcLocator);
   StatusCode initialize();
   StatusCode execute();
   StatusCode finalize();
private:
   ///Does some manipulations on filter data to
   ///create some local variables.
   void CreateLocals();
   ///initializes the acd masks used in ACD_splash()
   void initMasks(int m_MaskFront[], int m_MaskY[], int m_MaskX[]);

   ///Allows the user to use the ACD digis instead of the tiles
   ///OnboardFilter says are hit.
   void useAcdDigi(const Event::AcdDigiCol& digiCol);

   ///Checks for cal triggers.
   ///Potentially sets NOCALLO_FILTER_TILE
   ///and SPLASH_0 vetoes.
   void CheckCal();

   ///Looks for patterns of hits in the ACD.
   ///Potentially sets E0_TIL and E350_FILTER_TILE vetoes.
   void evaluateAcd();

   ///Looks for a splash veto
   ///Returns true if there is a splash, false otherwise
   bool AFC_splash();

   ///Maps the ACD tile to a mask used when looking for
   ///a splash veto.
   ///Returns the index of the mask to use.
   int mapBitY(int bitnum);

   ///Maps the ACD tile to a mask used when looking for
   ///a splash veto.
   ///Returns the index of the mask to use.
   int mapBitX(int bitnum);

   ///Looks for a throttle, basically.
   ///Returns 1 if throttle is active, 0 otherwise.
   void evaluateAtf();

   ///Evaluates zbottom condition
   ///Potentially sets ZBOTTOM veto
   void evaluateZbottom();

   ///Does the Cal Ratio Checks
   void evaluateCal1();

   ///Checks whether the top or top two rows of ACD are hit
   ///Returns YES or NO.
   bool acdFilter();

   ///Checks to see if 4 of 6 planes closest to the cal are hit
   ///Returns 1 if true, 0 otherwise
   bool checkPlanes(int tower);

   ///Sets a veto bit.
   void setVeto(int m_veto);

   ///Counts all the tiles hit in the acd words.
   ///Returns the count of tiles hit.
   int sumBits(unsigned int acd_X, unsigned int acd_Y, unsigned int acd_Front);

   ///Counts the number of layers hit
   ///Returns the number of layers hit
   int sumBits(unsigned int xlayers, unsigned int ylayers);

   ///Given a word, it counts the bits in that word
   ///Returns the number of bits
   int cntBits(unsigned int word);

   ///Used by cntBits to find the leading bit in a word
   ///Returns the position of the bit
   int FFS(unsigned int lw);

   ///Alg that does the track finding and projecting
   ///to the ACD and skirt region.
   ///Potentially sets TKR_EQ_0, TKR_TOP, TKR_ROW01, TKR_ROW02, TKR_SKRIT
   void tkrFilter(ITkrGeometrySvc *tkrGeoSvc);

   ///Takes 2 and 3 in a row word, splits it up, and compacts it,
   ///creating a list of towers that have either a 2 or 3 in a row.
   ///Returns the word with towers to look at.
   int createTowerMask(int towerword);

   //! sets the id of the next tower to look at
   //@ return the tower id
   int getTowerID(unsigned int towers);

   //! sets the correct mask for the triggered tower
   void getMask(int towerID);

   //! remove the tower from the list, becuase it has been examined
   void removeTower(int twr, unsigned int& triggeredtowers);

   ///Does the projection finding for the tower
   ///projections is filled in.
   ///void findProjections(int tower, prjs& projections);

   ///Does the tile-track matching
   ///Fills in the first tile that's hit
   int ACDProject(ITkrGeometrySvc *tkrGeoSvc,int tower, const projections *prj);

   ///Converts an an acd tile id to face, row and column representation
   void convertId(int rowtile, int face, int &r_row, int &r_col);

   ///Creates a list of rows and columns to look at in the acd
   void createTileList(int face, int row, int col, int rowlist[], int collist[]);

   ///From a tile number and face, reconstructs a tile number from 0 to 88
   ///Returns this tile number between 0 and 88
   int reconstructTileNumber(int tile, short face);

   ///Loops over the projections and does the track-tile matching
   ///Returns the first tile that is intersected by 2 projections
   int projectionLoop(int prjcnt,int tile,int face,ITkrGeometrySvc *tkrGeoSvc,const projections *prj,
					  int tower,int view,double tiledim_x, double tiledim_y, double tiledim_z,
					  HepPoint3D acdCenter);

   ///Returns a 3D coordinate of the position of the hit within the tower.
   HepPoint3D findStripPosition(ITkrGeometrySvc *tkrGeoSvc,int tower, int layer, int view, double stripId);

   ///Converts the projection to a vector
   ///Returns that vector
   HepVector3D convertPrj(ITkrGeometrySvc *tkrGeoSvc, const projections *prj, int prjit,
	                      int tower, int view, HepPoint3D &prjStart);

   ///Returns a point that contains the coordinate of where the projection intersects
   HepPoint3D getAcdCoord(HepVector3D newPrj,HepPoint3D prjStart, HepPoint3D acdCenter,
						  double &theta, int view,int face,
						  double tiledim_x, double tiledim_y, double tiledim_z,
						  HepPoint3D &boundCoord1, HepPoint3D &boundCoord2,
						  HepPoint3D &checkPoint1, HepPoint3D &checkPoint2,
						  HepPoint3D &checkPoint3, HepPoint3D &checkPoint4);
   void getTileBoundaries(int tile, int face, int view,
						  float &lesserBoundary, float &greaterBoundary);


   ///Determines if the first tile hit is from the top or a side row
   ///Returns an id that tells where it was hit.
   int evaluateTiles(int tile);

   //! gets the tower ID and mask, then calls the functions
   //! that do the checking
   bool compare(int &count);

   ///Determines which tiles to consider for veto 24
   void getRowMask(int tower, int row);

   ///Returns the list of towers that have a 2 in a row
   ///or a 3 in a row.
   int getPossible();

   ///Creates a list of towers that have any activity (hits)
   void getActiveTowers();

   ///Returns the starting layer of the coincidence closest to the top
   int getStart(int tower, int rows);

   ///Determines whether the tower has a 4/4,6/6, or 7/8 coincidence
   ///Returns true if it has a 6/6 or 7/8
   int coincidenceLevel(int tower);

   ///Reorders an xlayers or ylayers variable to put it in an easier
   ///to understand format.
   void reorderLayers(int& layers);

   /// access to the Glast Detector Service to read in geometry constants from XML files
   IGlastDetSvc *m_glastDetSvc;

   bool m_throttle;
   int m_triggered_towers, m_number_triggered, m_active_towers;
   int m_trigger_word, m_maskTop, m_maskX, m_maskY;

   int m_CAL_LO;
   int m_CAL_HI;
   const static int m_THROTTLE_SET = 1;

   int m_veto, m_tmsk;
   int m_acd_X, m_acd_Y, m_acd_Front;
   int m_rowMaskX, m_rowMaskY;
   float m_energy;
   int m_tcids, m_coincidences_3[16], m_coincidences_2[16];
   //m_coincidences[] has a list of coincidences for each tower.
   //It is numbered from lsb to msb, with layer 0 at the top.  If
   //a bit is set for a layer, that means a 3-in-a-row starts in
   //that layer (6 out of 6 layers hit)
   const float* m_layerEnergy;
   const int* m_xy00;
   const int* m_xy11;
   const int* m_xy22;
   const int* m_xy33;
   const int* m_xcapture;
   const int* m_ycapture;

   unsigned int m_FilterStatus_HI, m_FilterStatus_LO;

   int m_MaskFront[65];
   int m_MaskY[65];
   int m_MaskX[65];

   unsigned int m_vetoword;

   int m_NOCALLO_FILTER_TILE_VETO;
   int m_SPLASH_0_VETO;
   int m_E0_TILE_VETO;
   int m_E350_FILTER_TILE_VETO;
   int m_SPLASH_1_VETO;
   int m_TOP_VETO;
   int m_SIDE_VETO;
   int m_EL0_ETOT_01_VETO;
   int m_EL0_ETOT_90_VETO;
   int m_ZBOTTOM_VETO;
   int m_TKR_TOP_VETO;
   int m_TKR_ROW01_VETO;
   int m_TKR_ROW23_VETO;
   int m_TKR_EQ_0_VETO;
   int m_TKR_SKIRT_VETO;
   int m_TKR_LT_2_ELO_VETO;

   int m_count;//for debugging purposes

   //for jobOptions
   int m_evaluateCal1;
   int m_usegleamacdvetoes;
   int m_evaluateAcdUncomment1;
   int m_useFilterProjecting;
   int m_useGleamTileGeometry;
   int m_PrjColMatch;
};

#endif
