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

/********************************************************************20**
  
        Name:    LTE RLC file 
    
        Type:    C include file
  
        Desc:    This file contains all the data structures and 
                 prototypes for LTE RLC.
 
        File:    kw.x

*********************************************************************21*/
/** @file kw.x
@brief RLC Product Structures, prototypes
*/

#ifndef __KWX__
#define __KWX__
 
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Local typedefs */
typedef U32    KwSn;   /*!< Sequence Number length */

typedef RguDDatIndInfo KwDatIndInfo;

typedef RguDStaIndInfo KwDStaIndInfo;

typedef RguPduInfo KwPduInfo; /* kw002.201 : Aligning the structure with RGU */

typedef struct _amRlcStats
{
   U32   numDLStaPduSent;
   U32   numDLNacksInStaPdu;
   U32   numDLBytesUnused;
   U32   numDLPollTimerExpiresSrb;
   U32   numDLPollTimerExpiresDrb;
   U32   numDLMaxRetx;
   U32   numDLRetransPdus;
   U32   numULPdusDiscarded;
   U32   numULReOrdTimerExpires;
   U32   numULStaPduRcvd;
   U32   numULNackInStaPduRcvd;
   U32   numRlcAmCellSduTx; /* Count of SDUs transmitted in DL for all UEs */
   U32   numRlcAmCellSduBytesTx; /*Total number of bytes transmitted in DL for all Ues */
   U32   numRlcAmCellRetxPdu; /*Count of PDUs retransmitted for all Ues */
   U32   numRlcAmMaxRetx; /*Total number of Max-RLC retransmissions hit for all the Ues */
   U32   numRlcAmCellDupPduRx; /*Count of Duplicate PDUs detected for a UE in UL for all Ues */
   U32   numRlcAmCellDropOutWinRx; /*Count of PDUs dropped due to Out of Window reception for all Ues */
   U32   numRlcAmCellSduRx; /* Count of SDUs received in UL for all UEs*/
   U32   numRlcAmCellSduBytesRx;/*Total number of bytes received in UL for all Ues*/
   U32   numRlcAmCellNackRx; /*Total number of UL PDUs nacked for all the Ues*/
   U32   numRlcAmCellWinStall; /*Number of window stalls detected for all the Ues */
}AMRLCStats;

typedef struct _umRlcStats
{
   U32   numDLBytesUnused;
   U32   numDLMaxRetx;
   U32   numULPdusDiscarded;
   U32   numULReOrdTimerExpires;
   U32   numULPdusOutsideWindow;
}UMRLCStats;

typedef struct _rlcStats
{
   AMRLCStats   amRlcStats;
   UMRLCStats   umRlcStats;
}RLCStats;

EXTERN RLCStats gRlcStats;

/* kw005.201 added support for L2 Measurement */
#ifdef LTE_L2_MEAS
typedef struct kwSduSnMap KwSduSnMap;
typedef RguLchMapInfo KwLchMapInfo;
#endif /*  LTE_L2_MEAS */


/** @defgroup ummode UM Module Info 
*/
/** 
 * @brief  Structure to hold an Unacknowledged Mode header
 *
 * @details
 *    - fi    : Framing Info
 *    - sn    : Sequence number
 *    - numLi : Number of length indicators in the following array (li)
 *    - li    : Length indicators
*/
typedef struct kwUmHdr
{
   U8     fi;              /*!< Framing Info */
   KwSn   sn;              /*!< Sequence number */
   U16    numLi;           /*!< Number of LIs */
   U16    li[KW_MAX_UL_LI];   /*!< Array of LIs */
}KwUmHdr;

/** 
 * @brief  Structure to hold an Acknowledged Mode header
 *
 * @details
 *    - dc    : Data/Control PDU
 *    - rf    : Resegmentation flag
 *    - p     : Poll bit
 *    - fi    : Framing Info
 *    - e     : Extension bit
 *    - lsf   : Last segment flat
 *    - sn    : Sequence number
 *    - so    : Segment offset
 *    - numLi : Number of length indicators in the following array (li)
 *    - li    : Length indicators
*/
typedef struct kwAmHdr
{
   U8     dc;              /*!< Data/Control PDU */
   U8     p;               /*!< Poll bit */
   U8     si;              /*!< Segmentation Info: 5GNR */ 
   KwSn   sn;              /*!< Sequence number */
   U32    so;              /*!< Segment offset */
}KwAmHdr;

