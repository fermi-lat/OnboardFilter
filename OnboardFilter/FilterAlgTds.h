#ifndef Event_FilterAlg_H
#define Event_FilterAlg_H

#include <iostream>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"

#include "Event/TopLevel/Definitions.h"
#include "Event/TopLevel/EventModel.h"

/**
 * @class FilterAlgData
 *
 * @brief TDS for storing an event in a format similar to ebf
 *
 * The data is stored as one continuos string of bytes
 * No attempt is made to verify that the data stored is correctly
 * formated ebf.
 * $Header: /nfs/slac/g/glast/ground/cvs/FilterAlgTds/FilterAlgTds/FilterAlgTds.h,v 1.8 2003/08/17 22:27:23 burnett Exp $
 */

//extern const CLID& CLID_FilterAlgTds;

namespace FilterAlgTds{
    class FilterAlgData : public DataObject{
    public:
        FilterAlgData();
        virtual ~FilterAlgData();

        int getVetoWord() const;
        void setVetoWord(const int vetoword);
       
        virtual std::ostream& fillStream(std::ostream &s) const;
        friend std::ostream& operator << (std::ostream &s, const FilterAlgData& obj);
    private:   
        int m_vetoword;
	};
    //inline stuff for client
    inline FilterAlgData::FilterAlgData(){ /*m_data=0; m_length=0;*/}

    inline int FilterAlgData::getVetoWord() const{
        return m_vetoword;
    }

    inline void FilterAlgData::setVetoWord(const int vetoword){
        m_vetoword=vetoword;
    }

    inline std::ostream& FilterAlgData::fillStream( std::ostream &s) const{
      //if(m_length>0)
      //  s.write(m_data,m_length);
      return s;
    }

	inline FilterAlgData::~FilterAlgData(){
      //if(m_data!=NULL)
        //delete[] m_data;
    }

    inline std::ostream& operator << (std::ostream& s, const FilterAlgData& obj){
      return obj.fillStream(s);
    }

}// namespace
#endif
