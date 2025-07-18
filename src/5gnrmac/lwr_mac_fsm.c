 /*******************************************************************************
 ################################################################################
 #   Copyright (c) [2017-2019] [Radisys]                                        #
 #                                                                              #
 #   Licensed under the Apache License, Version 2.0 (the "License");            #
 #   you may not use this file except in compliance with the License.           #
 #   You may obtain a copy of the License at                                    #
 #                                                                              #
 #       http://www.apache.org/licenses/LICENSE-2.0                             #
 #                                                                              #
 #   Unless required by applicable law or agreed to in writing, software        #
 #   distributed under the License is distributed on an "AS IS" BASIS,          #
 #   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
 #   See the License for the specific language governing permissions and        #
 #   limitations under the License.                                             #
 ################################################################################
 *******************************************************************************/


/* header include files -- defines (.h) */
#include "common_def.h"
#include "lrg.h"
#include "lrg.x"
#include "du_app_mac_inf.h"
#include "mac_sch_interface.h"
#include "lwr_mac_upr_inf.h"
#include "mac.h"
#include "lwr_mac.h"
#ifdef INTEL_FAPI
#include "nr5g_fapi_internal.h"
#include "fapi_vendor_extension.h"
#endif
#ifdef INTEL_WLS_MEM
#include "wls_lib.h"
#endif
#include "lwr_mac_fsm.h"
#include "lwr_mac_phy.h"
#include "lwr_mac_utils.h"
#include "mac_utils.h"
#include "nfapi_interface.h"

#define MIB_SFN_BITMASK 0xFC
#define PDCCH_PDU_TYPE 0
#define PDSCH_PDU_TYPE 1
#define SSB_PDU_TYPE 3
#define PRACH_PDU_TYPE 0
#define PUSCH_PDU_TYPE 1
#define PUCCH_PDU_TYPE 2
#define PDU_PRESENT 1
#define SET_MSG_LEN(x, size) x += size

/* Global variables */
LwrMacCb lwrMacCb;

extern uint8_t UnrestrictedSetNcsTable[MAX_ZERO_CORR_CFG_IDX];
void fapiMacConfigRsp(uint16_t cellId);
uint16_t sendTxDataReq(SlotTimingInfo currTimingInfo, MacDlSlot *dlSlot, p_fapi_api_queue_elem_t prevElem, fapi_vendor_tx_data_req_t *vendorTxDataReq);
uint16_t fillUlTtiReq(SlotTimingInfo currTimingInfo, p_fapi_api_queue_elem_t prevElem, fapi_vendor_ul_tti_req_t* vendorUlTti);
uint16_t fillUlDciReq(SlotTimingInfo currTimingInfo, p_fapi_api_queue_elem_t prevElem, fapi_vendor_ul_dci_req_t *vendorUlDciReq);
uint8_t lwr_mac_procStopReqEvt(SlotTimingInfo slotInfo, p_fapi_api_queue_elem_t  prevElem, fapi_stop_req_vendor_msg_t *vendorMsg);

void lwrMacLayerInit(Region region, Pool pool)
{
#ifdef INTEL_WLS_MEM
   uint8_t idx;
#endif

   memset(&lwrMacCb, 0, sizeof(LwrMacCb));
   lwrMacCb.region = region;
   lwrMacCb.pool = pool;
   lwrMacCb.clCfgDone = TRUE;
   lwrMacCb.numCell = 0;
   lwrMacCb.phyState = PHY_STATE_IDLE;

#ifdef INTEL_WLS_MEM
   /* Initializing WLS free mem list */
   lwrMacCb.phySlotIndCntr = 1;
   for(idx = 0; idx < WLS_MEM_FREE_PRD; idx++)
   {
      cmLListInit(&wlsBlockToFreeList[idx]);
   }
#endif
}

/*******************************************************************
 *
 * @brief Handles Invalid Request Event
 *
 * @details
 *
 *    Function : lwr_mac_procInvalidEvt
 *
 *    Functionality:
 *         - Displays the PHY state when the invalid event occurs
 *
 * @params[in]
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t lwr_mac_procInvalidEvt(void *msg)
{
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : INVALID_EVENT\n");
#endif
   DU_LOG("\nERROR  -->  LWR_MAC: Error Indication Event[%d] received in state [%d]", lwrMacCb.event, lwrMacCb.phyState);
   return ROK;
}

#ifdef OAI_TESTING
uint16_t reverseBytes16(uint16_t num) {
    return (num >> 8) | (num << 8);
}

uint32_t reverseBytes32(uint32_t num) {
    return ((num >> 24) & 0x000000FF) |
           ((num >> 8) & 0x0000FF00) |
           ((num << 8) & 0x00FF0000) |
           ((num << 24) & 0xFF000000);
}
#endif

#ifdef INTEL_FAPI
/*******************************************************************
 *
 * @brief Fills FAPI message header
 *
 * @details
 *
 *    Function : fillMsgHeader
 *
 *    Functionality:
 *         -Fills FAPI message header
 *
 * @params[in] Pointer to header
 *             Number of messages
 *             Messae Type
 *             Length of message
 * @return void
 *
 * ****************************************************************/
void fillMsgHeader(fapi_msg_t *hdr, uint16_t msgType, uint32_t msgLen)
{
   memset(hdr, 0, sizeof(fapi_msg_t));
#ifdef OAI_TESTING 
   hdr->msg_id = reverseBytes16(msgType);
   hdr->length = reverseBytes32(msgLen);
#else
   hdr->msg_id = (msgType);
   hdr->length = (msgLen);
#endif
}

/*******************************************************************
 *
 * @brief Fills FAPI Config Request message header
 *
 * @details
 *
 *    Function : fillTlvs
 *
 *    Functionality:
 *         -Fills FAPI Config Request message header
 *
 * @params[in] Pointer to TLV
 *             Tag
 *             Length
 *             Value
 *             MsgLen
 * @return void
 *
 * ****************************************************************/
void fillTlvs(fapi_uint32_tlv_t *tlv, uint16_t tag, uint16_t length,
      uint32_t value, uint32_t *msgLen)
{
#ifdef OAI_TESTING 
   tlv->tl.tag    = reverseBytes16(tag);
   tlv->tl.length = reverseBytes16(length);
   tlv->value     = reverseBytes32(value);
#else
   tlv->tl.tag    = (tag);
   tlv->tl.length = (length);
   tlv->value     = (value);
#endif
   *msgLen        = *msgLen + sizeof(tag) + sizeof(length) + length;

}
/*******************************************************************
 *
 * @brief fills the cyclic prefix by comparing the bitmask
 *
 * @details
 *
 *    Function : fillCyclicPrefix
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's cyclic prefix.
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ********************************************************************/
void fillCyclicPrefix(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_NORMAL_CYCLIC_PREFIX_MASK) == FAPI_NORMAL_CYCLIC_PREFIX_MASK)
   {
      (*cellPtr)->cyclicPrefix   = NORMAL_CYCLIC_PREFIX_MASK;
   }
   else if((value & FAPI_EXTENDED_CYCLIC_PREFIX_MASK) == FAPI_EXTENDED_CYCLIC_PREFIX_MASK)
   {
      (*cellPtr)->cyclicPrefix   = EXTENDED_CYCLIC_PREFIX_MASK;
   }
   else
   {
      (*cellPtr)->cyclicPrefix = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the subcarrier spacing of Downlink by comparing the bitmask
 *
 * @details
 *
 *    Function : fillSubcarrierSpaceDl
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's subcarrier spacing in DL
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 * ****************************************************************/

void fillSubcarrierSpaceDl(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_15KHZ_MASK) == FAPI_15KHZ_MASK)
   {
      (*cellPtr)->supportedSubcarrierSpacingDl = SPACING_15_KHZ;
   }
   else if((value & FAPI_30KHZ_MASK) == FAPI_30KHZ_MASK)
   {
      (*cellPtr)->supportedSubcarrierSpacingDl = SPACING_30_KHZ;
   }
   else if((value & FAPI_60KHZ_MASK) == FAPI_60KHZ_MASK)
   {
      (*cellPtr)->supportedSubcarrierSpacingDl = SPACING_60_KHZ;
   }
   else if((value & FAPI_120KHZ_MASK) == FAPI_120KHZ_MASK)
   {
      (*cellPtr)->supportedSubcarrierSpacingDl = SPACING_120_KHZ;
   }
   else
   {
      (*cellPtr)->supportedSubcarrierSpacingDl = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the downlink bandwidth by comparing the bitmask
 *
 * @details
 *
 *    Function : fillBandwidthDl
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *         -fills the cellPtr's DL Bandwidth
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 * ****************************************************************/

void fillBandwidthDl(uint16_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_5MHZ_BW_MASK) == FAPI_5MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_5MHZ;
   }
   else if((value & FAPI_10MHZ_BW_MASK) == FAPI_10MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_10MHZ;
   }
   else if((value & FAPI_15MHZ_BW_MASK) == FAPI_15MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_15MHZ;
   }
   else if((value & FAPI_20MHZ_BW_MASK) == FAPI_20MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_20MHZ;
   }
   else if((value & FAPI_40MHZ_BW_MASK) == FAPI_40MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_40MHZ;
   }
   else if((value & FAPI_50MHZ_BW_MASK) == FAPI_50MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_50MHZ;
   }
   else if((value & FAPI_60MHZ_BW_MASK) == FAPI_60MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_60MHZ;
   }
   else if((value & FAPI_70MHZ_BW_MASK) == FAPI_70MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_70MHZ;
   }
   else if((value & FAPI_80MHZ_BW_MASK) == FAPI_80MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_80MHZ;
   }
   else if((value & FAPI_90MHZ_BW_MASK) == FAPI_90MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_90MHZ;
   }
   else if((value & FAPI_100MHZ_BW_MASK) == FAPI_100MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_100MHZ;
   }
   else if((value & FAPI_200MHZ_BW_MASK) == FAPI_200MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_200MHZ;
   }
   else if((value & FAPI_400MHZ_BW_MASK) == FAPI_400MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthDl = BW_400MHZ;
   }
   else
   {
      (*cellPtr)->supportedBandwidthDl = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the subcarrier spacing of Uplink by comparing the bitmask
 *
 * @details
 *
 *    Function : fillSubcarrierSpaceUl
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *         -fills cellPtr's subcarrier spacing in UL
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 * ****************************************************************/

void fillSubcarrierSpaceUl(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_15KHZ_MASK) == FAPI_15KHZ_MASK)
   {
      (*cellPtr)->supportedSubcarrierSpacingsUl = SPACING_15_KHZ;
   }
   else if((value & FAPI_30KHZ_MASK) == FAPI_30KHZ_MASK)
   {
      (*cellPtr)->supportedSubcarrierSpacingsUl = SPACING_30_KHZ;
   }
   else if((value & FAPI_60KHZ_MASK) == FAPI_60KHZ_MASK)
   {
      (*cellPtr)->supportedSubcarrierSpacingsUl = SPACING_60_KHZ;
   }
   else if((value & FAPI_120KHZ_MASK) == FAPI_120KHZ_MASK)
   {
      (*cellPtr)->supportedSubcarrierSpacingsUl = SPACING_120_KHZ;
   }
   else
   {
      (*cellPtr)->supportedSubcarrierSpacingsUl = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the uplink bandwidth by comparing the bitmask
 *
 * @details
 *
 *    Function : fillBandwidthUl
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's UL Bandwidth
 *
 *
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 *
 * ****************************************************************/

void fillBandwidthUl(uint16_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_5MHZ_BW_MASK) == FAPI_5MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_5MHZ;
   }
   else if((value & FAPI_10MHZ_BW_MASK) == FAPI_10MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_10MHZ;
   }
   else if((value & FAPI_15MHZ_BW_MASK) == FAPI_15MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_15MHZ;
   }
   else if((value & FAPI_20MHZ_BW_MASK) == FAPI_20MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_20MHZ;
   }
   else if((value & FAPI_40MHZ_BW_MASK) == FAPI_40MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_40MHZ;
   }
   else if((value & FAPI_50MHZ_BW_MASK) == FAPI_50MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_50MHZ;
   }
   else if((value & FAPI_60MHZ_BW_MASK) == FAPI_60MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_60MHZ;
   }
   else if((value & FAPI_70MHZ_BW_MASK) == FAPI_70MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_70MHZ;
   }
   else if((value & FAPI_80MHZ_BW_MASK) == FAPI_80MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_80MHZ;
   }
   else if((value & FAPI_90MHZ_BW_MASK) == FAPI_90MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_90MHZ;
   }
   else if((value & FAPI_100MHZ_BW_MASK) == FAPI_100MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_100MHZ;
   }
   else if((value & FAPI_200MHZ_BW_MASK) == FAPI_200MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_200MHZ;
   }
   else if((value & FAPI_400MHZ_BW_MASK) == FAPI_400MHZ_BW_MASK)
   {
      (*cellPtr)->supportedBandwidthUl = BW_400MHZ;
   }
   else
   {
      (*cellPtr)->supportedBandwidthUl = INVALID_VALUE;
   }
}
/*******************************************************************
 *
 * @brief fills the CCE maping by comparing the bitmask
 *
 * @details
 *
 *    Function : fillCCEmaping
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's CCE Mapping Type
 *
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 * ****************************************************************/

void fillCCEmaping(uint8_t value,  ClCellParam **cellPtr)
{
   if ((value & FAPI_CCE_MAPPING_INTERLEAVED_MASK) == FAPI_CCE_MAPPING_INTERLEAVED_MASK)
   {
      (*cellPtr)->cceMappingType = CCE_MAPPING_INTERLEAVED_MASK;
   }
   else if((value & FAPI_CCE_MAPPING_INTERLEAVED_MASK) == FAPI_CCE_MAPPING_NONINTERLVD_MASK)
   {
      (*cellPtr)->cceMappingType = CCE_MAPPING_NONINTERLVD_MASK;
   }
   else
   {
      (*cellPtr)->cceMappingType = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PUCCH format by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPucchFormat
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's pucch format
 *
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 * ****************************************************************/

void fillPucchFormat(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_FORMAT_0_MASK) == FAPI_FORMAT_0_MASK)
   {
      (*cellPtr)->pucchFormats    = FORMAT_0;
   }
   else if((value & FAPI_FORMAT_1_MASK) == FAPI_FORMAT_1_MASK)
   {
      (*cellPtr)->pucchFormats    = FORMAT_1;
   }
   else if((value & FAPI_FORMAT_2_MASK) == FAPI_FORMAT_2_MASK)
   {
      (*cellPtr)->pucchFormats    = FORMAT_2;
   }
   else if((value & FAPI_FORMAT_3_MASK) == FAPI_FORMAT_3_MASK)
   {
      (*cellPtr)->pucchFormats    = FORMAT_3;
   }
   else if((value & FAPI_FORMAT_4_MASK) == FAPI_FORMAT_4_MASK)
   {
      (*cellPtr)->pucchFormats    = FORMAT_4;
   }
   else
   {
      (*cellPtr)->pucchFormats    = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PDSCH Mapping Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPdschMappingType
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PDSCH MappingType
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 * ****************************************************************/

void fillPdschMappingType(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PDSCH_MAPPING_TYPE_A_MASK) == FAPI_PDSCH_MAPPING_TYPE_A_MASK)
   {
      (*cellPtr)->pdschMappingType = MAPPING_TYPE_A;
   }
   else if((value & FAPI_PDSCH_MAPPING_TYPE_B_MASK) == FAPI_PDSCH_MAPPING_TYPE_B_MASK)
   {
      (*cellPtr)->pdschMappingType = MAPPING_TYPE_B;
   }
   else
   {
      (*cellPtr)->pdschMappingType = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PDSCH Allocation Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPdschAllocationType
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PDSCH AllocationType
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 * ****************************************************************/

void fillPdschAllocationType(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PDSCH_ALLOC_TYPE_0_MASK) == FAPI_PDSCH_ALLOC_TYPE_0_MASK)
   {
      (*cellPtr)->pdschAllocationTypes = ALLOCATION_TYPE_0;
   }
   else if((value & FAPI_PDSCH_ALLOC_TYPE_1_MASK) == FAPI_PDSCH_ALLOC_TYPE_1_MASK)
   {
      (*cellPtr)->pdschAllocationTypes = ALLOCATION_TYPE_1;
   }
   else
   {
      (*cellPtr)->pdschAllocationTypes = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PDSCH PRB Mapping Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPrbMappingType
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PRB Mapping Type
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/
void fillPrbMappingType(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PDSCH_VRB_TO_PRB_MAP_NON_INTLV_MASK) == FAPI_PDSCH_VRB_TO_PRB_MAP_NON_INTLV_MASK)
   {
      (*cellPtr)->pdschVrbToPrbMapping = VRB_TO_PRB_MAP_NON_INTLV;
   }
   else if((value & FAPI_PDSCH_VRB_TO_PRB_MAP_INTLVD_MASK) == FAPI_PDSCH_VRB_TO_PRB_MAP_INTLVD_MASK)
   {
      (*cellPtr)->pdschVrbToPrbMapping = VRB_TO_PRB_MAP_INTLVD;
   }
   else
   {
      (*cellPtr)->pdschVrbToPrbMapping = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PDSCH DmrsConfig Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPdschDmrsConfigType
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's DmrsConfig Type
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPdschDmrsConfigType(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PDSCH_DMRS_CONFIG_TYPE_1_MASK) == FAPI_PDSCH_DMRS_CONFIG_TYPE_1_MASK)
   {
      (*cellPtr)->pdschDmrsConfigTypes = DMRS_CONFIG_TYPE_1;
   }
   else if((value & FAPI_PDSCH_DMRS_CONFIG_TYPE_2_MASK) == FAPI_PDSCH_DMRS_CONFIG_TYPE_2_MASK)
   {
      (*cellPtr)->pdschDmrsConfigTypes = DMRS_CONFIG_TYPE_2;
   }
   else
   {
      (*cellPtr)->pdschDmrsConfigTypes = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PDSCH DmrsLength by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPdschDmrsLength
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PdschDmrsLength
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/
void fillPdschDmrsLength(uint8_t value, ClCellParam **cellPtr)
{
   if(value == FAPI_PDSCH_DMRS_MAX_LENGTH_1)
   {
      (*cellPtr)->pdschDmrsMaxLength = DMRS_MAX_LENGTH_1;
   }
   else if(value == FAPI_PDSCH_DMRS_MAX_LENGTH_2)
   {
      (*cellPtr)->pdschDmrsMaxLength = DMRS_MAX_LENGTH_2;
   }
   else
   {
      (*cellPtr)->pdschDmrsMaxLength = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PDSCH Dmrs Additional Pos by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPdschDmrsAddPos
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's Pdsch DmrsAddPos
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPdschDmrsAddPos(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_DMRS_ADDITIONAL_POS_0_MASK) == FAPI_DMRS_ADDITIONAL_POS_0_MASK)
   {
      (*cellPtr)->pdschDmrsAdditionalPos = DMRS_ADDITIONAL_POS_0;
   }
   else if((value & FAPI_DMRS_ADDITIONAL_POS_1_MASK) == FAPI_DMRS_ADDITIONAL_POS_1_MASK)
   {
      (*cellPtr)->pdschDmrsAdditionalPos = DMRS_ADDITIONAL_POS_1;
   }
   else if((value & FAPI_DMRS_ADDITIONAL_POS_2_MASK) == FAPI_DMRS_ADDITIONAL_POS_2_MASK)
   {
      (*cellPtr)->pdschDmrsAdditionalPos = DMRS_ADDITIONAL_POS_2;
   }
   else if((value & FAPI_DMRS_ADDITIONAL_POS_3_MASK) == FAPI_DMRS_ADDITIONAL_POS_3_MASK)
   {
      (*cellPtr)->pdschDmrsAdditionalPos = DMRS_ADDITIONAL_POS_3;
   }
   else
   {
      (*cellPtr)->pdschDmrsAdditionalPos = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the Modulation Order in DL by comparing the bitmask
 *
 * @details
 *
 *    Function : fillModulationOrderDl
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's ModulationOrder in DL.
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/
void fillModulationOrderDl(uint8_t value, ClCellParam **cellPtr)
{
   if(value == 0 )
   {
      (*cellPtr)->supportedMaxModulationOrderDl = MOD_QPSK;
   }
   else if(value == 1)
   {
      (*cellPtr)->supportedMaxModulationOrderDl = MOD_16QAM;
   }
   else if(value == 2)
   {
      (*cellPtr)->supportedMaxModulationOrderDl = MOD_64QAM;
   }
   else if(value == 3)
   {
      (*cellPtr)->supportedMaxModulationOrderDl = MOD_256QAM;
   }
   else
   {
      (*cellPtr)->supportedMaxModulationOrderDl = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PUSCH DmrsConfig Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPuschDmrsConfigType
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PUSCH DmrsConfigType
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPuschDmrsConfig(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PUSCH_DMRS_CONFIG_TYPE_1_MASK) == FAPI_PUSCH_DMRS_CONFIG_TYPE_1_MASK)
   {
      (*cellPtr)->puschDmrsConfigTypes = DMRS_CONFIG_TYPE_1;
   }
   else if((value & FAPI_PUSCH_DMRS_CONFIG_TYPE_2_MASK) == FAPI_PUSCH_DMRS_CONFIG_TYPE_2_MASK)
   {
      (*cellPtr)->puschDmrsConfigTypes = DMRS_CONFIG_TYPE_2;
   }
   else
   {
      (*cellPtr)->puschDmrsConfigTypes = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PUSCH DmrsLength by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPuschDmrsLength
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PUSCH DmrsLength
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPuschDmrsLength(uint8_t value, ClCellParam **cellPtr)
{
   if(value  == FAPI_PUSCH_DMRS_MAX_LENGTH_1)
   {
      (*cellPtr)->puschDmrsMaxLength = DMRS_MAX_LENGTH_1;
   }
   else if(value  == FAPI_PUSCH_DMRS_MAX_LENGTH_2)
   {
      (*cellPtr)->puschDmrsMaxLength = DMRS_MAX_LENGTH_2;
   }
   else
   {
      (*cellPtr)->puschDmrsMaxLength = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PUSCH Dmrs Additional position by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPuschDmrsAddPos
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PUSCH DmrsAddPos
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPuschDmrsAddPos(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_DMRS_ADDITIONAL_POS_0_MASK) == FAPI_DMRS_ADDITIONAL_POS_0_MASK)
   {
      (*cellPtr)->puschDmrsAdditionalPos = DMRS_ADDITIONAL_POS_0;
   }
   else if((value & FAPI_DMRS_ADDITIONAL_POS_1_MASK) == FAPI_DMRS_ADDITIONAL_POS_1_MASK)
   {
      (*cellPtr)->puschDmrsAdditionalPos = DMRS_ADDITIONAL_POS_1;
   }
   else if((value & FAPI_DMRS_ADDITIONAL_POS_2_MASK) == FAPI_DMRS_ADDITIONAL_POS_2_MASK)
   {
      (*cellPtr)->puschDmrsAdditionalPos = DMRS_ADDITIONAL_POS_2;
   }
   else if((value & FAPI_DMRS_ADDITIONAL_POS_3_MASK) == FAPI_DMRS_ADDITIONAL_POS_3_MASK)
   {
      (*cellPtr)->puschDmrsAdditionalPos = DMRS_ADDITIONAL_POS_3;
   }
   else
   {
      (*cellPtr)->puschDmrsAdditionalPos = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PUSCH Mapping Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPuschMappingType
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PUSCH MappingType
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPuschMappingType(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PUSCH_MAPPING_TYPE_A_MASK) == FAPI_PUSCH_MAPPING_TYPE_A_MASK)
   {
      (*cellPtr)->puschMappingType = MAPPING_TYPE_A;
   }
   else if((value & FAPI_PUSCH_MAPPING_TYPE_B_MASK) == FAPI_PUSCH_MAPPING_TYPE_B_MASK)
   {
      (*cellPtr)->puschMappingType = MAPPING_TYPE_B;
   }
   else
   {
      (*cellPtr)->puschMappingType = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PUSCH Allocation Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPuschAllocationType
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PUSCH AllocationType
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPuschAllocationType(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PUSCH_ALLOC_TYPE_0_MASK) == FAPI_PUSCH_ALLOC_TYPE_0_MASK)
   {
      (*cellPtr)->puschAllocationTypes = ALLOCATION_TYPE_0;
   }
   else if((value & FAPI_PUSCH_ALLOC_TYPE_0_MASK) == FAPI_PUSCH_ALLOC_TYPE_0_MASK)
   {
      (*cellPtr)->puschAllocationTypes = ALLOCATION_TYPE_1;
   }
   else
   {
      (*cellPtr)->puschAllocationTypes = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PUSCH PRB Mapping Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPuschPrbMappingType
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PUSCH PRB MApping Type
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPuschPrbMappingType(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PUSCH_VRB_TO_PRB_MAP_NON_INTLV_MASK) == FAPI_PUSCH_VRB_TO_PRB_MAP_NON_INTLV_MASK)
   {
      (*cellPtr)->puschVrbToPrbMapping = VRB_TO_PRB_MAP_NON_INTLV;
   }
   else if((value & FAPI_PUSCH_VRB_TO_PRB_MAP_INTLVD_MASK) == FAPI_PUSCH_VRB_TO_PRB_MAP_INTLVD_MASK)
   {
      (*cellPtr)->puschVrbToPrbMapping = VRB_TO_PRB_MAP_INTLVD;
   }
   else
   {
      (*cellPtr)->puschVrbToPrbMapping = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the Modulation Order in Ul by comparing the bitmask
 *
 * @details
 *
 *    Function : fillModulationOrderUl
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's Modualtsion Order in UL.
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillModulationOrderUl(uint8_t value, ClCellParam **cellPtr)
{
   if(value == 0)
   {
      (*cellPtr)->supportedModulationOrderUl = MOD_QPSK;
   }
   else if(value == 1)
   {
      (*cellPtr)->supportedModulationOrderUl = MOD_16QAM;
   }
   else if(value == 2)
   {
      (*cellPtr)->supportedModulationOrderUl = MOD_64QAM;
   }
   else if(value == 3)
   {
      (*cellPtr)->supportedModulationOrderUl = MOD_256QAM;
   }
   else
   {
      (*cellPtr)->supportedModulationOrderUl = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PUSCH Aggregation Factor by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPuschAggregationFactor
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PUSCH Aggregation Factor
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPuschAggregationFactor(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_FORMAT_0_MASK) == FAPI_FORMAT_0_MASK)
   {
      (*cellPtr)->puschAggregationFactor    = AGG_FACTOR_1;
   }
   else if((value & FAPI_FORMAT_1_MASK) == FAPI_FORMAT_1_MASK)
   {
      (*cellPtr)->puschAggregationFactor    = AGG_FACTOR_2;
   }
   else if((value & FAPI_FORMAT_2_MASK) == FAPI_FORMAT_2_MASK)
   {
      (*cellPtr)->puschAggregationFactor    = AGG_FACTOR_4;
   }
   else if((value & FAPI_FORMAT_3_MASK) == FAPI_FORMAT_3_MASK)
   {
      (*cellPtr)->puschAggregationFactor    = AGG_FACTOR_8;
   }
   else
   {
      (*cellPtr)->puschAggregationFactor    = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PRACH Long Format by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPrachLongFormat
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PRACH Long Format
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPrachLongFormat(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PRACH_LF_FORMAT_0_MASK) == FAPI_PRACH_LF_FORMAT_0_MASK)
   {
      (*cellPtr)->prachLongFormats    = FORMAT_0;
   }
   else if((value & FAPI_PRACH_LF_FORMAT_1_MASK) == FAPI_PRACH_LF_FORMAT_1_MASK)
   {
      (*cellPtr)->prachLongFormats    = FORMAT_1;
   }
   else if((value & FAPI_PRACH_LF_FORMAT_2_MASK) == FAPI_PRACH_LF_FORMAT_2_MASK)
   {
      (*cellPtr)->prachLongFormats    = FORMAT_2;
   }
   else if((value & FAPI_PRACH_LF_FORMAT_3_MASK) == FAPI_PRACH_LF_FORMAT_3_MASK)
   {
      (*cellPtr)->prachLongFormats    = FORMAT_3;
   }
   else
   {
      (*cellPtr)->prachLongFormats    = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the PRACH Short Format by comparing the bitmask
 *
 * @details
 *
 *    Function : fillPrachShortFormat
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's PRACH ShortFormat
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillPrachShortFormat(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_PRACH_SF_FORMAT_A1_MASK) == FAPI_PRACH_SF_FORMAT_A1_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_A1;
   }
   else if((value & FAPI_PRACH_SF_FORMAT_A2_MASK) == FAPI_PRACH_SF_FORMAT_A2_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_A2;
   }
   else if((value & FAPI_PRACH_SF_FORMAT_A3_MASK) == FAPI_PRACH_SF_FORMAT_A3_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_A3;
   }
   else if((value & FAPI_PRACH_SF_FORMAT_B1_MASK) == FAPI_PRACH_SF_FORMAT_B1_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_B1;
   }
   else if((value & FAPI_PRACH_SF_FORMAT_B2_MASK) == FAPI_PRACH_SF_FORMAT_B2_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_B2;
   }
   else if((value & FAPI_PRACH_SF_FORMAT_B3_MASK) == FAPI_PRACH_SF_FORMAT_B3_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_B3;
   }
   else if((value & FAPI_PRACH_SF_FORMAT_B4_MASK) == FAPI_PRACH_SF_FORMAT_B4_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_B4;
   }
   else if((value & FAPI_PRACH_SF_FORMAT_C0_MASK) == FAPI_PRACH_SF_FORMAT_C0_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_C0;
   }
   else if((value & FAPI_PRACH_SF_FORMAT_C2_MASK) == FAPI_PRACH_SF_FORMAT_C2_MASK)
   {
      (*cellPtr)->prachShortFormats    = SF_FORMAT_C2;
   }
   else
   {
      (*cellPtr)->prachShortFormats    = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the Fd Occasions Type by comparing the bitmask
 *
 * @details
 *
 *    Function : fillFdOccasions
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's Fd Occasions
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillFdOccasions(uint8_t value, ClCellParam **cellPtr)
{
   if(value == 0)
   {
      (*cellPtr)->maxPrachFdOccasionsInASlot = PRACH_FD_OCC_IN_A_SLOT_1;
   }
   else if(value == 1)
   {
      (*cellPtr)->maxPrachFdOccasionsInASlot = PRACH_FD_OCC_IN_A_SLOT_2;
   }
   else if(value == 3)
   {
      (*cellPtr)->maxPrachFdOccasionsInASlot = PRACH_FD_OCC_IN_A_SLOT_4;
   }
   else if(value == 4)
   {
      (*cellPtr)->maxPrachFdOccasionsInASlot = PRACH_FD_OCC_IN_A_SLOT_8;
   }
   else
   {
      (*cellPtr)->maxPrachFdOccasionsInASlot = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief fills the RSSI Measurement by comparing the bitmask
 *
 * @details
 *
 *    Function : fillRssiMeas
 *
 *    Functionality:
 *         -checks the value with the bitmask and
 *          fills the cellPtr's RSSI Measurement report
 *
 * @params[in] Pointer to ClCellParam
 *             Value to be compared
 * @return void
 *
 ******************************************************************/

void fillRssiMeas(uint8_t value, ClCellParam **cellPtr)
{
   if((value & FAPI_RSSI_REPORT_IN_DBM_MASK) == FAPI_RSSI_REPORT_IN_DBM_MASK)
   {
      (*cellPtr)->rssiMeasurementSupport    = RSSI_REPORT_DBM;
   }
   else if((value & FAPI_RSSI_REPORT_IN_DBFS_MASK) == FAPI_RSSI_REPORT_IN_DBFS_MASK)
   {
      (*cellPtr)->rssiMeasurementSupport    = RSSI_REPORT_DBFS;
   }
   else
   {
      (*cellPtr)->rssiMeasurementSupport    = INVALID_VALUE;
   }
}

/*******************************************************************
 *
 * @brief Returns the TLVs value
 *
 * @details
 *
 *    Function : getParamValue
 *
 *    Functionality:
 *         -return TLVs value
 *
 * @params[in]
 * @return ROK     - temp
 *         RFAILED - failure
 *
 * ****************************************************************/

uint32_t getParamValue(fapi_uint16_tlv_t *tlv, uint16_t type)
{
   void *posPtr;
   posPtr   = &tlv->tl.tag;
   posPtr   += sizeof(tlv->tl.tag);
   posPtr   += sizeof(tlv->tl.length);
   /*TO DO: malloc to SSI memory */
   if(type == FAPI_UINT_8)
   {
      return(*(uint8_t *)posPtr);
   }
   else if(type == FAPI_UINT_16)
   {
      return(*(uint16_t *)posPtr);
   }
   else if(type == FAPI_UINT_32)
   {
      return(*(uint32_t *)posPtr);
   }
   else
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Value Extraction failed" );
      return RFAILED;
   }
}
#endif /* FAPI */

/*******************************************************************
 *
 * @brief Modifes the received mibPdu to uint32 bit
 *        and stores it in MacCellCfg
 *
 * @details
 *
 *    Function : setMibPdu
 *
 *    Functionality:
 *         -Sets the MibPdu
 *
 * @params[in] Pointer to mibPdu
 *             pointer to modified value
 ******************************************************************/
void setMibPdu(uint8_t *mibPdu, uint32_t *val, uint16_t sfn)
{
#ifndef OAI_TESTING
   *mibPdu |= (((uint8_t)(sfn << 2)) & MIB_SFN_BITMASK);
   *val = (mibPdu[0] << 24 | mibPdu[1] << 16 | mibPdu[2] << 8);
#else
   *mibPdu |= ((uint8_t)((sfn >> 4) & 0x3f) << 1);
   *val = (mibPdu[2] << 24 | mibPdu[1] << 16 | mibPdu[0] << 8);
#endif
   DU_LOG("\nDEBUG  -->  LWR_MAC: MIB PDU %x", *val);
}

/*******************************************************************
 *
 * @brief Sends FAPI Param req to PHY
 *
 * @details
 *
 *    Function : lwr_mac_procParamReqEvt
 *
 *    Functionality:
 *         -Sends FAPI Param req to PHY
 *
 * @params[in]
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/

uint8_t lwr_mac_procParamReqEvt(void *msg)
{
#ifdef INTEL_FAPI
#ifdef CALL_FLOW_DEBUG_LOG 
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : PARAM_REQ\n");
#endif

   /* startGuardTimer(); */
   fapi_param_req_t         *paramReq = NULL;
   fapi_msg_header_t        *msgHeader;
   p_fapi_api_queue_elem_t  paramReqElem;
   p_fapi_api_queue_elem_t  headerElem;

   LWR_MAC_ALLOC(paramReqElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_param_req_t)));
   if(paramReq != NULL)
   {
      FILL_FAPI_LIST_ELEM(paramReqElem, NULLP, FAPI_PARAM_REQUEST, 1, \
         sizeof(fapi_tx_data_req_t));
      paramReq = (fapi_param_req_t *)(paramReqElem +1);
      memset(paramReq, 0, sizeof(fapi_param_req_t));
      fillMsgHeader(&paramReq->header, FAPI_PARAM_REQUEST, sizeof(fapi_param_req_t));

      /* Fill message header */
      LWR_MAC_ALLOC(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
      if(!headerElem)
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for param req header");
         LWR_MAC_FREE(paramReqElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_param_req_t)));
         return RFAILED;
      }
      FILL_FAPI_LIST_ELEM(headerElem, paramReqElem, FAPI_VENDOR_MSG_HEADER_IND, 1, \
         sizeof(fapi_msg_header_t));
      msgHeader = (fapi_msg_header_t *)(headerElem + 1);
      msgHeader->num_msg = 1;
      msgHeader->handle = 0;

      DU_LOG("\nDEBUG  -->  LWR_MAC: Sending Param Request to Phy");
      LwrMacSendToL1(headerElem);
   }
   else
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for Param Request");
      return RFAILED;
   }
#endif
   return ROK;
}

/*******************************************************************
 *
 * @brief Sends FAPI Param Response to MAC via PHY
 *
 * @details
 *
 *    Function : lwr_mac_procParamRspEvt
 *
 *    Functionality:
 *         -Sends FAPI Param rsp to MAC via PHY
 *
 * @params[in]
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/

uint8_t lwr_mac_procParamRspEvt(void *msg)
{
#ifdef INTEL_FAPI
   /* stopGuardTimer(); */
   uint8_t index;
   uint32_t encodedVal;
   fapi_param_resp_t *paramRsp;
   ClCellParam *cellParam = NULLP;

   paramRsp = (fapi_param_resp_t *)msg;
   DU_LOG("\nINFO  -->  LWR_MAC: Received EVENT[%d] at STATE[%d]", lwrMacCb.event, lwrMacCb.phyState);

   if(paramRsp != NULLP)
   {
      MAC_ALLOC(cellParam, sizeof(ClCellParam));
      if(cellParam != NULLP)
      {
	 DU_LOG("\nDEBUG  -->  LWR_MAC: Filling TLVS into MAC API");
	 if(paramRsp->error_code == MSG_OK)
	 {
	    for(index = 0; index < paramRsp->number_of_tlvs; index++)
	    {
	       switch(paramRsp->tlvs[index].tl.tag)
	       {
		  case FAPI_RELEASE_CAPABILITY_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_16);
		     if(encodedVal != RFAILED && (encodedVal & RELEASE_15) == RELEASE_15)
		     {
			cellParam->releaseCapability = RELEASE_15;
		     }
		     break;

		  case FAPI_PHY_STATE_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != lwrMacCb.phyState)
		     {
			DU_LOG("\nERROR  -->  PhyState mismatch [%d][%d]", lwrMacCb.phyState, lwrMacCb.event);
			return RFAILED;
		     }
		     break;

		  case FAPI_SKIP_BLANK_DL_CONFIG_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->skipBlankDlConfig = SUPPORTED;
		     }
		     else
		     {
			cellParam->skipBlankDlConfig = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_SKIP_BLANK_UL_CONFIG_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->skipBlankUlConfig = SUPPORTED;
		     }
		     else
		     {
			cellParam->skipBlankUlConfig = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_NUM_CONFIG_TLVS_TO_REPORT_TYPE_TAG:
		     cellParam->numTlvsToReport = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_16);
		     break;

		  case FAPI_CYCLIC_PREFIX_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillCyclicPrefix(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_SUPPORTED_SUBCARRIER_SPACING_DL_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillSubcarrierSpaceDl(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_SUPPORTED_BANDWIDTH_DL_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_16);
		     if(encodedVal != RFAILED)
		     {
			fillBandwidthDl(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_SUPPORTED_SUBCARRIER_SPACING_UL_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillSubcarrierSpaceUl(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_SUPPORTED_BANDWIDTH_UL_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_16);
		     if(encodedVal != RFAILED)
		     {
			fillBandwidthUl(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_CCE_MAPPING_TYPE_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillCCEmaping(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_CORESET_OUTSIDE_FIRST_3_OFDM_SYMS_OF_SLOT_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->coresetOutsideFirst3OfdmSymsOfSlot = SUPPORTED;
		     }
		     else
		     {
			cellParam->coresetOutsideFirst3OfdmSymsOfSlot = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PRECODER_GRANULARITY_CORESET_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->precoderGranularityCoreset = SUPPORTED;
		     }
		     else
		     {
			cellParam->precoderGranularityCoreset = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PDCCH_MU_MIMO_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->pdcchMuMimo = SUPPORTED;
		     }
		     else
		     {
			cellParam->pdcchMuMimo = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PDCCH_PRECODER_CYCLING_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->pdcchPrecoderCycling = SUPPORTED;
		     }
		     else
		     {
			cellParam->pdcchPrecoderCycling = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_MAX_PDCCHS_PER_SLOT_TAG:
		     cellParam->maxPdcchsPerSlot = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     break;

		  case FAPI_PUCCH_FORMATS_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPucchFormat(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_MAX_PUCCHS_PER_SLOT_TAG:
		     cellParam->maxPucchsPerSlot   = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     break;

		  case FAPI_PDSCH_MAPPING_TYPE_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPdschMappingType(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PDSCH_ALLOCATION_TYPES_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPdschAllocationType(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PDSCH_VRB_TO_PRB_MAPPING_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPrbMappingType(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PDSCH_CBG_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->pdschCbg = SUPPORTED;
		     }
		     else
		     {
			cellParam->pdschCbg = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PDSCH_DMRS_CONFIG_TYPES_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPdschDmrsConfigType(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PDSCH_DMRS_MAX_LENGTH_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPdschDmrsLength(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PDSCH_DMRS_ADDITIONAL_POS_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPdschDmrsAddPos(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_MAX_PDSCHS_TBS_PER_SLOT_TAG:
		     cellParam->maxPdschsTBsPerSlot = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     break;

		  case FAPI_MAX_NUMBER_MIMO_LAYERS_PDSCH_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal < FAPI_MAX_NUMBERMIMO_LAYERS_PDSCH)
		     {
			cellParam->maxNumberMimoLayersPdsch   = encodedVal;
		     }
		     break;

		  case FAPI_SUPPORTED_MAX_MODULATION_ORDER_DL_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillModulationOrderDl(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_MAX_MU_MIMO_USERS_DL_TAG:
		     cellParam->maxMuMimoUsersDl         = \
		        getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     break;

		  case FAPI_PDSCH_DATA_IN_DMRS_SYMBOLS_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->pdschDataInDmrsSymbols = SUPPORTED;
		     }
		     else
		     {
			cellParam->pdschDataInDmrsSymbols = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PREMPTIONSUPPORT_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->premptionSupport = SUPPORTED;
		     }
		     else
		     {
			cellParam->premptionSupport = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PDSCH_NON_SLOT_SUPPORT_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->pdschNonSlotSupport = SUPPORTED;
		     }
		     else
		     {
			cellParam->pdschNonSlotSupport = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_UCI_MUX_ULSCH_IN_PUSCH_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->uciMuxUlschInPusch = SUPPORTED;
		     }
		     else
		     {
			cellParam->uciMuxUlschInPusch = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_UCI_ONLY_PUSCH_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->uciOnlyPusch = SUPPORTED;
		     }
		     else
		     {
			cellParam->uciOnlyPusch = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PUSCH_FREQUENCY_HOPPING_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->puschFrequencyHopping = SUPPORTED;
		     }
		     else
		     {
			cellParam->puschFrequencyHopping = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PUSCH_DMRS_CONFIG_TYPES_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPuschDmrsConfig(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PUSCH_DMRS_MAX_LEN_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPuschDmrsLength(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PUSCH_DMRS_ADDITIONAL_POS_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPuschDmrsAddPos(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PUSCH_CBG_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->puschCbg = SUPPORTED;
		     }
		     else
		     {
			cellParam->puschCbg = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PUSCH_MAPPING_TYPE_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPuschMappingType(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PUSCH_ALLOCATION_TYPES_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPuschAllocationType(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PUSCH_VRB_TO_PRB_MAPPING_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPuschPrbMappingType(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PUSCH_MAX_PTRS_PORTS_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal < FAPI_PUSCH_MAX_PTRS_PORTS_UB)
		     {
			cellParam->puschMaxPtrsPorts = encodedVal;
		     }
		     break;

		  case FAPI_MAX_PDUSCHS_TBS_PER_SLOT_TAG:
		     cellParam->maxPduschsTBsPerSlot = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     break;

		  case FAPI_MAX_NUMBER_MIMO_LAYERS_NON_CB_PUSCH_TAG:
		     cellParam->maxNumberMimoLayersNonCbPusch = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     break;

		  case FAPI_SUPPORTED_MODULATION_ORDER_UL_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillModulationOrderUl(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_MAX_MU_MIMO_USERS_UL_TAG:
		     cellParam->maxMuMimoUsersUl = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     break;

		  case FAPI_DFTS_OFDM_SUPPORT_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->dftsOfdmSupport = SUPPORTED;
		     }
		     else
		     {
			cellParam->dftsOfdmSupport = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_PUSCH_AGGREGATION_FACTOR_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPuschAggregationFactor(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PRACH_LONG_FORMATS_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPrachLongFormat(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PRACH_SHORT_FORMATS_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillPrachShortFormat(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_PRACH_RESTRICTED_SETS_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED && encodedVal != 0)
		     {
			cellParam->prachRestrictedSets = SUPPORTED;
		     }
		     else
		     {
			cellParam->prachRestrictedSets = NOT_SUPPORTED;
		     }
		     break;

		  case FAPI_MAX_PRACH_FD_OCCASIONS_IN_A_SLOT_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillFdOccasions(encodedVal, &cellParam);
		     }
		     break;

		  case FAPI_RSSI_MEASUREMENT_SUPPORT_TAG:
		     encodedVal = getParamValue(&paramRsp->tlvs[index], FAPI_UINT_8);
		     if(encodedVal != RFAILED)
		     {
			fillRssiMeas(encodedVal, &cellParam);
		     }
		     break;
		  default:
		     //DU_LOG("\nERROR  -->   Invalid value for TLV[%x] at index[%d]", paramRsp->tlvs[index].tl.tag, index);
		     break;
	       }
	    }
	    MAC_FREE(cellParam, sizeof(ClCellParam));
	    sendToLowerMac(FAPI_CONFIG_REQUEST, 0, (void *)NULL);
	    return ROK;
	 }
	 else
	 {
	    DU_LOG("\nERROR  -->   LWR_MAC: Invalid error code %d", paramRsp->error_code);
	    return RFAILED;
	 }
      }
      else
      {
	 DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for cell param");
	 return RFAILED;
      }
   }
   else
   {
      DU_LOG("\nERROR  -->  LWR_MAC:  Param Response received from PHY is NULL");
      return RFAILED;
   }
#else
   return ROK;
#endif
}

#ifdef INTEL_TIMER_MODE
uint8_t lwr_mac_procIqSamplesReqEvt(void *msg)
{
   void * wlsHdlr = NULLP;
   fapi_msg_header_t *msgHeader;
   fapi_vendor_ext_iq_samples_req_t *iqSampleReq;
   p_fapi_api_queue_elem_t  headerElem;
   p_fapi_api_queue_elem_t  iqSampleElem;
   char filename[100] = "/root/intel/FlexRAN/testcase/ul/mu0_20mhz/2/uliq00_prach_tst2.bin"; 

   uint8_t buffer[] ={0,0,0,0,0,2,11,0,212,93,40,0,20,137,38,0,20,0,20,0,0,8,0,8,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,1,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0,1,0};

   size_t bufferSize = sizeof(buffer) / sizeof(buffer[0]);

   /* Fill IQ sample req */
   mtGetWlsHdl(&wlsHdlr);
   //iqSampleElem = (p_fapi_api_queue_elem_t)WLS_Alloc(wlsHdlr, \
      (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_ext_iq_samples_req_t))); 
   LWR_MAC_ALLOC(iqSampleElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_ext_iq_samples_req_t)));
   if(!iqSampleElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for IQ sample req");
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(iqSampleElem, NULLP, FAPI_VENDOR_EXT_UL_IQ_SAMPLES, 1, \
      sizeof(fapi_vendor_ext_iq_samples_req_t));

   iqSampleReq = (fapi_vendor_ext_iq_samples_req_t *)(iqSampleElem + 1);
   memset(iqSampleReq, 0, sizeof(fapi_vendor_ext_iq_samples_req_t));
   fillMsgHeader(&iqSampleReq->header, FAPI_VENDOR_EXT_UL_IQ_SAMPLES, \
      sizeof(fapi_vendor_ext_iq_samples_req_t));

   iqSampleReq->iq_samples_info.carrNum = 0;
   iqSampleReq->iq_samples_info.numSubframes = 40;
   iqSampleReq->iq_samples_info.nIsRadioMode = 0;
   iqSampleReq->iq_samples_info.timerModeFreqDomain = 0;
   iqSampleReq->iq_samples_info.phaseCompensationEnable = 0;
   iqSampleReq->iq_samples_info.startFrameNum = 0;
   iqSampleReq->iq_samples_info.startSlotNum = 0;
   iqSampleReq->iq_samples_info.startSymNum = 0;
   strncpy(iqSampleReq->iq_samples_info.filename_in_ul_iq[0], filename, 100);
   memcpy(iqSampleReq->iq_samples_info.buffer, buffer, bufferSize);

   /* TODO : Fill remaining parameters */

   /* Fill message header */
   LWR_MAC_ALLOC(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
   if(!headerElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for FAPI header in lwr_mac_procIqSamplesReqEvt");
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(headerElem, iqSampleElem, FAPI_VENDOR_MSG_HEADER_IND, 1, \
     sizeof(fapi_msg_header_t));
   msgHeader = (fapi_msg_header_t *)(headerElem + 1);
   msgHeader->num_msg = 1; 
   msgHeader->handle = 0;

   DU_LOG("\nINFO   -->  LWR_MAC: Sending IQ Sample request to Phy");
   LwrMacSendToL1(headerElem);
   return ROK;
}
#endif

#ifdef OAI_TESTING

/*******************************************************************
 *
 * @brief Pack and send DL TTI message to OAI L1
 *
 * @details
 *
 *    Function : packDlTtiReq
 *
 *    Functionality:
 *         -Pack and send DL TTI message to OAI L1
 *
 * @params[in] fapi_dl_tti_req_t *dlTtiReq,uint8_t *out , uint32_t *len
 * @return Void
 *
 * ****************************************************************/

void packDlTtiReq(fapi_dl_tti_req_t *dlTtiReq,uint8_t *out , uint32_t *len)
{
	uint8_t pduIdx = 0, freqIdx = 0, dciIndex = 0, ueGrpIdx = 0;
	uint8_t numBytes = 0, ret = ROK;
	uint32_t totalLen=0,lenIdx=0, lenDifference=0;
	uint8_t *mBuf = out;

	//header
	CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->header.numMsg, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->header.opaque, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt16,dlTtiReq->header.msg_id, &mBuf, &totalLen);
	dlTtiReq->header.length = totalLen;  // Update header length with the total length

	CMCHKPKLEN(oduPackPostUInt32,dlTtiReq->header.length, &mBuf, &totalLen);


	CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->sfn, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->slot, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->nPdus, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->nGroup, &mBuf, &totalLen);
	for(pduIdx = 0; pduIdx < dlTtiReq->nPdus; pduIdx++)
	{
		CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pduType, &mBuf, &totalLen);
		CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pduSize, &mBuf, &totalLen);
		lenIdx=totalLen;
		switch(reverseBytes16(dlTtiReq->pdus[pduIdx].pduType))
		{
			case FAPI_PDCCH_PDU_TYPE:
				{
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.bwpSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.bwpStart, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.subCarrierSpacing, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.cyclicPrefix, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.startSymbolIndex, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.durationSymbols, &mBuf, &totalLen);
					for(freqIdx = 0; freqIdx < 6; freqIdx++)
					{
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.freqDomainResource[freqIdx], &mBuf, &totalLen);
					}
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.cceRegMappingType, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.regBundleSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.interleaverSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.coreSetType, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.shiftIndex, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.precoderGranularity, &mBuf, &totalLen);

					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.numDlDci, &mBuf, &totalLen);
					// Extract the pdcch_pdu fields for the given pduIdx

					for(dciIndex = 0; dciIndex <  reverseBytes16(dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.numDlDci); dciIndex++)
					{
						CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].rnti, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].scramblingId, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].scramblingRnti, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].cceIndex, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].aggregationLevel, &mBuf, &totalLen);

						fapi_precoding_bmform_t *preCodingAndBeamforming= &dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].pc_and_bform;

						CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(preCodingAndBeamforming->numPrgs), &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(preCodingAndBeamforming->prgSize), &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, preCodingAndBeamforming->digBfInterfaces, &mBuf, &totalLen);

						for(uint16_t prgIdx = 0; prgIdx < preCodingAndBeamforming->numPrgs; prgIdx++)
						{
							CMCHKPKLEN(oduPackPostUInt16, preCodingAndBeamforming->pmi_bfi[prgIdx].pmIdx, &mBuf, &totalLen);
							for(uint8_t digBfIdx = 0; digBfIdx < preCodingAndBeamforming->digBfInterfaces; digBfIdx++)
							{
								CMCHKPKLEN(oduPackPostUInt16, preCodingAndBeamforming->pmi_bfi[prgIdx].beamIdx[digBfIdx].beamidx, &mBuf, &totalLen);
							}
						}
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].beta_pdcch_1_0, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].powerControlOffsetSS, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].payloadSizeBits, &mBuf, &totalLen);
						numBytes = reverseBytes16(dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].payloadSizeBits) / 8;

						if(reverseBytes16(dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].payloadSizeBits) % 8)
						{
						   numBytes += 1;
						}
						for(uint8_t payloadIdx = 0; payloadIdx < numBytes; payloadIdx++)
						{
							CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdcch_pdu.dlDci[dciIndex].payload[payloadIdx], &mBuf, &totalLen);
						}
					}
					lenDifference=totalLen- lenIdx;
					*((uint16_t *)(out + lenIdx-2)) = reverseBytes16(lenDifference);
					break;
				}

			case FAPI_PDSCH_PDU_TYPE:
				{

					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.pduBitMap, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.rnti, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.pdu_index, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.bwpSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.bwpStart, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.subCarrierSpacing, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.cyclicPrefix, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.nrOfCodeWords, &mBuf, &totalLen);

					for(uint8_t cwIdx = 0; cwIdx <  dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.nrOfCodeWords; cwIdx++)
					{
						CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.cwInfo[cwIdx].targetCodeRate, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.cwInfo[cwIdx].qamModOrder, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.cwInfo[cwIdx].mcsIndex, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.cwInfo[cwIdx].mcsTable, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.cwInfo[cwIdx].rvIndex, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt32, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.cwInfo[cwIdx].tbSize, &mBuf, &totalLen);
					}
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.dataScramblingId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.nrOfLayers, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.transmissionScheme, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.refPoint, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.dlDmrsSymbPos, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.dmrsConfigType, &mBuf, &totalLen);

					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.dlDmrsScramblingId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.scid, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.numDmrsCdmGrpsNoData, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.dmrsPorts, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.resourceAlloc, &mBuf, &totalLen);
					for(uint8_t rbBitMapIdx = 0; rbBitMapIdx < 36; rbBitMapIdx++)
					{
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.rbBitmap[rbBitMapIdx], &mBuf, &totalLen);
					}

					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.rbStart, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.rbSize, &mBuf, &totalLen);

					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.vrbToPrbMapping, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.startSymbIndex, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.nrOfSymbols, &mBuf, &totalLen);
					if (reverseBytes16(dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.pduBitMap) & 0b1)
					{
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.ptrsPortIndex, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.ptrsTimeDensity, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.ptrsFreqDensity, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.ptrsReOffset, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.nEpreRatioOfPdschToPtrs, &mBuf, &totalLen);
					}
					fapi_precoding_bmform_t *preCodingAndBeamforming= &dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.preCodingAndBeamforming;

					CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(preCodingAndBeamforming->numPrgs), &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(preCodingAndBeamforming->prgSize), &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, preCodingAndBeamforming->digBfInterfaces, &mBuf, &totalLen);

					for(uint16_t prgIdx = 0; prgIdx < (preCodingAndBeamforming->numPrgs); prgIdx++)
					{
						CMCHKPKLEN(oduPackPostUInt16, preCodingAndBeamforming->pmi_bfi[prgIdx].pmIdx, &mBuf, &totalLen);
						for(uint8_t digBfIdx = 0; digBfIdx < (preCodingAndBeamforming->digBfInterfaces); digBfIdx++)
						{
							CMCHKPKLEN(oduPackPostUInt16, preCodingAndBeamforming->pmi_bfi[prgIdx].beamIdx[digBfIdx].beamidx, &mBuf, &totalLen);
						}
					}

					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.powerControlOffset, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.powerControlOffsetSS, &mBuf, &totalLen);
					if (reverseBytes16(dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.pduBitMap)  & 0b10)
					{
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.isLastCbPresent, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.isInlineTbCrc, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt32, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.dlTbCrc, &mBuf, &totalLen);
					}
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.maintParamV3.ldpcBaseGraph, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt32, dlTtiReq->pdus[pduIdx].pdu.pdsch_pdu.maintParamV3.tbSizeLbrmBytes, &mBuf, &totalLen);
					lenDifference=totalLen- lenIdx;
					*((uint16_t *)(out + lenIdx-2)) = reverseBytes16(lenDifference);
					break;
				}

			case FAPI_PBCH_PDU_TYPE:
				{
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.ssb_pdu.physCellId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.ssb_pdu.betaPss, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.ssb_pdu.ssbBlockIndex, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.ssb_pdu.ssbSubCarrierOffset, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, dlTtiReq->pdus[pduIdx].pdu.ssb_pdu.ssbOffsetPointA, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->pdus[pduIdx].pdu.ssb_pdu.bchPayloadFlag, &mBuf, &totalLen);

					CMCHKPKLEN(oduPackPostUInt32, dlTtiReq->pdus[pduIdx].pdu.ssb_pdu.bchPayload.bchPayload, &mBuf, &totalLen);
					fapi_precoding_bmform_t *preCodingAndBeamforming= &dlTtiReq->pdus[pduIdx].pdu.ssb_pdu.preCodingAndBeamforming;
					CMCHKPKLEN(oduPackPostUInt16, preCodingAndBeamforming->numPrgs, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, preCodingAndBeamforming->prgSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, preCodingAndBeamforming->digBfInterfaces, &mBuf, &totalLen);

					for(uint16_t prgIdx = 0; prgIdx < reverseBytes16(preCodingAndBeamforming->numPrgs); prgIdx++)
					{
						CMCHKPKLEN(oduPackPostUInt16, preCodingAndBeamforming->pmi_bfi[prgIdx].pmIdx, &mBuf, &totalLen);
						for(uint8_t digBfIdx = 0; digBfIdx < (preCodingAndBeamforming->digBfInterfaces); digBfIdx++)
						{
							CMCHKPKLEN(oduPackPostUInt16, preCodingAndBeamforming->pmi_bfi[prgIdx].beamIdx[digBfIdx].beamidx, &mBuf, &totalLen);
						}
					}
					lenDifference=totalLen- lenIdx;
					*((uint16_t *)(out + lenIdx-2)) = reverseBytes16(lenDifference);

					break;
				}
			default:
				{
					return NULLP;
				}
		}
	}
	for(ueGrpIdx = 0; ueGrpIdx < dlTtiReq->nGroup; ueGrpIdx++)
	{
		CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->ue_grp_info[ueGrpIdx].nUe, &mBuf, &totalLen);

		for(uint8_t ueIdx = 0; ueIdx < dlTtiReq->ue_grp_info[ueGrpIdx].nUe; ueIdx++)
		{
			CMCHKPKLEN(oduPackPostUInt8, dlTtiReq->ue_grp_info[ueGrpIdx].pduIdx[ueIdx], &mBuf, &totalLen);
		}
	}
	if(totalLen !=sizeof(fapi_dl_tti_req_t))
	{
		*((uint32_t *)(out + 4)) = reverseBytes32(totalLen);
	}

	*len=totalLen;

}

