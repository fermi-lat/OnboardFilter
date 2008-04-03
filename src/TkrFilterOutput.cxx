#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "TkrFilterOutput.h"
#include "trackProj.h"
#include "GrbTrack.h"
#include "OnboardFilterTds/FilterStatus.h"
#include "OnboardFilterTds/Obf_TFC_prjs.h"

#include "GaudiKernel/MsgStream.h"

#include "EFC/EFC_edsFw.h"
#include "EDS/EBF_dir.h"
#include "EDS/EBF_cid.h"
#include "EDS/EBF_tkr.h"
#include "EDS/EDR_cal.h"
#include "EDS/EDR_tkrUnpack.h"
#include "EDS/FFS.h"
#include "EFC_DB/EFC_DB_sampler.h"
#include "EFC/../src/GFC_def.h"

// Constructor
TkrFilterOutput::TkrFilterOutput(void* cfgPrms) 
{
    m_cfgPrms   = reinterpret_cast<GFC*>(cfgPrms);

    m_trackProj = new trackProj(m_cfgPrms->cfg);
    m_grbTrack  = new GrbFindTrack(m_cfgPrms->cfg);

    return;
}
/* ---------------------------------------------------------------------- *//*!

  \fn  void extractFilterInfo(Stream *ostream, EDS_fwIxb *ixb)
                            
  \brief         Extracts info from the onboard filter algorithm. 
  \param ostream dummy var
  \param     ixb The information exchange block.

   This routine extracts both the results and the some of the intermediate
   info used by the filter in deciding if an event should be vetoed or not.
                                                                          */
/* ---------------------------------------------------------------------- */
void TkrFilterOutput::eovProcessing(void* callBackParm, EDS_fwIxb* ixb)
{
    // Get pointer to the TDS info
    ObfOutputCallBackParm* tdsPointers = reinterpret_cast<ObfOutputCallBackParm*>(callBackParm);

    // Recover pointers to the TDS objects
    OnboardFilterTds::FilterStatus* filterStatus = tdsPointers->m_filterStatus;
    OnboardFilterTds::TowerHits*    towerHits    = tdsPointers->m_towerHits;

    // Store the track information
    storeTrackInfo(ixb);

    // Get the best track information
    extractBestTrackInfo(tdsPointers->m_filterStatus, ixb);

    // Get the standard tracker information
    extractFilterTkrInfo(tdsPointers->m_filterStatus, ixb);

    // If we have a hit info block then get that too
    if (tdsPointers->m_towerHits) {
        extractTkrTwrHitInfo(tdsPointers->m_towerHits, ixb);
    }

    return;
   
}

void TkrFilterOutput::eorProcessing(MsgStream& log)
{
    // Recover 
    return;
}

void TkrFilterOutput::storeTrackInfo(EDS_fwIxb* ixb)
{
    // Check to see if there is any track information
    EDR_tkr* tkr = ixb->blk.evt.tkr;

    if (tkr->twrMap)
    {
        // Get the track projections
        TFC_prjs *prjs = (TFC_prjs *)ixb->blk.ptrs[EFC_EDS_FW_OBJ_K_TFC_PRJS];

        // Try mating XZ and YZ projections to form "best" tracks
        GrbTrack track = m_grbTrack->findTrack(prjs);

        // Test...
        OnboardFilterTds::Obf_TFC_prjs reconObjects(prjs);

        const TFC_prj& testprj = reconObjects.getPrj(0);

        if (reconObjects.getCurCnt() > 1)
        {
            const TFC_prj& testprj2 = reconObjects.getPrj(1);
            int j = 0;
        }
    }

    return;
}