/* structures used for encoding/decoding the headers */
typedef struct kwCntrlInfo
{
   U16  val;
   U8   len;
   U16  idx;
   U8   emtBits;
   U16  e1Idx;
   U16  e2Idx;   
   U8   e1eb;
}KwCntrlInfo;

typedef struct kwHdrInfo
{
   U32  val;
   U8   len;
   U8   eb;
   U8   *hdr;
   U16  idx;
   U8   pEb;
   U8   pLen;
}KwHdrInfo;

typedef struct kwExtHdr
{
   U32 val;
   U16 len;
   U8  hdr;
   U8  pLen;
}KwExtHdr;

/** 
 * @brief  Structure to hold information about a Logical channel
 *
 * @details
 *    - lChId    : Logical channel Id
 *    - lChType  : Logical channel type 
*/ 
typedef struct kwLchInfo
{
   CmLteLcId     lChId;     /*!< Logical channel Id */
   CmLteLcType   lChType;   /*!< Logical channel type */
}KwLchInfo;

/* kw005.201 added support for L2 Measurement */
#ifdef LTE_L2_MEAS

/** @struct KwL2Cntr
 * RLC L2 Counter  */
typedef struct kwL2Cntr
{
   struct
   {
      U32  numActvUe;        /*!< number of active Ue */
      U32  sampOc;           /*!< Total number of sampling occasion */
   }actUe;
   struct
   {
      U32  dLoss;            /*!< Total number of lost packets */  
      U32  posPkts;          /*!< Total number of positively acknowlegded 
                                  packets */
   }uuLoss;
   struct                    /*!< For DL IP throughput */
   {
      U32 volSummation;      /*!< Sum of data in bytes */
      U32 timeSummation;     /*!< Sum of time difference in milli sec*/
   }dlIpThruput;
   struct                    /*!< For UL IP throughput */
   {
      U32 volSummation;      /*!< Sum of data in bytes */
      U32 timeSummation;     /*!< Sum of time difference in milli sec*/
   }ulIpThruput;
   /* Discard new changes starts */
   struct                    /*!< For UL IP throughput */
   {
      U32 discSdus;          /*!< Total RLC SDUs discarded */
      U32 totSdus;           /*!< Total RLC SDUs received */
   }dlDisc;
   struct                    /*!< For UL IP throughput */
   {
      U64 sduDelay;          /*!< Total SDUs delay */
      U32 numSdus;
   }dlPjSduDelay;
   U32    totDrbsPerQci;     /*!< Total Count of DRB's for this QCI */
}KwL2Cntr;

struct kwSduSnMap
{
   CmLList     lstEnt;
   Bool        failMarked;
   Bool        fullySent;
   U32         sduId;
   U16         numSn;
   U16         snList[KW_MAX_PDU_MAP];
   U16         harqAck;
   U16         reqSent;
   U16         rspRcvd;
};

typedef struct kwSnSduMap
{
   U16         sn;
   CmLteLcId   lChId;              /*!< Logical channel Id */
   U16         numSdu;
#ifdef LTE_RLC_R9
   Bool        isBurstSplitted;    /*!< true: burst for this LCH is splitted */
#endif /* LTE_RLC_R9 */
   KwSduSnMap  *sduList[KW_MAX_DL_LI];
}KwSnSduMap;

typedef struct kwTbSnMap
{
   CmHashListEnt  hlTbEnt;
   U32            tbId;
   U16            prevNumSn;
   U16            numSn;
   KwSnSduMap     snSduMap[RGU_MAX_PDU * RGU_MAX_LC];
}KwTbSnMap;

typedef struct kwL2MeasCbUeMeasInfo
{
   CmLteRnti   ueId;                    /*!< UE ID (Used only for IP Throughput
                                             in UL/DL */
   CmLteCellId cellId;                  /*!< UE ID (Used only for IP Throughput
                                             in UL/DL */
   Bool        isValid;                 /*! < is this UE entry valid */
   U8          numLcId;                 /*!< Holds the number of LCh for which Ul Ip
                                             measurement is ON */
   U8          lcId[KW_MAX_LCH_PER_UE]; /*!< Holds the list of LCh for which Ul ip
                                             measurement is ON */
   KwL2Cntr    measData[LKW_MAX_QCI];
   U16         numQci;                  /*!< number of valid qcI */
   U8          qci[LKW_MAX_QCI];        /*!< list of valid qcI */
}KwL2MeasCbUeMeasInfo;