/*******************************************************************
 *
 * @brief Pack and send UL TTI message to OAI L1
 *
 * @details
 *
 *    Function : packUlTtiReq
 *
 *    Functionality:
 *         -Pack and send UL TTI message to OAI L1
 *
 * @params[in] fapi_ul_tti_req_t *ulTti,uint8_t *out, uint32_t *len
 * @return Void
 *
 * ****************************************************************/

void packUlTtiReq(fapi_ul_tti_req_t *ulTtiReq,uint8_t *out, uint32_t *len)
{
	uint8_t pduIdx = 0, ueGrpIdx = 0, ret = ROK;
	uint32_t lenIdx=0, lenDifference=0, totalLen=0;


	uint8_t *mBuf = out;
	CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->header.numMsg, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->header.opaque, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt16,ulTtiReq->header.msg_id, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt32,ulTtiReq->header.length, &mBuf, &totalLen);

	CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->sfn, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->slot, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->nPdus, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->rachPresent, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->nUlsch, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->nUlcch, &mBuf, &totalLen);
	CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->nGroup, &mBuf, &totalLen);

	for(pduIdx = 0; pduIdx < ulTtiReq->nPdus; pduIdx++)
	{
		CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pduType, &mBuf, &totalLen);
		CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pduSize, &mBuf, &totalLen);
		lenIdx=totalLen;
		switch(reverseBytes16(ulTtiReq->pdus[pduIdx].pduType))
		{
			case FAPI_PRACH_PDU_TYPE:
				{
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.prach_pdu.physCellId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.prach_pdu.numPrachOcas, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.prach_pdu.prachFormat, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.prach_pdu.numRa, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.prach_pdu.prachStartSymbol, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.prach_pdu.numCs, &mBuf, &totalLen);
					fapi_ul_rx_bmform_pdu_t *ulBmform=&ulTtiReq->pdus[pduIdx].pdu.prach_pdu.beamforming;
					CMCHKPKLEN(oduPackPostUInt8, ulBmform->trp_scheme, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulBmform->numPrgs, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulBmform->prgSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulBmform->digBfInterface, &mBuf, &totalLen);

					for(uint8_t prgIdx = 0; prgIdx < reverseBytes16(ulBmform->numPrgs); prgIdx++)
					{
						for(uint8_t digBfIdx = 0; digBfIdx < ulBmform->digBfInterface; digBfIdx++)
						{
							CMCHKPKLEN(oduPackPostUInt16, ulBmform->rx_bfi[prgIdx].beamIdx[digBfIdx].beamidx, &mBuf, &totalLen);
						}
					}
					lenDifference=totalLen- lenIdx;
					*((uint16_t *)(out + lenIdx-2)) = reverseBytes16(lenDifference);
					break;
				}
			case FAPI_PUSCH_PDU_TYPE:
				{
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.pduBitMap, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.rnti, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt32, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.handle, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.bwpSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.bwpStart, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.subCarrierSpacing, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.cyclicPrefix, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.targetCodeRate, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.qamModOrder, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.mcsIndex, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.mcsTable, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.transformPrecoding, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.dataScramblingId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.nrOfLayers, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.ulDmrsSymbPos, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.dmrsConfigType, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.ulDmrsScramblingId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschIdentity, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.scid, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.numDmrsCdmGrpsNoData, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.dmrsPorts, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.resourceAlloc, &mBuf, &totalLen);
					for(uint8_t rbBitMapIdx = 0; rbBitMapIdx < 36; rbBitMapIdx++)
					{
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.rbBitmap[rbBitMapIdx], &mBuf, &totalLen);
					}
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.rbStart, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.rbSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.vrbToPrbMapping, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.frequencyHopping, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.txDirectCurrentLocation, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.uplinkFrequencyShift7p5khz, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.startSymbIndex, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.nrOfSymbols, &mBuf, &totalLen);

					//Fill fapi_pusch_data_t
					if(reverseBytes16(ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.pduBitMap) &0x01)
					{
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschData.rvIndex, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschData.harqProcessId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschData.newDataIndicator, &mBuf, &totalLen);

					CMCHKPKLEN(oduPackPostUInt32, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschData.tbSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschData.numCb, &mBuf, &totalLen);
					for(int i=0;i<reverseBytes16(ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschData.numCb);i++)
					{
						//TODO - Above loop length must be cb length= (numCB+7)/8;
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschData.cbPresentAndPosition[i], &mBuf, &totalLen);
					}
					}
					//Fill fapi_pusch_uci_t
					if(reverseBytes16(ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.pduBitMap) &0x02)
					{
						CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschUci.harqAckBitLength, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschUci.csiPart1BitLength, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschUci.csiPart2BitLength, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschUci.alphaScaling, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschUci.betaOffsetHarqAck, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschUci.betaOffsetCsi1, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschUci.betaOffsetCsi2, &mBuf, &totalLen);
					}

					//Fill fapi_pusch_ptrs_t
					if(reverseBytes16(ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.pduBitMap) &0x04)
					{
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschPtrs.numPtrsPorts, &mBuf, &totalLen);
						for(uint8_t portIdx = 0; portIdx < ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschPtrs.numPtrsPorts; portIdx++)
						{
							CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschPtrs.ptrsInfo[portIdx].ptrsPortIndex, &mBuf, &totalLen);
							CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschPtrs.ptrsInfo[portIdx].ptrsDmrsPort, &mBuf, &totalLen);
							CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschPtrs.ptrsInfo[portIdx].ptrsReOffset, &mBuf, &totalLen);
						}
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschPtrs.ptrsTimeDensity, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschPtrs.ptrsFreqDensity, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.puschPtrs.ulPtrsPower, &mBuf, &totalLen);
					}
					//Fill fapi_dfts_ofdm_t
					if(reverseBytes16(ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.pduBitMap) &0x08)
					{
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.dftsOfdm.lowPaprGroupNumber, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.dftsOfdm.lowPaprSequenceNumber, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.dftsOfdm.ulPtrsSampleDensity, &mBuf, &totalLen);
						CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.dftsOfdm.ulPtrsTimeDensityTransformPrecoding, &mBuf, &totalLen);
					}

					//Fill fapi_ul_rx_bmform_pdu_t
					fapi_ul_rx_bmform_pdu_t *ulBmform=&ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.beamforming;
					CMCHKPKLEN(oduPackPostUInt8, ulBmform->trp_scheme, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulBmform->numPrgs, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulBmform->prgSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulBmform->digBfInterface, &mBuf, &totalLen);

					for(uint8_t prgIdx = 0; prgIdx < reverseBytes16(ulBmform->numPrgs); prgIdx++)
					{
						for(uint8_t digBfIdx = 0; digBfIdx < ulBmform->digBfInterface; digBfIdx++)
						{
							CMCHKPKLEN(oduPackPostUInt16, ulBmform->rx_bfi[prgIdx].beamIdx[digBfIdx].beamidx, &mBuf, &totalLen);
						}
					}
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.maintParamV3.ldpcBaseGraph, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt32, ulTtiReq->pdus[pduIdx].pdu.pusch_pdu.maintParamV3.tbSizeLbrmBytes, &mBuf, &totalLen);
					lenDifference=totalLen- lenIdx;
					*((uint16_t *)(out + lenIdx-2)) = reverseBytes16(lenDifference);
					break;
				}
			case FAPI_PUCCH_PDU_TYPE:
				{
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.rnti, &mBuf, &totalLen);

					CMCHKPKLEN(oduPackPostUInt32, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.handle, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.bwpSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.bwpStart, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.subCarrierSpacing, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.cyclicPrefix, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.formatType, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.multiSlotTxIndicator, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.pi2Bpsk, &mBuf, &totalLen);

					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.prbStart, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.prbSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.startSymbolIndex, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.nrOfSymbols, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.freqHopFlag, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.secondHopPrb, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.groupHopFlag, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.sequenceHopFlag, &mBuf, &totalLen);

					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.hoppingId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.initialCyclicShift, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.dataScramblingId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.timeDomainOccIdx, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.preDftOccIdx, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.preDftOccLen, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.addDmrsFlag, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.dmrsScramblingId, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.dmrsCyclicShift, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.srFlag, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.bitLenHarq, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.bitLenCsiPart1, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.bitLenCsiPart2, &mBuf, &totalLen);

					//Fill fapi_ul_rx_bmform_pdu_t
					fapi_ul_rx_bmform_pdu_t *ulBmform=&ulTtiReq->pdus[pduIdx].pdu.pucch_pdu.beamforming;
					CMCHKPKLEN(oduPackPostUInt8, ulBmform->trp_scheme, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulBmform->numPrgs, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt16, ulBmform->prgSize, &mBuf, &totalLen);
					CMCHKPKLEN(oduPackPostUInt8, ulBmform->digBfInterface, &mBuf, &totalLen);

					for(uint8_t prgIdx = 0; prgIdx < reverseBytes16(ulBmform->numPrgs); prgIdx++)
					{
						for(uint8_t digBfIdx = 0; digBfIdx < ulBmform->digBfInterface; digBfIdx++)
						{
							CMCHKPKLEN(oduPackPostUInt16, ulBmform->rx_bfi[prgIdx].beamIdx[digBfIdx].beamidx, &mBuf, &totalLen);
						}
					}
					lenDifference=totalLen- lenIdx;
					*((uint16_t *)(out + lenIdx-2)) = reverseBytes16(lenDifference);
					break;
				}
			default:
				{
					return RFAILED;
				}
		}
	}

	for(ueGrpIdx = 0; ueGrpIdx < ulTtiReq->nGroup; ueGrpIdx++)
	{
		CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->ueGrpInfo[ueGrpIdx].nUe, &mBuf, &totalLen);
		for(uint8_t ueIdx = 0; ueIdx < ulTtiReq->ueGrpInfo[ueGrpIdx].nUe; ueIdx++)
		{
			CMCHKPKLEN(oduPackPostUInt8, ulTtiReq->ueGrpInfo[ueGrpIdx].pduIdx[ueIdx], &mBuf, &totalLen);
		}
	}

	*len=totalLen;
	if(totalLen != sizeof(fapi_ul_tti_req_t))
	{
		*((uint32_t *)(out + 4)) = reverseBytes32(totalLen);
	}

}

/*******************************************************************
 *
 * @brief Pack and send TX Data Req message to OAI L1
 *
 * @details
 *
 *    Function : packTxDataReqBuffer
 *
 *    Functionality:
 *         -Pack and send TX Data Req message to OAI L1
 *
 * @params[in] fapi_tx_data_req_t *txdata, uint8_t *mBuf ,uint32_t *len
 * @return Void
 *
 * ****************************************************************/

void packTxDataReqBuffer(fapi_tx_data_req_t *txDataReq, uint8_t *mBuf ,uint32_t *len)
{
        uint16_t pduIdx = 0;
        uint8_t ret = ROK;
        uint16_t payloadSize = 0;
        uint32_t totalLen = 0;

        uint8_t *out = mBuf;

        CMCHKPKLEN(oduPackPostUInt8, txDataReq->header.numMsg, &out, &totalLen);
        CMCHKPKLEN(oduPackPostUInt8, txDataReq->header.opaque, &out, &totalLen);
        CMCHKPKLEN(oduPackPostUInt16, txDataReq->header.msg_id, &out, &totalLen);
        CMCHKPKLEN(oduPackPostUInt32, txDataReq->header.length, &out, &totalLen);

        CMCHKPKLEN(oduPackPostUInt16, txDataReq->sfn, &out, &totalLen);
        CMCHKPKLEN(oduPackPostUInt16, txDataReq->slot, &out, &totalLen);
        CMCHKPKLEN(oduPackPostUInt16, txDataReq->num_pdus, &out, &totalLen);

        for (pduIdx = 0; pduIdx <reverseBytes16( txDataReq->num_pdus); pduIdx++)
        {

                CMCHKPKLEN(oduPackPostUInt32, txDataReq->pdu_desc[pduIdx].pdu_length, &out, &totalLen);
                CMCHKPKLEN(oduPackPostUInt16, txDataReq->pdu_desc[pduIdx].pdu_index, &out, &totalLen);
                CMCHKPKLEN(oduPackPostUInt32, txDataReq->pdu_desc[pduIdx].num_tlvs, &out, &totalLen);

                for (uint32_t tlvIdx = 0; tlvIdx < reverseBytes32(txDataReq->pdu_desc[pduIdx].num_tlvs); tlvIdx++)
                {

                        CMCHKPKLEN(oduPackPostUInt16, txDataReq->pdu_desc[pduIdx].tlvs[tlvIdx].tag, &out, &totalLen);
                        CMCHKPKLEN(oduPackPostUInt32, txDataReq->pdu_desc[pduIdx].tlvs[tlvIdx].length, &out, &totalLen);

                        payloadSize = reverseBytes32(txDataReq->pdu_desc[pduIdx].tlvs[tlvIdx].length);
			payloadSize = (payloadSize + 3 )/4;
                        for (uint32_t payloadByte = 0; payloadByte < payloadSize; payloadByte++)
                        {
				//TODO- CHeck
                                CMCHKPKLEN(oduPackPostUInt32_S, txDataReq->pdu_desc[pduIdx].tlvs[tlvIdx].value.direct[payloadByte], &out, &totalLen);
                        }
                }
        }

	*len = totalLen;
	if(totalLen != sizeof(fapi_tx_data_req_t))
	{
		*((uint32_t *)(mBuf + 4)) = reverseBytes32(totalLen);
	}
}

/*******************************************************************
 *
 * @brief Pack and send Config Req message to OAI L1
 *
 * @details
 *
 *    Function : packConfigReq
 *
 *    Functionality:
 *         -Pack and send Config Req message to OAI L1
 *
 * @params[in] fapi_config_req_t *configReq,uint8_t *mBuf,uint32_t *len
 * @return Void
 *
 * ****************************************************************/

