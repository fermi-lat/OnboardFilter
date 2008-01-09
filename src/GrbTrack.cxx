#include "GrbTrack.h"

#include "EFC/TFC_prjDef.h"
#include "EDS/EDR_tkr.h"
#include "EDS/FFS.h"


#ifndef NULL
#define NULL ((void *)(0))
#endif

GrbFindTrack::GrbFindTrack()
{
    m_strip_pitch = 228; // From TKR_STRIP_PITCH = TKR_STRIP_PITCH_MM * 1000 + 0.5
    m_dz_scale    = 2 * 2048;
    m_dxy_scale   = m_strip_pitch * 2 * 2048;

    return;
}

GrbFindTrack::~GrbFindTrack()
{
    return;
}

GrbTrack GrbFindTrack::findTrack(EDS_fwIxb* ixb)
{
    //
    // The following code was modelled on that in the function grb_process which
    // can be found in the GRBP_server.c module in the GRBP package. 
    //

    GrbTrack           grbTrack = GrbTrack();
    EBF_dir                *dir = ixb->blk.evt.dir;
    EDR_tkr                *tkr = ixb->blk.evt.tkr;
    TFC_prjs       *projections = reinterpret_cast<TFC_prjs*>(ixb->blk.ptrs[EFC_EDS_FW_OBJ_K_TFC_PRJS]);
    unsigned int   topLayerMask = projections_classify (projections, dir, tkr);

    if ( (topLayerMask & 0xffff0000) & (topLayerMask << 16) )
    {
        GRBP_prjs grbp_prjs[2];

        int       dxi;
        int       dyi;
        int       dzi;
        int        nx;
        int        ny;

        /*
         | Need to compute
         |    dx/dz
         |    dy/dz
        */
        
        /* Find the X best projections */
        prjsSelect (&grbp_prjs[0], 
                    topLayerMask & 0xffff0000, 
                    projections->top[0]);        

        nx  = grbp_prjs[0].cnt;
        dxi = dcos_prepare (grbp_prjs[0].prjs, nx, m_dxy_scale);

        /* Find the Y best projections */
        prjsSelect (&grbp_prjs[1],
                    topLayerMask <<  16, 
                    projections->top[1]);
        ny  = grbp_prjs[1].cnt;
        dyi = dcos_prepare (grbp_prjs[1].prjs, ny, m_dxy_scale);
        dzi = m_dz_scale;

        grbTrack = GrbTrack(grbp_prjs[0].prjs[0], grbp_prjs[1].prjs[0], dxi, dyi, dzi);
    }

    return grbTrack;
}

unsigned int GrbFindTrack::projections_classify(TFC_prjs  *projections,
                                            EBF_dir           *dir,
                                            EDR_tkr           *tkr)
{
    int                          idx = 0;
    TFC_prjDir                *pdirs = projections->dir;
    EDR_tkrTower               *ttrs = tkr->twrs;
    TFC_prj                     *prj = projections->prjs;
    TFC_prj                  *prjmax = prj + sizeof (projections->prjs) / sizeof (projections->prjs[0]);
    unsigned int         topLayerMsk = 0;
    unsigned int                tmsk = 0;

    //tmsk        = EBF_DIR_TEMS_TKR (dir->redux.ctids);
    //topLayerMsk = 0;

    //EBF_dirReassemble (dir, -1);
    //EDR_tkrUnpack     (tkr, dir, tmsk);

    //pdirs  = projections->dir;
    //prj    = projections->prjs;
    //prjmax = prj + sizeof (projections->prjs) / sizeof (projections->prjs[0]);
    //ttrs   = tkr->twrs;

    prjList_init (projections->top);

    tmsk = projections->twrMsk << 16;
    while (tmsk)
    {
        int                   tower;
        TFC_prjDir            *pdir;
        int                     beg;
        int                     end;

        tower = FFS (tmsk);
        tmsk  = FFS_eliminate (tmsk, tower);


        /* Get the projection directory for this tower */
        pdir  = pdirs + tower;


        /* Compute beginning and end X projection */
        beg  = pdir->idx;
        end  = beg + pdir->xCnt;


        /* Accumulate X list */
        for (idx = beg; idx < end; idx++)
        {
            int top_layer = 17 - prj[idx].top.layer;
            TFC__prjListInsert (&projections->top[0][top_layer], 
                                &prj[idx].topNode);
            topLayerMsk |= FFS_mask (top_layer); 
        }


        /* Y projections start where the X left off */
        end += pdir->yCnt;
        for (; idx < end; idx++)
        {
            int top_layer = 17 - prj[idx].top.layer;
            TFC__prjListInsert (&projections->top[1][top_layer],
                                &prj[idx].topNode);
            topLayerMsk |= FFS_mask (top_layer) >> 16;
        }

    }


    /* Find the top layer and only consider tracks within two */
    {
        unsigned int tmp;
        int        layer;
        tmp   = topLayerMsk | (topLayerMsk << 16);
        layer = FFS      (tmp);
        layer = FFS_mask (layer);
        tmp   = (layer) | (layer >> 1) | (layer >> 2);
        topLayerMsk &= (tmp | tmp >> 16);
    }


    return topLayerMsk;
}

