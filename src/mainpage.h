/** @mainpage package OnboardFilter
 *
 * @section intro Introduction
 * This package provides drivers for the DFC code. This driver allows the DFC code to
 * run in Gleam. This package requires EbfWriter to convert Events into a format similar
 * to EBF.
 * <B>Filter Status Code</b>
 * Refer to file: DFC_status.
 * The status word consists of 32 bits.  Bits 0-14 indicate the status of the filtering 
 * and whether certain events had taken place (was a tile struck, was a track found, 
 * etc.)  Bits 15-30 contain 16 reasons why an Event could be vetoed.  If any one of 
 * these bits is ON, bit 31 will also be ON.  Bit 31 indicates that there was a veto 
 * somewhere.
 *
 @verbatim
  Bit mask name                                Bit     Comment
  DFC_M_STATUS_ACD                             0       ACD was analyzed
  DFC_M_STATUS_DIR                             1       DIR was decoded	
  DFC_M_STATUS_ATF                             2       ACD/TKR veto was analyzed
  DFC_M_STATUS_CAL1                            3       CAL was analyzed, phase 1
  
  DFC_M_STATUS_TKR                             4       TKR finding was done
  DFC_M_STATUS_ACD_TOP                         5       ACD top tile struck
  DFC_M_STATUS_ACD_SIDE                        6       ACD side tile struck
  DFC_M_STATUS_ACD_SIDE_FILTER                 7       ACD filter tile struck
  
  DFC_M_STATUS_TKR_POSSIBLE                    8       Possible track
  DFC_M_STATUS_TKR_TRIGGER                     9       Have a 3-in-a-row trigger
  DFC_M_STATUS_CAL_LO                          10      Cal Lo Trigger
  DFC_M_STATUS_CAL_HI			       11      Cal Hi Trigger
  
  DFC_M_STATUS_TKR_EQ_1                        12      Exactly 1 track
  DFC_M_STATUS_TKR_GE_2                        13      Greater or equal 2 tracks
  DFC_M_STATUS_TKR_THROTTLE                    14      Throttle bit set
  DFC_M_STATUS_TKR_LT_2_ELO                    15      Low energy, no 2 track evidence
  
  DFC_M_STATUS_TKR_SKIRT                       16      Event into the skirt region
  DFC_M_STATUS_TKR_EQ_0                        17      No tracks
  DFC_M_STATUS_TKR_ROW2                        18      Track Row 2 match
  DFC_M_STATUS_TKR_ROW01                       19      Track Row 0 or 1 match
  
  DFC_M_STATUS_TKR_TOP                         20      Track Top match
  DFC_M_STATUS_ZBOTTOM                         21      No tracks into CAL with energys
  DFC_M_STATUS_EL0_ETOT_90                     22      E layer 0/ETOT > 0.90
  DFC_M_STATUS_EL0_ETOT_01                     23      E layer 0/ETOT < 0.01
 
  DFC_M_STATUS_SIDE                            24      Event has a side face veto
  DFC_M_STATUS_TOP                             25      Event has a top face veto
  DFC_M_STATUS_SPLASH_1                        26      Event has a splash veto
  DFC_M_STATUS_E350_FILTER_TILE                27      Event <350Mev + Filter tiles
 
  DFC_M_STATUS_E0_TILE                         28      Event 0 energy + tile hit
  DFC_M_STATUS_SPLASH_0                        29      Event has a splash veto
  DFC_M_STATUS_NOCALLO_FILTER_TILE             30      No CAL LO trigger + filter tile
  DFC_M_STATUS_VETOED                          31      Any veto
 @endverbatim
 *
 * @image html OnboardFilter.jpg
 * <hr>
 * @section Properties Properties
 * @param OnboardFilter.passThrough
 *  Parameter to disable/enable "Pass through mode" of the filter
 *  default value 1 (enabled)
 * @param OnboardFilter.mask
 * <hr>
 * @section jobOptions jobOptions
 * @include jobOptions.txt
 * <hr>
 * @section notes release.notes
 * release.notes
 * <hr>
 * @section requirements requirements
 * @include requirements
 * <hr>
 * @todo add real unit test
 */