void packConfigReq(fapi_config_req_t *configReq,  uint8_t *mBuf, uint32_t *len)
{
    uint8_t *out = mBuf;
    uint32_t msgLen = 0;
    uint16_t totalTlv = 0;
    uint16_t tlvSize=10;//uint16_t [5]
    uint16_t value[5] = {0,273,0,0,0};

    CMCHKPKLEN(oduPackPostUInt8, configReq->header.numMsg, &out, &msgLen);
    CMCHKPKLEN(oduPackPostUInt8, configReq->header.opaque, &out, &msgLen);
    CMCHKPKLEN(oduPackPostUInt16, configReq->header.msg_id, &out, &msgLen);
    CMCHKPKLEN(oduPackPostUInt32, configReq->header.length, &out, &msgLen);

    totalTlv = configReq->number_of_tlvs;
    uint8_t randmTlvCnt=  25; //This value is randomly assigned
    CMCHKPKLEN(oduPackPostUInt8, randmTlvCnt, &out, &msgLen);

    for(uint16_t idx=0;idx<totalTlv;idx++)
    {
            fapi_uint32_tlv_t tlv =configReq->tlvs[idx];

            CMCHKPKLEN(oduPackPostUInt16, tlv.tl.tag, &out, &msgLen);
            CMCHKPKLEN(oduPackPostUInt16, tlv.tl.length, &out, &msgLen);
            switch(reverseBytes16(tlv.tl.length))
            {
                    case 1:
                            {
                                    uint8_t val=tlv.value;
                                    CMCHKPKLEN(oduPackPostUInt8, val, &out, &msgLen);
                                    CMCHKPKLEN(oduPackPostUInt8, 0, &out, &msgLen);
                                    CMCHKPKLEN(oduPackPostUInt8, 0, &out, &msgLen);
                                    CMCHKPKLEN(oduPackPostUInt8, 0, &out, &msgLen);
                                    break;
                            }
                    case 2:
                            {
                                    uint16_t val=tlv.value;
                                    CMCHKPKLEN(oduPackPostUInt16, val, &out, &msgLen);
                                    CMCHKPKLEN(oduPackPostUInt16, 0, &out, &msgLen);
                                    break;
                            }
                    case 4:
                            {
                                    uint32_t val=tlv.value;
                                    CMCHKPKLEN(oduPackPostUInt32, val, &out, &msgLen);
                                    break;
                            }


            }
    }
    // adding dlGridSize and ulGridSize tlv
    CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(FAPI_DL_GRIDSIZE_TAG), &out, &msgLen);
    CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(tlvSize), &out, &msgLen);
    for(uint8_t arrIdx=0;arrIdx<5;arrIdx++)
    {
	    CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(value[arrIdx]), &out, &msgLen);
    }
    CMCHKPKLEN(oduPackPostUInt16, 0, &out, &msgLen);

    CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(FAPI_UL_GRID_SIZE_TAG), &out, &msgLen);
    CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(tlvSize), &out, &msgLen);
    for(uint8_t arrIdx=0;arrIdx<5;arrIdx++)
    {
	    CMCHKPKLEN(oduPackPostUInt16, reverseBytes16(value[arrIdx]), &out, &msgLen);
    }
    CMCHKPKLEN(oduPackPostUInt16, 0, &out, &msgLen);
    
    *len = msgLen;
    if(msgLen != sizeof(configReq->header.length))
    {
            *((uint32_t *)(mBuf + 4)) = reverseBytes32(msgLen);
    }
}

#define TLV_ALIGN(_tlv_size) (32-_tlv_size)
/*******************************************************************
 *
 * @brief Build FAPI Config Req as per OAI code and send to PHY
 *
 * @details
 *
 *    Function : buildAndSendOAIConfigReqToL1 
 *
 *    Functionality:
 *         -Build FAPI Config Req as per OAI code and send to PHY
 *
 * @params[in] void *msg
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t buildAndSendOAIConfigReqToL1(void *msg)
{
#ifdef INTEL_FAPI
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : CONFIG_REQ\n");
#endif
#ifdef NR_TDD
   uint8_t slotIdx = 0;
   uint8_t symbolIdx =0;
   uint8_t numSlotsInMaxPeriodicity = 0; /*number of TDD Slots in MAX_PERIODICITY(10ms) as per numerology*/
   uint8_t numSlotsInCurrPeriodicity = 0; /*number of TDD Slots in Configured_PERIODICITY(0.5ms to 10ms) as per numerology*/
   uint8_t cntSlotCfg = 0; /*number of Slot Cfg repeatition*/
#endif
   uint16_t totalTlv=0;
   uint16_t index = 0;
   uint16_t *cellId =NULLP;
   uint16_t cellIdx =0;
   uint32_t msgLen = 0;
   uint32_t totalCfgReqMsgLen=0;
   uint32_t mib = 0;
   uint32_t dlFreq = 0, ulFreq = 0;
   MacCellCfg macCfgParams;
   fapi_config_req_t *configReq;
   fapi_msg_header_t *msgHeader;
   p_fapi_api_queue_elem_t  headerElem;
   p_fapi_api_queue_elem_t  cfgReqQElem;

   DU_LOG("\nINFO  -->  LWR_MAC: Received EVENT[%d] at STATE[%d]", lwrMacCb.event, \
         lwrMacCb.phyState);

   cellId = (uint16_t *)msg;
   GET_CELL_IDX(*cellId, cellIdx);
   macCfgParams = macCb.macCell[cellIdx]->macCellCfg;

   /* Fill Cell Configuration in lwrMacCb */
   memset(&lwrMacCb.cellCb[lwrMacCb.numCell], 0, sizeof(LwrMacCellCb));
   lwrMacCb.cellCb[lwrMacCb.numCell].cellId = macCfgParams.cellId;
   lwrMacCb.cellCb[lwrMacCb.numCell].phyCellId = macCfgParams.cellCfg.phyCellId;
   lwrMacCb.numCell++;
   uint16_t psize=sizeof(fapi_api_queue_elem_t)+(sizeof(fapi_config_req_t));

#ifndef NR_TDD
   totalTlv = 24;
#else
   numSlotsInMaxPeriodicity = MAX_TDD_PERIODICITY * pow(2, macCb.macCell[cellIdx]->numerology);
   numSlotsInCurrPeriodicity = calcNumSlotsInCurrPeriodicity(macCfgParams.tddCfg.tddPeriod, macCb.macCell[cellIdx]->numerology);

   if(numSlotsInCurrPeriodicity == 0)
   {
      DU_LOG("\nERROR  --> LWR_MAC: CONFIG_REQ: numSlotsInCurrPeriodicity is 0 thus exiting");
      return RFAILED;
   }
   DU_LOG("\nINFO   --> LWR_MAC: CONFIG_REQ: numberofTDDSlot in MAX_PERIOICITY(10ms) = %d", numSlotsInMaxPeriodicity);
   DU_LOG("\nINFO   --> LWR_MAC: CONFIG_REQ: numberofTDDSlot in CURRENT PERIOICITY(enumVal = %d) = %d\n", macCfgParams.tddCfg.tddPeriod, numSlotsInCurrPeriodicity);
   //configReq->number_of_tlvs = 25 + 1 + MAX_TDD_PERIODICITY_SLOTS * MAX_SYMB_PER_SLOT;
   totalTlv = 26 + 1+ numSlotsInMaxPeriodicity * MAX_SYMB_PER_SLOT;
#endif
   /* totalCfgReqMsgLen = size of config req's msg header + size of tlv supporting + size of tlv supporting *sizeof(fapi_uint32_tlv_t)   */
   totalCfgReqMsgLen += sizeof(configReq->header) + sizeof( configReq->number_of_tlvs) + totalTlv*sizeof(fapi_uint32_tlv_t);
   
   /* Fill FAPI config req */
   LWR_MAC_ALLOC(configReq, sizeof(fapi_config_req_t));
   memset(configReq, 0, sizeof(fapi_config_req_t));
   fillMsgHeader(&configReq->header, FAPI_CONFIG_REQUEST, totalCfgReqMsgLen);
   configReq->number_of_tlvs = totalTlv;
   msgLen = sizeof(configReq->number_of_tlvs);

   fillTlvs(&configReq->tlvs[index++], FAPI_DL_BANDWIDTH_TAG,           \
         sizeof(uint16_t), macCfgParams.carrCfg.dlBw << TLV_ALIGN(16) , &msgLen);
   dlFreq = convertArfcnToFreqKhz(macCfgParams.carrCfg.arfcnDL);
   fillTlvs(&configReq->tlvs[index++], FAPI_DL_FREQUENCY_TAG,           \
         sizeof(uint32_t), dlFreq << TLV_ALIGN(32), &msgLen);
   /* Due to bug in Intel FT code, commenting TLVs that are are not
    * needed to avoid error. Must be uncommented when FT bug is fixed */
   //fillTlvs(&configReq->tlvs[index++], FAPI_DL_K0_TAG,                  \
   sizeof(uint16_t), macCfgParams.dlCarrCfg.k0[0], &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_DL_GRIDSIZE_TAG,            \
   sizeof(uint16_t), macCfgParams.dlCarrCfg.gridSize[0], &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_NUM_TX_ANT_TAG,             \
         sizeof(uint16_t), macCfgParams.carrCfg.numTxAnt << TLV_ALIGN(16), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_UPLINK_BANDWIDTH_TAG,       \
         sizeof(uint16_t), macCfgParams.carrCfg.ulBw << TLV_ALIGN(16), &msgLen);
   ulFreq = convertArfcnToFreqKhz(macCfgParams.carrCfg.arfcnUL);
   fillTlvs(&configReq->tlvs[index++], FAPI_UPLINK_FREQUENCY_TAG,       \
         sizeof(uint32_t), ulFreq << TLV_ALIGN(32), &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_UL_K0_TAG,                  \
   sizeof(uint16_t), macCfgParams.ulCarrCfg.k0[0], &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_UL_GRID_SIZE_TAG,           \
   sizeof(uint16_t), macCfgParams.ulCarrCfg.gridSize[0], &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_NUM_RX_ANT_TAG,             \
         sizeof(uint16_t), macCfgParams.carrCfg.numRxAnt << TLV_ALIGN(16), &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_FREQUENCY_SHIFT_7P5_KHZ_TAG,   \
   sizeof(uint8_t), macCfgParams.freqShft, &msgLen);

   /* fill cell config */
   fillTlvs(&configReq->tlvs[index++], FAPI_PHY_CELL_ID_TAG,               \
         sizeof(uint16_t), macCfgParams.cellCfg.phyCellId << TLV_ALIGN(16), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_FRAME_DUPLEX_TYPE_TAG,         \
         sizeof(uint8_t), macCfgParams.cellCfg.dupType << TLV_ALIGN(8), &msgLen);

   /* fill SSB configuration */
   fillTlvs(&configReq->tlvs[index++], FAPI_SS_PBCH_POWER_TAG,             \
         sizeof(uint32_t), macCfgParams.ssbCfg.ssbPbchPwr << TLV_ALIGN(32), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_BCH_PAYLOAD_TAG,               \
   sizeof(uint8_t), macCfgParams.ssbCfg.bchPayloadFlag<<TLV_ALIGN(8), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_SCS_COMMON_TAG,                \
         sizeof(uint8_t), macCfgParams.ssbCfg.scsCmn << TLV_ALIGN(8), &msgLen);

   /* fill PRACH configuration */
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_SEQUENCE_LENGTH_TAG,     \
   sizeof(uint8_t), macCfgParams.prachCfg.prachSeqLen << TLV_ALIGN(8), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_SUBC_SPACING_TAG,        \
         sizeof(uint8_t), convertScsValToScsEnum(macCfgParams.prachCfg.prachSubcSpacing) << TLV_ALIGN(8), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_RESTRICTED_SET_CONFIG_TAG,     \
         sizeof(uint8_t), macCfgParams.prachCfg.prachRstSetCfg << TLV_ALIGN(8), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_NUM_PRACH_FD_OCCASIONS_TAG,
         sizeof(uint8_t), macCfgParams.prachCfg.msg1Fdm << TLV_ALIGN(8), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_CONFIG_INDEX_TAG,
         sizeof(uint8_t), macCfgParams.prachCfg.prachCfgIdx << TLV_ALIGN(8), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_ROOT_SEQUENCE_INDEX_TAG, \
         sizeof(uint16_t), macCfgParams.prachCfg.fdm[0].rootSeqIdx << TLV_ALIGN(16), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_NUM_ROOT_SEQUENCES_TAG,        \
   sizeof(uint8_t), macCfgParams.prachCfg.fdm[0].numRootSeq << TLV_ALIGN(8), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_K1_TAG,                        \
         sizeof(uint16_t), macCfgParams.prachCfg.fdm[0].k1 << TLV_ALIGN(16), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_ZERO_CORR_CONF_TAG ,     \
         sizeof(uint8_t), macCfgParams.prachCfg.fdm[0].zeroCorrZoneCfg << TLV_ALIGN(8), &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_NUM_UNUSED_ROOT_SEQUENCES_TAG, \
   sizeof(uint8_t), macCfgParams.prachCfg.fdm[0].numUnusedRootSeq, &msgLen);
   /* if(macCfgParams.prachCfg.fdm[0].numUnusedRootSeq)
      {
      for(idx = 0; idx < macCfgParams.prachCfg.fdm[0].numUnusedRootSeq; idx++)
      fillTlvs(&configReq->tlvs[index++], FAPI_UNUSED_ROOT_SEQUENCES_TAG,   \
      sizeof(uint8_t), macCfgParams.prachCfg.fdm[0].unsuedRootSeq[idx], \
      &msgLen);
      }
      else
      {
      macCfgParams.prachCfg.fdm[0].unsuedRootSeq = NULL;
      }*/

   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_PER_RACH_TAG,              \
         sizeof(uint8_t), macCfgParams.prachCfg.ssbPerRach << TLV_ALIGN(8), &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_MULTIPLE_CARRIERS_IN_A_BAND_TAG,  \
   sizeof(uint8_t), macCfgParams.prachCfg.prachMultCarrBand, &msgLen);

   /* fill SSB table */
   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_OFFSET_POINT_A_TAG,        \
         sizeof(uint16_t), macCfgParams.ssbCfg.ssbOffsetPointA << TLV_ALIGN(16), &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_BETA_PSS_TAG,                  \
   sizeof(uint8_t),  macCfgParams.ssbCfg.betaPss, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_PERIOD_TAG,                \
         sizeof(uint8_t),  macCfgParams.ssbCfg.ssbPeriod << TLV_ALIGN(8), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_SUBCARRIER_OFFSET_TAG,     \
         sizeof(uint8_t),  macCfgParams.ssbCfg.ssbScOffset << TLV_ALIGN(8), &msgLen);

   setMibPdu(macCfgParams.ssbCfg.mibPdu, &mib, 0);
   fillTlvs(&configReq->tlvs[index++], FAPI_MIB_TAG ,                      \
         sizeof(uint32_t), mib << TLV_ALIGN(32), &msgLen);

   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_MASK_TAG,                  \
         sizeof(uint32_t), macCfgParams.ssbCfg.ssbMask[0] << TLV_ALIGN(32), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_BEAM_ID_TAG,                   \
         sizeof(uint8_t),  macCfgParams.ssbCfg.beamId[0] << TLV_ALIGN(8), &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_SS_PBCH_MULTIPLE_CARRIERS_IN_A_BAND_TAG, \
   sizeof(uint8_t), macCfgParams.ssbCfg.multCarrBand, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_MULTIPLE_CELLS_SS_PBCH_IN_A_CARRIER_TAG, \
   sizeof(uint8_t), macCfgParams.ssbCfg.multCellCarr, &msgLen);

#ifdef NR_TDD
   /* fill TDD table */
   fillTlvs(&configReq->tlvs[index++], FAPI_TDD_PERIOD_TAG,                \
         sizeof(uint8_t), macCfgParams.tddCfg.tddPeriod << TLV_ALIGN(8), &msgLen);

   cntSlotCfg = numSlotsInMaxPeriodicity/numSlotsInCurrPeriodicity;
   while(cntSlotCfg)
   {
      for(slotIdx =0 ;slotIdx < numSlotsInCurrPeriodicity; slotIdx++) 
      {
         for(symbolIdx = 0; symbolIdx < MAX_SYMB_PER_SLOT; symbolIdx++)
         {
            /*Fill Full-DL Slots as well as DL symbols ini 1st Flexi Slo*/
            if(slotIdx < macCfgParams.tddCfg.nrOfDlSlots || \
                  (slotIdx == macCfgParams.tddCfg.nrOfDlSlots && symbolIdx < macCfgParams.tddCfg.nrOfDlSymbols)) 
            {
               fillTlvs(&configReq->tlvs[index++], FAPI_SLOT_CONFIG_TAG,               \
                    sizeof(uint8_t), DL_SYMBOL<< TLV_ALIGN(8), &msgLen);
            }

            /*Fill Full-FLEXI SLOT and as well as Flexi Symbols in 1 slot preceding FULL-UL slot*/ 
            else if(slotIdx < (numSlotsInCurrPeriodicity - macCfgParams.tddCfg.nrOfUlSlots -1) ||  \
                     (slotIdx == (numSlotsInCurrPeriodicity - macCfgParams.tddCfg.nrOfUlSlots -1) && \
                     symbolIdx < (MAX_SYMB_PER_SLOT - macCfgParams.tddCfg.nrOfUlSymbols)))
            {
               fillTlvs(&configReq->tlvs[index++], FAPI_SLOT_CONFIG_TAG,               \
                     sizeof(uint8_t), FLEXI_SYMBOL<< TLV_ALIGN(8), &msgLen);
            }
            /*Fill Partial UL symbols and Full-UL slot*/
            else
            {
               fillTlvs(&configReq->tlvs[index++], FAPI_SLOT_CONFIG_TAG,               \
                     sizeof(uint8_t), UL_SYMBOL<< TLV_ALIGN(8), &msgLen);
            }
         }
      }
      cntSlotCfg--;
   }
#endif

   /* fill measurement config */
   //fillTlvs(&configReq->tlvs[index++], FAPI_RSSI_MEASUREMENT_TAG,          \
   sizeof(uint8_t), macCfgParams.rssiUnit, &msgLen);

   /* fill DMRS Type A Pos */
   // fillTlvs(&configReq->tlvs[index++], FAPI_DMRS_TYPE_A_POS_TAG,           \
   sizeof(uint8_t), macCfgParams.ssbCfg.dmrsTypeAPos, &msgLen);
   uint32_t  bufferLen=0;
   uint8_t mBuf[2500];
   packConfigReq(configReq, mBuf, &bufferLen);
   //hexdump1(mBuf, bufferLen);
   LWR_MAC_ALLOC(cfgReqQElem,(sizeof(fapi_api_queue_elem_t)+bufferLen));

   if(!cfgReqQElem)
   {
	   DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for config req");
	   return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(cfgReqQElem, NULLP, FAPI_CONFIG_REQUEST, 1,  bufferLen);
   memcpy((uint8_t *)(cfgReqQElem +1), mBuf, bufferLen);

   /* Fill message header */
   LWR_MAC_ALLOC(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
   if(!headerElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for vendor msg in config req");
      LWR_MAC_FREE(cfgReqQElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_config_req_t)));
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(headerElem, cfgReqQElem, FAPI_VENDOR_MSG_HEADER_IND, 1, \
         sizeof(fapi_msg_header_t));
   msgHeader = (fapi_msg_header_t*)(headerElem+1);
   msgHeader->num_msg = 1; /* Config req msg */
   msgHeader->handle = 0;  
   
   DU_LOG("\nDEBUG  -->  LWR_MAC: Sending Config Request to Phy");
   LwrMacSendToL1(headerElem);
   LWR_MAC_FREE(configReq, sizeof(fapi_config_req_t));
   return ROK;
#endif
}
#endif

/*******************************************************************
 *
 * @brief Sends FAPI Config req to PHY
 *
 * @details
 *
 *    Function : lwr_mac_procConfigReqEvt
 *
 *    Functionality:
 *         -Sends FAPI Config Req to PHY
 *
 * @params[in]
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/

uint8_t lwr_mac_procConfigReqEvt(void *msg)
{
#ifndef OAI_TESTING 
#ifdef INTEL_FAPI
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : CONFIG_REQ\n");
#endif
#ifdef NR_TDD
   uint8_t slotIdx = 0; 
   uint8_t symbolIdx =0;
   uint8_t numSlotsInMaxPeriodicity = 0; /*number of TDD Slots in MAX_PERIODICITY(10ms) as per numerology*/
   uint8_t numSlotsInCurrPeriodicity = 0; /*number of TDD Slots in Configured_PERIODICITY(0.5ms to 10ms) as per numerology*/
   uint8_t cntSlotCfg = 0; /*number of Slot Cfg repeatition*/
#endif   
   uint16_t index = 0;
   uint16_t *cellId =NULLP;
   uint16_t cellIdx =0;
   uint32_t msgLen = 0;
   uint32_t mib = 0;
   uint32_t dlFreq = 0, ulFreq = 0;
   MacCellCfg macCfgParams;
   fapi_vendor_msg_t *vendorMsg;
   fapi_config_req_t *configReq;
   fapi_msg_header_t *msgHeader;
   p_fapi_api_queue_elem_t  headerElem;
   p_fapi_api_queue_elem_t  vendorMsgQElem;
   p_fapi_api_queue_elem_t  cfgReqQElem;

   DU_LOG("\nINFO  -->  LWR_MAC: Received EVENT[%d] at STATE[%d]", lwrMacCb.event, \
         lwrMacCb.phyState);

   cellId = (uint16_t *)msg;
   GET_CELL_IDX(*cellId, cellIdx);
   macCfgParams = macCb.macCell[cellIdx]->macCellCfg;

   /* Fill Cell Configuration in lwrMacCb */
   memset(&lwrMacCb.cellCb[lwrMacCb.numCell], 0, sizeof(LwrMacCellCb));
   lwrMacCb.cellCb[lwrMacCb.numCell].cellId = macCfgParams.cellId;
   lwrMacCb.cellCb[lwrMacCb.numCell].phyCellId = macCfgParams.cellCfg.phyCellId; 
   lwrMacCb.numCell++;
#ifdef NR_TDD
   numSlotsInMaxPeriodicity = MAX_TDD_PERIODICITY * pow(2, macCb.macCell[cellIdx]->numerology);
   numSlotsInCurrPeriodicity = calcNumSlotsInCurrPeriodicity(macCfgParams.tddCfg.tddPeriod, macCb.macCell[cellIdx]->numerology);

   if(numSlotsInCurrPeriodicity == 0)
   {
      DU_LOG("\nERROR  --> LWR_MAC: CONFIG_REQ: numSlotsInCurrPeriodicity is 0 thus exiting");
      return RFAILED;
   }
   DU_LOG("\nINFO   --> LWR_MAC: CONFIG_REQ: numberofTDDSlot in MAX_PERIOICITY(10ms) = %d", numSlotsInMaxPeriodicity);
   DU_LOG("\nINFO   --> LWR_MAC: CONFIG_REQ: numberofTDDSlot in CURRENT PERIOICITY(enumVal = %d) = %d\n", macCfgParams.tddCfg.tddPeriod, numSlotsInCurrPeriodicity);
#endif

   /* Allocte And fill Vendor msg */
   LWR_MAC_ALLOC(vendorMsgQElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_msg_t)));  
   if(!vendorMsgQElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for vendor msg in config req");
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(vendorMsgQElem, NULLP, FAPI_VENDOR_MESSAGE, 1, sizeof(fapi_vendor_msg_t)); 
   vendorMsg = (fapi_vendor_msg_t *)(vendorMsgQElem + 1);
   fillMsgHeader(&vendorMsg->header, FAPI_VENDOR_MESSAGE, sizeof(fapi_vendor_msg_t));
   vendorMsg->config_req_vendor.hopping_id = 0;
   vendorMsg->config_req_vendor.carrier_aggregation_level = 0;
   vendorMsg->config_req_vendor.group_hop_flag = 0;
   vendorMsg->config_req_vendor.sequence_hop_flag = 0;
   vendorMsg->config_req_vendor.urllc_capable = 0;
   vendorMsg->config_req_vendor.urllc_mini_slot_mask =0;
   vendorMsg->config_req_vendor.nr_of_dl_ports =1;
   vendorMsg->config_req_vendor.nr_of_ul_ports =1;
   vendorMsg->config_req_vendor.prach_nr_of_rx_ru =1;
   vendorMsg->config_req_vendor.ssb_subc_spacing =1;
   vendorMsg->config_req_vendor.use_vendor_EpreXSSB = USE_VENDOR_EPREXSSB;
   vendorMsg->start_req_vendor.sfn = 0;
   vendorMsg->start_req_vendor.slot = 0;
   vendorMsg->start_req_vendor.mode = 4;
#ifdef DEBUG_MODE
   vendorMsg->start_req_vendor.count = 0;
   vendorMsg->start_req_vendor.period = 1;
#endif
   /* Fill FAPI config req */
   LWR_MAC_ALLOC(cfgReqQElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_config_req_t)));
   if(!cfgReqQElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for config req");
      LWR_MAC_FREE(vendorMsgQElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_msg_t)));
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(cfgReqQElem, vendorMsgQElem, FAPI_CONFIG_REQUEST, 1, \
         sizeof(fapi_config_req_t));

   configReq = (fapi_config_req_t *)(cfgReqQElem + 1);
   memset(configReq, 0, sizeof(fapi_config_req_t));
   fillMsgHeader(&configReq->header, FAPI_CONFIG_REQUEST, sizeof(fapi_config_req_t));
#ifndef NR_TDD
   configReq->number_of_tlvs = 25;
#else
   configReq->number_of_tlvs = 25 + 1 + numSlotsInMaxPeriodicity * MAX_SYMB_PER_SLOT;
#endif

   msgLen = sizeof(configReq->number_of_tlvs);

   fillTlvs(&configReq->tlvs[index++], FAPI_DL_BANDWIDTH_TAG,           \
         sizeof(uint32_t), macCfgParams.carrCfg.dlBw, &msgLen);
   dlFreq = convertArfcnToFreqKhz(macCfgParams.carrCfg.arfcnDL);
   fillTlvs(&configReq->tlvs[index++], FAPI_DL_FREQUENCY_TAG,           \
         sizeof(uint32_t), dlFreq, &msgLen);
   /* Due to bug in Intel FT code, commenting TLVs that are are not 
    * needed to avoid error. Must be uncommented when FT bug is fixed */
   //fillTlvs(&configReq->tlvs[index++], FAPI_DL_K0_TAG,                  \
   sizeof(uint16_t), macCfgParams.dlCarrCfg.k0[0], &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_DL_GRIDSIZE_TAG,            \
   sizeof(uint16_t), macCfgParams.dlCarrCfg.gridSize[0], &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_NUM_TX_ANT_TAG,             \
         sizeof(uint16_t), macCfgParams.carrCfg.numTxAnt, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_UPLINK_BANDWIDTH_TAG,       \
         sizeof(uint32_t), macCfgParams.carrCfg.ulBw, &msgLen);
   ulFreq = convertArfcnToFreqKhz(macCfgParams.carrCfg.arfcnUL);
   fillTlvs(&configReq->tlvs[index++], FAPI_UPLINK_FREQUENCY_TAG,       \
         sizeof(uint32_t), ulFreq, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_UL_K0_TAG,                  \
   sizeof(uint16_t), macCfgParams.ulCarrCfg.k0[0], &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_UL_GRID_SIZE_TAG,           \
   sizeof(uint16_t), macCfgParams.ulCarrCfg.gridSize[0], &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_NUM_RX_ANT_TAG,             \
         sizeof(uint16_t), macCfgParams.carrCfg.numRxAnt, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_FREQUENCY_SHIFT_7P5_KHZ_TAG,   \
   sizeof(uint8_t), macCfgParams.freqShft, &msgLen);

   /* fill cell config */
   fillTlvs(&configReq->tlvs[index++], FAPI_PHY_CELL_ID_TAG,               \
         sizeof(uint8_t), macCfgParams.cellCfg.phyCellId, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_FRAME_DUPLEX_TYPE_TAG,         \
         sizeof(uint8_t), macCfgParams.cellCfg.dupType, &msgLen);

   /* fill SSB configuration */
   fillTlvs(&configReq->tlvs[index++], FAPI_SS_PBCH_POWER_TAG,             \
         sizeof(uint32_t), macCfgParams.ssbCfg.ssbPbchPwr, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_BCH_PAYLOAD_TAG,               \
   sizeof(uint8_t), macCfgParams.ssbCfg.bchPayloadFlag, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_SCS_COMMON_TAG,                \
         sizeof(uint8_t), macCfgParams.ssbCfg.scsCmn, &msgLen);

   /* fill PRACH configuration */
   //fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_SEQUENCE_LENGTH_TAG,     \
   sizeof(uint8_t), macCfgParams.prachCfg.prachSeqLen, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_SUBC_SPACING_TAG,        \
         sizeof(uint8_t), convertScsValToScsEnum(macCfgParams.prachCfg.prachSubcSpacing), &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_RESTRICTED_SET_CONFIG_TAG,     \
         sizeof(uint8_t), macCfgParams.prachCfg.prachRstSetCfg, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_NUM_PRACH_FD_OCCASIONS_TAG,
         sizeof(uint8_t), macCfgParams.prachCfg.msg1Fdm, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_CONFIG_INDEX_TAG,
         sizeof(uint8_t), macCfgParams.prachCfg.prachCfgIdx, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_ROOT_SEQUENCE_INDEX_TAG, \
         sizeof(uint16_t), macCfgParams.prachCfg.fdm[0].rootSeqIdx, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_NUM_ROOT_SEQUENCES_TAG,        \
   sizeof(uint8_t), macCfgParams.prachCfg.fdm[0].numRootSeq, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_K1_TAG,                        \
         sizeof(uint16_t), macCfgParams.prachCfg.fdm[0].k1, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_ZERO_CORR_CONF_TAG ,     \
         sizeof(uint8_t), macCfgParams.prachCfg.fdm[0].zeroCorrZoneCfg, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_NUM_UNUSED_ROOT_SEQUENCES_TAG, \
   sizeof(uint8_t), macCfgParams.prachCfg.fdm[0].numUnusedRootSeq, &msgLen);
   /* if(macCfgParams.prachCfg.fdm[0].numUnusedRootSeq)
      {
      for(idx = 0; idx < macCfgParams.prachCfg.fdm[0].numUnusedRootSeq; idx++)
      fillTlvs(&configReq->tlvs[index++], FAPI_UNUSED_ROOT_SEQUENCES_TAG,   \
      sizeof(uint8_t), macCfgParams.prachCfg.fdm[0].unsuedRootSeq[idx], \
      &msgLen);
      }
      else
      {
      macCfgParams.prachCfg.fdm[0].unsuedRootSeq = NULL;
      }*/

   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_PER_RACH_TAG,              \
         sizeof(uint8_t), macCfgParams.prachCfg.ssbPerRach, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_PRACH_MULTIPLE_CARRIERS_IN_A_BAND_TAG,  \
   sizeof(uint8_t), macCfgParams.prachCfg.prachMultCarrBand, &msgLen);

   /* fill SSB table */
   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_OFFSET_POINT_A_TAG,        \
         sizeof(uint16_t), macCfgParams.ssbCfg.ssbOffsetPointA, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_BETA_PSS_TAG,                  \
   sizeof(uint8_t),  macCfgParams.ssbCfg.betaPss, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_PERIOD_TAG,                \
         sizeof(uint8_t),  macCfgParams.ssbCfg.ssbPeriod, &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_SUBCARRIER_OFFSET_TAG,     \
         sizeof(uint8_t),  macCfgParams.ssbCfg.ssbScOffset, &msgLen);

   setMibPdu(macCfgParams.ssbCfg.mibPdu, &mib, 0);
   fillTlvs(&configReq->tlvs[index++], FAPI_MIB_TAG ,                      \
         sizeof(uint32_t), mib, &msgLen);

   fillTlvs(&configReq->tlvs[index++], FAPI_SSB_MASK_TAG,                  \
         sizeof(uint32_t), macCfgParams.ssbCfg.ssbMask[0], &msgLen);
   fillTlvs(&configReq->tlvs[index++], FAPI_BEAM_ID_TAG,                   \
         sizeof(uint8_t),  macCfgParams.ssbCfg.beamId[0], &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_SS_PBCH_MULTIPLE_CARRIERS_IN_A_BAND_TAG, \
   sizeof(uint8_t), macCfgParams.ssbCfg.multCarrBand, &msgLen);
   //fillTlvs(&configReq->tlvs[index++], FAPI_MULTIPLE_CELLS_SS_PBCH_IN_A_CARRIER_TAG, \
   sizeof(uint8_t), macCfgParams.ssbCfg.multCellCarr, &msgLen);

#ifdef NR_TDD
   /* fill TDD table */
   fillTlvs(&configReq->tlvs[index++], FAPI_TDD_PERIOD_TAG,                \
         sizeof(uint8_t), macCfgParams.tddCfg.tddPeriod, &msgLen);

   cntSlotCfg = numSlotsInMaxPeriodicity/numSlotsInCurrPeriodicity;
   while(cntSlotCfg)
   {
      for(slotIdx =0 ;slotIdx < numSlotsInCurrPeriodicity; slotIdx++) 
      {
         for(symbolIdx = 0; symbolIdx < MAX_SYMB_PER_SLOT; symbolIdx++)
         {
            /*Fill Full-DL Slots as well as DL symbols ini 1st Flexi Slo*/
            if(slotIdx < macCfgParams.tddCfg.nrOfDlSlots || \
                  (slotIdx == macCfgParams.tddCfg.nrOfDlSlots && symbolIdx < macCfgParams.tddCfg.nrOfDlSymbols)) 
            {
               fillTlvs(&configReq->tlvs[index++], FAPI_SLOT_CONFIG_TAG,               \
                    sizeof(uint8_t), DL_SYMBOL, &msgLen);
            }

            /*Fill Full-FLEXI SLOT and as well as Flexi Symbols in 1 slot preceding FULL-UL slot*/ 
            else if(slotIdx < (numSlotsInCurrPeriodicity - macCfgParams.tddCfg.nrOfUlSlots -1) ||  \
                     (slotIdx == (numSlotsInCurrPeriodicity - macCfgParams.tddCfg.nrOfUlSlots -1) && \
                     symbolIdx < (MAX_SYMB_PER_SLOT - macCfgParams.tddCfg.nrOfUlSymbols)))
            {
               fillTlvs(&configReq->tlvs[index++], FAPI_SLOT_CONFIG_TAG,               \
                     sizeof(uint8_t), FLEXI_SYMBOL, &msgLen);
            }
            /*Fill Partial UL symbols and Full-UL slot*/
            else
            {
               fillTlvs(&configReq->tlvs[index++], FAPI_SLOT_CONFIG_TAG,               \
                     sizeof(uint8_t), UL_SYMBOL, &msgLen);
            }
         }
      }
      cntSlotCfg--;
   }
#endif   

   /* fill measurement config */
   //fillTlvs(&configReq->tlvs[index++], FAPI_RSSI_MEASUREMENT_TAG,          \
   sizeof(uint8_t), macCfgParams.rssiUnit, &msgLen);

   /* fill DMRS Type A Pos */
   fillTlvs(&configReq->tlvs[index++], FAPI_DMRS_TYPE_A_POS_TAG,           \
         sizeof(uint8_t), macCfgParams.ssbCfg.dmrsTypeAPos, &msgLen);

   /* Fill message header */
   LWR_MAC_ALLOC(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
   if(!headerElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for vendor msg in config req");
      LWR_MAC_FREE(cfgReqQElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_config_req_t)));
      LWR_MAC_FREE(vendorMsgQElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_msg_t)));
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(headerElem, cfgReqQElem, FAPI_VENDOR_MSG_HEADER_IND, 1, \
         sizeof(fapi_msg_header_t));
   msgHeader = (fapi_msg_header_t *)(headerElem + 1);
   msgHeader->num_msg = 2; /* Config req msg and vendor specific msg */
   msgHeader->handle = 0;

   DU_LOG("\nDEBUG  -->  LWR_MAC: Sending Config Request to Phy");
   LwrMacSendToL1(headerElem);
#endif
#else
   buildAndSendOAIConfigReqToL1(msg);
#endif
   return ROK;
} /* lwr_mac_handleConfigReqEvt */

/*******************************************************************
 *
 * @brief Processes config response from phy
 *
 * @details
 *
 *    Function : lwr_mac_procConfigRspEvt
 *
 *    Functionality:
 *          Processes config response from phy
 *
 * @params[in] FAPI message pointer 
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/

uint8_t lwr_mac_procConfigRspEvt(void *msg)
{
#ifdef INTEL_FAPI
   fapi_config_resp_t *configRsp;
   configRsp = (fapi_config_resp_t *)msg;

   DU_LOG("\nINFO  -->  LWR_MAC: Received EVENT[%d] at STATE[%d]", lwrMacCb.event, \
	 lwrMacCb.phyState);

   if(configRsp != NULL)
   {
      if(configRsp->error_code == MSG_OK)
      {
	 DU_LOG("\nDEBUG  -->  LWR_MAC: PHY has moved to Configured state \n");
	 lwrMacCb.phyState = PHY_STATE_CONFIGURED;
	 lwrMacCb.cellCb[0].state = PHY_STATE_CONFIGURED;
	 /* TODO : 
	  * Store config response into an intermediate struture and send to MAC
	  * Support LC and LWLC for sending config rsp to MAC 
	  */
	 fapiMacConfigRsp(lwrMacCb.cellCb[0].cellId);
      }
      else
      {
	 DU_LOG("\nERROR  -->  LWR_MAC: Invalid error code %d", configRsp->error_code);
	 return RFAILED;
      }
   }
   else
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Config Response received from PHY is NULL");
      return RFAILED;
   }
#endif

   return ROK;
} /* lwr_mac_procConfigRspEvt */