typedef struct kwL2MeasCbIpThMeas
{
   U8                   numUes;
   U8                   totNumQci;
   U8                   totQci[LKW_MAX_QCI];
   KwL2MeasCbUeMeasInfo ueInfoLst[LKW_MAX_UE]; /*Added for handling meas for multiple ues*/ 
}KwL2MeasCbIpThMeas;

typedef struct kwL2MeasCbNonIpThMeas
{
   U16        numSamples;              /*!< Number of samples to take on numActUe */
   U16        numQci;                  /*!< number of valid qcI */
   U8         qci[LKW_MAX_QCI];        /*!< list of valid qcI */
   KwL2Cntr   measData[LKW_MAX_QCI];   /*!< Measurement CB */
}KwL2MeasCbNonIpThMeas;

typedef union kwL2MeasCbIpNonIpThMeasVal
{
   KwL2MeasCbIpThMeas    ipThMeas;
   KwL2MeasCbNonIpThMeas nonIpThMeas;
}KwL2MeasCbIpNonIpThMeasVal;

/** @struct KwL2MeasCb
 * RLC L2 Measurement CB */
typedef struct kwL2MeasCb
{
   U8               measType;        /*!< Bit-wise set measurement types */
   KwL2MeasCbIpNonIpThMeasVal val;   /* Union of IP tpt or non-ip tpt */
}KwL2MeasCb;

/** @struct KwL2MeasEvtCb
 * RLC L2 Measurement Evt CB */
typedef struct kwL2MeasEvtCb
{
   U32           transId;                /*!< TransId of Measurement Req */
   U32           cbIdx;                  /*!< TransId of Measurement Req */
   CmTimer       l2Tmr; /* NOT USED */                 /*!< L2 Timer per request */
   TmrCfg        l2TmrCfg; /* NOT USED */               /*!< Time period of measurement */
   KwL2MeasCb    measCb;                 /*!< Measurement CB */ 
   EpcTime       startTime; /* NOT USED */            /*!<  start time when meas starts*/ 
}KwL2MeasEvtCb;

/** @struct KwL2MeasRbCb
 * RLC L2 Measurement Rb CB */
typedef struct kwL2MeasRbCb
{
   U8            measOn;                      /*!< Measurements that are running */ 
   KwL2Cntr      *l2Sts[KW_MAX_L2MEAS_EVT];  /*!< L2 Mesurement statistics */     
}KwL2MeasRbCb;

/** @struct KwL2Cb
 * RLC L2  CB */
typedef struct kwL2Cb
{
   U16            kwNumMeas;                   /*!< Number of measurements going on */
   KwL2MeasEvtCb  kwL2EvtCb[LKW_MAX_L2MEAS];  /*!< Pointers to Measurement Cb */
   U8             measOn[LKW_MAX_QCI];          /*!< Measurement on */
   U32            numActUe[LKW_MAX_QCI];       /*!< Measurement on */
}KwL2Cb;


typedef enum _dlIpThrputState
{
   KW_DL_IPTHRU_RESET = 0,
   KW_DL_IPTHRU_BURST_STARTED,
   KW_DL_IPTHRU_BURST_CONTINUE,
   KW_DL_IPTHRU_BURST_COMPLETED
}DlIpThrputState;

/** 
* @struct kwL2MeasSduLst
* Structure to hold parameters of 
* burst sdus in DL for a RB */
typedef struct kwOutStngSduInfo
{
   U32       sduId;            /*!< SDU Id of sdu */
   MsgLen    sduLen;           /*!< Size of sdu */
   U32       numTb;            /*!< Hold the number of TBs for this sdu in DL */
}KwOutStngSduInfo;

/** 
* @struct kwL2MeasDlIpTh
* Structure to hold parameters for DL ip 
* throughput for a RB */
typedef struct kwL2MeasDlIpTh
{
   Bool            isBurstAct;            /*!< Set to TRUE when burst is active in DL */
   U64             burstStartTime;        /*!< Holds the starting time of the burst */
   U32             burstEndSduId;         /*!< Sdu ID when burst ends */
   U8              lastSduIdx;            /*!< Holds the index of last outStanding sdu */
   KwOutStngSduInfo  outStngSduArr[KW_L2MEAS_MAX_OUTSTNGSDU];/*!< Hold the burst sdu information */
}KwL2MeasDlIpTh;