void TkrFilterOutput::extractFilterTkrInfo(OnboardFilterTds::FilterStatus* filterStatus, EDS_fwIxb *ixb)
{
    int            cid;
    EDS_fwEvt     *evt = &ixb->blk.evt;
    const EBF_dir *dir =  evt->dir;

    //Get the layer energies
    filterStatus->setLayerEnergy(ixb->blk.evt.cal->layerEnergies);

    // Set Tkr first...
    EDR_tkr *tkr = ixb->blk.evt.tkr;
    
    if (tkr->twrMap)
    {
        const EDR_tkr& tkrRef = *tkr;
        filterStatus->setTkr(tkrRef);

        int                  tcids        = 0;
        int                  tids         = dir->redux.ctids << 16;
        int                  cids         = EBF_DIR_TEMS_TKR (dir->redux.ctids);
        const EBF_dirCtbDsc *contributors = dir->ctbs;
        unsigned int         twrMsk       = 0xffffffff;

        //  Get the projections 
        const TFC_prjs *prjs = (const TFC_prjs *)ixb->blk.ptrs[EFC_EDS_FW_OBJ_K_TFC_PRJS];
    
        if (twrMsk == -1) twrMsk  = prjs->twrMsk << 16;
        else              twrMsk &= 0xffff0000;

        filterStatus->setTmsk(twrMsk);

        // The following was in the previous code but I don't see where it was 
        // copied to TDS output... 
        int xCnt[16],yCnt[16];

        for (int idx=0;idx<16;idx++) {xCnt[idx]=0;yCnt[idx]=0;}

        while (twrMsk)
        {
            int towerId = FFS (twrMsk);
            const TFC_prjDir *dir = prjs->dir + towerId;

            xCnt[towerId]=dir->xCnt;
            yCnt[towerId]=dir->yCnt;
            //        printf("towerid %d dir->idx %d dir->xCnt %d yCnt %d\n",
            //            towerId,dir->idx,dir->xCnt,dir->yCnt);
      
            twrMsk = FFS_eliminate (twrMsk, towerId);
        }

        //   printf("twrMsk %x ebftwrmsk %x \n",twrMsk,EBF_DIR_TEMS_TKR (dir->redux.ctids));
        filterStatus->setTmsk(EBF_DIR_TEMS_TKR (dir->redux.ctids));

        //const TFC_prjs& prjsRef = *prjs;
        //filterStatus->setProjections(prjsRef);
        TFC_prjs* tdsPrjs = filterStatus->getProjections();

        // When FilterStatus is created/initialized, the memory locations are all zeroed. 
        // Because of this we can just copy in the non-pointer values to fill out the projections
        tdsPrjs->maxCnt = prjs->maxCnt;
        tdsPrjs->curCnt = prjs->curCnt;
        tdsPrjs->twrMsk = prjs->twrMsk;
        memcpy(tdsPrjs->dir, prjs->dir, 16*sizeof(TFC_prjDir));

        // Loop through and copy the valid projections (hopefully not many!)
        for(int idx = 0; idx < prjs->curCnt; idx++)
        {
            // projection just in case...
            if (idx > 999) break;

            TFC_prj* tdsPrj = &tdsPrjs->prjs[idx];

            memcpy(&tdsPrj->top, &prjs->prjs[idx].top, 
                2*sizeof(TFC_prjPrms)+3*sizeof(int)+4*sizeof(unsigned char)+sizeof(unsigned)+18*sizeof(TFC_hit));
        }

        int xy00Array[16];
        int xy11Array[16];
        int xy22Array[16];
        int xy33Array[16];
        int xcaptureArray[16];
        int ycaptureArray[16];

        /* Loop over all towers that may have tracker data */
        for (cid = EBF_CID_K_TEM; cid < EBF_CID_K_TEM + 16; cid++)
        {
            xy00Array[cid]     = 0;
            xy11Array[cid]     = 0;
            xy22Array[cid]     = 0;
            xy33Array[cid]     = 0;
            xcaptureArray[cid] = 0;
            ycaptureArray[cid] = 0;
        }

        int acd_xz = 0;
        int acd_yz = 0;
        int acd_xy = 0;

        /* Loop over all towers that may have tracker data */
        while (tids)
        {
            const EBF_dirCtbDsc *contributor;
            int                     xcapture;
            int                     ycapture;
            int         xy00, xy11, xy22, xy33;
            const EBF_tkr               *tkr;
            int                          cid;


            /*
            | Find the next tower with tracker hits and then eliminate it
            | from further consideration
            */
            cid  = FFS (tids);
            tids = FFS_eliminate (tids, cid);

            /* Locate the TEM contributor and its tracker data */
            contributor = &contributors[cid];
            tkr         = EBF__dirCtbTkrLocate (contributor);
  
            EBF_TKR_ACCEPTS_RIGHT_ORED_UNPACK (tkr->accepts, xcapture, ycapture);
            xcaptureArray[cid]=xcapture;
            ycaptureArray[cid]=ycapture;
            //printf("tids %x xcap %x ycap %x\n",cid,xcapture,ycapture);
            xy00 = xcapture & ycapture;
            if (xy00)
            {
                /*
                | Need to form the coincidence between the odd and even
                | portions. Want to form a variable which has layer n
                | and layer n+1 adjacent.
                |
                |             10fedcba9876543210
                |   xy      = 13579bdfh02468aceg  xy00
                |   xy >> 9 = .........13579bdfh  xy11
                |   xy << 1 = ........02468aceg_  xy22
                |   xy >> a = ..........13579bdf  xy33
                |
                |   triplet = xy00 & xy11 & xy22
                |           | xy33 & xy00 & xy11
                |
                | Note that when shifting down by 8, must eliminate the '1'
                | which appears in bit position 0.
                |
                | If one ORs the xy and xy << 1 (ignoring the bits
                | past bit 9, each bit position will contain the OR of
                | layer n-1 and layer n+1 relative to the xy >> 9
                | variable. Thus ANDing this variable| against this ORd
                | variable will produce a coincidence of layer N and
                | either layer N-1 or layer N+1, exactly what is needed.
                |
                |
                */
                xy11           = xy00 >> 9;
                xy22           = xy00 << 1;
                xy33           = xy00 >> 0xa;
                xy00Array[cid] = xy00;
                xy11Array[cid] = xy11;
                xy22Array[cid] = xy22;
                xy33Array[cid] = xy33;

                if ( xy00 & xy11  & xy22){
                    tcids |= 0x80000000 >> cid;
                } else if ( xy33 & xy00  & xy11) {
                    tcids |= 0x80000000 >> cid;
                } else if ((xy00 | xy22) & xy11) {
                    tcids |= 0x00008000 >> cid;
                }
            
                filterStatus->setTcids(tcids);
            }           
        }

        // Fill in the last of the variables
        filterStatus->setCapture(xcaptureArray,ycaptureArray);
        filterStatus->setXY(xy00Array,xy11Array,xy22Array,xy33Array);
    }

    return;
}