/*******************************************************************
 *
 * @brief Build and send start request to phy
 *
 * @details
 *
 *    Function : lwr_mac_procStartReqEvt
 *
 *    Functionality:
 *       Build and send start request to phy
 *
 * @params[in] FAPI message pointer
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint8_t lwr_mac_procStartReqEvt(void *msg)
{
#ifndef OAI_TESTING 
#ifdef INTEL_FAPI
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : START_REQ\n");
#endif
   fapi_msg_header_t *msgHeader;
   fapi_start_req_t *startReq;
   fapi_vendor_msg_t *vendorMsg;
   p_fapi_api_queue_elem_t  headerElem;
   p_fapi_api_queue_elem_t  startReqElem;
   p_fapi_api_queue_elem_t  vendorMsgElem;

   /* Allocte And fill Vendor msg */
   LWR_MAC_ALLOC(vendorMsgElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_msg_t)));
   if(!vendorMsgElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for vendor msg in start req");
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(vendorMsgElem, NULLP, FAPI_VENDOR_MESSAGE, 1, sizeof(fapi_vendor_msg_t));
   vendorMsg = (fapi_vendor_msg_t *)(vendorMsgElem + 1);
   fillMsgHeader(&vendorMsg->header, FAPI_VENDOR_MESSAGE, sizeof(fapi_vendor_msg_t));
   vendorMsg->start_req_vendor.sfn = 0;
   vendorMsg->start_req_vendor.slot = 0;
   vendorMsg->start_req_vendor.mode = 4; /* for Radio mode */
#ifdef DEBUG_MODE
   vendorMsg->start_req_vendor.count = 0;
   vendorMsg->start_req_vendor.period = 1;
#endif

   /* Fill FAPI config req */
   LWR_MAC_ALLOC(startReqElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_start_req_t)));
   if(!startReqElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for start req");
      LWR_MAC_FREE(vendorMsgElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_msg_t)));
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(startReqElem, vendorMsgElem, FAPI_START_REQUEST, 1, \
      sizeof(fapi_start_req_t));

   startReq = (fapi_start_req_t *)(startReqElem + 1);
   memset(startReq, 0, sizeof(fapi_start_req_t));
   fillMsgHeader(&startReq->header, FAPI_START_REQUEST, sizeof(fapi_start_req_t));

   /* Fill message header */
   LWR_MAC_ALLOC(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
   if(!headerElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for vendor msg in config req");
      LWR_MAC_FREE(startReqElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_start_req_t)));
      LWR_MAC_FREE(vendorMsgElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_msg_t)));
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(headerElem, startReqElem, FAPI_VENDOR_MSG_HEADER_IND, 1, \
      sizeof(fapi_msg_header_t));
   msgHeader = (fapi_msg_header_t *)(headerElem + 1);
   msgHeader->num_msg = 2; /* Start req msg and vendor specific msg */
   msgHeader->handle = 0;

   /* Send to PHY */
   DU_LOG("\nDEBUG  -->  LWR_MAC: Sending Start Request to Phy");
   LwrMacSendToL1(headerElem);
#endif
#else
   fapi_msg_header_t *msgHeader;
   fapi_start_req_t *startReq;
   p_fapi_api_queue_elem_t  headerElem;
   p_fapi_api_queue_elem_t  startReqElem;

   /* Fill FAPI config req */
   LWR_MAC_ALLOC(startReqElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_start_req_t)));
   if(!startReqElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for start req");
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(startReqElem, NULLP, FAPI_START_REQUEST, 1, sizeof(fapi_start_req_t));
   startReq = (fapi_start_req_t *)(startReqElem + 1);
   memset(startReq, 0, sizeof(fapi_start_req_t));
   fillMsgHeader(&startReq->header, FAPI_START_REQUEST, sizeof(fapi_start_req_t));
   /* Fill message header */
   LWR_MAC_ALLOC(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
   if(!headerElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for vendor msg in config req");
      LWR_MAC_FREE(startReqElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_start_req_t)));
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(headerElem, startReqElem, FAPI_VENDOR_MSG_HEADER_IND, 1, \
         sizeof(fapi_msg_header_t));
   msgHeader = (fapi_msg_header_t *)(headerElem + 1);
   msgHeader->num_msg = 1; /* Start req msg */
   msgHeader->handle = 0;
   /* Send to PHY */
   DU_LOG("\nDEBUG  -->  LWR_MAC: Sending Start Request to Phy");
   LwrMacSendToL1(headerElem);
#endif
   return ROK;
} /* lwr_mac_procStartReqEvt */

/*******************************************************************
 *
 * @brief Sends FAPI Stop Req to PHY
 *
 * @details
 *
 *    Function : lwr_mac_procStopReqEvt
 *
 *    Functionality:
 *         -Sends FAPI Stop Req to PHY
 *
 * @params[in]
 * @return ROK     - success
 *         RFAILED - failure
 *
 ********************************************************************/

uint8_t lwr_mac_procStopReqEvt(SlotTimingInfo slotInfo, p_fapi_api_queue_elem_t  prevElem, fapi_stop_req_vendor_msg_t *vendorMsg)
{
#ifdef INTEL_FAPI
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : STOP_REQ\n");
#endif

   fapi_stop_req_t   *stopReq;
   p_fapi_api_queue_elem_t  stopReqElem;

#ifndef OAI_TESTING
   vendorMsg->sfn = slotInfo.sfn;
   vendorMsg->slot = slotInfo.slot;
#endif
   /* Fill FAPI stop req */
   LWR_MAC_ALLOC(stopReqElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_stop_req_t)));
   if(!stopReqElem)
   {
      DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for stop req");
      return RFAILED;
   }
   FILL_FAPI_LIST_ELEM(stopReqElem, NULLP, FAPI_STOP_REQUEST, 1, sizeof(fapi_stop_req_t));
   stopReq = (fapi_stop_req_t *)(stopReqElem + 1);
   memset(stopReq, 0, sizeof(fapi_stop_req_t));
   fillMsgHeader(&stopReq->header, FAPI_STOP_REQUEST, sizeof(fapi_stop_req_t));

   /* Send to PHY */
   DU_LOG("\nINFO  -->  LWR_MAC: Sending Stop Request to Phy");
   prevElem->p_next = stopReqElem;

#endif
   return ROK;
}

#ifdef INTEL_FAPI
/*******************************************************************
 *
 * @brief fills SSB PDU required for DL TTI info in MAC
 *
 * @details
 *
 *    Function : fillSsbPdu
 *
 *    Functionality:
 *         -Fills the SSB PDU info
 *          stored in MAC
 *
 * @params[in] Pointer to FAPI DL TTI Req
 *             Pointer to RgCellCb
 *             Pointer to msgLen of DL TTI Info
 * @return ROK
 *
 ******************************************************************/

uint8_t fillSsbPdu(fapi_dl_tti_req_pdu_t *dlTtiReqPdu, MacCellCfg *macCellCfg,
      MacDlSlot *currDlSlot, uint8_t ssbIdxCount, uint16_t sfn)
{
   uint32_t mibPayload = 0;
   if(dlTtiReqPdu != NULL)
   {
#ifdef OAI_TESTING 
      dlTtiReqPdu->pduType = reverseBytes16(SSB_PDU_TYPE);     /* SSB PDU */
      dlTtiReqPdu->pduSize = reverseBytes16(sizeof(fapi_dl_ssb_pdu_t));  /* Size of SSB PDU */
      dlTtiReqPdu->pdu.ssb_pdu.physCellId = reverseBytes16(macCellCfg->cellCfg.phyCellId);
      dlTtiReqPdu->pdu.ssb_pdu.ssbOffsetPointA = reverseBytes16(macCellCfg->ssbCfg.ssbOffsetPointA);
#else
      dlTtiReqPdu->pduType = (SSB_PDU_TYPE);     /* SSB PDU */
      dlTtiReqPdu->pduSize = (sizeof(fapi_dl_ssb_pdu_t));  /* Size of SSB PDU */
      dlTtiReqPdu->pdu.ssb_pdu.physCellId = (macCellCfg->cellCfg.phyCellId);
      dlTtiReqPdu->pdu.ssb_pdu.ssbOffsetPointA = (macCellCfg->ssbCfg.ssbOffsetPointA);
#endif
      dlTtiReqPdu->pdu.ssb_pdu.betaPss = macCellCfg->ssbCfg.betaPss;
      dlTtiReqPdu->pdu.ssb_pdu.ssbBlockIndex = currDlSlot->dlInfo.brdcstAlloc.ssbInfo[ssbIdxCount].ssbIdx;
      dlTtiReqPdu->pdu.ssb_pdu.ssbSubCarrierOffset = macCellCfg->ssbCfg.ssbScOffset;
      /* ssbOfPdufstA to be filled in ssbCfg */
      dlTtiReqPdu->pdu.ssb_pdu.bchPayloadFlag = macCellCfg->ssbCfg.bchPayloadFlag;
      /* Bit manipulation for SFN */
      setMibPdu(macCellCfg->ssbCfg.mibPdu, &mibPayload, sfn);
#ifdef OAI_TESTING 
      dlTtiReqPdu->pdu.ssb_pdu.bchPayload.bchPayload = reverseBytes32(mibPayload);
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.numPrgs = reverseBytes16(1);
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.prgSize = reverseBytes16(275);
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.digBfInterfaces = 1;
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.pmi_bfi[0].pmIdx = reverseBytes16(0);
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.pmi_bfi[0].beamIdx[0].beamidx = reverseBytes16(macCellCfg->ssbCfg.beamId[0]);
#else
      dlTtiReqPdu->pdu.ssb_pdu.bchPayload.bchPayload = (mibPayload);
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.numPrgs = 0;
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.prgSize = 0;
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.digBfInterfaces = 0;
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.pmi_bfi[0].pmIdx = 0;
      dlTtiReqPdu->pdu.ssb_pdu.preCodingAndBeamforming.pmi_bfi[0].beamIdx[0].beamidx = macCellCfg->ssbCfg.beamId[0];
#endif
      return ROK;
   }
   return RFAILED;
}

/*******************************************************************
 *
 * @brief fills Dl DCI PDU required for DL TTI info in MAC
 *
 * @details
 *
 *    Function : fillSib1DlDciPdu
 *
 *    Functionality:
 *         -Fills the Dl DCI PDU
 *
 * @params[in] Pointer to fapi_dl_dci_t
 *             Pointer to PdcchCfg
 * @return ROK
 *
 ******************************************************************/

void fillSib1DlDciPdu(fapi_dl_dci_t *dlDciPtr, PdcchCfg *sib1PdcchInfo)
{
   if(dlDciPtr != NULLP)
   {
      uint8_t numBytes=0;
      uint8_t bytePos=0;
      uint8_t bitPos=0;

      uint16_t coreset0Size=0;
      uint16_t rbStart=0;
      uint16_t rbLen=0;
      uint32_t freqDomResAssign=0;
      uint32_t timeDomResAssign=0;
      uint8_t  VRB2PRBMap=0;
      uint32_t modNCodScheme=0;
      uint8_t  redundancyVer=0;
      uint32_t sysInfoInd=0;
      uint32_t reserved=0;

      /* Size(in bits) of each field in DCI format 0_1 
       * as mentioned in spec 38.214 */
      uint8_t freqDomResAssignSize = 0;
      uint8_t timeDomResAssignSize = 4;
      uint8_t VRB2PRBMapSize       = 1;
      uint8_t modNCodSchemeSize    = 5;
      uint8_t redundancyVerSize    = 2;
      uint8_t sysInfoIndSize       = 1;
      uint8_t reservedSize         = 15;

#ifdef OAI_TESTING 
      dlDciPtr[0].rnti = reverseBytes16(sib1PdcchInfo->dci[0].rnti);
      dlDciPtr[0].scramblingId = reverseBytes16(sib1PdcchInfo->dci[0].scramblingId);    
      dlDciPtr[0].scramblingRnti = reverseBytes16(sib1PdcchInfo->dci[0].scramblingRnti);
      dlDciPtr[0].pc_and_bform.numPrgs = reverseBytes16(sib1PdcchInfo->dci[0].beamPdcchInfo.numPrgs);
      dlDciPtr[0].pc_and_bform.prgSize = reverseBytes16(sib1PdcchInfo->dci[0].beamPdcchInfo.prgSize);
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].pmIdx = reverseBytes16(sib1PdcchInfo->dci[0].beamPdcchInfo.prg[0].pmIdx);
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = reverseBytes16(sib1PdcchInfo->dci[0].beamPdcchInfo.prg[0].beamIdx[0]);
#else
      dlDciPtr[0].rnti = sib1PdcchInfo->dci[0].rnti;
      dlDciPtr[0].scramblingId = sib1PdcchInfo->dci[0].scramblingId;    
      dlDciPtr[0].scramblingRnti = sib1PdcchInfo->dci[0].scramblingRnti;
      dlDciPtr[0].pc_and_bform.numPrgs = sib1PdcchInfo->dci[0].beamPdcchInfo.numPrgs;
      dlDciPtr[0].pc_and_bform.prgSize = sib1PdcchInfo->dci[0].beamPdcchInfo.prgSize;
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].pmIdx = sib1PdcchInfo->dci[0].beamPdcchInfo.prg[0].pmIdx;
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = sib1PdcchInfo->dci[0].beamPdcchInfo.prg[0].beamIdx[0];
#endif
      dlDciPtr[0].pc_and_bform.digBfInterfaces = sib1PdcchInfo->dci[0].beamPdcchInfo.digBfInterfaces;
      dlDciPtr[0].cceIndex = sib1PdcchInfo->dci[0].cceIndex;
      dlDciPtr[0].aggregationLevel = sib1PdcchInfo->dci[0].aggregLevel;
      dlDciPtr[0].beta_pdcch_1_0 = sib1PdcchInfo->dci[0].txPdcchPower.beta_pdcch_1_0;           

      /* Calculating freq domain resource allocation field value and size
       * coreset0Size = Size of coreset 0
       * RBStart = Starting Virtual Rsource block
       * RBLen = length of contiguously allocted RBs
       * Spec 38.214 Sec 5.1.2.2.2
       */
      coreset0Size= sib1PdcchInfo->coresetCfg.coreSetSize;
      rbStart = sib1PdcchInfo->dci[0].pdschCfg.pdschFreqAlloc.startPrb;
      rbLen = sib1PdcchInfo->dci[0].pdschCfg.pdschFreqAlloc.numPrb;

      if((rbLen >=1) && (rbLen <= coreset0Size - rbStart))
      {
	 if((rbLen - 1) <= floor(coreset0Size / 2))
	    freqDomResAssign = (coreset0Size * (rbLen-1)) + rbStart;
	 else
	    freqDomResAssign = (coreset0Size * (coreset0Size - rbLen + 1)) \
			       + (coreset0Size - 1 - rbStart);

	 freqDomResAssignSize = ceil(log2(coreset0Size * (coreset0Size + 1) / 2));
      }

      /* Fetching DCI field values */
#ifndef OAI_TESTING
      timeDomResAssign = sib1PdcchInfo->dci[0].pdschCfg.pdschTimeAlloc.rowIndex -1;
#else
      timeDomResAssign = sib1PdcchInfo->dci[0].pdschCfg.pdschTimeAlloc.rowIndex;
#endif
      VRB2PRBMap       = sib1PdcchInfo->dci[0].pdschCfg.pdschFreqAlloc.vrbPrbMapping;
      modNCodScheme    = sib1PdcchInfo->dci[0].pdschCfg.codeword[0].mcsIndex;
      redundancyVer    = sib1PdcchInfo->dci[0].pdschCfg.codeword[0].rvIndex;
      sysInfoInd       = 0;           /* 0 for SIB1; 1 for SI messages */
      reserved         = 0;

      /* Reversing bits in each DCI field */
#ifndef OAI_TESTING
#ifndef INTEL_XFAPI
      freqDomResAssign = reverseBits(freqDomResAssign, freqDomResAssignSize);
      timeDomResAssign = reverseBits(timeDomResAssign, timeDomResAssignSize);
      VRB2PRBMap       = reverseBits(VRB2PRBMap, VRB2PRBMapSize);
      modNCodScheme    = reverseBits(modNCodScheme, modNCodSchemeSize);
      redundancyVer    = reverseBits(redundancyVer, redundancyVerSize);
      sysInfoInd       = reverseBits(sysInfoInd, sysInfoIndSize);
#endif
#endif

      /* Calulating total number of bytes in buffer */
      dlDciPtr[0].payloadSizeBits = freqDomResAssignSize + timeDomResAssignSize\
				  + VRB2PRBMapSize + modNCodSchemeSize + redundancyVerSize\
				  + sysInfoIndSize + reservedSize;
      numBytes = dlDciPtr[0].payloadSizeBits / 8;
      if(dlDciPtr[0].payloadSizeBits % 8)
         numBytes += 1;

      if(numBytes > FAPI_DCI_PAYLOAD_BYTE_LEN)
      {
         DU_LOG("\nERROR  -->  LWR_MAC : Total bytes for DCI is more than expected");
         return;
      }
#ifdef OAI_TESTING
      dlDciPtr[0].payloadSizeBits  = reverseBytes16(dlDciPtr[0].payloadSizeBits);
#endif
      /* Initialize buffer */
      for(bytePos = 0; bytePos < numBytes; bytePos++)
	 dlDciPtr[0].payload[bytePos] = 0;

#ifndef INTEL_XFAPI
      bytePos = numBytes - 1; 
#else
      bytePos = 0; /*For XFAPI, DCI is filled from 0th Index*/
#endif
#ifndef OAI_TESTING
      bitPos = 0;
#else
      bitPos = 1;
#endif

      /* Packing DCI format fields */
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    freqDomResAssign, freqDomResAssignSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    timeDomResAssign, timeDomResAssignSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    VRB2PRBMap, VRB2PRBMapSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    modNCodScheme, modNCodSchemeSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    redundancyVer, redundancyVerSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    sysInfoInd, sysInfoIndSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    reserved, reservedSize);
   }
} /* fillSib1DlDciPdu */


/*******************************************************************
 *
 * @brief fills Dl DCI PDU for Paging required for DL TTI info in MAC
 *
 * @details
 *
 *    Function : fillPageDlDciPdu
 *
 *    Functionality:
 *         -Fills the Dl DCI PDU for Paging
 *
 * @params[in] Pointer to fapi_dl_dci_t
 *             Pointer to dlPageAlloc
 * @return ROK
 *
 ******************************************************************/

void fillPageDlDciPdu(fapi_dl_dci_t *dlDciPtr, DlPageAlloc *dlPageAlloc, MacCellCfg *macCellCfg)
{
   if(dlDciPtr != NULLP)
   {
      uint8_t numBytes=0;
      uint8_t bytePos=0;
      uint8_t bitPos=0;

      uint16_t coreset0Size     = 0;
      uint16_t rbStart          = 0;
      uint16_t rbLen            = 0;
      uint8_t  shortMsgInd      = 0;
      uint8_t  shortMsg         = 0;
      uint32_t freqDomResAssign = 0;
      uint32_t timeDomResAssign = 0;
      uint8_t  VRB2PRBMap       = 0;
      uint32_t modNCodScheme    = 0;
      uint8_t  tbScaling        = 0;
      uint32_t reserved         = 0;

      /* Size(in bits) of each field in DCI format 1_0 
       * as mentioned in spec 38.214 */
      uint8_t shortMsgIndSize      = 2;
      uint8_t shortMsgSize         = 8;
      uint8_t freqDomResAssignSize = 0;
      uint8_t timeDomResAssignSize = 4;
      uint8_t VRB2PRBMapSize       = 1;
      uint8_t modNCodSchemeSize    = 5;
      uint8_t tbScalingSize        = 2;
      uint8_t reservedSize         = 6;

#ifdef OAI_TESTING 
      dlDciPtr[0].rnti = reverseBytes16(P_RNTI);
      dlDciPtr[0].scramblingId = reverseBytes16(macCellCfg->cellCfg.phyCellId);
      dlDciPtr[0].scramblingRnti = reverseBytes16(0);
#else
      dlDciPtr[0].rnti = P_RNTI;
      dlDciPtr[0].scramblingId = macCellCfg->cellCfg.phyCellId;
      dlDciPtr[0].scramblingRnti = 0;
#endif
      dlDciPtr[0].cceIndex = dlPageAlloc->pageDlDci.cceIndex;
      dlDciPtr[0].aggregationLevel = dlPageAlloc->pageDlDci.aggregLevel;
      dlDciPtr[0].pc_and_bform.numPrgs = 1;
      dlDciPtr[0].pc_and_bform.prgSize = 1;
      dlDciPtr[0].pc_and_bform.digBfInterfaces = 0;
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].pmIdx = 0;
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = 0;
      dlDciPtr[0].beta_pdcch_1_0 = 0;
      dlDciPtr[0].powerControlOffsetSS = 0;

      /* Calculating freq domain resource allocation field value and size
       * coreset0Size = Size of coreset 0
       * RBStart = Starting Virtual Rsource block
       * RBLen = length of contiguously allocted RBs
       * Spec 38.214 Sec 5.1.2.2.2
       */
      coreset0Size = dlPageAlloc->pageDlDci.coreSetSize;
      rbStart = dlPageAlloc->pageDlSch.freqAlloc.startPrb;
      rbLen = dlPageAlloc->pageDlSch.freqAlloc.numPrb;

      if((rbLen >=1) && (rbLen <= coreset0Size - rbStart))
      {
         if((rbLen - 1) <= floor(coreset0Size / 2))
            freqDomResAssign = (coreset0Size * (rbLen-1)) + rbStart;
         else
            freqDomResAssign = (coreset0Size * (coreset0Size - rbLen + 1)) \
                               + (coreset0Size - 1 - rbStart);

         freqDomResAssignSize = ceil(log2(coreset0Size * (coreset0Size + 1) / 2));
      }

      /*Fetching DCI field values */

      /*Refer:38.212 - Table 7.3.1.2.1-1: Short Message indicator >*/
      if(dlPageAlloc->shortMsgInd != TRUE)
      {
         /*When Short Msg is absent*/
         shortMsgInd = 1;
         shortMsg    = 0;
      }
      else
      {
         /*Short Msg is Present*/
         if(dlPageAlloc->pageDlSch.dlPagePduLen == 0 || dlPageAlloc->pageDlSch.dlPagePdu == NULLP)
         {
            /*When Paging Msg is absent*/
            shortMsgInd = 2;
         }
         else
         {
            /*Both Short and Paging is present*/
            shortMsgInd = 3;
         }
         shortMsg = dlPageAlloc->shortMsg;
      }

      timeDomResAssign = 0;
      VRB2PRBMap       = dlPageAlloc->pageDlSch.vrbPrbMapping;
      modNCodScheme    = dlPageAlloc->pageDlSch.tbInfo.mcs;
      tbScaling        = 0;
      reserved         = 0;

      /* Reversing bits in each DCI field */
      shortMsgInd      = reverseBits(shortMsgInd, shortMsgIndSize);
      shortMsg         = reverseBits(shortMsg, shortMsgSize);
      timeDomResAssign = reverseBits(timeDomResAssign, timeDomResAssignSize);
      freqDomResAssign = reverseBits(freqDomResAssign, freqDomResAssignSize);
      timeDomResAssign = reverseBits(timeDomResAssign, timeDomResAssignSize);
      VRB2PRBMap       = reverseBits(VRB2PRBMap, VRB2PRBMapSize);
      modNCodScheme    = reverseBits(modNCodScheme, modNCodSchemeSize);
      tbScaling        = reverseBits(tbScaling, tbScalingSize); 

      /* Calulating total number of bytes in buffer */
      dlDciPtr[0].payloadSizeBits = shortMsgIndSize + shortMsgSize + freqDomResAssignSize\
                                  + timeDomResAssignSize + VRB2PRBMapSize + modNCodSchemeSize\
                                  + tbScaling + reservedSize;

      numBytes = dlDciPtr[0].payloadSizeBits / 8;
      if(dlDciPtr[0].payloadSizeBits % 8)
      {
         numBytes += 1;
      }

#ifdef OAI_TESTING
      dlDciPtr[0].payloadSizeBits = reverseBytes16(dlDciPtr[0].payloadSizeBits);
#endif

      if(numBytes > FAPI_DCI_PAYLOAD_BYTE_LEN)
      {
         DU_LOG("\nERROR  -->  LWR_MAC : Total bytes for DCI is more than expected");
         return;
      }

      /* Initialize buffer */
      for(bytePos = 0; bytePos < numBytes; bytePos++)
      {
         dlDciPtr[0].payload[bytePos] = 0;
      }

      bytePos = numBytes - 1;
      bitPos = 0;

      /* Packing DCI format fields */
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
            shortMsgInd, shortMsgIndSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
            shortMsg, shortMsgSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
            freqDomResAssign, freqDomResAssignSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
            timeDomResAssign, timeDomResAssignSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
            VRB2PRBMap, VRB2PRBMapSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
            modNCodScheme, modNCodSchemeSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
            tbScaling, tbScalingSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
            reserved, reservedSize);
   }
} /* fillPageDlDciPdu */

/*******************************************************************
 *
 * @brief fills Dl DCI PDU required for DL TTI info in MAC
 *
 * @details
 *
 *    Function : fillRarDlDciPdu
 *
 *    Functionality:
 *         -Fills the Dl DCI PDU
 *
 * @params[in] Pointer to fapi_dl_dci_t
 *             Pointer to PdcchCfg
 * @return ROK
 *
 ******************************************************************/

void fillRarDlDciPdu(fapi_dl_dci_t *dlDciPtr, PdcchCfg *rarPdcchInfo)
{
   if(dlDciPtr != NULLP)
   {
      uint8_t numBytes =0;
      uint8_t bytePos =0;
      uint8_t bitPos =0;

      uint16_t coreset0Size =0;
      uint16_t rbStart =0;
      uint16_t rbLen =0;
      uint32_t freqDomResAssign =0;
      uint8_t timeDomResAssign =0;
      uint8_t  VRB2PRBMap =0;
      uint8_t modNCodScheme =0;
      uint8_t tbScaling =0;
      uint32_t reserved =0;

      /* Size(in bits) of each field in DCI format 1_0 */
      uint8_t freqDomResAssignSize = 0;
      uint8_t timeDomResAssignSize = 4;
      uint8_t VRB2PRBMapSize       = 1;
      uint8_t modNCodSchemeSize    = 5;
      uint8_t tbScalingSize        = 2;
      uint8_t reservedSize         = 16;
      
#ifdef OAI_TESTING 
      dlDciPtr[0].rnti = reverseBytes16(rarPdcchInfo->dci[0].rnti);
      dlDciPtr[0].scramblingId = reverseBytes16(rarPdcchInfo->dci[0].scramblingId);    
      dlDciPtr[0].scramblingRnti = reverseBytes16(rarPdcchInfo->dci[0].scramblingRnti);
      dlDciPtr[0].pc_and_bform.numPrgs = reverseBytes16(rarPdcchInfo->dci[0].beamPdcchInfo.numPrgs);
      dlDciPtr[0].pc_and_bform.prgSize = reverseBytes16(rarPdcchInfo->dci[0].beamPdcchInfo.prgSize);
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].pmIdx = reverseBytes16(rarPdcchInfo->dci[0].beamPdcchInfo.prg[0].pmIdx);
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = reverseBytes16(rarPdcchInfo->dci[0].beamPdcchInfo.prg[0].beamIdx[0]);
#else
      dlDciPtr[0].rnti = rarPdcchInfo->dci[0].rnti;
      dlDciPtr[0].scramblingId = rarPdcchInfo->dci[0].scramblingId;    
      dlDciPtr[0].scramblingRnti = rarPdcchInfo->dci[0].scramblingRnti;
      dlDciPtr[0].pc_and_bform.numPrgs = rarPdcchInfo->dci[0].beamPdcchInfo.numPrgs;
      dlDciPtr[0].pc_and_bform.prgSize = rarPdcchInfo->dci[0].beamPdcchInfo.prgSize;
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].pmIdx = rarPdcchInfo->dci[0].beamPdcchInfo.prg[0].pmIdx;
      dlDciPtr[0].pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = rarPdcchInfo->dci[0].beamPdcchInfo.prg[0].beamIdx[0];
#endif
      dlDciPtr[0].cceIndex = rarPdcchInfo->dci[0].cceIndex;
      dlDciPtr[0].aggregationLevel = rarPdcchInfo->dci[0].aggregLevel;
      dlDciPtr[0].pc_and_bform.digBfInterfaces = rarPdcchInfo->dci[0].beamPdcchInfo.digBfInterfaces;
      dlDciPtr[0].beta_pdcch_1_0 = rarPdcchInfo->dci[0].txPdcchPower.beta_pdcch_1_0;           
      dlDciPtr[0].powerControlOffsetSS = rarPdcchInfo->dci[0].txPdcchPower.powerControlOffsetSS;

      /* Calculating freq domain resource allocation field value and size
       * coreset0Size = Size of coreset 0
       * RBStart = Starting Virtual Rsource block
       * RBLen = length of contiguously allocted RBs
       * Spec 38.214 Sec 5.1.2.2.2
       */

      /* TODO: Fill values of coreset0Size, rbStart and rbLen */
      coreset0Size= rarPdcchInfo->coresetCfg.coreSetSize;
      rbStart = rarPdcchInfo->dci[0].pdschCfg.pdschFreqAlloc.startPrb;
      rbLen = rarPdcchInfo->dci[0].pdschCfg.pdschFreqAlloc.numPrb;

      if((rbLen >=1) && (rbLen <= coreset0Size - rbStart))
      {
	 if((rbLen - 1) <= floor(coreset0Size / 2))
	    freqDomResAssign = (coreset0Size * (rbLen-1)) + rbStart;
	 else
	    freqDomResAssign = (coreset0Size * (coreset0Size - rbLen + 1)) \
			       + (coreset0Size - 1 - rbStart);

	 freqDomResAssignSize = ceil(log2(coreset0Size * (coreset0Size + 1) / 2));
      }

      /* Fetching DCI field values */
      timeDomResAssign = rarPdcchInfo->dci[0].pdschCfg.pdschTimeAlloc.rowIndex;
      VRB2PRBMap       = rarPdcchInfo->dci[0].pdschCfg.pdschFreqAlloc.vrbPrbMapping;
      modNCodScheme    = rarPdcchInfo->dci[0].pdschCfg.codeword[0].mcsIndex;
      tbScaling        = 0; /* configured to 0 scaling */
      reserved         = 0;

      /* Reversing bits in each DCI field */
      freqDomResAssign = reverseBits(freqDomResAssign, freqDomResAssignSize);
      timeDomResAssign = reverseBits(timeDomResAssign, timeDomResAssignSize);
      VRB2PRBMap       = reverseBits(VRB2PRBMap, VRB2PRBMapSize);
      modNCodScheme    = reverseBits(modNCodScheme, modNCodSchemeSize);
      tbScaling        = reverseBits(tbScaling, tbScalingSize); 

      /* Calulating total number of bytes in buffer */
      dlDciPtr[0].payloadSizeBits = freqDomResAssignSize + timeDomResAssignSize\
				  + VRB2PRBMapSize + modNCodSchemeSize + tbScalingSize + reservedSize;

      numBytes = dlDciPtr[0].payloadSizeBits / 8;
      if(dlDciPtr[0].payloadSizeBits % 8)
         numBytes += 1;

      if(numBytes > FAPI_DCI_PAYLOAD_BYTE_LEN)
      {
         DU_LOG("\nERROR  -->  LWR_MAC : Total bytes for DCI is more than expected");
         return;
      }

#ifdef OAI_TESTING 
      dlDciPtr[0].payloadSizeBits = reverseBytes16(39);
#endif
      /* Initialize buffer */
      for(bytePos = 0; bytePos < numBytes; bytePos++)
	 dlDciPtr[0].payload[bytePos] = 0;

      bytePos = numBytes - 1;
      bitPos = 0;

      /* Packing DCI format fields */
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    freqDomResAssign, freqDomResAssignSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    timeDomResAssign, timeDomResAssignSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    VRB2PRBMap, VRB2PRBMapSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    modNCodScheme, modNCodSchemeSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    tbScaling, tbScalingSize);
      fillDlDciPayload(dlDciPtr[0].payload, &bytePos, &bitPos,\
	    reserved, reservedSize);
#ifdef OAI_TESTING
      dlDciPtr[0].payload[4] = 0x15;
#endif

   }
} /* fillRarDlDciPdu */

/*******************************************************************
 *
 * @brief fills DL DCI PDU required for DL TTI info in MAC
 *
 * @details
 *
 *    Function : fillDlMsgDlDciPdu
 *
 *    Functionality:
 *         -Fills the Dl DCI PDU  
 *
 * @params[in] Pointer to fapi_dl_dci_t
 *             Pointer to PdcchCfg
 * @return ROK
 *
 ******************************************************************/