/** 
* @struct kwL2MeasIpThruput
* Structure to hold parameters for UL/DL ip 
* throughput for a RB */
typedef struct kwL2MeasIpThruput
{
   U32             dataVol;                 /*!< Holds volume of new data in bytes
                                              for UL IP throughput */
   U32             ttiCnt;                  /*!< Holds ttiCnt received from MAC in UL */
   U32             prevTtiCnt;        /*!< Holds previous ttiCnt received from MAC in UL */
   KwL2MeasDlIpTh  dlIpTh;
}KwL2MeasIpThruput;

#endif /* LTE_L2_MEAS */

/** 
 * @brief  Structure to hold an UE key for the UE hast lists
 *
 * @details
 *    - ueId    : UE Id
 *    - cellId  : Cell Id 
*/
typedef struct kwUeKey
{
   CmLteRnti     ueId;     /*!< UE Id */
   CmLteCellId   cellId;   /*!< Cell Id */
}KwUeKey;

/** 
 * @brief  Structure to hold an information about the CKW SAP
 *
 * @details
 *    - pst   : Service user post structure
 *    - spId  : Service provider Id
 *    - suId  : Service user Id
 *    - state : State of the SAP
 *    - sts   : SAP specific statistics 
*/
typedef struct kwCkwSapCb
{
   Pst           pst;     /*!< Service user post structure */
   SpId          spId;    /*!< Service provider Id */
   SuId          suId;    /*!< Service user Id */
   U8            state;   /*!< Sap Status */
   KwCkwCntSts   sts;     /*!< Statistics */
}KwCkwSapCb;

/** 
 * @brief  Structure to hold an information about the KWU SAP
 *
 * @details
 *    - pst   : Service user post structure
 *    - spId  : Service provider Id
 *    - suId  : Service user Id
 *    - state : State of the SAP
 *    - sts   : SAP specific statistics 
*/
typedef struct kwKwuSapCb
{
   Pst           pst;     /*!< Service user post structure */
   SpId          spId;    /*!< Service provider Id */
   SuId          suId;    /*!< Service user Id */
   U8            state;   /*!< Sap Status */
   KwKwuSapSts   sts;     /*!< Statistics */
}KwKwuSapCb;

/** 
 * @brief  Structure to hold an information about the RGU SAP
 *
 * @details
 *    - pst       : Service user post structure
 *    - spId      : Service provider Id
 *    - suId      : Service user Id
 *    - state     : State of the SAP
 *    - bndTmr    : Bind Timer
 *    - bndTmrInt : Timer Interval
 *    - retryCnt  : Bind Retry Count
*/
typedef struct kwRguSapCb
{
   Pst       pst;         /*!< Service user post structure */
   SpId      spId;        /*!< Service provider Id */
   SuId      suId;        /*!< Service user Id */
   U8        state;       /*!< Sap Status */
   CmTimer   bndTmr;      /*!< Bind Timer */
   U16       bndTmrInt;   /*!< Timer Interval */
   U8        retryCnt;    /*!< Bind Retry Count */
}KwRguSapCb;

/** 
 * @brief  Structure to hold an information about the UDX UL SAP
 *
 * @details
 *    - pst       : Service user post structure
 *    - spId      : Service provider Id
 *    - suId      : Service user Id
 *    - state     : State of the SAP
 *    - bndTmr    : Bind Timer
 *    - bndTmrInt : Timer Interval
 *    - retryCnt  : Bind Retry Count
*/
typedef struct kwUdxUlSapCb
{
   Pst       pst;         /*!< Service user post structure */
   SpId      spId;        /*!< Service provider Id */
   SuId      suId;        /*!< Service user Id */
   U8        state;       /*!< Sap Status */
   CmTimer   bndTmr;      /*!< Bind Timer */
   U16       bndTmrInt;   /*!< Timer Interval */
   U8        retryCnt;    /*!< Bind Retry Count */
}KwUdxUlSapCb;

