#ifndef FILTER_STATUS_H
#define FILTER_STATUS_H

#include <iostream>
#include <vector>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"

#include "Event/TopLevel/Definitions.h"
#include "Event/TopLevel/EventModel.h"

//#include "TFC_projectionDef.h"
/**
 * @class FilterStatus
 * @brief TDS for storing the information returned by the filter
 */
//extern const CLID& CLID_FilterStatus;


namespace OnboardFilterTds{
	struct TFC_projection{
        int           intercept; /*!< Intercept at the beginning layer         */
        int               slope; /*!< Slope                                    */
        int          acdTopMask; /*!< ACD top tile candidates                  */
        int            acdXMask; /*!< ACD x facing candidates                  */
        int            acdYMask; /*!< ACD y facing candidates                  */
        unsigned char skirtMask; /*!< Mask of which skirt region prj strikes   */
        unsigned char       min; /*!< Beginning layer number of the projection */
        unsigned char       max; /*!< Ending    layer number of the projection */
        unsigned char     nhits; /*!< Number of hits assigned                  */
        unsigned         layers; /*!< Bit mask representing the struck layers  */
        TFC_strip      hits[18]; /*!< Hits assigned to proj                    */
	};

	struct TFC_projections{
        unsigned short int maxCnt; /*!< Maximum number of projections available*/
        unsigned short int curCnt; /*!< Current number of projections in use   */
        unsigned short int  xy[2]; /*!< Count of X/Y projections               */
        TFC_projection prjs[1000]; /*!< List of projections                    */
	};

    class FilterStatus : public DataObject{
    public:
        FilterStatus();
        FilterStatus(const unsigned int code, const int energey=0,
			         const int ids=0, const int xz=0, const int yz=0,
					 const int xy=0, const int *acdstatus=NULL,
					 const int *layerCode=NULL,
					 const TFC_projections *projections=NULL);
        virtual ~FilterStatus();
    
        ///Set the statuscode of the filter
        void set(const unsigned int code);
    
        ///Return the statuscode of the filter
        unsigned int get() const;
    
        ///Returns the 16 most significant bits of the filter code
        unsigned int getHigh() const;

        ///Returns the 16 least signifcat bits of the filter code
        unsigned int getLow() const;

        ///Set the Energy in CAL
        void setCalEnergy(const int energy);

        ///Returns the value stored in CalEnergy
        int getCalEnergy() const;

        ///Set the Code specifying the towers with triggers or possible triggers
        void setTcids(const int ids);
        ///Return the Code specifying the towers with triggers or possible triggers
        int getTcids() const;

        ///Set the ACD hit map results
        void setAcdMap(const int xz, const int yz, const int xy);
        ///Return the ACD hit map results
        void getAcdMap(int &xz, int &yz, int &xy) const;

        ///Set the ACD faces intersected by projections
        void setAcdStatus(const int tower, const int status);
        ///Return the ACD faces intersected by projections
        void getAcdStatus(int *copy) const;

        ///Set what layers were hit in each tower
        void setLayers(const int *layerCode);
        ///Return what layers were hit in each tower
        int *getLayers();
        
		///Set the projection of a specific tower
		void setProjection(const int tower,const TFC_projections projections);
		///Return the projections for a specific tower
		std::vector<TFC_projection> getProjection(const int tower);

        virtual std::ostream& fillStream(std::ostream &s) const;
        friend std::ostream& operator << (std::ostream &s, const FilterStatus &obj);
    private:
        ///Filter status code
        unsigned int m_status;
        ///Energy in CAL
        int m_calEnergy;
        ///Towers with triggers
        int m_tcids;
        ///ACD hit map
        int m_acd_xz;
        int m_acd_yz;
        int m_acd_xy;
        ///ACD faces
        int m_acdStatus[16];
        ///Layers hit in each tower
        int m_layers[16];
		///Projections for the towers
		std::vector<TFC_projection> m_prjs[16];
    };
	inline unsigned int FilterStatus::get() const{
		return m_status;
	}

	inline unsigned int FilterStatus::getHigh() const{
		return m_status>>15;
	}

	inline unsigned int FilterStatus::getLow() const{
		return m_status & 0x7FFF;
	}
};
#endif