void fillDlMsgDlDciPdu(fapi_dl_dci_t *dlDciPtr, PdcchCfg *pdcchInfo,\
      DlMsgSchInfo *dlMsgSchInfo)
{
   uint8_t dciIndex = 0;
   if(dlDciPtr != NULLP)
   {
      uint8_t numBytes;
      uint8_t bytePos;
      uint8_t bitPos;

      uint16_t coresetSize = 0;
      uint16_t rbStart = 0;
      uint16_t rbLen = 0;
      uint8_t  dciFormatId;
      uint32_t freqDomResAssign;
      uint8_t  timeDomResAssign;
      uint8_t  VRB2PRBMap;
      uint8_t  modNCodScheme;
      uint8_t  ndi = 0;
      uint8_t  redundancyVer = 0;
      uint8_t  harqProcessNum = 0;
      uint8_t  dlAssignmentIdx = 0;
      uint8_t  pucchTpc = 0;
      uint8_t  pucchResoInd = 0;
      uint8_t  harqFeedbackInd = 0;

      /* Size(in bits) of each field in DCI format 1_0 */
      uint8_t dciFormatIdSize    = 1;
      uint8_t freqDomResAssignSize = 0;
      uint8_t timeDomResAssignSize = 4;
      uint8_t VRB2PRBMapSize       = 1;
      uint8_t modNCodSchemeSize    = 5;
      uint8_t ndiSize              = 1;
      uint8_t redundancyVerSize    = 2;
      uint8_t harqProcessNumSize   = 4;
      uint8_t dlAssignmentIdxSize  = 2;
      uint8_t pucchTpcSize         = 2;
      uint8_t pucchResoIndSize     = 3;
      uint8_t harqFeedbackIndSize  = 3;

      for(dciIndex = 0; dciIndex < pdcchInfo->numDlDci; dciIndex++)
      {
#ifdef OAI_TESTING 
         dlDciPtr[dciIndex].rnti = reverseBytes16(pdcchInfo->dci[dciIndex].rnti);
         dlDciPtr[dciIndex].scramblingId = reverseBytes16(pdcchInfo->dci[dciIndex].scramblingId);
         dlDciPtr[dciIndex].scramblingRnti = reverseBytes16(pdcchInfo->dci[dciIndex].scramblingRnti);
         dlDciPtr[dciIndex].pc_and_bform.numPrgs = reverseBytes16(pdcchInfo->dci[dciIndex].beamPdcchInfo.numPrgs);
         dlDciPtr[dciIndex].pc_and_bform.prgSize = reverseBytes16(pdcchInfo->dci[dciIndex].beamPdcchInfo.prgSize);
         dlDciPtr[dciIndex].pc_and_bform.pmi_bfi[0].pmIdx = reverseBytes16(pdcchInfo->dci[dciIndex].beamPdcchInfo.prg[0].pmIdx);
         dlDciPtr[dciIndex].pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = reverseBytes16(pdcchInfo->dci[dciIndex].beamPdcchInfo.prg[0].beamIdx[0]);
#else
         dlDciPtr[dciIndex].rnti = pdcchInfo->dci[dciIndex].rnti;
         dlDciPtr[dciIndex].scramblingId = pdcchInfo->dci[dciIndex].scramblingId;
         dlDciPtr[dciIndex].scramblingRnti = pdcchInfo->dci[dciIndex].scramblingRnti;
         dlDciPtr[dciIndex].pc_and_bform.numPrgs = pdcchInfo->dci[dciIndex].beamPdcchInfo.numPrgs;
         dlDciPtr[dciIndex].pc_and_bform.prgSize = pdcchInfo->dci[dciIndex].beamPdcchInfo.prgSize;
         dlDciPtr[dciIndex].pc_and_bform.pmi_bfi[0].pmIdx = pdcchInfo->dci[dciIndex].beamPdcchInfo.prg[0].pmIdx;
         dlDciPtr[dciIndex].pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = pdcchInfo->dci[dciIndex].beamPdcchInfo.prg[0].beamIdx[0];
#endif
         dlDciPtr[dciIndex].cceIndex = pdcchInfo->dci[dciIndex].cceIndex;
         dlDciPtr[dciIndex].aggregationLevel = pdcchInfo->dci[dciIndex].aggregLevel;
         dlDciPtr[dciIndex].pc_and_bform.digBfInterfaces = pdcchInfo->dci[dciIndex].beamPdcchInfo.digBfInterfaces;
         dlDciPtr[dciIndex].beta_pdcch_1_0 = pdcchInfo->dci[dciIndex].txPdcchPower.beta_pdcch_1_0;
         dlDciPtr[dciIndex].powerControlOffsetSS = pdcchInfo->dci[dciIndex].txPdcchPower.powerControlOffsetSS;

         /* Calculating freq domain resource allocation field value and size
          * coreset0Size = Size of coreset 0
          * RBStart = Starting Virtual Rsource block
          * RBLen = length of contiguously allocted RBs
          * Spec 38.214 Sec 5.1.2.2.2
          */
         coresetSize = pdcchInfo->coresetCfg.coreSetSize;
         rbStart = pdcchInfo->dci[dciIndex].pdschCfg.pdschFreqAlloc.startPrb;
         rbLen = pdcchInfo->dci[dciIndex].pdschCfg.pdschFreqAlloc.numPrb;

         if((rbLen >=1) && (rbLen <= coresetSize - rbStart))
         {
            if((rbLen - 1) <= floor(coresetSize / 2))
               freqDomResAssign = (coresetSize * (rbLen-1)) + rbStart;
            else
               freqDomResAssign = (coresetSize * (coresetSize - rbLen + 1)) \
                                  + (coresetSize - 1 - rbStart);

            freqDomResAssignSize = ceil(log2(coresetSize * (coresetSize + 1) / 2));
         }

         /* Fetching DCI field values */
         dciFormatId      = dlMsgSchInfo->dciFormatId;     /* Always set to 1 for DL */
         timeDomResAssign = pdcchInfo->dci[dciIndex].pdschCfg.pdschTimeAlloc.rowIndex -1;
         VRB2PRBMap       = pdcchInfo->dci[dciIndex].pdschCfg.pdschFreqAlloc.vrbPrbMapping;
         modNCodScheme    = pdcchInfo->dci[dciIndex].pdschCfg.codeword[0].mcsIndex;
         ndi              = dlMsgSchInfo->transportBlock[0].ndi;
         redundancyVer    = pdcchInfo->dci[dciIndex].pdschCfg.codeword[0].rvIndex;
         harqProcessNum   = dlMsgSchInfo->harqProcNum;
         dlAssignmentIdx  = dlMsgSchInfo->dlAssignIdx;
         pucchTpc         = dlMsgSchInfo->pucchTpc;
         pucchResoInd     = dlMsgSchInfo->pucchResInd;
         harqFeedbackInd  = dlMsgSchInfo->harqFeedbackInd;

         /* Reversing bits in each DCI field */
         dciFormatId      = reverseBits(dciFormatId, dciFormatIdSize);
         freqDomResAssign = reverseBits(freqDomResAssign, freqDomResAssignSize);
         timeDomResAssign = reverseBits(timeDomResAssign, timeDomResAssignSize);
         VRB2PRBMap       = reverseBits(VRB2PRBMap, VRB2PRBMapSize);
         modNCodScheme    = reverseBits(modNCodScheme, modNCodSchemeSize);
         ndi              = reverseBits(ndi, ndiSize);
         redundancyVer    = reverseBits(redundancyVer, redundancyVerSize);
         harqProcessNum   = reverseBits(harqProcessNum, harqProcessNumSize);
         dlAssignmentIdx  = reverseBits(dlAssignmentIdx , dlAssignmentIdxSize);
         pucchTpc         = reverseBits(pucchTpc, pucchTpcSize);
         pucchResoInd     = reverseBits(pucchResoInd, pucchResoIndSize);
         harqFeedbackInd  = reverseBits(harqFeedbackInd, harqFeedbackIndSize);


         /* Calulating total number of bytes in buffer */
         dlDciPtr[dciIndex].payloadSizeBits = (dciFormatIdSize + freqDomResAssignSize\
               + timeDomResAssignSize + VRB2PRBMapSize + modNCodSchemeSize\
               + ndiSize + redundancyVerSize + harqProcessNumSize + dlAssignmentIdxSize\
               + pucchTpcSize + pucchResoIndSize + harqFeedbackIndSize);

         numBytes = dlDciPtr[dciIndex].payloadSizeBits / 8;
         if(dlDciPtr[dciIndex].payloadSizeBits % 8)
            numBytes += 1;

#ifdef OAI_TESTING 
         dlDciPtr[dciIndex].payloadSizeBits = reverseBytes16(dlDciPtr[dciIndex].payloadSizeBits);
#endif

         if(numBytes > FAPI_DCI_PAYLOAD_BYTE_LEN)
         {
            DU_LOG("\nERROR  -->  LWR_MAC : Total bytes for DCI is more than expected");
            return;
         }

         /* Initialize buffer */
         for(bytePos = 0; bytePos < numBytes; bytePos++)
            dlDciPtr[dciIndex].payload[bytePos] = 0;

         bytePos = numBytes - 1;
         bitPos = 0;

         /* Packing DCI format fields */
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               dciFormatId, dciFormatIdSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               freqDomResAssign, freqDomResAssignSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               timeDomResAssign, timeDomResAssignSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               VRB2PRBMap, VRB2PRBMapSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               modNCodScheme, modNCodSchemeSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               ndi, ndiSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               redundancyVer, redundancyVerSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               redundancyVer, redundancyVerSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               harqProcessNum, harqProcessNumSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               dlAssignmentIdx, dlAssignmentIdxSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               pucchTpc, pucchTpcSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               pucchResoInd, pucchResoIndSize);
         fillDlDciPayload(dlDciPtr[dciIndex].payload, &bytePos, &bitPos,\
               harqFeedbackInd, harqFeedbackIndSize);
      }
   }
}

/*******************************************************************
 *
 * @brief fills Dl PDCCH Info from DL PageAlloc
 *
 * @details
 *
 *    Function : fillPdcchInfoFrmPageAlloc
 *
 *    Functionality:
 *         -Fills the PdcchInfo
 *
 * @params[in] Pointer to DlPageAlloc
 *             Pointer to PdcchCfg
 * @return ROK
 *
 ******************************************************************/
void fillPagePdcchPdu(fapi_dl_tti_req_pdu_t *dlTtiReqPdu, fapi_vendor_dl_tti_req_pdu_t *dlTtiVendorPdu, DlPageAlloc *pageAlloc, MacCellCfg *macCellCfg)
{
   if(dlTtiReqPdu != NULLP)
   {
      BwpCfg *bwp = NULLP;

      memset(&dlTtiReqPdu->pdu.pdcch_pdu, 0, sizeof(fapi_dl_pdcch_pdu_t));
      bwp = &pageAlloc->bwp;
      fillPageDlDciPdu(dlTtiReqPdu->pdu.pdcch_pdu.dlDci, pageAlloc, macCellCfg);

#ifdef OAI_TESTING 
      /* Calculating PDU length. Considering only one dl dci pdu for now */
      dlTtiReqPdu->pduSize = reverseBytes16(sizeof(fapi_dl_pdcch_pdu_t));
      dlTtiReqPdu->pduType = reverseBytes16(PDCCH_PDU_TYPE);
      dlTtiReqPdu->pdu.pdcch_pdu.bwpSize           = reverseBytes16(bwp->freqAlloc.numPrb);
      dlTtiReqPdu->pdu.pdcch_pdu.bwpStart          = reverseBytes16(bwp->freqAlloc.startPrb);
      dlTtiReqPdu->pdu.pdcch_pdu.shiftIndex          = reverseBytes16(pageAlloc->pageDlDci.cceReg.interleaved.shiftIndex);
      dlTtiReqPdu->pdu.pdcch_pdu.numDlDci            = reverseBytes16(1);
#else
      /* Calculating PDU length. Considering only one dl dci pdu for now */
      dlTtiReqPdu->pduType = PDCCH_PDU_TYPE;
      dlTtiReqPdu->pduSize = sizeof(fapi_dl_pdcch_pdu_t);
      dlTtiReqPdu->pdu.pdcch_pdu.bwpSize           = bwp->freqAlloc.numPrb;
      dlTtiReqPdu->pdu.pdcch_pdu.bwpStart          = bwp->freqAlloc.startPrb;
      dlTtiReqPdu->pdu.pdcch_pdu.shiftIndex          = pageAlloc->pageDlDci.cceReg.interleaved.shiftIndex;
      dlTtiReqPdu->pdu.pdcch_pdu.numDlDci            = 1;
      
      /* Filling Vendor message PDU */
      dlTtiVendorPdu->pdu_type = FAPI_PDCCH_PDU_TYPE;
      dlTtiVendorPdu->pdu_size = sizeof(fapi_vendor_dl_pdcch_pdu_t);
      dlTtiVendorPdu->pdu.pdcch_pdu.num_dl_dci = dlTtiReqPdu->pdu.pdcch_pdu.numDlDci;
      dlTtiVendorPdu->pdu.pdcch_pdu.dl_dci[0].epre_ratio_of_pdcch_to_ssb = 0;
      dlTtiVendorPdu->pdu.pdcch_pdu.dl_dci[0].epre_ratio_of_dmrs_to_ssb = 0;
#endif
      dlTtiReqPdu->pdu.pdcch_pdu.subCarrierSpacing = bwp->subcarrierSpacing;
      dlTtiReqPdu->pdu.pdcch_pdu.cyclicPrefix      = bwp->cyclicPrefix;

      dlTtiReqPdu->pdu.pdcch_pdu.startSymbolIndex    = pageAlloc->pageDlDci.ssStartSymbolIndex;
      dlTtiReqPdu->pdu.pdcch_pdu.durationSymbols     = pageAlloc->pageDlDci.durationSymbols;
      convertFreqDomRsrcMapToIAPIFormat(pageAlloc->pageDlDci.freqDomainResource, \
                                         dlTtiReqPdu->pdu.pdcch_pdu.freqDomainResource);
      dlTtiReqPdu->pdu.pdcch_pdu.cceRegMappingType   = pageAlloc->pageDlDci.cceRegMappingType;
      dlTtiReqPdu->pdu.pdcch_pdu.regBundleSize       = pageAlloc->pageDlDci.cceReg.interleaved.regBundleSize;
      dlTtiReqPdu->pdu.pdcch_pdu.interleaverSize     = pageAlloc->pageDlDci.cceReg.interleaved.interleaverSize;
      dlTtiReqPdu->pdu.pdcch_pdu.precoderGranularity = pageAlloc->pageDlDci.precoderGranularity;
      dlTtiReqPdu->pdu.pdcch_pdu.coreSetType         = CORESET_TYPE0;


   }
}

/*******************************************************************
 *
 * @brief fills PDCCH PDU required for DL TTI info in MAC
 *
 * @details
 *
 *    Function : fillPdcchPdu
 *
 *    Functionality:
 *         -Fills the Pdcch PDU info
 *          stored in MAC
 *
 * @params[in] Pointer to FAPI DL TTI Req
 *             Pointer to PdcchCfg
 * @return ROK
 *
 ******************************************************************/
uint8_t fillPdcchPdu(fapi_dl_tti_req_pdu_t *dlTtiReqPdu, fapi_vendor_dl_tti_req_pdu_t *dlTtiVendorPdu, MacDlSlot *dlSlot, int8_t dlMsgSchInfoIdx, \
      RntiType rntiType, uint8_t coreSetType, uint8_t ueIdx)
{
   uint8_t dciIndex = 0;

   if(dlTtiReqPdu != NULLP)
   {
      PdcchCfg *pdcchInfo = NULLP;
      BwpCfg *bwp = NULLP;

      memset(&dlTtiReqPdu->pdu.pdcch_pdu, 0, sizeof(fapi_dl_pdcch_pdu_t));
      if(rntiType == SI_RNTI_TYPE)
      {
         pdcchInfo = dlSlot->dlInfo.brdcstAlloc.sib1Alloc.sib1PdcchCfg;
         bwp = &dlSlot->dlInfo.brdcstAlloc.sib1Alloc.bwp;
         fillSib1DlDciPdu(dlTtiReqPdu->pdu.pdcch_pdu.dlDci, pdcchInfo);
      }
      else if(rntiType == RA_RNTI_TYPE)
      {
         pdcchInfo = dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdcchCfg;
         bwp = &dlSlot->dlInfo.rarAlloc[ueIdx]->bwp;
         fillRarDlDciPdu(dlTtiReqPdu->pdu.pdcch_pdu.dlDci, pdcchInfo);
      }
      else if(rntiType == TC_RNTI_TYPE || rntiType == C_RNTI_TYPE)
      {
         pdcchInfo = dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdcchCfg;
         bwp = &dlSlot->dlInfo.dlMsgAlloc[ueIdx]->bwp;
         fillDlMsgDlDciPdu(dlTtiReqPdu->pdu.pdcch_pdu.dlDci, pdcchInfo,\
               dlSlot->dlInfo.dlMsgAlloc[ueIdx]);
      }
      else
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Failed filling PDCCH Pdu");
         return RFAILED;
      }

      /* Calculating PDU length. Considering only one dl dci pdu for now */
#ifdef OAI_TESTING 
      dlTtiReqPdu->pduSize = reverseBytes16(sizeof(fapi_dl_pdcch_pdu_t));
      dlTtiReqPdu->pduType = reverseBytes16(PDCCH_PDU_TYPE);
      dlTtiReqPdu->pdu.pdcch_pdu.bwpSize = reverseBytes16(bwp->freqAlloc.numPrb);
      dlTtiReqPdu->pdu.pdcch_pdu.bwpStart = reverseBytes16(bwp->freqAlloc.startPrb);
      dlTtiReqPdu->pdu.pdcch_pdu.shiftIndex =  reverseBytes16(pdcchInfo->coresetCfg.shiftIndex);
      dlTtiReqPdu->pdu.pdcch_pdu.numDlDci = reverseBytes16(pdcchInfo->numDlDci);
      memcpy(dlTtiReqPdu->pdu.pdcch_pdu.freqDomainResource, pdcchInfo->coresetCfg.freqDomainResource, \
		        sizeof(uint8_t)*6);
#else
      dlTtiReqPdu->pduSize = (sizeof(fapi_dl_pdcch_pdu_t));
      dlTtiReqPdu->pduType = (PDCCH_PDU_TYPE);
      dlTtiReqPdu->pdu.pdcch_pdu.bwpSize = (bwp->freqAlloc.numPrb);
      dlTtiReqPdu->pdu.pdcch_pdu.bwpStart =(bwp->freqAlloc.startPrb);
      dlTtiReqPdu->pdu.pdcch_pdu.shiftIndex =  (pdcchInfo->coresetCfg.shiftIndex);
      dlTtiReqPdu->pdu.pdcch_pdu.numDlDci = (pdcchInfo->numDlDci);
#ifndef INTEL_XFAPI
      convertFreqDomRsrcMapToIAPIFormat(pdcchInfo->coresetCfg.freqDomainResource,\
            dlTtiReqPdu->pdu.pdcch_pdu.freqDomainResource);
#else
      memcpy(dlTtiReqPdu->pdu.pdcch_pdu.freqDomainResource, pdcchInfo->coresetCfg.freqDomainResource, \
		        sizeof(uint8_t)*6);
#endif
#endif
      dlTtiReqPdu->pdu.pdcch_pdu.subCarrierSpacing = bwp->subcarrierSpacing; 
      dlTtiReqPdu->pdu.pdcch_pdu.cyclicPrefix = bwp->cyclicPrefix; 

      dlTtiReqPdu->pdu.pdcch_pdu.startSymbolIndex = pdcchInfo->coresetCfg.startSymbolIndex;
      dlTtiReqPdu->pdu.pdcch_pdu.durationSymbols = pdcchInfo->coresetCfg.durationSymbols;
      dlTtiReqPdu->pdu.pdcch_pdu.cceRegMappingType = pdcchInfo->coresetCfg.cceRegMappingType;
      dlTtiReqPdu->pdu.pdcch_pdu.regBundleSize = pdcchInfo->coresetCfg.regBundleSize;
      dlTtiReqPdu->pdu.pdcch_pdu.interleaverSize = pdcchInfo->coresetCfg.interleaverSize;
      dlTtiReqPdu->pdu.pdcch_pdu.precoderGranularity = pdcchInfo->coresetCfg.precoderGranularity;
      dlTtiReqPdu->pdu.pdcch_pdu.coreSetType = coreSetType;


#ifndef OAI_TESTING 
      /* Filling Vendor message PDU */
      dlTtiVendorPdu->pdu_type = FAPI_PDCCH_PDU_TYPE;
      dlTtiVendorPdu->pdu_size = sizeof(fapi_vendor_dl_pdcch_pdu_t);
      dlTtiVendorPdu->pdu.pdcch_pdu.num_dl_dci = dlTtiReqPdu->pdu.pdcch_pdu.numDlDci;
      for(dciIndex = 0; dciIndex <  dlTtiReqPdu->pdu.pdcch_pdu.numDlDci; dciIndex++)
      {
         dlTtiVendorPdu->pdu.pdcch_pdu.dl_dci[dciIndex].epre_ratio_of_pdcch_to_ssb = 0;
         dlTtiVendorPdu->pdu.pdcch_pdu.dl_dci[dciIndex].epre_ratio_of_dmrs_to_ssb = 0;
      }
#endif
   }

   return ROK;
}

/*******************************************************************
 *
 * @brief fills PDSCH PDU from PageAlloc required for DL TTI info in MAC
 *
 * @details
 *
 *    Function : fillPagePdschPdu
 *
 *    Functionality:
 *         -Fills the Pdsch PDU info
 *          stored in MAC
 *
 * @params[in] Pointer to FAPI DL TTI Req
 *             Pointer to PdschCfg
 *             Pointer to msgLen of DL TTI Info
 * @return ROK
 *
 ******************************************************************/
void fillPagePdschPdu(fapi_dl_tti_req_pdu_t *dlTtiReqPdu, fapi_vendor_dl_tti_req_pdu_t *dlTtiVendorPdu, DlPageAlloc *pageAlloc,
                       uint16_t pduIndex, MacCellCfg *macCellCfg)
{
   uint8_t idx;

   if(dlTtiReqPdu != NULLP)
   {
      memset(&dlTtiReqPdu->pdu.pdsch_pdu, 0, sizeof(fapi_dl_pdsch_pdu_t));
#ifdef OAI_TESTING 
      dlTtiReqPdu->pduType = reverseBytes16(PDSCH_PDU_TYPE);
      dlTtiReqPdu->pduSize = reverseBytes16(sizeof(fapi_dl_pdsch_pdu_t));
      dlTtiReqPdu->pdu.pdsch_pdu.pduBitMap = reverseBytes16(0); /* PTRS and CBG params are excluded */
      dlTtiReqPdu->pdu.pdsch_pdu.rnti = reverseBytes16(P_RNTI);
      dlTtiReqPdu->pdu.pdsch_pdu.pdu_index = reverseBytes16(pduIndex);
      dlTtiReqPdu->pdu.pdsch_pdu.bwpSize = reverseBytes16(pageAlloc->bwp.freqAlloc.numPrb);
      dlTtiReqPdu->pdu.pdsch_pdu.bwpStart = reverseBytes16(pageAlloc->bwp.freqAlloc.startPrb);
      dlTtiReqPdu->pdu.pdsch_pdu.dataScramblingId = reverseBytes16(macCellCfg->cellCfg.phyCellId);
      dlTtiReqPdu->pdu.pdsch_pdu.dlDmrsSymbPos = reverseBytes16(DL_DMRS_SYMBOL_POS);
      dlTtiReqPdu->pdu.pdsch_pdu.dlDmrsScramblingId = reverseBytes16(macCellCfg->cellCfg.phyCellId);
      dlTtiReqPdu->pdu.pdsch_pdu.dmrsPorts = reverseBytes16(0x0001);
      dlTtiReqPdu->pdu.pdsch_pdu.rbStart = reverseBytes16(pageAlloc->pageDlSch.freqAlloc.startPrb);
      dlTtiReqPdu->pdu.pdsch_pdu.rbSize = reverseBytes16(pageAlloc->pageDlSch.freqAlloc.numPrb);
#else
      dlTtiReqPdu->pduType = PDSCH_PDU_TYPE;
      dlTtiReqPdu->pduSize = sizeof(fapi_dl_pdsch_pdu_t);
      dlTtiReqPdu->pdu.pdsch_pdu.pduBitMap = 0; /* PTRS and CBG params are excluded */
      dlTtiReqPdu->pdu.pdsch_pdu.rnti = P_RNTI;
      dlTtiReqPdu->pdu.pdsch_pdu.pdu_index = pduIndex;
      dlTtiReqPdu->pdu.pdsch_pdu.bwpSize = pageAlloc->bwp.freqAlloc.numPrb;
      dlTtiReqPdu->pdu.pdsch_pdu.bwpStart = pageAlloc->bwp.freqAlloc.startPrb;
      dlTtiReqPdu->pdu.pdsch_pdu.dataScramblingId = macCellCfg->cellCfg.phyCellId;
      dlTtiReqPdu->pdu.pdsch_pdu.dlDmrsSymbPos = DL_DMRS_SYMBOL_POS;
      dlTtiReqPdu->pdu.pdsch_pdu.dlDmrsScramblingId = macCellCfg->cellCfg.phyCellId;
      dlTtiReqPdu->pdu.pdsch_pdu.dmrsPorts = 0x0001;
      dlTtiReqPdu->pdu.pdsch_pdu.rbStart = pageAlloc->pageDlSch.freqAlloc.startPrb;
      dlTtiReqPdu->pdu.pdsch_pdu.rbSize = pageAlloc->pageDlSch.freqAlloc.numPrb;
#endif
      dlTtiReqPdu->pdu.pdsch_pdu.subCarrierSpacing = pageAlloc->bwp.subcarrierSpacing;
      dlTtiReqPdu->pdu.pdsch_pdu.cyclicPrefix = pageAlloc->bwp.cyclicPrefix;
      dlTtiReqPdu->pdu.pdsch_pdu.nrOfCodeWords = 1;
      for(idx = 0; idx < MAX_CODEWORDS ; idx++)
      { 
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].qamModOrder = 2;
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].mcsIndex = pageAlloc->pageDlSch.tbInfo.mcs;
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].mcsTable = 0;
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].rvIndex = 0;
#ifdef OAI_TESTING 
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].targetCodeRate = reverseBytes16(308);
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].tbSize = reverseBytes32(pageAlloc->pageDlSch.tbInfo.tbSize);
#else
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].targetCodeRate = 308;
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].tbSize = pageAlloc->pageDlSch.tbInfo.tbSize;
#endif
      }
      dlTtiReqPdu->pdu.pdsch_pdu.nrOfLayers = 1;
      dlTtiReqPdu->pdu.pdsch_pdu.transmissionScheme = 0;
      dlTtiReqPdu->pdu.pdsch_pdu.refPoint = 0;
      dlTtiReqPdu->pdu.pdsch_pdu.dmrsConfigType = pageAlloc->pageDlSch.dmrs.dmrsType;
      dlTtiReqPdu->pdu.pdsch_pdu.scid = 0;
      dlTtiReqPdu->pdu.pdsch_pdu.numDmrsCdmGrpsNoData = 1;
      dlTtiReqPdu->pdu.pdsch_pdu.resourceAlloc = 1;
      /* since we are using type-1, hence rbBitmap excluded */
      dlTtiReqPdu->pdu.pdsch_pdu.vrbToPrbMapping = pageAlloc->pageDlSch.vrbPrbMapping;
      dlTtiReqPdu->pdu.pdsch_pdu.startSymbIndex = pageAlloc->pageDlSch.timeAlloc.startSymb;
      dlTtiReqPdu->pdu.pdsch_pdu.nrOfSymbols = pageAlloc->pageDlSch.timeAlloc.numSymb;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.digBfInterfaces = 0;
      dlTtiReqPdu->pdu.pdsch_pdu.powerControlOffset = 0;
      dlTtiReqPdu->pdu.pdsch_pdu.powerControlOffsetSS = 0;
#ifndef OAI_TESTING 
      dlTtiReqPdu->pdu.pdsch_pdu.mappingType =   pageAlloc->pageDlSch.timeAlloc.mappingType;
      dlTtiReqPdu->pdu.pdsch_pdu.nrOfDmrsSymbols = pageAlloc->pageDlSch.dmrs.nrOfDmrsSymbols;
      dlTtiReqPdu->pdu.pdsch_pdu.dmrsAddPos = pageAlloc->pageDlSch.dmrs.dmrsAddPos;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.numPrgs = 1;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.prgSize = 0;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.pmi_bfi[0].pmIdx = 0;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.pmi_bfi[0].beamIdx[0].beamidx = 0;
#else
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.numPrgs = reverseBytes16(1);
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.prgSize = reverseBytes16(0);
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.pmi_bfi[0].pmIdx = reverseBytes16(0);
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.pmi_bfi[0].beamIdx[0].beamidx = reverseBytes16(0);
#endif


      /* DL TTI Request vendor message */
      dlTtiVendorPdu->pdu_type = FAPI_PDSCH_PDU_TYPE;
      dlTtiVendorPdu->pdu_size = sizeof(fapi_vendor_dl_pdsch_pdu_t);
      dlTtiVendorPdu->pdu.pdsch_pdu.nr_of_antenna_ports = 1;
      for(int i =0; i< FAPI_VENDOR_MAX_TXRU_NUM; i++)
      {
	      dlTtiVendorPdu->pdu.pdsch_pdu.tx_ru_idx[i] =0;
      }
   }
}

/*******************************************************************
 *
 * @brief fills PDSCH PDU required for DL TTI info in MAC
 *
 * @details
 *
 *    Function : fillPdschPdu
 *
 *    Functionality:
 *         -Fills the Pdsch PDU info
 *          stored in MAC
 *
 * @params[in] Pointer to FAPI DL TTI Req
 *             Pointer to PdschCfg
 *             Pointer to msgLen of DL TTI Info
 * @return ROK
 *
 ******************************************************************/

void fillPdschPdu(fapi_dl_tti_req_pdu_t *dlTtiReqPdu, fapi_vendor_dl_tti_req_pdu_t *dlTtiVendorPdu, PdschCfg *pdschInfo,
      BwpCfg bwp, uint16_t pduIndex)
{
   uint8_t idx;

   if(dlTtiReqPdu != NULLP)
   {
      memset(&dlTtiReqPdu->pdu.pdsch_pdu, 0, sizeof(fapi_dl_pdsch_pdu_t));
#ifdef OAI_TESTING 
      dlTtiReqPdu->pduType = reverseBytes16(PDSCH_PDU_TYPE);
      dlTtiReqPdu->pduSize = reverseBytes16(sizeof(fapi_dl_pdsch_pdu_t));
      dlTtiReqPdu->pdu.pdsch_pdu.pduBitMap = reverseBytes16(pdschInfo->pduBitmap);
      dlTtiReqPdu->pdu.pdsch_pdu.rnti = reverseBytes16(pdschInfo->rnti);         
      dlTtiReqPdu->pdu.pdsch_pdu.pdu_index = reverseBytes16(pduIndex);
      dlTtiReqPdu->pdu.pdsch_pdu.bwpSize = reverseBytes16(bwp.freqAlloc.numPrb);       
      dlTtiReqPdu->pdu.pdsch_pdu.bwpStart = reverseBytes16(bwp.freqAlloc.startPrb);
      dlTtiReqPdu->pdu.pdsch_pdu.dataScramblingId = reverseBytes16(pdschInfo->dataScramblingId);       
      dlTtiReqPdu->pdu.pdsch_pdu.dlDmrsSymbPos = reverseBytes16(pdschInfo->dmrs.dlDmrsSymbPos);
      dlTtiReqPdu->pdu.pdsch_pdu.dlDmrsScramblingId = reverseBytes16(pdschInfo->dmrs.dlDmrsScramblingId);
      dlTtiReqPdu->pdu.pdsch_pdu.dmrsPorts = reverseBytes16(pdschInfo->dmrs.dmrsPorts);
      dlTtiReqPdu->pdu.pdsch_pdu.rbStart = reverseBytes16(pdschInfo->pdschFreqAlloc.startPrb);
      dlTtiReqPdu->pdu.pdsch_pdu.rbSize = reverseBytes16(pdschInfo->pdschFreqAlloc.numPrb);
#else
      dlTtiReqPdu->pduType = PDSCH_PDU_TYPE;
      dlTtiReqPdu->pduSize = sizeof(fapi_dl_pdsch_pdu_t);
      dlTtiReqPdu->pdu.pdsch_pdu.pduBitMap = pdschInfo->pduBitmap;
      dlTtiReqPdu->pdu.pdsch_pdu.rnti = pdschInfo->rnti;         
      dlTtiReqPdu->pdu.pdsch_pdu.pdu_index = pduIndex;
      dlTtiReqPdu->pdu.pdsch_pdu.bwpSize = bwp.freqAlloc.numPrb;       
      dlTtiReqPdu->pdu.pdsch_pdu.bwpStart = bwp.freqAlloc.startPrb;
      dlTtiReqPdu->pdu.pdsch_pdu.dataScramblingId = pdschInfo->dataScramblingId;       
      dlTtiReqPdu->pdu.pdsch_pdu.dlDmrsSymbPos = pdschInfo->dmrs.dlDmrsSymbPos;
      dlTtiReqPdu->pdu.pdsch_pdu.dlDmrsScramblingId = pdschInfo->dmrs.dlDmrsScramblingId;
      dlTtiReqPdu->pdu.pdsch_pdu.dmrsPorts = pdschInfo->dmrs.dmrsPorts;
      dlTtiReqPdu->pdu.pdsch_pdu.rbStart = pdschInfo->pdschFreqAlloc.startPrb;
      dlTtiReqPdu->pdu.pdsch_pdu.rbSize = pdschInfo->pdschFreqAlloc.numPrb;
#endif
      dlTtiReqPdu->pdu.pdsch_pdu.subCarrierSpacing = bwp.subcarrierSpacing;
      dlTtiReqPdu->pdu.pdsch_pdu.cyclicPrefix = bwp.cyclicPrefix;
      dlTtiReqPdu->pdu.pdsch_pdu.nrOfCodeWords = pdschInfo->numCodewords;
      for(idx = 0; idx < MAX_CODEWORDS ; idx++)
      { 
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].qamModOrder = pdschInfo->codeword[idx].qamModOrder;
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].mcsIndex = pdschInfo->codeword[idx].mcsIndex;
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].mcsTable = pdschInfo->codeword[idx].mcsTable;
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].rvIndex = pdschInfo->codeword[idx].rvIndex;
#ifdef OAI_TESTING
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].targetCodeRate = reverseBytes16(pdschInfo->codeword[idx].targetCodeRate);
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].tbSize = reverseBytes32(pdschInfo->codeword[idx].tbSize);
#else
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].targetCodeRate = pdschInfo->codeword[idx].targetCodeRate;
         dlTtiReqPdu->pdu.pdsch_pdu.cwInfo[idx].tbSize = pdschInfo->codeword[idx].tbSize;
#endif
      }
      dlTtiReqPdu->pdu.pdsch_pdu.nrOfLayers = pdschInfo->numLayers;
      dlTtiReqPdu->pdu.pdsch_pdu.transmissionScheme = pdschInfo->transmissionScheme;
      dlTtiReqPdu->pdu.pdsch_pdu.refPoint = pdschInfo->refPoint;
      dlTtiReqPdu->pdu.pdsch_pdu.dmrsConfigType = pdschInfo->dmrs.dmrsConfigType;
      dlTtiReqPdu->pdu.pdsch_pdu.scid = pdschInfo->dmrs.scid;
      dlTtiReqPdu->pdu.pdsch_pdu.numDmrsCdmGrpsNoData = pdschInfo->dmrs.numDmrsCdmGrpsNoData;
      dlTtiReqPdu->pdu.pdsch_pdu.resourceAlloc = pdschInfo->pdschFreqAlloc.resourceAllocType;
      /* since we are using type-1, hence rbBitmap excluded */
      dlTtiReqPdu->pdu.pdsch_pdu.vrbToPrbMapping = pdschInfo->pdschFreqAlloc.vrbPrbMapping;
      dlTtiReqPdu->pdu.pdsch_pdu.startSymbIndex = pdschInfo->pdschTimeAlloc.startSymb;
      dlTtiReqPdu->pdu.pdsch_pdu.nrOfSymbols = pdschInfo->pdschTimeAlloc.numSymb;
      dlTtiReqPdu->pdu.pdsch_pdu.powerControlOffset = pdschInfo->txPdschPower.powerControlOffset;  
      dlTtiReqPdu->pdu.pdsch_pdu.powerControlOffsetSS = pdschInfo->txPdschPower.powerControlOffsetSS;
#ifdef OAI_TESTING 
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.digBfInterfaces = 1;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.numPrgs = reverseBytes16(0);
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.prgSize = reverseBytes16(0);
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.pmi_bfi[0].pmIdx = reverseBytes16(pdschInfo->beamPdschInfo.prg[0].pmIdx);
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.pmi_bfi[0].beamIdx[0].beamidx = reverseBytes16(pdschInfo->beamPdschInfo.prg[0].beamIdx[0]);
      dlTtiReqPdu->pdu.pdsch_pdu.maintParamV3.ldpcBaseGraph=2;
      dlTtiReqPdu->pdu.pdsch_pdu.maintParamV3.tbSizeLbrmBytes=reverseBytes32(57376);
#else
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.digBfInterfaces = pdschInfo->beamPdschInfo.digBfInterfaces;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.numPrgs = pdschInfo->beamPdschInfo.numPrgs;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.prgSize = pdschInfo->beamPdschInfo.prgSize;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.pmi_bfi[0].pmIdx = pdschInfo->beamPdschInfo.prg[0].pmIdx;
      dlTtiReqPdu->pdu.pdsch_pdu.preCodingAndBeamforming.pmi_bfi[0].beamIdx[0].beamidx = pdschInfo->beamPdschInfo.prg[0].beamIdx[0];
      dlTtiReqPdu->pdu.pdsch_pdu.mappingType =   pdschInfo->dmrs.mappingType;
      dlTtiReqPdu->pdu.pdsch_pdu.nrOfDmrsSymbols = pdschInfo->dmrs.nrOfDmrsSymbols;
      dlTtiReqPdu->pdu.pdsch_pdu.dmrsAddPos = pdschInfo->dmrs.dmrsAddPos;
#endif

#ifndef OAI_TESTING 
      /* DL TTI Request vendor message */
      dlTtiVendorPdu->pdu_type = FAPI_PDSCH_PDU_TYPE;
      dlTtiVendorPdu->pdu_size = sizeof(fapi_vendor_dl_pdsch_pdu_t);
      dlTtiVendorPdu->pdu.pdsch_pdu.nr_of_antenna_ports = 1;
      for(int i =0; i< FAPI_VENDOR_MAX_TXRU_NUM; i++)
      {
	      dlTtiVendorPdu->pdu.pdsch_pdu.tx_ru_idx[i] =0;
      }
#endif
   }
}

/***********************************************************************
 *
 * @brief calculates the total size to be allocated for DL TTI Req
 *
 * @details
 *
 *    Function : calcDlTtiReqPduCount
 *
 *    Functionality:
 *         -calculates the total pdu count to be allocated for DL TTI Req
 *
 * @params[in]   MacDlSlot *dlSlot 
 * @return count
 *
 * ********************************************************************/
uint8_t calcDlTtiReqPduCount(MacDlSlot *dlSlot)
{
   uint8_t count = 0;
   uint8_t idx = 0, ueIdx=0;

   if(dlSlot->dlInfo.isBroadcastPres)
   {
      if(dlSlot->dlInfo.brdcstAlloc.ssbTransmissionMode)
      {
         for(idx = 0; idx < dlSlot->dlInfo.brdcstAlloc.ssbIdxSupported; idx++)
         {
            /* SSB PDU is filled */
            count++;
         }
      }
      if(dlSlot->dlInfo.brdcstAlloc.sib1TransmissionMode)
      {
         /* PDCCH and PDSCH PDU is filled */
         count += 2;
      }
   }

   if(dlSlot->pageAllocInfo)
   {
      /* PDCCH and PDSCH PDU is filled */
      count += 2;
   }

   for(ueIdx=0; ueIdx<MAX_NUM_UE; ueIdx++)
   {
      if(dlSlot->dlInfo.rarAlloc[ueIdx] != NULLP)
      {
         /* PDCCH and PDSCH PDU is filled */
         if(dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg && dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdcchCfg)
            count += 2;
         else
            count += 1;
      }

      if(dlSlot->dlInfo.dlMsgAlloc[ueIdx] != NULLP)
      {
         /* PDCCH and PDSCH PDU is filled */
         if(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdcchCfg)
            count += 1;
         if(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg)
            count += 1;
      }
   }
   return count;
}

/***********************************************************************
 *
 * @brief calculates the total size to be allocated for DL TTI Req
 *
 * @details
 *
 *    Function : calcTxDataReqPduCount
 *
 *    Functionality:
 *         -calculates the total pdu count to be allocated for DL TTI Req
 *
 * @params[in]    DlBrdcstAlloc *cellBroadcastInfo
 * @return count
 *
 * ********************************************************************/