void TkrFilterOutput::extractBestTrackInfo(OnboardFilterTds::FilterStatus* filterStatus, EDS_fwIxb *ixb)
{
    // Local variables
    int    xHits   = 0;
    int    yHits   = 0;
    double slopeXZ = 0.0;
    double slopeYZ = 0.0;
    double intXZ   = 0.0;
    double intYZ   = 0.0;

    double trkSlpXZ = 0.;
    double trkSlpYZ = 0.;
    int    trkNX    = 0;
    int    trkNY    = 0;

    // Get the projections 
    TFC_prjs *prjs = (TFC_prjs *)ixb->blk.ptrs[EFC_EDS_FW_OBJ_K_TFC_PRJS];

    // use the trackProj class to do the real work here... but only if data...
    //if (filterStatus->getTcids()) 
    if (prjs->curCnt > 0) 
    {
        m_trackProj->execute(prjs, xHits, yHits, slopeXZ, slopeYZ, intXZ, intYZ);
        GrbTrack track = m_grbTrack->findTrack(prjs);

        if (track.valid())
        {
            trkNX = track.getProjectionX()->nhits;
            trkNY = track.getProjectionY()->nhits;
            trkSlpXZ = static_cast<double>(track.get_dxzi()) / static_cast<double>(track.get_dzi());
            trkSlpYZ = static_cast<double>(track.get_dyzi()) / static_cast<double>(track.get_dzi());
            int j = 0;
        }
    }

    filterStatus->setBestTrack(xHits, yHits, slopeXZ, slopeYZ, intXZ, intYZ, trkNX, trkNY, trkSlpXZ, trkSlpYZ);

    return;
}

void TkrFilterOutput::extractTkrTwrHitInfo(OnboardFilterTds::TowerHits* towerHits, EDS_fwIxb *ixb)
{
    // Extract the directory pointer
    EDS_fwEvt     *evt    = &ixb->blk.evt;
    const EBF_dir *dir    =  evt->dir;
    EDR_tkr       *tkr    = ixb->blk.evt.tkr;
    unsigned int   twrMsk = 0xffff0000;

    EDR_tkrUnpack (tkr, dir, twrMsk);
    EDR_tkrTower *ttrs = tkr->twrs;
//
    // Look over towers
    while (twrMsk)
    {
        int towerId = FFS (twrMsk);

        EDR_tkrTower *ttr = ttrs + towerId;

        towerHits->m_hits[towerId].lcnt[0]      = ttr->lexycnts[0];
        towerHits->m_hits[towerId].lcnt[0]      = ttr->lexycnts[0];
        towerHits->m_hits[towerId].lcnt[1]      = ttr->lexycnts[0];
        towerHits->m_hits[towerId].layerMaps[0] = ttr->layerMaps[0];
        towerHits->m_hits[towerId].layerMaps[1] = ttr->layerMaps[1];

        for(int layers=0; layers<36; layers++)
        {
            if (ttr->layers[layers].cnt > 0)
            {
                towerHits->m_hits[towerId].cnt[layers] = ttr->layers[layers].cnt;
                towerHits->m_hits[towerId].beg[layers] = 
                    (TFC_hit*)malloc(towerHits->m_hits[towerId].cnt[layers]*sizeof(TFC_hit));
            
                memcpy(towerHits->m_hits[towerId].beg[layers],
                   ttr->layers[layers].beg,
                   towerHits->m_hits[towerId].cnt[layers]*sizeof(TFC_hit));
            }
        }

        twrMsk = FFS_eliminate (twrMsk, towerId);
    }

    return;
}