int GrbFindTrack::dcos_prepare(const TFC_prj *prjs[2],
                           int                cnt,
                           unsigned int     scale)
{
    int dcos;

    dcos  = scale * prjs[0]->top.dxy;
    dcos /= prjs[0]->top.dz;

    /*
    if (cnt == 2) 
    {
        dcos  += scale * prjs[1]->top.dxy / prjs[1]->top.dz;
        dcos  /= 2;
    }
    */


    return dcos;
}

int GrbFindTrack::prjsSelect(GRBP_prjs     *grbp_prjs,
                         unsigned int      lyrMsk,
                         const TFC_prjList *lists)
{
    //
    // This code is copied directly from the module GRBP_prjsSelect.c in the GRBP package
    // Tracy Usher 1/8/08
    //
    int             cnt = 0;
    int              n0 = 0;
    int              n1 = 0;
    const TFC_prj* prj0 = 0;
    const TFC_prj* prj1 = 0;


    //printf ("LyrMsk = %8.8x\n", lyrMsk);
    while (lyrMsk)
    {
        int layer;
        const TFC_prjList *list;
        const TFC_prjNode *node;

        layer   = FFS (lyrMsk);
        lyrMsk  = FFS_eliminate (lyrMsk, layer);
        list    = lists + layer;
        node    = TFC__prjListFirst (list);
        //printf ("Layer = %u\n", layer + 2);

        do
        {
            const TFC_prj *prj = TFC__prjFromTopNode (node);
            int         nrange = prj->top.layer - prj->bot.layer;

            /*
            printf ("Twr Top:Bot %x:%u %x:%u nrange:n0 %u %u:%u\n", 
                    prj->top.tower,
                    prj->top.layer,
                    prj->bot.tower,
                    prj->bot.layer,
                    nrange, n0, n1);
            */
            if (nrange >= n0) 
            {
                n1 = n0;      prj1 = prj0;
                n0 = nrange;  prj0 = prj;
            }
            else if (nrange >= n1)
            {
                n1 = nrange; prj1 = prj;
            }
        }
        while ((node = TFC__prjListNext (list, node)));

    }

    grbp_prjs->cnt     = cnt = n1==0 ? 1 : 2;
    grbp_prjs->n[0]    = n0;
    grbp_prjs->n[1]    = n1;
    grbp_prjs->prjs[0] = prj0;
    grbp_prjs->prjs[1] = prj1;

    return cnt;
}

void GrbFindTrack::prjList_init (TFC_prjList lists[2][16])
{
    int idx;
    for (idx = 0; idx < 16; idx++)
    {
        TFC__prjListInit (&lists[0][idx]);
        TFC__prjListInit (&lists[1][idx]);
    }

    return;
}