uint8_t calcTxDataReqPduCount(MacDlSlot *dlSlot)
{
   uint8_t count = 0, ueIdx=0;

   if(dlSlot->dlInfo.isBroadcastPres && dlSlot->dlInfo.brdcstAlloc.sib1TransmissionMode)
   {
      count++;
   }
   if(dlSlot->pageAllocInfo)
   {
      count++;
   }

   for(ueIdx=0; ueIdx<MAX_NUM_UE; ueIdx++)
   {
      if((dlSlot->dlInfo.rarAlloc[ueIdx] != NULLP) &&  (dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg))
         count++;

      if(dlSlot->dlInfo.dlMsgAlloc[ueIdx] != NULLP)
      {
         if(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg)
            count++;
      }
   }
   return count;
}

uint8_t get_tlv_padding(uint16_t tlv_length)
{
   DU_LOG("\nDEBUG  -->  LWR_MAC: get_tlv_padding tlv_length %u = padding = %d\n",tlv_length, ((4 - (tlv_length % 4)) % 4));
   return (4 - (tlv_length % 4)) % 4;
}

/***********************************************************************
 *
 * @brief fills the SIB1 TX-DATA request message
 *
 * @details
 *
 *    Function : fillSib1TxDataReq
 *
 *    Functionality:
 *         - fills the SIB1 TX-DATA request message
 *
 * @params[in]    fapi_tx_pdu_desc_t *pduDesc
 * @params[in]    macCellCfg consist of SIB1 pdu
 * @params[in]    uint32_t *msgLen
 * @params[in]    uint16_t pduIndex
 * @return ROK
 *
 * ********************************************************************/
uint8_t fillSib1TxDataReq(fapi_tx_pdu_desc_t *pduDesc, uint16_t pduIndex, MacCellCfg *macCellCfg,
      PdschCfg *pdschCfg)
{
#ifndef OAI_TESTING 
   uint32_t payloadSize = 0;
   uint8_t *sib1Payload = NULLP;
   fapi_api_queue_elem_t *payloadElem = NULLP;
#ifdef INTEL_WLS_MEM
   void * wlsHdlr = NULLP;
#endif

   pduDesc[pduIndex].pdu_index = pduIndex;
   pduDesc[pduIndex].num_tlvs = 1;

   /* fill the TLV */
   payloadSize = pdschCfg->codeword[0].tbSize;
   pduDesc[pduIndex].tlvs[0].tl.tag = ((payloadSize & 0xff0000) >> 8) | FAPI_TX_DATA_PTR_TO_PAYLOAD_64;
   pduDesc[pduIndex].tlvs[0].tl.length = (payloadSize & 0x0000ffff);
   LWR_MAC_ALLOC(sib1Payload, payloadSize);
   if(sib1Payload == NULLP)
   {
      return RFAILED;
   }
   payloadElem = (fapi_api_queue_elem_t *)sib1Payload;
   FILL_FAPI_LIST_ELEM(payloadElem, NULLP, FAPI_VENDOR_MSG_PHY_ZBC_BLOCK_REQ, 1, \
      macCellCfg->cellCfg.sib1Cfg.sib1PduLen);
#ifndef INTEL_XFAPI
   memcpy(sib1Payload + TX_PAYLOAD_HDR_LEN, macCellCfg->cellCfg.sib1Cfg.sib1Pdu, macCellCfg->cellCfg.sib1Cfg.sib1PduLen);
#else
   memcpy(sib1Payload, macCellCfg->cellCfg.sib1Cfg.sib1Pdu, macCellCfg->cellCfg.sib1Cfg.sib1PduLen);
#endif

#ifdef INTEL_WLS_MEM
   mtGetWlsHdl(&wlsHdlr);
   pduDesc[pduIndex].tlvs[0].value = (uint8_t *)(WLS_VA2PA(wlsHdlr, sib1Payload));
#else
   pduDesc[pduIndex].tlvs[0].value = sib1Payload;
#endif
   pduDesc[pduIndex].pdu_length = payloadSize;

#ifdef INTEL_WLS_MEM
   addWlsBlockToFree(sib1Payload, payloadSize, (lwrMacCb.phySlotIndCntr-1));
#else
   LWR_MAC_FREE(sib1Payload, payloadSize);
#endif


#else

   uint8_t *sib1Payload = NULLP;
   fapi_api_queue_elem_t *payloadElem = NULLP;
#ifdef INTEL_WLS_MEM
   void * wlsHdlr = NULLP;
#endif
   uint8_t tlvPaddingLen =get_tlv_padding(pdschCfg->codeword[0].tbSize);
   uint16_t totalLen= pdschCfg->codeword[0].tbSize + 4;

   pduDesc[pduIndex].pdu_length = totalLen; 
#ifdef OAI_TESTING
  pduDesc[pduIndex].pdu_length = reverseBytes32(pduDesc[pduIndex].pdu_length);
#endif 
   pduDesc[pduIndex].pdu_index = reverseBytes16(pduIndex);
   pduDesc[pduIndex].num_tlvs = reverseBytes32(1);
   pduDesc[pduIndex].tlvs[0].tag = reverseBytes16(FAPI_TX_DATA_PAYLOAD);
   pduDesc[pduIndex].tlvs[0].length = reverseBytes32(pdschCfg->codeword[0].tbSize);
   memcpy(pduDesc[pduIndex].tlvs[0].value.direct, macCellCfg->cellCfg.sib1Cfg.sib1Pdu, pdschCfg->codeword[0].tbSize);
#endif
	
   return ROK;
}

/***********************************************************************
 *
 * @brief fills the PAGE TX-DATA request message
 *
 * @details
 *
 *    Function : fillPageTxDataReq
 *
 *    Functionality:
 *         - fills the Page TX-DATA request message
 *
 * @params[in]    fapi_tx_pdu_desc_t *pduDesc
 * @params[in]    macCellCfg consist of SIB1 pdu
 * @params[in]    uint32_t *msgLen
 * @params[in]    uint16_t pduIndex
 * @return ROK
 *
 * ********************************************************************/
uint8_t fillPageTxDataReq(fapi_tx_pdu_desc_t *pduDesc, uint16_t pduIndex, DlPageAlloc *pageAllocInfo)
{

#ifndef OAI_TESTING 
#ifndef OAI_TESTING 
   uint32_t payloadSize = 0;
#else
   uint16_t payloadSize = 0;
#endif

   uint8_t *pagePayload = NULLP;
   fapi_api_queue_elem_t *payloadElem = NULLP;
#ifdef INTEL_WLS_MEM
   void * wlsHdlr = NULLP;
#endif

   payloadSize = pageAllocInfo->pageDlSch.tbInfo.tbSize;

#ifndef OAI_TESTING 
   pduDesc[pduIndex].pdu_index = pduIndex;
   pduDesc[pduIndex].num_tlvs = 1;
   /* fill the TLV */
   pduDesc[pduIndex].tlvs[0].tl.tag = ((payloadSize & 0xff0000) >> 8) | FAPI_TX_DATA_PTR_TO_PAYLOAD_64;
   pduDesc[pduIndex].tlvs[0].tl.length = (payloadSize & 0x0000ffff);
#else
   pduDesc[pduIndex].pdu_index = reverseBytes16(pduIndex);
   pduDesc[pduIndex].num_tlvs = reverseBytes32(1);
   /* fill the TLV */
   pduDesc[pduIndex].tlvs[0].tag = reverseBytes16(((payloadSize & 0xff0000) >> 8) | FAPI_TX_DATA_PTR_TO_PAYLOAD_32);
   pduDesc[pduIndex].tlvs[0].length = reverseBytes16((payloadSize & 0x0000ffff));
#endif

   LWR_MAC_ALLOC(pagePayload, payloadSize);
   if(pagePayload == NULLP)
   {
      return RFAILED;
   }
   payloadElem = (fapi_api_queue_elem_t *)pagePayload;
   FILL_FAPI_LIST_ELEM(payloadElem, NULLP, FAPI_VENDOR_MSG_PHY_ZBC_BLOCK_REQ, 1, \
         pageAllocInfo->pageDlSch.dlPagePduLen);
   memcpy(pagePayload + TX_PAYLOAD_HDR_LEN, pageAllocInfo->pageDlSch.dlPagePdu, pageAllocInfo->pageDlSch.dlPagePduLen);
#ifdef INTEL_WLS_MEM
   mtGetWlsHdl(&wlsHdlr);
   pduDesc[pduIndex].tlvs[0].value = (uint8_t *)(WLS_VA2PA(wlsHdlr, pagePayload));
#else
   pduDesc[pduIndex].tlvs[0].value = pagePayload;
#endif

   pduDesc[pduIndex].pdu_length = payloadSize; 
#ifdef OAI_TESTING 
   pduDesc[pduIndex].pdu_length = reverseBytes16(pduDesc[pduIndex].pdu_length);
#endif

#ifdef INTEL_WLS_MEM   
   addWlsBlockToFree(pagePayload, payloadSize, (lwrMacCb.phySlotIndCntr-1));
#else
   LWR_MAC_FREE(pagePayload, payloadSize);
#endif
#endif

   return ROK;
}

/***********************************************************************
 *
 * @brief fills the RAR TX-DATA request message
 *
 * @details
 *
 *    Function : fillRarTxDataReq
 *
 *    Functionality:
 *         - fills the RAR TX-DATA request message
 *
 * @params[in]    fapi_tx_pdu_desc_t *pduDesc
 * @params[in]    RarInfo *rarInfo
 * @params[in]    uint32_t *msgLen
 * @params[in]    uint16_t pduIndex
 * @return ROK
 *
 * ********************************************************************/
uint8_t fillRarTxDataReq(fapi_tx_pdu_desc_t *pduDesc, uint16_t pduIndex, RarInfo *rarInfo, PdschCfg *pdschCfg)
{

#ifndef OAI_TESTING 
   uint16_t payloadSize = 0;
   uint8_t  *rarPayload = NULLP;
   fapi_api_queue_elem_t *payloadElem = NULLP;
#ifdef INTEL_WLS_MEM
   void * wlsHdlr = NULLP;
#endif

   payloadSize = pdschCfg->codeword[0].tbSize;
   pduDesc[pduIndex].pdu_index = pduIndex;
   pduDesc[pduIndex].num_tlvs = 1;
   /* fill the TLV */
   pduDesc[pduIndex].tlvs[0].tl.tag = FAPI_TX_DATA_PTR_TO_PAYLOAD_64;
   pduDesc[pduIndex].tlvs[0].tl.length = payloadSize;

   LWR_MAC_ALLOC(rarPayload, payloadSize);
   if(rarPayload == NULLP)
   {
      return RFAILED;
   }
   payloadElem = (fapi_api_queue_elem_t *)rarPayload;
   FILL_FAPI_LIST_ELEM(payloadElem, NULLP, FAPI_VENDOR_MSG_PHY_ZBC_BLOCK_REQ, 1, rarInfo->rarPduLen);
   memcpy(rarPayload + TX_PAYLOAD_HDR_LEN, rarInfo->rarPdu, rarInfo->rarPduLen);
#ifdef INTEL_WLS_MEM
   mtGetWlsHdl(&wlsHdlr);
   pduDesc[pduIndex].tlvs[0].value = (uint8_t *)(WLS_VA2PA(wlsHdlr, rarPayload));
#else
   pduDesc[pduIndex].tlvs[0].value = rarPayload;
#endif
   pduDesc[pduIndex].pdu_length = payloadSize;

#ifdef INTEL_WLS_MEM
   addWlsBlockToFree(rarPayload, payloadSize, (lwrMacCb.phySlotIndCntr-1));
#else
   LWR_MAC_FREE(rarPayload, payloadSize);
#endif

#else

   uint8_t tlvPaddingLen =get_tlv_padding(rarInfo->rarPduLen);
   uint16_t totalLen= rarInfo->rarPduLen +tlvPaddingLen;
   pduDesc[pduIndex].pdu_length = totalLen;
   pduDesc[pduIndex].pdu_length = reverseBytes32(pduDesc[pduIndex].pdu_length);

   pduDesc[pduIndex].pdu_index = reverseBytes16(pduIndex);
   pduDesc[pduIndex].num_tlvs = reverseBytes32(1);
   /* fill the TLV */
   pduDesc[pduIndex].tlvs[0].tag = reverseBytes16(FAPI_TX_DATA_PAYLOAD);
   pduDesc[pduIndex].tlvs[0].length = reverseBytes32(rarInfo->rarPduLen);

   memcpy(pduDesc[pduIndex].tlvs[0].value.direct, rarInfo->rarPdu, rarInfo->rarPduLen);

#endif /* FAPI */
   return ROK;
}

/***********************************************************************
 *
 * @brief fills the DL dedicated Msg TX-DATA request message
 *
 * @details
 *
 *    Function : fillDlMsgTxDataReq
 *
 *    Functionality:
 *         - fills the Dl Dedicated Msg TX-DATA request message
 *
 * @params[in]    fapi_tx_pdu_desc_t *pduDesc
 * @params[in]    DlMsgInfo *dlMsgInfo
 * @params[in]    uint32_t *msgLen
 * @params[in]    uint16_t pduIndex
 * @return ROK
 *
 * ********************************************************************/
uint8_t fillDlMsgTxDataReq(fapi_tx_pdu_desc_t *pduDesc, uint16_t pduIndex, DlMsgSchInfo *dlMsgSchInfo, PdschCfg *pdschCfg)
{

#ifndef OAI_TESTING 
   uint16_t payloadSize;
   uint8_t  *dlMsgPayload = NULLP;
   fapi_api_queue_elem_t *payloadElem = NULLP;
#ifdef INTEL_WLS_MEM
   void * wlsHdlr = NULLP;
#endif

   payloadSize = pdschCfg->codeword[0].tbSize;
#ifndef OAI_TESTING 
   pduDesc[pduIndex].pdu_index = pduIndex;
   pduDesc[pduIndex].num_tlvs = 1;
   /* fill the TLV */
   pduDesc[pduIndex].tlvs[0].tl.tag = FAPI_TX_DATA_PTR_TO_PAYLOAD_64;
   pduDesc[pduIndex].tlvs[0].tl.length = payloadSize;
#else
   pduDesc[pduIndex].pdu_index = reverseBytes16(pduIndex);
   pduDesc[pduIndex].num_tlvs = reverseBytes32(1);
   /* fill the TLV */
   pduDesc[pduIndex].tlvs[0].tl.tag = reverseBytes16(FAPI_TX_DATA_PTR_TO_PAYLOAD_32);
   pduDesc[pduIndex].tlvs[0].tl.length = reverseBytes16(payloadSize);
#endif

   LWR_MAC_ALLOC(dlMsgPayload, payloadSize);
   if(dlMsgPayload == NULLP)
   {
      return RFAILED;
   }
   payloadElem = (fapi_api_queue_elem_t *)dlMsgPayload;
   FILL_FAPI_LIST_ELEM(payloadElem, NULLP, FAPI_VENDOR_MSG_PHY_ZBC_BLOCK_REQ, 1, dlMsgSchInfo->dlMsgPduLen);
   memcpy(dlMsgPayload + TX_PAYLOAD_HDR_LEN, dlMsgSchInfo->dlMsgPdu, dlMsgSchInfo->dlMsgPduLen);

#ifdef INTEL_WLS_MEM
   mtGetWlsHdl(&wlsHdlr);
   pduDesc[pduIndex].tlvs[0].value = (uint8_t *)(WLS_VA2PA(wlsHdlr, dlMsgPayload));
#else
   pduDesc[pduIndex].tlvs[0].value = dlMsgPayload;
#endif

   pduDesc[pduIndex].pdu_length = payloadSize;
#ifdef OAI_TESTING 
   pduDesc[pduIndex].pdu_length = reverseBytes16(pduDesc[pduIndex].pdu_length);
#endif

#ifdef INTEL_WLS_MEM
   addWlsBlockToFree(dlMsgPayload, payloadSize, (lwrMacCb.phySlotIndCntr-1));
#else
   LWR_MAC_FREE(dlMsgPayload, payloadSize);
#endif
#endif /* FAPI */
   return ROK;
}

#endif /* FAPI */

/*******************************************************************
 *
 * @brief Sends DL TTI Request to PHY
 *
 * @details
 *
 *    Function : fillDlTtiReq
 *
 *    Functionality:
 *         -Sends FAPI DL TTI req to PHY
 *
 * @params[in]    timing info
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint16_t fillDlTtiReq(SlotTimingInfo currTimingInfo,  p_fapi_api_queue_elem_t prevElemt)
{
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : DL_TTI_REQUEST\n");
#endif

#ifdef INTEL_FAPI
#ifndef OAI_TESTING
   uint8_t idx =0;
   uint8_t nPdu = 0;
   uint8_t numPduEncoded = 0;
   uint8_t  ueIdx;
   uint16_t cellIdx =0;
   uint16_t pduIndex = 0;

   SlotTimingInfo dlTtiReqTimingInfo;
   MacDlSlot *currDlSlot = NULLP;
   MacCellCfg macCellCfg;
   RntiType rntiType;
   fapi_dl_tti_req_t *dlTtiReq = NULLP;
   fapi_msg_header_t *msgHeader = NULLP;
   p_fapi_api_queue_elem_t dlTtiElem;
   p_fapi_api_queue_elem_t headerElem;
   p_fapi_api_queue_elem_t prevElem;

   if(lwrMacCb.phyState == PHY_STATE_RUNNING)
   {
      GET_CELL_IDX(currTimingInfo.cellId, cellIdx);
      /* consider phy delay */
      ADD_DELTA_TO_TIME(currTimingInfo,dlTtiReqTimingInfo,gConfigInfo.gPhyDeltaDl, macCb.macCell[cellIdx]->numOfSlots);
      dlTtiReqTimingInfo.cellId = currTimingInfo.cellId;
      macCellCfg = macCb.macCell[cellIdx]->macCellCfg;
      currDlSlot = &macCb.macCell[cellIdx]->dlSlot[dlTtiReqTimingInfo.slot];

      /* Vendor Message */
      fapi_vendor_msg_t *vendorMsg;
      p_fapi_api_queue_elem_t  vendorMsgQElem;
      /* Allocte And fill Vendor msg */
      LWR_MAC_ALLOC(vendorMsgQElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_vendor_msg_t)));
      if(!vendorMsgQElem)
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for vendor msg in config req");
         return RFAILED;
      }
      FILL_FAPI_LIST_ELEM(vendorMsgQElem, NULLP, FAPI_VENDOR_MESSAGE, 1, sizeof(fapi_vendor_msg_t));
      vendorMsg = (fapi_vendor_msg_t *)(vendorMsgQElem + 1);
      fillMsgHeader(&vendorMsg->header, FAPI_VENDOR_MESSAGE, sizeof(fapi_vendor_msg_t));

      LWR_MAC_ALLOC(dlTtiElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_dl_tti_req_t)));
      if(dlTtiElem)
      {
         FILL_FAPI_LIST_ELEM(dlTtiElem, NULLP, FAPI_DL_TTI_REQUEST, 1, \
               sizeof(fapi_dl_tti_req_t));
         /* Fill message header */
         LWR_MAC_ALLOC(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
         if(!headerElem)
         {
            DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for header in DL TTI req");
            LWR_MAC_FREE(dlTtiElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_dl_tti_req_t)));
            return RFAILED;
         }
         FILL_FAPI_LIST_ELEM(headerElem, dlTtiElem, FAPI_VENDOR_MSG_HEADER_IND, 1, \
               sizeof(fapi_msg_header_t));

         msgHeader = (fapi_msg_header_t *)(headerElem + 1);
         msgHeader->num_msg = 2;
         msgHeader->handle = 0;

        /* Fill Dl TTI Request */
         dlTtiReq = (fapi_dl_tti_req_t *)(dlTtiElem +1);
         memset(dlTtiReq, 0, sizeof(fapi_dl_tti_req_t));
         fillMsgHeader(&dlTtiReq->header, FAPI_DL_TTI_REQUEST, sizeof(fapi_dl_tti_req_t));
         
         dlTtiReq->sfn  = dlTtiReqTimingInfo.sfn;
         dlTtiReq->slot = dlTtiReqTimingInfo.slot;
         dlTtiReq->nPdus = calcDlTtiReqPduCount(currDlSlot);  /* get total Pdus */
         nPdu = dlTtiReq->nPdus;

         vendorMsg->p7_req_vendor.dl_tti_req.num_pdus = nPdu;
         vendorMsg->p7_req_vendor.dl_tti_req.sym = 0;

         dlTtiReq->nGroup = 0;
         if(dlTtiReq->nPdus > 0)
         {
            if(currDlSlot->dlInfo.isBroadcastPres)
            {
               if(currDlSlot->dlInfo.brdcstAlloc.ssbTransmissionMode)
               {
                  if(dlTtiReq->pdus != NULLP)
                  {
                     for(idx = 0; idx < currDlSlot->dlInfo.brdcstAlloc.ssbIdxSupported; idx++)
                     {
                        fillSsbPdu(&dlTtiReq->pdus[numPduEncoded], &macCellCfg,\
                              currDlSlot, idx, dlTtiReq->sfn);
                        numPduEncoded++;
                     }
                  }
                  DU_LOG("\033[1;31m");
                  DU_LOG("\nDEBUG  -->  LWR_MAC: MIB sent..");
                  DU_LOG("\033[0m");
               }
               if(currDlSlot->dlInfo.brdcstAlloc.sib1TransmissionMode)
               {
                  /* Filling SIB1 param */
                  if(numPduEncoded != nPdu)
                  {
                     if(currDlSlot->dlInfo.brdcstAlloc.crnti == SI_RNTI)
                        rntiType = SI_RNTI_TYPE;

                     /* PDCCH PDU */
                     fillPdcchPdu(&dlTtiReq->pdus[numPduEncoded], &vendorMsg->p7_req_vendor.dl_tti_req.pdus[numPduEncoded],
                           currDlSlot, -1, rntiType, CORESET_TYPE0, MAX_NUM_UE);
                     numPduEncoded++;

                     /* PDSCH PDU */
                     fillPdschPdu(&dlTtiReq->pdus[numPduEncoded], &vendorMsg->p7_req_vendor.dl_tti_req.pdus[numPduEncoded],
                           &currDlSlot->dlInfo.brdcstAlloc.sib1Alloc.sib1PdcchCfg->dci[0].pdschCfg,
                           currDlSlot->dlInfo.brdcstAlloc.sib1Alloc.bwp, pduIndex);
                     dlTtiReq->ue_grp_info[dlTtiReq->nGroup].pduIdx[pduIndex] = pduIndex;
                     pduIndex++;
                     numPduEncoded++;
                  }
                  DU_LOG("\033[1;34m");
                  DU_LOG("\nDEBUG  -->  LWR_MAC: SIB1 sent...");
                  DU_LOG("\033[0m");
               }
            }
            if(currDlSlot->pageAllocInfo != NULLP)
            {
               /* Filling DL Paging Alloc param */
               if(numPduEncoded != nPdu)
               {
                  rntiType = P_RNTI_TYPE;

                  fillPagePdcchPdu(&dlTtiReq->pdus[numPduEncoded], &vendorMsg->p7_req_vendor.dl_tti_req.pdus[numPduEncoded], \
                        currDlSlot->pageAllocInfo, &macCellCfg);
                  numPduEncoded++;

                  fillPagePdschPdu(&dlTtiReq->pdus[numPduEncoded], &vendorMsg->p7_req_vendor.dl_tti_req.pdus[numPduEncoded],
                        currDlSlot->pageAllocInfo, pduIndex, &macCellCfg);
                  dlTtiReq->ue_grp_info[dlTtiReq->nGroup].pduIdx[pduIndex] = pduIndex;
                  pduIndex++;
                  numPduEncoded++;
               }
               DU_LOG("\033[1;34m");
               DU_LOG("\nDEBUG  -->  LWR_MAC: PAGE sent...");
               DU_LOG("\033[0m");
            }
            for(ueIdx=0; ueIdx<MAX_NUM_UE; ueIdx++)
            {
               if(currDlSlot->dlInfo.rarAlloc[ueIdx] != NULLP)
               {
                  /* Filling RAR param */
                  rntiType = RA_RNTI_TYPE;
                  if(currDlSlot->dlInfo.rarAlloc[ueIdx]->rarPdcchCfg)
                  {

                     fillPdcchPdu(&dlTtiReq->pdus[numPduEncoded], &vendorMsg->p7_req_vendor.dl_tti_req.pdus[numPduEncoded],
                           currDlSlot, -1, rntiType, CORESET_TYPE0, ueIdx);
                     numPduEncoded++;
                     MAC_FREE(currDlSlot->dlInfo.rarAlloc[ueIdx]->rarPdcchCfg, sizeof(PdcchCfg));
                  }
                  if(currDlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg)
                  {
                     fillPdschPdu(&dlTtiReq->pdus[numPduEncoded], &vendorMsg->p7_req_vendor.dl_tti_req.pdus[numPduEncoded],
                           currDlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg, currDlSlot->dlInfo.rarAlloc[ueIdx]->bwp, pduIndex);
                     numPduEncoded++;
                     pduIndex++;
                     DU_LOG("\033[1;32m");
                     DU_LOG("\nDEBUG  -->  LWR_MAC: RAR sent...");
                     DU_LOG("\033[0m");
                  }
               }
               if(currDlSlot->dlInfo.dlMsgAlloc[ueIdx] != NULLP)
               {
                  if(currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdcchCfg)  \
                  {
                     rntiType = C_RNTI_TYPE;
                     fillPdcchPdu(&dlTtiReq->pdus[numPduEncoded], &vendorMsg->p7_req_vendor.dl_tti_req.pdus[numPduEncoded],
                           currDlSlot, idx, rntiType, currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdcchCfg->coresetCfg.coreSetType, ueIdx);
                     numPduEncoded++;
                  }
                  if(currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdu != NULLP)
                  {
                     if(currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg)
                     {
                        fillPdschPdu(&dlTtiReq->pdus[numPduEncoded], &vendorMsg->p7_req_vendor.dl_tti_req.pdus[numPduEncoded], \
                              currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg,\
                              currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->bwp, pduIndex);
                        numPduEncoded++;
                        pduIndex++;
                        DU_LOG("\033[1;32m");
                        if((macCb.macCell[cellIdx]->macRaCb[ueIdx].macMsg4Status) && \
                             (*macCb.macCell[cellIdx]->macRaCb[ueIdx].macMsg4Status == TRUE))
                        {
                           DU_LOG("\nDEBUG  -->  LWR_MAC: MSG4 sent...");
                           MAC_FREE(macCb.macCell[cellIdx]->macRaCb[ueIdx].macMsg4Status, sizeof(bool));
                        }
                        else
                        {
                           DU_LOG("\nDEBUG  -->  LWR_MAC: DL MSG sent...");
                        }
                        DU_LOG("\033[0m");
                     }
                  }
                  MAC_FREE(currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdcchCfg,sizeof(PdcchCfg));
                  /*   else
                       {
                       MAC_FREE(currDlSlot->dlInfo.dlMsgAlloc[ueIdx], sizeof(DlMsgAlloc));
                       currDlSlot->dlInfo.dlMsgAlloc[ueIdx] = NULLP;
                       }
                       */
               }
            }
            dlTtiReq->ue_grp_info[dlTtiReq->nGroup].nUe = MAX_NUM_UE_PER_TTI;
            dlTtiReq->nGroup++;
#ifdef ODU_SLOT_IND_DEBUG_LOG
            DU_LOG("\nDEBUG  -->  LWR_MAC: Sending DL TTI Request");
#endif
            /* Intel L1 expects UL_TTI.request following DL_TTI.request */

            fillUlTtiReq(currTimingInfo, dlTtiElem, &(vendorMsg->p7_req_vendor.ul_tti_req));
            msgHeader->num_msg++;
            /* Intel L1 expects UL_DCI.request following DL_TTI.request */
            fillUlDciReq(dlTtiReqTimingInfo, dlTtiElem->p_next, &(vendorMsg->p7_req_vendor.ul_dci_req));
            msgHeader->num_msg++;
            /* send Tx-DATA req message */
            sendTxDataReq(dlTtiReqTimingInfo, currDlSlot, dlTtiElem->p_next->p_next, &(vendorMsg->p7_req_vendor.tx_data_req));
            if(dlTtiElem->p_next->p_next->p_next)
            {
               msgHeader->num_msg++;
               prevElem = dlTtiElem->p_next->p_next->p_next;
            }
            else
               prevElem = dlTtiElem->p_next->p_next;
         }
         else
         {
#ifdef ODU_SLOT_IND_DEBUG_LOG
            DU_LOG("\nDEBUG  -->  LWR_MAC: Sending DL TTI Request");
#endif
            /* Intel L1 expects UL_TTI.request following DL_TTI.request */
            fillUlTtiReq(currTimingInfo, dlTtiElem, &(vendorMsg->p7_req_vendor.ul_tti_req));
            msgHeader->num_msg++;

            /* Intel L1 expects UL_DCI.request following DL_TTI.request */

            fillUlDciReq(dlTtiReqTimingInfo, dlTtiElem->p_next, &(vendorMsg->p7_req_vendor.ul_dci_req));
            msgHeader->num_msg++;
            prevElem = dlTtiElem->p_next->p_next;
         }
         if(macCb.macCell[cellIdx]->state == CELL_TO_BE_STOPPED)
         {
            /* Intel L1 expects UL_DCI.request following DL_TTI.request */
            lwr_mac_procStopReqEvt(currTimingInfo, prevElem, &(vendorMsg->stop_req_vendor));
            msgHeader->num_msg++;
            macCb.macCell[cellIdx]->state = CELL_STOP_IN_PROGRESS;
            prevElem = prevElem->p_next;
         }

         prevElem->p_next = vendorMsgQElem;
         LwrMacSendToL1(headerElem);
         memset(currDlSlot, 0, sizeof(MacDlSlot));
         return ROK;
      }
      else
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for DL TTI Request");
         memset(currDlSlot, 0, sizeof(MacDlSlot));
         return RFAILED;
      }
   }
   else
   {
      lwr_mac_procInvalidEvt(&currTimingInfo);
      return RFAILED;
   }

#else

   uint8_t idx =0;
   uint8_t nPdus = 0;
   uint8_t numPduEncoded = 0;
   uint8_t ueIdx =0;
   uint8_t nGroup = 0;
   uint16_t cellIdx =0;
   uint16_t pduIndex = 0;
   uint32_t msgLen= 0;

   SlotTimingInfo dlTtiReqTimingInfo;
   MacDlSlot *currDlSlot = NULLP;
   MacCellCfg macCellCfg;
   RntiType rntiType;
   fapi_dl_tti_req_t *dlTtiReq = NULLP;
   fapi_msg_header_t *msgHeader = NULLP;
   p_fapi_api_queue_elem_t dlTtiElem;
   p_fapi_api_queue_elem_t headerElem;
   p_fapi_api_queue_elem_t prevElem;

   if(lwrMacCb.phyState == PHY_STATE_RUNNING)
   {
      GET_CELL_IDX(currTimingInfo.cellId, cellIdx);
      /* consider phy delay */
      ADD_DELTA_TO_TIME(currTimingInfo,dlTtiReqTimingInfo,gConfigInfo.gPhyDeltaDl, macCb.macCell[cellIdx]->numOfSlots);
      dlTtiReqTimingInfo.cellId = currTimingInfo.cellId;
      macCellCfg = macCb.macCell[cellIdx]->macCellCfg;
      currDlSlot = &macCb.macCell[cellIdx]->dlSlot[dlTtiReqTimingInfo.slot];

      nPdus = calcDlTtiReqPduCount(currDlSlot);

      if(nPdus>0)
      {
         nGroup=1; /*As per 5G FAPI: PHY API spec v 222.10.02, section 3.4.2 DL_TTI.request, For SU-MIMO, one group includes one UE only */
      }
      else
      {
         //DU_LOG("\nINFO   --> LWR_MAC: NO PDU to be scheduled in DL");
	 return ROK;
      }
      /* Below msg length is calculated based on the parameter present in fapi_dl_tti_req_t structure
       * the prameters of fapi_dl_tti_req_t structure are ->
       * header = sizeof(fapi_msg_t) , {sfn, slot} = 2*sizeof(uint16_t),
       * {numPdus, nGroup} = 2 * sizeof(uint8_t), total number of pdu supproted = numPdus*sizeof(fapi_dl_tti_req_pdu_t)
       * and number of Group supported = ngroup * sizeof(fapi_ue_info_t) */

//      msgLen=sizeof(fapi_msg_t)+ (2*sizeof(uint16_t)) + (2*sizeof(uint8_t)) + (nPdus*sizeof(fapi_dl_tti_req_pdu_t)) + (nGroup * sizeof(fapi_ue_info_t));

      LWR_MAC_ALLOC(dlTtiReq, (sizeof(fapi_dl_tti_req_t)));
      if(dlTtiReq)
      {
         memset(dlTtiReq, 0, sizeof(fapi_dl_tti_req_t));
         fillMsgHeader(&dlTtiReq->header, FAPI_DL_TTI_REQUEST, msgLen);
         dlTtiReq->sfn  = reverseBytes16(dlTtiReqTimingInfo.sfn);
         dlTtiReq->slot = reverseBytes16(dlTtiReqTimingInfo.slot);
         dlTtiReq->nPdus = nPdus;  /* get total Pdus */
         dlTtiReq->nGroup = nGroup;
         
         if(dlTtiReq->nPdus > 0)
         {
            if(currDlSlot->dlInfo.isBroadcastPres)
            {
               if(currDlSlot->dlInfo.brdcstAlloc.ssbTransmissionMode)
               {
                  if(dlTtiReq->pdus != NULLP)
                  {
                     for(idx = 0; idx < currDlSlot->dlInfo.brdcstAlloc.ssbIdxSupported; idx++)
                     {
                        fillSsbPdu(&dlTtiReq->pdus[numPduEncoded], &macCellCfg,\
                              currDlSlot, idx, dlTtiReqTimingInfo.sfn);
                        numPduEncoded++;
                     }
                  }
                  DU_LOG("\033[1;31m");
                  DU_LOG("\nDEBUG  -->  LWR_MAC: MIB sent..");
                  DU_LOG("\033[0m");
               }
               if(currDlSlot->dlInfo.brdcstAlloc.sib1TransmissionMode)
               {
                  /* Filling SIB1 param */
                  if(numPduEncoded != nPdus)
                  {
                     if(currDlSlot->dlInfo.brdcstAlloc.crnti == SI_RNTI)
                        rntiType = SI_RNTI_TYPE;
                     /* PDCCH PDU */
                     fillPdcchPdu(&dlTtiReq->pdus[numPduEncoded], NULLP, currDlSlot, -1, rntiType, CORESET_TYPE0, MAX_NUM_UE);

                     numPduEncoded++;
                     fillPdschPdu(&dlTtiReq->pdus[numPduEncoded], NULLP, &currDlSlot->dlInfo.brdcstAlloc.sib1Alloc.sib1PdcchCfg->dci[0].pdschCfg,
                           currDlSlot->dlInfo.brdcstAlloc.sib1Alloc.bwp, pduIndex);

                     dlTtiReq->ue_grp_info[dlTtiReq->nGroup-1].pduIdx[pduIndex] = pduIndex;
                     pduIndex++;
                     numPduEncoded++;
                  }
                  DU_LOG("\033[1;34m");
                  DU_LOG("\nDEBUG  -->  LWR_MAC: SIB1 sent...");
                  DU_LOG("\033[0m");
               }
            }
            if(currDlSlot->pageAllocInfo != NULLP)
            {
               /* Filling DL Paging Alloc param */
               if(numPduEncoded != nPdus)
               {
                  rntiType = P_RNTI_TYPE;
                  fillPagePdcchPdu(&dlTtiReq->pdus[numPduEncoded], NULLP, currDlSlot->pageAllocInfo, &macCellCfg);

                  numPduEncoded++;
                  fillPagePdschPdu(&dlTtiReq->pdus[numPduEncoded], NULLP,  currDlSlot->pageAllocInfo, pduIndex, &macCellCfg);

                  dlTtiReq->ue_grp_info[dlTtiReq->nGroup-1].pduIdx[pduIndex] = pduIndex;
                  pduIndex++;
                  numPduEncoded++;
               }
               DU_LOG("\033[1;34m");
               DU_LOG("\nDEBUG  -->  LWR_MAC: PAGE sent...");
               DU_LOG("\033[0m");
            }
            for(ueIdx=0; ueIdx<MAX_NUM_UE; ueIdx++)
            {
               if(currDlSlot->dlInfo.rarAlloc[ueIdx] != NULLP)
               {
                  /* Filling RAR param */
                  rntiType = RA_RNTI_TYPE;
                  if(currDlSlot->dlInfo.rarAlloc[ueIdx]->rarPdcchCfg)
                  {
                     fillPdcchPdu(&dlTtiReq->pdus[numPduEncoded], NULLP, currDlSlot, -1, rntiType, CORESET_TYPE0, ueIdx);
                     numPduEncoded++;
                     MAC_FREE(currDlSlot->dlInfo.rarAlloc[ueIdx]->rarPdcchCfg, sizeof(PdcchCfg));
                  }
                  if(currDlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg)
                  {
                     fillPdschPdu(&dlTtiReq->pdus[numPduEncoded], NULLP,
                           currDlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg, currDlSlot->dlInfo.rarAlloc[ueIdx]->bwp, pduIndex);
                     numPduEncoded++;
                     pduIndex++;
                     DU_LOG("\033[1;32m");
                     DU_LOG("\nDEBUG  -->  LWR_MAC: RAR sent...");
                     DU_LOG("\033[0m");
                  }
               }
               if(currDlSlot->dlInfo.dlMsgAlloc[ueIdx] != NULLP)
               {
                  if(currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdcchCfg)  \
                  {
                     rntiType = C_RNTI_TYPE;
                     fillPdcchPdu(&dlTtiReq->pdus[numPduEncoded], NULLP,
                           currDlSlot, idx, rntiType, currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdcchCfg->coresetCfg.coreSetType, ueIdx);
                     numPduEncoded++;
                  }
                  if(currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdu != NULLP)
                  {
                     if(currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg)
                     {
                        fillPdschPdu(&dlTtiReq->pdus[numPduEncoded], NULLP, currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg,\
                              currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->bwp, pduIndex);
                        numPduEncoded++;
                        pduIndex++;
                        DU_LOG("\033[1;32m");
                        if((macCb.macCell[cellIdx]->macRaCb[ueIdx].macMsg4Status) && \
                              (*macCb.macCell[cellIdx]->macRaCb[ueIdx].macMsg4Status == TRUE))
                        {
                           DU_LOG("\nDEBUG  -->  LWR_MAC: MSG4 sent...");
                           MAC_FREE(macCb.macCell[cellIdx]->macRaCb[ueIdx].macMsg4Status, sizeof(bool));
                        }
                        else
                        {
                           DU_LOG("\nDEBUG  -->  LWR_MAC: DL MSG sent...");
                        }
                        DU_LOG("\033[0m");
                     }
                  }
                  MAC_FREE(currDlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdcchCfg,sizeof(PdcchCfg));
                  /*   else
                       {
                       MAC_FREE(currDlSlot->dlInfo.dlMsgAlloc[ueIdx], sizeof(DlMsgAlloc));
                       currDlSlot->dlInfo.dlMsgAlloc[ueIdx] = NULLP;
                       }
                       */
               }
            }
            dlTtiReq->ue_grp_info[dlTtiReq->nGroup-1].nUe = MAX_NUM_UE_PER_TTI;
            
#ifdef ODU_SLOT_IND_DEBUG_LOG
            DU_LOG("\nDEBUG  -->  LWR_MAC: Sending DL TTI Request");
#endif
#ifdef ODU_SLOT_IND_DEBUG_LOG
            DU_LOG("\nDEBUG  -->  LWR_MAC: Sending DL TTI Request");
#endif
	    uint32_t  bufferLen=0;
	    uint8_t mBuf[2500];
	    packDlTtiReq(dlTtiReq, mBuf, &bufferLen);
	    LWR_MAC_ALLOC(dlTtiElem, (sizeof(fapi_api_queue_elem_t)  + bufferLen));
	    if(dlTtiElem==NULLP)
	    {
		    DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for DL TTI Request");
		    return RFAILED;
	    }
	    FILL_FAPI_LIST_ELEM(dlTtiElem, NULLP, FAPI_DL_TTI_REQUEST, 1, bufferLen);

	    memcpy((uint8_t *)( dlTtiElem +1), mBuf, bufferLen);
	    prevElemt->p_next = dlTtiElem;
	    LWR_MAC_FREE(dlTtiReq, (sizeof(fapi_dl_tti_req_t)));
	    //LWR_MAC_FREE(mBuf, sizeof(fapi_dl_tti_req_t));
	 }
         if(macCb.macCell[cellIdx]->state == CELL_TO_BE_STOPPED)
         {
            /* Intel L1 expects UL_DCI.request following DL_TTI.request */
            lwr_mac_procStopReqEvt(currTimingInfo, prevElem, NULLP);

            macCb.macCell[cellIdx]->state = CELL_STOP_IN_PROGRESS;
            prevElem = prevElem->p_next;
         }
         
         return ROK;
      }
      else
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for DL TTI Request");
         memset(currDlSlot, 0, sizeof(MacDlSlot));
         return RFAILED;
      }
   }
   else
   {
      lwr_mac_procInvalidEvt(&currTimingInfo);
      return RFAILED;
   }