/** 
 * @brief  Structure to hold an information about the UDX DL SAP
 *
 * @details
 *    - pst       : Service user post structure
 *    - spId      : Service provider Id
 *    - suId      : Service user Id
 *    - state     : State of the SAP
*/
typedef struct kwUdxDlSapCb
{
   Pst    pst;     /*!< Service user post structure */
   SpId   spId;    /*!< Service provider Id */
   SuId   suId;    /*!< Service user Id */
   U8     state;   /*!< Sap Status */
}KwUdxDlSapCb;

/** 
 * @brief  Structure to hold info about memory to be freed
 *
 * @details
 *    - sduLst  : The SDU queues are appended to this queue, used 
 *                for the UM SDU queues
 *    - txLst   : Stores to be released AM Mode TX PDUs
 *    - reTxLst : Stores to be released AM Re TX PDU's
 *    - rbLst   : List of AM DL RBs to be freed 
*/
typedef struct kwDlDataToBeFreed
{
   CmLListCp   sduLst;     /*!< Queue of SDU's to be freed  */
   CmLListCp   txLst;     /*!< Stores to be released TX PDUs */
   CmLListCp   reTxLst;   /*!< Stores to be released ReTX PDUs */
   CmLListCp   rbLst;     /*!< List of AM DL RBs to be freed */
}KwDlDataToBeFreed;

/** 
 * @brief  Structure to hold an information about DL RLC instance
 *
 * @details
 *    - numKwuSaps        : Number of RLC KWU Saps
 *    - numUdxSaps        : Number of RLC UDX Saps
 *    - kwuDlSap          : Pointer to the array of KWU SAPS
 *    - udxDlSap          : Pointer to the array of UDX SAPS
 *    - rguDlSap          : RGU Sap Control Block
 *    - cellLstCp         : Hashlist of CellCb
 *    - ueLstCp           : Hashlist of UeCb 
 *    - toBeFreed         : Pointer to data to be freed
 *    - shutdownReveived  : Request for shutdown recevied or not
 *    - eventInQueue      : Event for cleanup exists in queue or not
 */
typedef struct _kwDlCb
{
   U8                  numKwuSaps;         /*!< Number of RLC Data Saps */
   U8                  numUdxSaps;         /*!< Number of RLC Data Saps */
   KwKwuSapCb          *kwuDlSap;          /*!< KWU Sap Control Block */
   KwUdxDlSapCb        *udxDlSap;          /*!< UDX DL Sap Control Block */
   KwRguSapCb          *rguDlSap;          /*!< RGU Sap Control Block */
   CmHashListCp        cellLstCp;          /*!< Hashlist of CellCb */
   CmHashListCp        ueLstCp;            /*!< Hashlist of UeCb */
   KwDlDataToBeFreed   toBeFreed;          /*!< Pointer to data to be freed */        
   Pst                 selfPst;            /*!< Pst to post events to self */
   Buffer              *selfPstMBuf;       /*!< Buffer used for self post */
   Bool                shutdownReceived;   /*!< Request for shutdown recevied */
   Bool                eventInQueue;       /*!< Event exists in queue or not */
#ifdef LTE_L2_MEAS
   KwL2Cb              kwL2Cb; /*!< Control Block for L2 Measurements in RLC */
#endif /* LTE_L2_MEAS */
}KwDlCb;

/** 
 * @brief  Structure to hold an information about UL RLC instance
 *
 * @details
 *    - ckwSap       : CKW Sap Conrol Block
 *    - numKwuSaps   : Number of RLC KWU Saps
 *    - numUdxSaps   : Number of RLC UDX Saps
 *    - udxUlSap     : Pointer to the array of UDX SAPS 
 *    - kwuUlSap     : Pointer to the array of KWU SAPS
 *    - rguUlSap     : RGU Sap Control Block
 *    - cellLstCp    : Hashlist of CellCb
 *    - ueLstCp      : Hashlist of UeCb 
 *    - transIdLstCp : Hashlist of cfg trans
 */