#endif   
#endif

   return ROK;
}

/*******************************************************************
 *
 * @brief Sends TX data Request to PHY
 *
 * @details
 *
 *    Function : sendTxDataReq
 *
 *    Functionality:
 *         -Sends FAPI TX data req to PHY
 *
 * @params[in]    timing info
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
uint16_t sendTxDataReq(SlotTimingInfo currTimingInfo, MacDlSlot *dlSlot, p_fapi_api_queue_elem_t prevElem, fapi_vendor_tx_data_req_t *vendorTxDataReq)
{
#ifdef INTEL_FAPI
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : TX_DATA_REQ\n");
#endif

#ifndef OAI_TESTING
   uint8_t  nPdu = 0;
   uint8_t  ueIdx=0;
   uint16_t cellIdx=0;
   uint16_t pduIndex = 0;
   fapi_tx_data_req_t       *txDataReq =NULLP;
   p_fapi_api_queue_elem_t  txDataElem = 0;

   GET_CELL_IDX(currTimingInfo.cellId, cellIdx);

   /* send TX_Data request message */
   nPdu = calcTxDataReqPduCount(dlSlot);
   if(nPdu > 0)
   {
      LWR_MAC_ALLOC(txDataElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_tx_data_req_t)));
      if(txDataElem == NULLP)
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for TX data Request");
         return RFAILED;
      }

      FILL_FAPI_LIST_ELEM(txDataElem, NULLP, FAPI_TX_DATA_REQUEST, 1, \
            sizeof(fapi_tx_data_req_t));
      txDataReq = (fapi_tx_data_req_t *)(txDataElem +1);
      memset(txDataReq, 0, sizeof(fapi_tx_data_req_t));
      fillMsgHeader(&txDataReq->header, FAPI_TX_DATA_REQUEST, sizeof(fapi_tx_data_req_t));

      vendorTxDataReq->sym = 0;

      txDataReq->sfn  = currTimingInfo.sfn;
      txDataReq->slot = currTimingInfo.slot;
      if(dlSlot->dlInfo.brdcstAlloc.sib1TransmissionMode)
      {
         fillSib1TxDataReq(txDataReq->pdu_desc, pduIndex, &macCb.macCell[cellIdx]->macCellCfg, \
               &dlSlot->dlInfo.brdcstAlloc.sib1Alloc.sib1PdcchCfg->dci[0].pdschCfg);
         pduIndex++;
         MAC_FREE(dlSlot->dlInfo.brdcstAlloc.sib1Alloc.sib1PdcchCfg,sizeof(PdcchCfg));
         txDataReq->num_pdus++;
      }
      if(dlSlot->pageAllocInfo != NULLP)
      {
         fillPageTxDataReq(txDataReq->pdu_desc, pduIndex, dlSlot->pageAllocInfo);
         pduIndex++;
         txDataReq->num_pdus++;
         MAC_FREE(dlSlot->pageAllocInfo->pageDlSch.dlPagePdu, sizeof(dlSlot->pageAllocInfo->pageDlSch.dlPagePduLen));
         MAC_FREE(dlSlot->pageAllocInfo,sizeof(DlPageAlloc));
      }

      for(ueIdx=0; ueIdx<MAX_NUM_UE; ueIdx++)
      {
         if(dlSlot->dlInfo.rarAlloc[ueIdx] != NULLP)
         {
            if((dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg))
            {
               fillRarTxDataReq(txDataReq->pdu_desc, pduIndex, &dlSlot->dlInfo.rarAlloc[ueIdx]->rarInfo,\
                     dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg);
               pduIndex++;
               txDataReq->num_pdus++;
               MAC_FREE(dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg, sizeof(PdschCfg));
            }
            MAC_FREE(dlSlot->dlInfo.rarAlloc[ueIdx],sizeof(RarAlloc));
         }

         if(dlSlot->dlInfo.dlMsgAlloc[ueIdx] != NULLP)
         {
            if(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg) 
            {
               fillDlMsgTxDataReq(txDataReq->pdu_desc, pduIndex, \
                     dlSlot->dlInfo.dlMsgAlloc[ueIdx], \
                     dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg);
               pduIndex++;
               txDataReq->num_pdus++;
               MAC_FREE(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg,sizeof(PdschCfg));
            }
            MAC_FREE(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdu, \
                  dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPduLen);
            dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdu = NULLP;
            MAC_FREE(dlSlot->dlInfo.dlMsgAlloc[ueIdx], sizeof(DlMsgSchInfo));
         }
      }

      /* Fill message header */
      DU_LOG("\nDEBUG  -->  LWR_MAC: Sending TX DATA Request");
      prevElem->p_next = txDataElem;
   }

#else
   uint8_t  nPdu = 0;
   uint8_t  ueIdx=0;
   uint16_t cellIdx=0;
   uint16_t pduIndex = 0;
   uint32_t MsgLen=0;

   fapi_tx_data_req_t       *txDataReq =NULLP;
   p_fapi_api_queue_elem_t  txDataElem = 0;

   GET_CELL_IDX(currTimingInfo.cellId, cellIdx);

   /* send TX_Data request message */
   nPdu = calcTxDataReqPduCount(dlSlot);
   if(nPdu == 0)
   {
	   return ROK;
   }

      /* Below msg length is calculated based on the parameter present in fapi_tx_data_req_t structure
       * the prameters of fapi_tx_data_req_t structure are ->
       * header = sizeof(fapi_msg_t) , ((sfn, slot, numpdu) = 3*sizeof(uint16_t)),
       * total number of fapi_tx_pdu_desc_t supproted = numPdus*sizeof(fapi_tx_pdu_desc_t)*/

   MsgLen = sizeof(fapi_msg_t)+ (3*sizeof(uint16_t)) + (nPdu*sizeof(fapi_tx_pdu_desc_t));
   if(nPdu > 0)
   {
	   LWR_MAC_ALLOC(txDataReq, (sizeof(fapi_tx_data_req_t)));
	   memset(txDataReq, 0, MsgLen);
#if 0
      LWR_MAC_ALLOC(txDataElem, (sizeof(fapi_api_queue_elem_t) + MsgLen));
      if(txDataElem == NULLP)
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for TX data Request");
         return RFAILED;
      }

      FILL_FAPI_LIST_ELEM(txDataElem, NULLP, FAPI_TX_DATA_REQUEST, 1, MsgLen);
      txDataReq = (fapi_tx_data_req_t *)(txDataElem +1);
#endif
      fillMsgHeader(&txDataReq->header, FAPI_TX_DATA_REQUEST, sizeof(fapi_tx_data_req_t));

      txDataReq->sfn  = reverseBytes16(currTimingInfo.sfn);
      txDataReq->slot = reverseBytes16(currTimingInfo.slot);
      if(dlSlot->dlInfo.brdcstAlloc.sib1TransmissionMode)
      {
        fillSib1TxDataReq(txDataReq->pdu_desc, pduIndex, &macCb.macCell[cellIdx]->macCellCfg, \
               &dlSlot->dlInfo.brdcstAlloc.sib1Alloc.sib1PdcchCfg->dci[0].pdschCfg);
         pduIndex++;
         MAC_FREE(dlSlot->dlInfo.brdcstAlloc.sib1Alloc.sib1PdcchCfg,sizeof(PdcchCfg));
         txDataReq->num_pdus++;
      }
      if(dlSlot->pageAllocInfo != NULLP)
      {
         fillPageTxDataReq(txDataReq->pdu_desc, pduIndex, dlSlot->pageAllocInfo);
         pduIndex++;
         txDataReq->num_pdus++;
         MAC_FREE(dlSlot->pageAllocInfo->pageDlSch.dlPagePdu, sizeof(dlSlot->pageAllocInfo->pageDlSch.dlPagePduLen));
         MAC_FREE(dlSlot->pageAllocInfo,sizeof(DlPageAlloc));
      }

      for(ueIdx=0; ueIdx<MAX_NUM_UE; ueIdx++)
      {
         if(dlSlot->dlInfo.rarAlloc[ueIdx] != NULLP)
         {
            if((dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg))
            {
               fillRarTxDataReq(txDataReq->pdu_desc, pduIndex, &dlSlot->dlInfo.rarAlloc[ueIdx]->rarInfo,\
                     dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg);
               pduIndex++;
               txDataReq->num_pdus++;
               MAC_FREE(dlSlot->dlInfo.rarAlloc[ueIdx]->rarPdschCfg, sizeof(PdschCfg));
            }
            MAC_FREE(dlSlot->dlInfo.rarAlloc[ueIdx],sizeof(RarAlloc));
         }

         if(dlSlot->dlInfo.dlMsgAlloc[ueIdx] != NULLP)
         {
            if(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg) 
            {
               fillDlMsgTxDataReq(txDataReq->pdu_desc, pduIndex, \
                     dlSlot->dlInfo.dlMsgAlloc[ueIdx], \
                     dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg);
               pduIndex++;
               txDataReq->num_pdus++;
               MAC_FREE(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdschCfg,sizeof(PdschCfg));
            }
            MAC_FREE(dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdu, \
                  dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPduLen);
            dlSlot->dlInfo.dlMsgAlloc[ueIdx]->dlMsgPdu = NULLP;
            MAC_FREE(dlSlot->dlInfo.dlMsgAlloc[ueIdx], sizeof(DlMsgSchInfo));
         }
      }

      DU_LOG("\nDEBUG  -->  LWR_MAC: Sending TX DATA Request with total number pdu %u", txDataReq->num_pdus);
      txDataReq->num_pdus= reverseBytes16(txDataReq->num_pdus);
      DU_LOG("\nDEBUG  -->  LWR_MAC: After reversing total number pdu %u = ", txDataReq->num_pdus);

      /* Fill message header */
      uint32_t  bufferLen=0;
      uint8_t mBuf[2500];
      packTxDataReqBuffer(txDataReq, mBuf, &bufferLen);
      LWR_MAC_ALLOC(txDataElem, (sizeof(fapi_api_queue_elem_t) + bufferLen));
      if(txDataElem == NULLP)
      {
	      DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for TX data Request");
	      return RFAILED;
      }

      FILL_FAPI_LIST_ELEM(txDataElem, NULLP, FAPI_TX_DATA_REQUEST, 1, bufferLen);
      memcpy((uint8_t *)( txDataElem +1), mBuf, bufferLen);


      DU_LOG("\nDEBUG  -->  LWR_MAC: Sending TX DATA Request\n");
      prevElem->p_next = txDataElem;
      LWR_MAC_FREE(txDataReq, (sizeof(fapi_tx_data_req_t)));
     // LWR_MAC_FREE(mBuf, (sizeof(fapi_tx_data_req_t)));
   }
#endif
#endif
   return ROK;
}

/***********************************************************************
 *
 * @brief calculates the total size to be allocated for UL TTI Req
 *
 * @details
 *
 *    Function : getnPdus
 *
 *    Functionality:
 *         -calculates the total pdu count to be allocated for UL TTI Req
 *
 * @params[in] Pointer to fapi Ul TTI Req
 *             Pointer to CurrUlSlot
 * @return count
 * ********************************************************************/
#ifdef INTEL_FAPI
uint8_t getnPdus(fapi_ul_tti_req_t *ulTtiReq, MacUlSlot *currUlSlot)
{
   uint8_t pduCount = 0, ueIdx = 0;

   if(ulTtiReq && currUlSlot)
   {
      if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_PRACH)
      {
         pduCount++;
      }

      for(ueIdx = 0; ueIdx < MAX_NUM_UE; ueIdx++)
      {
         if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_PUSCH)
         {
            if(currUlSlot->ulSchInfo.schPuschInfo[ueIdx].crnti != 0)
            {
               pduCount++;
            }
         }
         if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_PUSCH_UCI)
         {
            if(currUlSlot->ulSchInfo.schPuschUci[ueIdx].crnti != 0)
            {
               pduCount++;
            }
         }
         if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_UCI)
         {
            if(currUlSlot->ulSchInfo.schPucchInfo[ueIdx].crnti != 0)
            {
               pduCount++;
            }
         }
      }
      if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_SRS)
      {
         pduCount++;
      }
   }
   return pduCount;
}
#endif

/***********************************************************************
 *
 * @brief Set the value of zero correlation config in PRACH PDU
 *
 * @details
 *
 *    Function : setNumCs
 *
 *    Functionality:
 *         -Set the value of zero correlation config in PRACH PDU
 *
 * @params[in] Pointer to zero correlation config
 *             Pointer to MacCellCfg
 * ********************************************************************/

void setNumCs(uint16_t *numCs, MacCellCfg *macCellCfg)
{
#ifdef INTEL_FAPI
   uint8_t idx;
   if(macCellCfg != NULLP)
   {
      idx = macCellCfg->prachCfg.fdm[0].zeroCorrZoneCfg;
#ifdef OAI_TESTING
      *numCs = reverseBytes16(UnrestrictedSetNcsTable[idx]);
#else
      *numCs = UnrestrictedSetNcsTable[idx];
#endif
   }
#endif
}

/***********************************************************************
 *
 * @brief Fills the PRACH PDU in UL TTI Request
 *
 * @details
 *
 *    Function : fillPrachPdu
 *
 *    Functionality:
 *         -Fills the PRACH PDU in UL TTI Request
 *
 * @params[in] Pointer to Prach Pdu
 *             Pointer to CurrUlSlot
 *             Pointer to macCellCfg
 *             Pointer to msgLen
 * ********************************************************************/

#ifdef INTEL_FAPI
void fillPrachPdu(fapi_ul_tti_req_pdu_t *ulTtiReqPdu, MacCellCfg *macCellCfg, MacUlSlot *currUlSlot)
{
   if(ulTtiReqPdu != NULLP)
   {
#ifdef OAI_TESTING 
      ulTtiReqPdu->pduType = reverseBytes16(PRACH_PDU_TYPE); 
      ulTtiReqPdu->pduSize = reverseBytes16(sizeof(fapi_ul_prach_pdu_t)); 
      ulTtiReqPdu->pdu.prach_pdu.physCellId = reverseBytes16(macCellCfg->cellCfg.phyCellId);
      setNumCs(&ulTtiReqPdu->pdu.prach_pdu.numCs, macCellCfg);
#else
      ulTtiReqPdu->pduType = PRACH_PDU_TYPE; 
      ulTtiReqPdu->pduSize = sizeof(fapi_ul_prach_pdu_t); 
      ulTtiReqPdu->pdu.prach_pdu.physCellId = macCellCfg->cellCfg.phyCellId;
      setNumCs(&ulTtiReqPdu->pdu.prach_pdu.numCs, macCellCfg);
#endif
      ulTtiReqPdu->pdu.prach_pdu.numPrachOcas = \
         currUlSlot->ulSchInfo.prachSchInfo.numPrachOcas;
      ulTtiReqPdu->pdu.prach_pdu.prachFormat = \
	 currUlSlot->ulSchInfo.prachSchInfo.prachFormat;
      ulTtiReqPdu->pdu.prach_pdu.numRa = currUlSlot->ulSchInfo.prachSchInfo.numRa;
      ulTtiReqPdu->pdu.prach_pdu.prachStartSymbol = \
	 currUlSlot->ulSchInfo.prachSchInfo.prachStartSymb;
      ulTtiReqPdu->pdu.prach_pdu.beamforming.digBfInterface = 0;
#ifdef OAI_TESTING
      ulTtiReqPdu->pdu.prach_pdu.beamforming.numPrgs = reverseBytes16(0);
      ulTtiReqPdu->pdu.prach_pdu.beamforming.prgSize = reverseBytes16(0);
      ulTtiReqPdu->pdu.prach_pdu.beamforming.rx_bfi[0].beamIdx[0].beamidx = reverseBytes16(0);
#else
      ulTtiReqPdu->pdu.prach_pdu.beamforming.numPrgs = 0;
      ulTtiReqPdu->pdu.prach_pdu.beamforming.prgSize = 0;
      ulTtiReqPdu->pdu.prach_pdu.beamforming.rx_bfi[0].beamIdx[0].beamidx = 0;
#endif
   }
}

/*******************************************************************
 *
 * @brief Filling PUSCH PDU in UL TTI Request
 *
 * @details
 *
 *    Function : fillPuschPdu
 *
 *    Functionality: Filling PUSCH PDU in UL TTI Request
 *
 * @params[in] 
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
void fillPuschPdu(fapi_ul_tti_req_pdu_t *ulTtiReqPdu, fapi_vendor_ul_tti_req_pdu_t *ulTtiVendorPdu, MacCellCfg *macCellCfg,\
                    SchPuschInfo *puschInfo)
{
   if(ulTtiReqPdu != NULLP)
   {
//      memset(&ulTtiReqPdu->pdu.pusch_pdu, 0, sizeof(fapi_ul_pusch_pdu_t));
#ifdef OAI_TESTING
      ulTtiReqPdu->pduType = reverseBytes16(PUSCH_PDU_TYPE);
      ulTtiReqPdu->pduSize = reverseBytes16(sizeof(fapi_ul_pusch_pdu_t));
      ulTtiReqPdu->pdu.pusch_pdu.pduBitMap = reverseBytes16(1);
      ulTtiReqPdu->pdu.pusch_pdu.rnti = reverseBytes16(puschInfo->crnti);
      ulTtiReqPdu->pdu.pusch_pdu.handle = reverseBytes32(100);
      /* TODO : Fill handle in raCb when scheduling pusch and access here */
      ulTtiReqPdu->pdu.pusch_pdu.bwpSize = reverseBytes16(macCellCfg->cellCfg.initialUlBwp.bwp.numPrb);
      ulTtiReqPdu->pdu.pusch_pdu.bwpStart = reverseBytes16(macCellCfg->cellCfg.initialUlBwp.bwp.firstPrb);
      ulTtiReqPdu->pdu.pusch_pdu.targetCodeRate = reverseBytes16(308);
      ulTtiReqPdu->pdu.pusch_pdu.dataScramblingId = reverseBytes16(macCellCfg->cellId);
      ulTtiReqPdu->pdu.pusch_pdu.ulDmrsSymbPos = reverseBytes16(4);
      ulTtiReqPdu->pdu.pusch_pdu.ulDmrsScramblingId = reverseBytes16(macCellCfg->cellId);
      ulTtiReqPdu->pdu.pusch_pdu.puschIdentity = reverseBytes16(0);
      ulTtiReqPdu->pdu.pusch_pdu.dmrsPorts = reverseBytes16(0);
      ulTtiReqPdu->pdu.pusch_pdu.rbStart = reverseBytes16(puschInfo->fdAlloc.resAlloc.type1.startPrb);
      ulTtiReqPdu->pdu.pusch_pdu.rbSize = reverseBytes16(puschInfo->fdAlloc.resAlloc.type1.numPrb);
      ulTtiReqPdu->pdu.pusch_pdu.txDirectCurrentLocation = reverseBytes16(0);
#else
      ulTtiReqPdu->pduType = PUSCH_PDU_TYPE;
      ulTtiReqPdu->pduSize = sizeof(fapi_ul_pusch_pdu_t);
      ulTtiReqPdu->pdu.pusch_pdu.pduBitMap = 1;
      ulTtiReqPdu->pdu.pusch_pdu.rnti = puschInfo->crnti;
      ulTtiReqPdu->pdu.pusch_pdu.handle = 100;
      /* TODO : Fill handle in raCb when scheduling pusch and access here */
      ulTtiReqPdu->pdu.pusch_pdu.bwpSize = macCellCfg->cellCfg.initialUlBwp.bwp.numPrb;
      ulTtiReqPdu->pdu.pusch_pdu.bwpStart = macCellCfg->cellCfg.initialUlBwp.bwp.firstPrb;
      ulTtiReqPdu->pdu.pusch_pdu.targetCodeRate = 308;
      ulTtiReqPdu->pdu.pusch_pdu.dataScramblingId = macCellCfg->cellId;
      ulTtiReqPdu->pdu.pusch_pdu.ulDmrsSymbPos = 4;
      ulTtiReqPdu->pdu.pusch_pdu.ulDmrsScramblingId = macCellCfg->cellId;
      ulTtiReqPdu->pdu.pusch_pdu.dmrsPorts = 0;
      ulTtiReqPdu->pdu.pusch_pdu.rbStart = puschInfo->fdAlloc.resAlloc.type1.startPrb;
      ulTtiReqPdu->pdu.pusch_pdu.rbSize = puschInfo->fdAlloc.resAlloc.type1.numPrb;
      ulTtiReqPdu->pdu.pusch_pdu.txDirectCurrentLocation = 0;
#endif
      ulTtiReqPdu->pdu.pusch_pdu.subCarrierSpacing = \
         macCellCfg->cellCfg.initialUlBwp.bwp.scs;
      ulTtiReqPdu->pdu.pusch_pdu.cyclicPrefix = \
         macCellCfg->cellCfg.initialUlBwp.bwp.cyclicPrefix;
      ulTtiReqPdu->pdu.pusch_pdu.qamModOrder = puschInfo->tbInfo.qamOrder;
      ulTtiReqPdu->pdu.pusch_pdu.mcsIndex = puschInfo->tbInfo.mcs;
      ulTtiReqPdu->pdu.pusch_pdu.mcsTable = puschInfo->tbInfo.mcsTable;
      ulTtiReqPdu->pdu.pusch_pdu.transformPrecoding = 1;
      ulTtiReqPdu->pdu.pusch_pdu.nrOfLayers = 1;
      ulTtiReqPdu->pdu.pusch_pdu.dmrsConfigType = 0;
      ulTtiReqPdu->pdu.pusch_pdu.scid = 0;
      ulTtiReqPdu->pdu.pusch_pdu.numDmrsCdmGrpsNoData = 1;
      ulTtiReqPdu->pdu.pusch_pdu.resourceAlloc = \
	 puschInfo->fdAlloc.resAllocType;
      ulTtiReqPdu->pdu.pusch_pdu.vrbToPrbMapping = 0;
      ulTtiReqPdu->pdu.pusch_pdu.frequencyHopping = 0;
      ulTtiReqPdu->pdu.pusch_pdu.uplinkFrequencyShift7p5khz = 0;
      ulTtiReqPdu->pdu.pusch_pdu.startSymbIndex = \
         puschInfo->tdAlloc.startSymb;
      ulTtiReqPdu->pdu.pusch_pdu.nrOfSymbols = \
         puschInfo->tdAlloc.numSymb;
      ulTtiReqPdu->pdu.pusch_pdu.puschData.rvIndex = \
         puschInfo->tbInfo.rv;
      ulTtiReqPdu->pdu.pusch_pdu.puschData.harqProcessId = \
         puschInfo->harqProcId;
      ulTtiReqPdu->pdu.pusch_pdu.puschData.newDataIndicator = \
         puschInfo->tbInfo.ndi;
#ifdef OAI_TESTING 
      ulTtiReqPdu->pdu.pusch_pdu.puschData.tbSize = reverseBytes32(puschInfo->tbInfo.tbSize);
      /* numCb is 0 for new transmission */
      ulTtiReqPdu->pdu.pusch_pdu.puschData.numCb = reverseBytes16(0);
#else 
      ulTtiReqPdu->pdu.pusch_pdu.puschData.tbSize = (puschInfo->tbInfo.tbSize);
      /* numCb is 0 for new transmission */
      ulTtiReqPdu->pdu.pusch_pdu.puschData.numCb = (0);
#ifdef INTEL_FAPI
      ulTtiReqPdu->pdu.pusch_pdu.mappingType = \
         puschInfo->dmrsMappingType;
      ulTtiReqPdu->pdu.pusch_pdu.nrOfDmrsSymbols = \
         puschInfo->nrOfDmrsSymbols;
      ulTtiReqPdu->pdu.pusch_pdu.dmrsAddPos = \
         puschInfo->dmrsAddPos;
#endif
      /* UL TTI Vendor PDU */
      ulTtiVendorPdu->pdu_type = FAPI_PUSCH_PDU_TYPE;
      ulTtiVendorPdu->pdu.pusch_pdu.nr_of_antenna_ports=1;
      ulTtiVendorPdu->pdu.pusch_pdu.nr_of_rx_ru=1;
      for(int i =0; i< FAPI_VENDOR_MAX_RXRU_NUM; i++)
      {
	      ulTtiVendorPdu->pdu.pusch_pdu.rx_ru_idx[i]=0;
      }
#endif
   }
}

/*******************************************************************
 *
 * @brief Fill PUCCH PDU in Ul TTI Request
 *
 * @details
 *
 *    Function : fillPucchPdu
 *
 *    Functionality: Fill PUCCH PDU in Ul TTI Request
 *
 * @params[in] 
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
void fillPucchPdu(fapi_ul_tti_req_pdu_t *ulTtiReqPdu, fapi_vendor_ul_tti_req_pdu_t *ulTtiVendorPdu, MacCellCfg *macCellCfg,\
          SchPucchInfo *pucchInfo)
{
   if(ulTtiReqPdu != NULLP)
   {
      memset(&ulTtiReqPdu->pdu.pucch_pdu, 0, sizeof(fapi_ul_pucch_pdu_t));
#ifdef OAI_TESTING 
      ulTtiReqPdu->pduType                    = reverseBytes16(PUCCH_PDU_TYPE);
      ulTtiReqPdu->pduSize                    = reverseBytes16(sizeof(fapi_ul_pucch_pdu_t));
      ulTtiReqPdu->pdu.pucch_pdu.rnti         = reverseBytes16(pucchInfo->crnti);
      /* TODO : Fill handle in raCb when scheduling pucch and access here */
      ulTtiReqPdu->pdu.pucch_pdu.handle       = reverseBytes32(100);
      ulTtiReqPdu->pdu.pucch_pdu.bwpSize      = reverseBytes16(macCellCfg->cellCfg.initialUlBwp.bwp.numPrb);
      ulTtiReqPdu->pdu.pucch_pdu.bwpStart     = reverseBytes16(macCellCfg->cellCfg.initialUlBwp.bwp.firstPrb);
      ulTtiReqPdu->pdu.pucch_pdu.prbStart     = reverseBytes16(pucchInfo->fdAlloc.startPrb);
      ulTtiReqPdu->pdu.pucch_pdu.prbSize      = reverseBytes16(pucchInfo->fdAlloc.numPrb);
      ulTtiReqPdu->pdu.pucch_pdu.secondHopPrb = reverseBytes16(pucchInfo->secondPrbHop);
      ulTtiReqPdu->pdu.pucch_pdu.hoppingId    = reverseBytes16(0);
      ulTtiReqPdu->pdu.pucch_pdu.initialCyclicShift = reverseBytes16(pucchInfo->initialCyclicShift);
      ulTtiReqPdu->pdu.pucch_pdu.dataScramblingId = reverseBytes16(0); /* Valid for Format 2, 3, 4 */
      ulTtiReqPdu->pdu.pucch_pdu.bitLenHarq       = reverseBytes16(pucchInfo->harqInfo.harqBitLength);
      ulTtiReqPdu->pdu.pucch_pdu.bitLenCsiPart1   = reverseBytes16(0); /* Valid for Format 2, 3, 4 */
      ulTtiReqPdu->pdu.pucch_pdu.bitLenCsiPart2   = reverseBytes16(0); /* Valid for Format 2, 3, 4 */
      ulTtiReqPdu->pdu.pucch_pdu.beamforming.numPrgs = reverseBytes16(pucchInfo->beamPucchInfo.numPrgs); 
      ulTtiReqPdu->pdu.pucch_pdu.beamforming.prgSize = reverseBytes16(pucchInfo->beamPucchInfo.prgSize);
      ulTtiReqPdu->pdu.pucch_pdu.beamforming.rx_bfi[0].beamIdx[0].beamidx = reverseBytes16(pucchInfo->beamPucchInfo.prg[0].beamIdx[0]);
#else
      ulTtiReqPdu->pduType                    = PUCCH_PDU_TYPE;
      ulTtiReqPdu->pduSize                    = sizeof(fapi_ul_pucch_pdu_t);
      ulTtiReqPdu->pdu.pucch_pdu.rnti         = pucchInfo->crnti;
      /* TODO : Fill handle in raCb when scheduling pucch and access here */
      ulTtiReqPdu->pdu.pucch_pdu.handle       = 100;
      ulTtiReqPdu->pdu.pucch_pdu.bwpSize      = macCellCfg->cellCfg.initialUlBwp.bwp.numPrb;
      ulTtiReqPdu->pdu.pucch_pdu.bwpStart     = macCellCfg->cellCfg.initialUlBwp.bwp.firstPrb;
      ulTtiReqPdu->pdu.pucch_pdu.prbStart     = pucchInfo->fdAlloc.startPrb;
      ulTtiReqPdu->pdu.pucch_pdu.prbSize      = pucchInfo->fdAlloc.numPrb;
      ulTtiReqPdu->pdu.pucch_pdu.secondHopPrb = pucchInfo->secondPrbHop;
      ulTtiReqPdu->pdu.pucch_pdu.hoppingId    = 0;
      ulTtiReqPdu->pdu.pucch_pdu.initialCyclicShift = pucchInfo->initialCyclicShift;
      ulTtiReqPdu->pdu.pucch_pdu.dataScramblingId = 0; /* Valid for Format 2, 3, 4 */
      ulTtiReqPdu->pdu.pucch_pdu.bitLenHarq       = pucchInfo->harqInfo.harqBitLength;
      ulTtiReqPdu->pdu.pucch_pdu.bitLenCsiPart1   = 0; /* Valid for Format 2, 3, 4 */
      ulTtiReqPdu->pdu.pucch_pdu.bitLenCsiPart2   = 0; /* Valid for Format 2, 3, 4 */
      ulTtiReqPdu->pdu.pucch_pdu.beamforming.numPrgs = pucchInfo->beamPucchInfo.numPrgs; 
      ulTtiReqPdu->pdu.pucch_pdu.beamforming.prgSize = pucchInfo->beamPucchInfo.prgSize;
      ulTtiReqPdu->pdu.pucch_pdu.beamforming.rx_bfi[0].beamIdx[0].beamidx = pucchInfo->beamPucchInfo.prg[0].beamIdx[0];
#endif
      ulTtiReqPdu->pdu.pucch_pdu.subCarrierSpacing = macCellCfg->cellCfg.initialUlBwp.bwp.scs;
      ulTtiReqPdu->pdu.pucch_pdu.cyclicPrefix = macCellCfg->cellCfg.initialUlBwp.bwp.cyclicPrefix;
      ulTtiReqPdu->pdu.pucch_pdu.formatType   = pucchInfo->pucchFormat; /* Supporting PUCCH Format 0 */
      ulTtiReqPdu->pdu.pucch_pdu.multiSlotTxIndicator = 0; /* No Multi Slot transmission */
      
      ulTtiReqPdu->pdu.pucch_pdu.startSymbolIndex = pucchInfo->tdAlloc.startSymb;
      ulTtiReqPdu->pdu.pucch_pdu.nrOfSymbols  = pucchInfo->tdAlloc.numSymb;
      ulTtiReqPdu->pdu.pucch_pdu.freqHopFlag  = pucchInfo->intraFreqHop;
      ulTtiReqPdu->pdu.pucch_pdu.groupHopFlag = 0;     
      ulTtiReqPdu->pdu.pucch_pdu.sequenceHopFlag = 0;
      ulTtiReqPdu->pdu.pucch_pdu.timeDomainOccIdx = pucchInfo->timeDomOCC; 
      ulTtiReqPdu->pdu.pucch_pdu.preDftOccIdx = pucchInfo->occIdx; /* Valid for Format 4 only */
      ulTtiReqPdu->pdu.pucch_pdu.preDftOccLen = pucchInfo->occLen; /* Valid for Format 4 only */
      ulTtiReqPdu->pdu.pucch_pdu.pi2Bpsk = pucchInfo->pi2BPSK;
      ulTtiReqPdu->pdu.pucch_pdu.addDmrsFlag = pucchInfo->addDmrs;/* Valid for Format 3, 4 only */
      ulTtiReqPdu->pdu.pucch_pdu.dmrsScramblingId = 0; /* Valid for Format 2 */
      ulTtiReqPdu->pdu.pucch_pdu.dmrsCyclicShift  = 0; /* Valid for Format 4 */
      ulTtiReqPdu->pdu.pucch_pdu.srFlag           = pucchInfo->srFlag;
      ulTtiReqPdu->pdu.pucch_pdu.beamforming.digBfInterface = pucchInfo->beamPucchInfo.digBfInterfaces;


      /* UL TTI Vendor PDU */
      ulTtiVendorPdu->pdu_type = FAPI_PUCCH_PDU_TYPE;
      ulTtiVendorPdu->pdu.pucch_pdu.nr_of_rx_ru=1;
      ulTtiVendorPdu->pdu.pucch_pdu.group_id=0;
      for(int i =0; i<FAPI_VENDOR_MAX_RXRU_NUM; i++)
      {
	      ulTtiVendorPdu->pdu.pucch_pdu.rx_ru_idx[i]=0;
      }
   }
}

#endif

/*******************************************************************
 *
 * @brief Sends UL TTI Request to PHY
 *
 * @details
 *
 *    Function : fillUlTtiReq
 *
 *    Functionality:
 *         -Sends FAPI Param req to PHY
 *
 * @params[in]  Pointer to CmLteTimingInfo
 * @return ROK     - success
 *         RFAILED - failure
 *
 ******************************************************************/
uint16_t fillUlTtiReq(SlotTimingInfo currTimingInfo, p_fapi_api_queue_elem_t prevElem, fapi_vendor_ul_tti_req_t* vendorUlTti)
{
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : UL_TTI_REQUEST\n");
#endif

#ifdef INTEL_FAPI
#ifndef OAI_TESTING 
   uint16_t   cellIdx =0;
   uint8_t    pduIdx = -1;
   uint8_t    ueIdx = 0;
   SlotTimingInfo ulTtiReqTimingInfo;
   MacUlSlot *currUlSlot = NULLP;
   MacCellCfg macCellCfg;
   fapi_ul_tti_req_t *ulTtiReq = NULLP;
   p_fapi_api_queue_elem_t ulTtiElem;

   if(lwrMacCb.phyState == PHY_STATE_RUNNING)
   {
      GET_CELL_IDX(currTimingInfo.cellId, cellIdx);
      macCellCfg = macCb.macCell[cellIdx]->macCellCfg;

      /* add PHY delta */
      ADD_DELTA_TO_TIME(currTimingInfo,ulTtiReqTimingInfo,gConfigInfo.gPhyDeltaUl, macCb.macCell[cellIdx]->numOfSlots);
      currUlSlot = &macCb.macCell[cellIdx]->ulSlot[ulTtiReqTimingInfo.slot % macCb.macCell[cellIdx]->numOfSlots];

      LWR_MAC_ALLOC(ulTtiElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_ul_tti_req_t)));
      if(ulTtiElem)
      {
         FILL_FAPI_LIST_ELEM(ulTtiElem, NULLP, FAPI_UL_TTI_REQUEST, 1, \
               sizeof(fapi_ul_tti_req_t));
         ulTtiReq = (fapi_ul_tti_req_t *)(ulTtiElem +1);
         memset(ulTtiReq, 0, sizeof(fapi_ul_tti_req_t));
         fillMsgHeader(&ulTtiReq->header, FAPI_UL_TTI_REQUEST, sizeof(fapi_ul_tti_req_t));
         ulTtiReq->sfn  = ulTtiReqTimingInfo.sfn;
         ulTtiReq->slot = ulTtiReqTimingInfo.slot;
         ulTtiReq->nPdus = getnPdus(ulTtiReq, currUlSlot);
         vendorUlTti->num_ul_pdu =  ulTtiReq->nPdus;
         vendorUlTti->sym = 0;
         ulTtiReq->nGroup = 0;
         if(ulTtiReq->nPdus > 0)
         {
#ifdef ODU_SLOT_IND_DEBUG_LOG
               DU_LOG("\nDEBUG --> LWR_MAC: UL_TTI_REQ, datatype:%d, sfn/slot:%d/%d", currUlSlot->ulSchInfo.dataType,  ulTtiReq->sfn,  ulTtiReq->slot);
#endif
            /* Fill Prach Pdu */
            if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_PRACH)
            {
               pduIdx++;
               fillPrachPdu(&ulTtiReq->pdus[pduIdx], &macCellCfg, currUlSlot);
               ulTtiReq->rachPresent++;
            }
            for(ueIdx = 0; ueIdx < MAX_NUM_UE; ueIdx++)
            {
               /* Fill PUSCH PDU */
               if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_PUSCH)
               {
                  if(currUlSlot->ulSchInfo.schPuschInfo[ueIdx].crnti != 0)
                  {
#ifdef ODU_SLOT_IND_DEBUG_LOG
               DU_LOG("\nDEBUG --> LWR_MAC: UL_TTI_REQ, PUSCH PDU ueId:%d", ueIdx);
#endif
                     pduIdx++;
                     fillPuschPdu(&ulTtiReq->pdus[pduIdx], &vendorUlTti->ul_pdus[pduIdx], &macCellCfg, &currUlSlot->ulSchInfo.schPuschInfo[ueIdx]);
                     ulTtiReq->nUlsch++;
                  }
               }
               /* Fill PUCCH PDU */
               if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_UCI)
               {
                  if(currUlSlot->ulSchInfo.schPucchInfo[ueIdx].crnti != 0)
                  {
                     pduIdx++;
                     fillPucchPdu(&ulTtiReq->pdus[pduIdx], &vendorUlTti->ul_pdus[pduIdx], &macCellCfg, &currUlSlot->ulSchInfo.schPucchInfo[ueIdx]);
                     ulTtiReq->nUlcch++;
                  }
               }
            }
            ulTtiReq->ueGrpInfo[ulTtiReq->nGroup].nUe = MAX_NUM_UE_PER_TTI;
            ulTtiReq->nGroup++;
         } 

#ifdef ODU_SLOT_IND_DEBUG_LOG
         DU_LOG("\nDEBUG  -->  LWR_MAC: Sending UL TTI Request");
#endif
         prevElem->p_next = ulTtiElem;

         memset(currUlSlot, 0, sizeof(MacUlSlot));
         return ROK;
      }
      else
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for UL TTI Request");
         memset(currUlSlot, 0, sizeof(MacUlSlot));
         return RFAILED;
      }
   }
   else
   {
      lwr_mac_procInvalidEvt(&currTimingInfo);
   }

#else

   uint8_t    pduIdx = -1;
   uint8_t    ueIdx = 0;
   uint8_t    nPdus = 0;
   uint16_t   cellIdx =0;
   uint32_t   msgLen = 0;

   SlotTimingInfo ulTtiReqTimingInfo;
   MacUlSlot *currUlSlot = NULLP;
   MacCellCfg macCellCfg;
   fapi_ul_tti_req_t *ulTtiReq = NULLP;
   p_fapi_api_queue_elem_t ulTtiElem;

   if(lwrMacCb.phyState == PHY_STATE_RUNNING)
   {
      GET_CELL_IDX(currTimingInfo.cellId, cellIdx);
      macCellCfg = macCb.macCell[cellIdx]->macCellCfg;
      
      /* add PHY delta */
      ADD_DELTA_TO_TIME(currTimingInfo,ulTtiReqTimingInfo,gConfigInfo.gPhyDeltaUl, macCb.macCell[cellIdx]->numOfSlots);
      currUlSlot = &macCb.macCell[cellIdx]->ulSlot[ulTtiReqTimingInfo.slot % macCb.macCell[cellIdx]->numOfSlots];
      
      LWR_MAC_ALLOC(ulTtiReq, (sizeof(fapi_ul_tti_req_t)));
      if(ulTtiReq)
      {
         memset(ulTtiReq, 0, sizeof(fapi_ul_tti_req_t));

         nPdus=getnPdus(ulTtiReq, currUlSlot);
         if(nPdus == 0)
         {
	      LWR_MAC_FREE(ulTtiReq, (sizeof(fapi_ul_tti_req_t)));
	      return ROK;
         }

         /* Below msg length is calculated based on the parameter present in fapi_ul_tti_req_t structure
          * the prameters of fapi_ul_tti_req_t structure are ->
          * header = sizeof(fapi_msg_t) , {sfn, slot} = 2*sizeof(uint16_t),
          * {numPdus, rachPresent, nGroup, nUlsch, nUlcch} = 5 * sizeof(uint8_t), total number of pdu supproted = numPdus*sizeof(fapi_ul_tti_req_pdu_t)
          * sizeof(fapi_ue_info_t) */

         msgLen=sizeof(fapi_msg_t)+ (2*sizeof(uint16_t)) + (5*sizeof(uint8_t)) + (nPdus*sizeof(fapi_ul_tti_req_pdu_t)) + sizeof(fapi_ue_info_t);
     
         fillMsgHeader(&ulTtiReq->header, FAPI_UL_TTI_REQUEST, msgLen);
         ulTtiReq->sfn  = reverseBytes16(ulTtiReqTimingInfo.sfn);
         ulTtiReq->slot = reverseBytes16(ulTtiReqTimingInfo.slot);
         ulTtiReq->nPdus = nPdus;
         ulTtiReq->nGroup = 0;

         if(ulTtiReq->nPdus > 0)
         {
#ifdef ODU_SLOT_IND_DEBUG_LOG
               DU_LOG("\nDEBUG --> LWR_MAC: UL_TTI_REQ, datatype:%d, sfn/slot:%d/%d", currUlSlot->ulSchInfo.dataType,  ulTtiReqTimingInfo.sfn,  ulTtiReqTimingInfo.slot);
#endif
            /* Fill Prach Pdu */
            if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_PRACH)
            {
               pduIdx++;
               fillPrachPdu(&ulTtiReq->pdus[pduIdx], &macCellCfg, currUlSlot);
               ulTtiReq->rachPresent++;
            }
            for(ueIdx = 0; ueIdx < MAX_NUM_UE; ueIdx++)
            {
               /* Fill PUSCH PDU */
               if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_PUSCH)
               {
                  if(currUlSlot->ulSchInfo.schPuschInfo[ueIdx].crnti != 0)
                  {
#ifdef ODU_SLOT_IND_DEBUG_LOG
               DU_LOG("\nDEBUG --> LWR_MAC: UL_TTI_REQ, PUSCH PDU ueId:%d", ueIdx);
#endif
                     pduIdx++;
                     fillPuschPdu(&ulTtiReq->pdus[pduIdx], NULLP, &macCellCfg, &currUlSlot->ulSchInfo.schPuschInfo[ueIdx]);
                     ulTtiReq->nUlsch++;
                  }
               }
               /* Fill PUCCH PDU */
               if(currUlSlot->ulSchInfo.dataType & SCH_DATATYPE_UCI)
               {
                  if(currUlSlot->ulSchInfo.schPucchInfo[ueIdx].crnti != 0)
                  {
                     pduIdx++;
                     fillPucchPdu(&ulTtiReq->pdus[pduIdx], NULLP, &macCellCfg, &currUlSlot->ulSchInfo.schPucchInfo[ueIdx]);
                     ulTtiReq->nUlcch++;
                  }
               }
            }
            ulTtiReq->ueGrpInfo[ulTtiReq->nGroup].nUe = MAX_NUM_UE_PER_TTI;
            ulTtiReq->nGroup++;
         }
	 uint32_t  bufferLen=0;
	 uint8_t mBuf[2500];
	 packUlTtiReq(ulTtiReq, mBuf, &bufferLen);
	 LWR_MAC_ALLOC(ulTtiElem, (sizeof(fapi_api_queue_elem_t)  + bufferLen));
	 if(ulTtiElem==NULLP)
	 {

		 DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for UL TTI Request");
		 return RFAILED;
	 }
	 FILL_FAPI_LIST_ELEM(ulTtiElem, NULLP, FAPI_UL_TTI_REQUEST, 1, bufferLen);

	 memcpy((uint8_t *)( ulTtiElem +1), mBuf, bufferLen);
#ifdef ODU_SLOT_IND_DEBUG_LOG
	 DU_LOG("\nDEBUG  -->  LWR_MAC: Sending UL TTI Request");
#endif
	 prevElem->p_next = ulTtiElem;

	 LWR_MAC_FREE(ulTtiReq, (sizeof(fapi_ul_tti_req_t)));
	 memset(currUlSlot, 0, sizeof(MacUlSlot));
	 return ROK;

      }
      else
      {
         DU_LOG("\nERROR  -->  LWR_MAC: Failed to allocate memory for UL TTI Request");
         memset(currUlSlot, 0, sizeof(MacUlSlot));
         return RFAILED;
      }
   }
   else
   {
      lwr_mac_procInvalidEvt(&currTimingInfo);
   }
  
#endif
#endif
   return ROK;
}

#ifdef INTEL_FAPI
/*******************************************************************
 *
 * @brief fills bsr Ul DCI PDU required for UL DCI Request to PHY
 *
 * @details
 *
 *    Function : fillUlDciPdu
 *
 *    Functionality:
 *         -Fills the Ul DCI PDU, spec Ref:38.212, Table 7.3.1-1
 *
 * @params[in] Pointer to fapi_dl_dci_t
 *             Pointer to DciInfo
 * @return ROK
 *
 ******************************************************************/
void fillUlDciPdu(fapi_dl_dci_t *ulDciPtr, DciInfo *schDciInfo)
{
#ifdef CALL_FLOW_DEBUG_LOG
   DU_LOG("\nCall Flow: ENTMAC -> ENTLWRMAC : UL_DCI_REQUEST\n");
#endif
   if(ulDciPtr != NULLP)
   {
      uint8_t numBytes =0;
      uint8_t bytePos =0;
      uint8_t bitPos =0;

      uint8_t  coreset1Size = 0;
      uint16_t rbStart = 0;
      uint16_t rbLen = 0;
      uint8_t  dciFormatId = 0;
      uint32_t freqDomResAssign =0;
      uint8_t  timeDomResAssign =0;
      uint8_t  freqHopFlag =0;
      uint8_t  modNCodScheme =0;
      uint8_t  ndi =0;
      uint8_t  redundancyVer = 0;
      uint8_t  harqProcessNum = 0;
      uint8_t  puschTpc = 0;
      uint8_t  ul_SlInd = 0;

      /* Size(in bits) of each field in DCI format 0_0 */
      uint8_t dciFormatIdSize      = 1;
      uint8_t freqDomResAssignSize = 0;
      uint8_t timeDomResAssignSize = 4;
      uint8_t freqHopFlagSize      = 1;
      uint8_t modNCodSchemeSize    = 5;
      uint8_t ndiSize              = 1;
      uint8_t redundancyVerSize    = 2;
      uint8_t harqProcessNumSize   = 4;
      uint8_t puschTpcSize         = 2;
      uint8_t ul_SlIndSize         = 1;

#ifndef OAI_TESTING
      ulDciPtr->rnti                          = schDciInfo->dciInfo.rnti;
      ulDciPtr->scramblingId                  = schDciInfo->dciInfo.scramblingId;    
      ulDciPtr->scramblingRnti                = schDciInfo->dciInfo.scramblingRnti;
      ulDciPtr->pc_and_bform.numPrgs          = schDciInfo->dciInfo.beamPdcchInfo.numPrgs;
      ulDciPtr->pc_and_bform.prgSize          = schDciInfo->dciInfo.beamPdcchInfo.prgSize;
      ulDciPtr->pc_and_bform.pmi_bfi[0].pmIdx = schDciInfo->dciInfo.beamPdcchInfo.prg[0].pmIdx;
      ulDciPtr->pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = schDciInfo->dciInfo.beamPdcchInfo.prg[0].beamIdx[0];
#else
      ulDciPtr->rnti                          = reverseBytes16(schDciInfo->dciInfo.rnti);
      ulDciPtr->scramblingId                  = reverseBytes16(schDciInfo->dciInfo.scramblingId);    
      ulDciPtr->scramblingRnti                = reverseBytes16(schDciInfo->dciInfo.scramblingRnti);
      ulDciPtr->pc_and_bform.numPrgs          = reverseBytes16(schDciInfo->dciInfo.beamPdcchInfo.numPrgs);
      ulDciPtr->pc_and_bform.prgSize          = reverseBytes16(schDciInfo->dciInfo.beamPdcchInfo.prgSize);
      ulDciPtr->pc_and_bform.pmi_bfi[0].pmIdx = reverseBytes16(schDciInfo->dciInfo.beamPdcchInfo.prg[0].pmIdx);
      ulDciPtr->pc_and_bform.pmi_bfi[0].beamIdx[0].beamidx = reverseBytes16(schDciInfo->dciInfo.beamPdcchInfo.prg[0].beamIdx[0]);
#endif
      ulDciPtr->cceIndex                      = schDciInfo->dciInfo.cceIndex;
      ulDciPtr->aggregationLevel              = schDciInfo->dciInfo.aggregLevel;
      ulDciPtr->pc_and_bform.digBfInterfaces  = schDciInfo->dciInfo.beamPdcchInfo.digBfInterfaces;
      ulDciPtr->beta_pdcch_1_0                = schDciInfo->dciInfo.txPdcchPower.beta_pdcch_1_0;           
      ulDciPtr->powerControlOffsetSS          = schDciInfo->dciInfo.txPdcchPower.powerControlOffsetSS;

      /* Calculating freq domain resource allocation field value and size
       * coreset1Size = Size of coreset 1
       * RBStart = Starting Virtual Rsource block
       * RBLen = length of contiguously allocted RBs
       * Spec 38.214 Sec 5.1.2.2.2
       */
      if(schDciInfo->dciFormatInfo.formatType == FORMAT0_0)
      {
         coreset1Size = schDciInfo->coresetCfg.coreSetSize;
         rbLen = schDciInfo->dciFormatInfo.format.format0_0.freqAlloc.resAlloc.type1.numPrb;
         rbStart = schDciInfo->dciFormatInfo.format.format0_0.freqAlloc.resAlloc.type1.startPrb;

         if((rbLen >=1) && (rbLen <= coreset1Size - rbStart))
         {
            if((rbLen - 1) <= floor(coreset1Size / 2))
               freqDomResAssign = (coreset1Size * (rbLen-1)) + rbStart;
            else
               freqDomResAssign = (coreset1Size * (coreset1Size - rbLen + 1)) \
                                  + (coreset1Size - 1 - rbStart);

            freqDomResAssignSize = ceil(log2(coreset1Size * (coreset1Size + 1) / 2));
         }
         /* Fetching DCI field values */
         dciFormatId      = schDciInfo->dciFormatInfo.formatType; /* DCI indentifier for UL DCI */
         timeDomResAssign = schDciInfo->dciFormatInfo.format.format0_0.rowIndex;
         freqHopFlag      = schDciInfo->dciFormatInfo.format.format0_0.freqHopFlag; 
         modNCodScheme    = schDciInfo->dciFormatInfo.format.format0_0.mcs;
         ndi              = schDciInfo->dciFormatInfo.format.format0_0.ndi; 
         redundancyVer    = schDciInfo->dciFormatInfo.format.format0_0.rvIndex;
         harqProcessNum   = schDciInfo->dciFormatInfo.format.format0_0.harqProcId; 
         puschTpc         = schDciInfo->dciFormatInfo.format.format0_0.tpcCmd;
         ul_SlInd         = schDciInfo->dciFormatInfo.format.format0_0.sulIndicator;
     
         /* Reversing bits in each DCI field */
         dciFormatId      = reverseBits(dciFormatId, dciFormatIdSize);
         freqDomResAssign = reverseBits(freqDomResAssign, freqDomResAssignSize);
         timeDomResAssign = reverseBits(timeDomResAssign, timeDomResAssignSize);
         modNCodScheme    = reverseBits(modNCodScheme, modNCodSchemeSize);
         redundancyVer    = reverseBits(redundancyVer, redundancyVerSize);
         harqProcessNum   = reverseBits(harqProcessNum, harqProcessNumSize);
         puschTpc         = reverseBits(puschTpc, puschTpcSize);
         ul_SlInd         = reverseBits(ul_SlInd, ul_SlIndSize);
      }
      /* Calulating total number of bytes in buffer */
      ulDciPtr->payloadSizeBits = (dciFormatIdSize + freqDomResAssignSize\
      + timeDomResAssignSize + freqHopFlagSize + modNCodSchemeSize + ndi \
      + redundancyVerSize + harqProcessNumSize + puschTpcSize + ul_SlIndSize);

      numBytes = ulDciPtr->payloadSizeBits / 8;
      if(ulDciPtr->payloadSizeBits % 8)
         numBytes += 1;

#ifdef OAI_TESTING
      ulDciPtr->payloadSizeBits = reverseBytes16(ulDciPtr->payloadSizeBits);
#endif
      if(numBytes > FAPI_DCI_PAYLOAD_BYTE_LEN)
      {
         DU_LOG("\nERROR  -->  LWR_MAC : Total bytes for DCI is more than expected");
         return;
      }

      /* Initialize buffer */
      for(bytePos = 0; bytePos < numBytes; bytePos++)
         ulDciPtr->payload[bytePos] = 0;

      bytePos = numBytes - 1;
      bitPos = 0;

      /* Packing DCI format fields */
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            dciFormatId, dciFormatIdSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            freqDomResAssign, freqDomResAssignSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            timeDomResAssign, timeDomResAssignSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            freqHopFlag, freqHopFlagSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            modNCodScheme, modNCodSchemeSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            ndi, ndiSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            redundancyVer, redundancyVerSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            harqProcessNum, harqProcessNumSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            puschTpc, puschTpcSize);
      fillDlDciPayload(ulDciPtr->payload, &bytePos, &bitPos,\
            ul_SlInd, ul_SlIndSize);
   }
} /* fillUlDciPdu */

/*******************************************************************
 *
 * @brief fills PDCCH PDU required for UL DCI REQ to PHY
 *
 * @details
 *
 *    Function : fillUlDciPdcchPdu
 *
 *    Functionality:
 *         -Fills the Pdcch PDU info
 *
 * @params[in] Pointer to FAPI DL TTI Req
 *             Pointer to PdcchCfg
 * @return ROK
 *
 ******************************************************************/
uint8_t fillUlDciPdcchPdu(fapi_dci_pdu_t *ulDciReqPdu, fapi_vendor_dci_pdu_t *vendorUlDciPdu, DlSchedInfo *dlInfo, uint8_t coreSetType)
{
   if(ulDciReqPdu != NULLP)
   {
      memset(&ulDciReqPdu->pdcchPduConfig, 0, sizeof(fapi_dl_pdcch_pdu_t));
      fillUlDciPdu(ulDciReqPdu->pdcchPduConfig.dlDci, dlInfo->ulGrant);

#ifndef OAI_TESTING
      /* Calculating PDU length. Considering only one Ul dci pdu for now */
      ulDciReqPdu->pduSize                          = sizeof(fapi_dl_pdcch_pdu_t);
      ulDciReqPdu->pduType                          = PDCCH_PDU_TYPE;
      ulDciReqPdu->pdcchPduConfig.bwpSize           = dlInfo->ulGrant->bwpCfg.freqAlloc.numPrb;
      ulDciReqPdu->pdcchPduConfig.bwpStart          = dlInfo->ulGrant->bwpCfg.freqAlloc.startPrb;
      ulDciReqPdu->pdcchPduConfig.shiftIndex        = dlInfo->ulGrant->coresetCfg.shiftIndex;
      ulDciReqPdu->pdcchPduConfig.numDlDci          = 1;
#else
      /* Calculating PDU length. Considering only one Ul dci pdu for now */
      ulDciReqPdu->pduSize                          = reverseBytes16(sizeof(fapi_dl_pdcch_pdu_t));
      ulDciReqPdu->pduType                          = reverseBytes16(PDCCH_PDU_TYPE);
      ulDciReqPdu->pdcchPduConfig.bwpSize           = reverseBytes16(dlInfo->ulGrant->bwpCfg.freqAlloc.numPrb);
      ulDciReqPdu->pdcchPduConfig.bwpStart          = reverseBytes16(dlInfo->ulGrant->bwpCfg.freqAlloc.startPrb);
      ulDciReqPdu->pdcchPduConfig.shiftIndex        = reverseBytes16(dlInfo->ulGrant->coresetCfg.shiftIndex);
      ulDciReqPdu->pdcchPduConfig.numDlDci          = reverseBytes16(1);
#endif
      ulDciReqPdu->pdcchPduConfig.subCarrierSpacing = dlInfo->ulGrant->bwpCfg.subcarrierSpacing; 
      ulDciReqPdu->pdcchPduConfig.cyclicPrefix      = dlInfo->ulGrant->bwpCfg.cyclicPrefix; 
      ulDciReqPdu->pdcchPduConfig.startSymbolIndex  = dlInfo->ulGrant->coresetCfg.startSymbolIndex;
      ulDciReqPdu->pdcchPduConfig.durationSymbols   = dlInfo->ulGrant->coresetCfg.durationSymbols;
      convertFreqDomRsrcMapToIAPIFormat(dlInfo->ulGrant->coresetCfg.freqDomainResource, \
                                           ulDciReqPdu->pdcchPduConfig.freqDomainResource);
      ulDciReqPdu->pdcchPduConfig.cceRegMappingType = dlInfo->ulGrant->coresetCfg.cceRegMappingType;
      ulDciReqPdu->pdcchPduConfig.regBundleSize     = dlInfo->ulGrant->coresetCfg.regBundleSize;
      ulDciReqPdu->pdcchPduConfig.interleaverSize   = dlInfo->ulGrant->coresetCfg.interleaverSize;
      ulDciReqPdu->pdcchPduConfig.precoderGranularity = dlInfo->ulGrant->coresetCfg.precoderGranularity;
      ulDciReqPdu->pdcchPduConfig.coreSetType       = coreSetType;


#ifndef OAI_TESTING
      /* Vendor UL DCI PDU */
      vendorUlDciPdu->pdcch_pdu_config.num_dl_dci = ulDciReqPdu->pdcchPduConfig.numDlDci;
      vendorUlDciPdu->pdcch_pdu_config.dl_dci[0].epre_ratio_of_pdcch_to_ssb = 0;
      vendorUlDciPdu->pdcch_pdu_config.dl_dci[0].epre_ratio_of_dmrs_to_ssb = 0;
#endif
   }
   return ROK;
}
#endif
/*******************************************************************
 *
 * @brief Sends UL DCI Request to PHY
 *
 * @details
 *
 *    Function : fillUlDciReq
 *
 *    Functionality:
 *         -Sends FAPI Ul Dci req to PHY
 *
 * @params[in]  Pointer to CmLteTimingInfo
 * @return ROK     - success
 *         RFAILED - failure
 *
 ******************************************************************/
uint16_t fillUlDciReq(SlotTimingInfo currTimingInfo, p_fapi_api_queue_elem_t prevElem, fapi_vendor_ul_dci_req_t *vendorUlDciReq)
{
#ifdef INTEL_FAPI
#ifndef OAI_TESTING
   uint8_t      cellIdx =0;
   uint8_t      numPduEncoded = 0;
   SlotTimingInfo  ulDciReqTimingInfo ={0};
   MacDlSlot    *currDlSlot = NULLP;
   fapi_ul_dci_req_t        *ulDciReq =NULLP;
   p_fapi_api_queue_elem_t  ulDciElem;

   if(lwrMacCb.phyState == PHY_STATE_RUNNING)
   {
      GET_CELL_IDX(currTimingInfo.cellId, cellIdx);
      memcpy(&ulDciReqTimingInfo, &currTimingInfo, sizeof(SlotTimingInfo));
      currDlSlot = &macCb.macCell[cellIdx]->dlSlot[ulDciReqTimingInfo.slot % macCb.macCell[cellIdx]->numOfSlots];

      LWR_MAC_ALLOC(ulDciElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_ul_dci_req_t)));
      if(ulDciElem)
      {
         FILL_FAPI_LIST_ELEM(ulDciElem, NULLP, FAPI_UL_DCI_REQUEST, 1, \
               sizeof(fapi_ul_dci_req_t));
         ulDciReq = (fapi_ul_dci_req_t *)(ulDciElem +1);
         memset(ulDciReq, 0, sizeof(fapi_ul_dci_req_t));
         fillMsgHeader(&ulDciReq->header, FAPI_UL_DCI_REQUEST, sizeof(fapi_ul_dci_req_t));

         ulDciReq->sfn  = ulDciReqTimingInfo.sfn;
         ulDciReq->slot = ulDciReqTimingInfo.slot;
         if(currDlSlot->dlInfo.ulGrant != NULLP)
         {
            vendorUlDciReq->sym = 0;
            ulDciReq->numPdus = 1;  // No. of PDCCH PDUs
            vendorUlDciReq->num_pdus = ulDciReq->numPdus;
            if(ulDciReq->numPdus > 0)
            {
               /* Fill PDCCH configuration Pdu */
               fillUlDciPdcchPdu(&ulDciReq->pdus[numPduEncoded], &vendorUlDciReq->pdus[numPduEncoded], &currDlSlot->dlInfo, CORESET_TYPE1);
               numPduEncoded++;
               /* free UL GRANT at SCH */
               MAC_FREE(currDlSlot->dlInfo.ulGrant, sizeof(DciInfo));
            }
#ifdef ODU_SLOT_IND_DEBUG_LOG
            DU_LOG("\nDEBUG  -->  LWR_MAC: Sending UL DCI Request");
#endif
         }
         prevElem->p_next = ulDciElem;
      }
   }
   else
   {
      lwr_mac_procInvalidEvt(&currTimingInfo);
   }
#else
   uint8_t      cellIdx =0;
   uint8_t      numPdus =0;
   uint8_t      numPduEncoded = 0;
   uint32_t     msgLen=0;

   SlotTimingInfo  ulDciReqTimingInfo ={0};
   MacDlSlot    *currDlSlot = NULLP;
   fapi_ul_dci_req_t        *ulDciReq =NULLP;
   p_fapi_api_queue_elem_t  ulDciElem;

   if(lwrMacCb.phyState == PHY_STATE_RUNNING)
   {
      GET_CELL_IDX(currTimingInfo.cellId, cellIdx);
      memcpy(&ulDciReqTimingInfo, &currTimingInfo, sizeof(SlotTimingInfo));
      currDlSlot = &macCb.macCell[cellIdx]->dlSlot[ulDciReqTimingInfo.slot % macCb.macCell[cellIdx]->numOfSlots];
      
      if(currDlSlot->dlInfo.ulGrant != NULLP)
      {
         numPdus = 1;
      }
      else
      {
	      return ROK;
      }

      /* Below msg length is calculated based on the parameter present in fapi_ul_dci_req_t structure
       * the prameters of fapi_ul_dci_req_t structure are ->
       * header = sizeof(fapi_msg_t) ,sfn, slot = 2*sizeof(uint16_t),
       * numPdus = sizeof(uint8_t) and total number of fapi_dci_pdu_t supproted = numPdus*sizeof(fapi_dci_pdu_t)*/

      msgLen = sizeof(fapi_msg_t) + (2*sizeof(uint16_t)) + sizeof(uint8_t) + (numPdus*sizeof(fapi_dci_pdu_t));
      
      LWR_MAC_ALLOC(ulDciElem, (sizeof(fapi_api_queue_elem_t) + msgLen));
      if(ulDciElem)
      {
         FILL_FAPI_LIST_ELEM(ulDciElem, NULLP, FAPI_UL_DCI_REQUEST, 1, msgLen);
               
         ulDciReq = (fapi_ul_dci_req_t *)(ulDciElem +1);
         memset(ulDciReq, 0, msgLen);
         fillMsgHeader(&ulDciReq->header, FAPI_UL_DCI_REQUEST, msgLen);

         ulDciReq->sfn  = reverseBytes16(ulDciReqTimingInfo.sfn);
         ulDciReq->slot = reverseBytes16(ulDciReqTimingInfo.slot);
         if(currDlSlot->dlInfo.ulGrant != NULLP)
         {
            ulDciReq->numPdus = 1;  // No. of PDCCH PDUs
            if(ulDciReq->numPdus > 0)
            {
               /* Fill PDCCH configuration Pdu */
               fillUlDciPdcchPdu(&ulDciReq->pdus[numPduEncoded], NULLP, &currDlSlot->dlInfo, CORESET_TYPE1);
               numPduEncoded++;
               /* free UL GRANT at SCH */
               MAC_FREE(currDlSlot->dlInfo.ulGrant, sizeof(DciInfo));
            }
#ifdef ODU_SLOT_IND_DEBUG_LOG
            DU_LOG("\nDEBUG  -->  LWR_MAC: Sending UL DCI Request");
#endif
         }
         prevElem->p_next = ulDciElem;
      }
   }
   else
   {
      lwr_mac_procInvalidEvt(&currTimingInfo);
   }

#endif
#endif
   return ROK;
}

#ifdef OAI_TESTING
uint8_t processTtiReq(SlotTimingInfo currTimingInfo)
{
	uint8_t step = 0;
	uint16_t cellIdx=0;
	MacDlSlot *currDlSlot = NULLP;
	SlotTimingInfo dlTtiReqTimingInfo;
	fapi_msg_header_t *msgHeader = NULLP;
	p_fapi_api_queue_elem_t headerElem, current;

	LWR_MAC_ALLOC(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
	if(!headerElem)
	{
		DU_LOG("\nERROR  -->  LWR_MAC: Memory allocation failed for header in DL TTI req");
		return RFAILED;
	}
	FILL_FAPI_LIST_ELEM(headerElem, NULL, FAPI_VENDOR_MSG_HEADER_IND, 1, sizeof(fapi_msg_header_t));

	msgHeader = (fapi_msg_header_t *)(headerElem + 1);
	msgHeader->num_msg = 1;
	msgHeader->handle = 0;

	GET_CELL_IDX(currTimingInfo.cellId, cellIdx);
	ADD_DELTA_TO_TIME(currTimingInfo,dlTtiReqTimingInfo,gConfigInfo.gPhyDeltaDl, macCb.macCell[cellIdx]->numOfSlots);
	dlTtiReqTimingInfo.cellId = currTimingInfo.cellId;
	currDlSlot = &macCb.macCell[cellIdx]->dlSlot[dlTtiReqTimingInfo.slot];

	current=headerElem;
	while (current != NULL)
	{
		switch (step)
		{
			case 0:
				{
					fillDlTtiReq(currTimingInfo, current);
					break;
				}
			case 1:
				{
					fillUlTtiReq(currTimingInfo, current, NULLP);
               break;
				}

			case 2:
				{
					fillUlDciReq(dlTtiReqTimingInfo, current, NULLP);
               break;
				}

			case 3:
				{
					sendTxDataReq(dlTtiReqTimingInfo, currDlSlot, current, NULLP);
               break;
				}

			default:
				current->p_next = NULLP;
				break;
		}
		if(current->p_next)
		{
			msgHeader->num_msg++;
         current = current->p_next;
		}
      if(step == 4)
         break;
      step++;
	}

	if(msgHeader->num_msg == 1)
	{
		LWR_MAC_FREE(headerElem, (sizeof(fapi_api_queue_elem_t) + sizeof(fapi_msg_header_t)));
	}
	else
	{
		LwrMacSendToL1(headerElem);
		memset(currDlSlot, 0, sizeof(MacDlSlot));
	}
	return ROK;
}
#endif

lwrMacFsmHdlr fapiEvtHdlr[MAX_STATE][MAX_EVENT] =
{
   {
      /* PHY_STATE_IDLE */
#ifdef INTEL_TIMER_MODE 
      lwr_mac_procIqSamplesReqEvt,
#endif
      lwr_mac_procParamReqEvt,
      lwr_mac_procParamRspEvt,
      lwr_mac_procConfigReqEvt,
      lwr_mac_procConfigRspEvt,
      lwr_mac_procInvalidEvt,
      lwr_mac_procInvalidEvt,
   },
   {
      /* PHY_STATE_CONFIGURED */
#ifdef INTEL_TIMER_MODE
      lwr_mac_procInvalidEvt,
#endif
      lwr_mac_procParamReqEvt,
      lwr_mac_procParamRspEvt,
      lwr_mac_procConfigReqEvt,
      lwr_mac_procConfigRspEvt,
      lwr_mac_procStartReqEvt,
      lwr_mac_procInvalidEvt,
   },
   {
      /* PHY_STATE_RUNNING */
#ifdef INTEL_TIMER_MODE
      lwr_mac_procInvalidEvt,
#endif
      lwr_mac_procInvalidEvt,
      lwr_mac_procInvalidEvt,
      lwr_mac_procConfigReqEvt,
      lwr_mac_procConfigRspEvt,
      lwr_mac_procInvalidEvt,
      lwr_mac_procInvalidEvt,
   }
};

/*******************************************************************
 *
 * @brief Sends message to LWR_MAC Fsm Event Handler
 *
 * @details
 *
 *    Function : sendToLowerMac
 *
 *    Functionality:
 *         -Sends message to LowerMac
 *
 * @params[in] Message Type
 *             Message Length
 *             Messaga Pointer
 *
 * @return void
 *
 ******************************************************************/
void sendToLowerMac(uint16_t msgType, uint32_t msgLen, void *msg)
{
   lwrMacCb.event = msgType;
   fapiEvtHdlr[lwrMacCb.phyState][lwrMacCb.event](msg);
}

/**********************************************************************
  End of file
 **********************************************************************/