typedef struct _kwUlCb
{
   KwCkwSapCb     ckwSap;         /*!< CKW Sap Conrol Block */ 
   U8             numKwuSaps;     /*!< Number of RLC Data Saps */
   U8             numUdxSaps;     /*!< Number of RLC Data Saps */
   KwUdxUlSapCb   *udxUlSap;      /*!< UDX DL Sap Control Block */
   KwKwuSapCb     *kwuUlSap;      /*!< KWU Sap Control Block */
   KwRguSapCb     *rguUlSap;      /*!< RGU Sap Control Block */
   CmHashListCp   cellLstCp;      /*!< Hashlist of CellCb */
   CmHashListCp   ueLstCp;        /*!< Hashlist of UeCb */
   CmHashListCp   transIdLstCp;   /*!< Hashlist of cfg trans */
/* kw005.201 added support for L2 Measurement */
#ifdef LTE_L2_MEAS
   KwL2Cb        kwL2Cb; /*!< Control Block for L2 Measurements in RLC */
#endif /* LTE_L2_MEAS */
}KwUlCb;


/** 
 * @brief  Structure to hold an information about a RLC instance
 *
 * @details
 *    - init    : Task Initialization Info
 *    - genCfg  : General Configuration
 *    - genSts  : General Statistics
 *    - trcLen  : Trace Length
 *    - trcMask : Trace Mask
 *    - kwTq    : Timer queue
 *    - kwTqCp  : Timer queue control point
 *    - u       : Union depending on whether the instance is UL or DL
 *      - ulCb  : UL instance Control Block
 *      - dlCb  : DL instance Control Block
 */
typedef struct _kwCb
{
   TskInit    init;               /*!< Task Initialization Info */
   KwGenCfg   genCfg;             /*!< General Configuration Structure */
   KwGenSts   genSts;             /*!< General Statistics */
   S16        trcLen;             /*!< Trace Length */
   U8         trcMask;            /*!< Trace Mask */
   CmTqType   kwTq[KW_TMR_LEN];   /*!< Timer queue */
   CmTqCp     kwTqCp;             /*!< Timer queue control point */
   union 
   {
      KwUlCb   *ulCb;   /*!< Ul Control Block */
      KwDlCb   *dlCb;   /*!< Dl Control Block */
   } u;
}KwCb;

EXTERN KwCb *kwCb[KW_MAX_RLC_INSTANCES];   /*!< RLC global control block */

/****************************************************************************
 *                      EXTERN Declarations
 ***************************************************************************/
EXTERN S16 kwGetSId ARGS((SystemId *s));

EXTERN Void kwTmrExpiry ARGS((PTR cb, S16 tmrEvnt));

EXTERN S16 kwLmmSendTrc ARGS ((KwCb *gCb, Event event, Buffer *mBuf));

EXTERN Void kwStartTmr ARGS((KwCb *gCb, PTR cb, S16 tmrEvnt));

EXTERN Void kwStopTmr  ARGS((KwCb *gCb, PTR cb, U8 tmrType));

EXTERN Bool kwChkTmr ARGS((KwCb *gCb,PTR cb, S16 tmrEvnt));

#ifdef LTE_L2_MEAS
EXTERN Void kwLmmSendAlarm ARGS (( KwCb *gCb,
                                   U16 category, 
                                   U16 event, 
                                   U16 cause, 
                                   SuId suId, 
                                   U32 ueId, 
                                   U8 qci));

EXTERN S16 KwMiLkwDlL2MeasReq ARGS (( Pst *pst, KwL2MeasReqEvt *measReqEvt ));
EXTERN S16 KwMiLkwDlL2MeasSendReq ARGS((Pst *pst,U8 measType));
EXTERN S16 KwMiLkwDlL2MeasStopReq ARGS((Pst *pst,U8 measType));
EXTERN S16 KwMiLkwUlL2MeasReq ARGS (( Pst *pst, KwL2MeasReqEvt *measReqEvt ));
EXTERN S16 KwMiLkwUlL2MeasSendReq ARGS((Pst *pst,U8 measType));
EXTERN S16 KwMiLkwUlL2MeasStopReq ARGS((Pst *pst,U8 measType));
EXTERN Void kwUtlPlcMeasDatInL2Sts ARGS((KwL2Cntr *measData, 
                                         KwL2MeasRbCb *rbL2Cb,
                                         U8 measType));
#else /* LTE_L2_MEAS */
EXTERN Void kwLmmSendAlarm ARGS ((KwCb *gCb,
                                  U16 category, 
                                  U16 event, 
                                  U16 cause, 
                                  SuId suId, 
                                  U32 ueId));
#endif /* LTE_L2_MEAS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KWX__ */

  
/********************************************************************30**
  
         End of file
**********************************************************************/
