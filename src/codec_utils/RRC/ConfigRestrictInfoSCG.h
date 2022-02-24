/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-InterNodeDefinitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_ConfigRestrictInfoSCG_H_
#define	_ConfigRestrictInfoSCG_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include "P-Max.h"
#include <constr_SEQUENCE.h>
#include "ServCellIndex.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct BandCombinationInfoList;

/* ConfigRestrictInfoSCG */
typedef struct ConfigRestrictInfoSCG {
	struct BandCombinationInfoList	*allowedBC_ListMRDC;	/* OPTIONAL */
	struct ConfigRestrictInfoSCG__powerCoordination_FR1 {
		P_Max_t	*p_maxNR_FR1;	/* OPTIONAL */
		P_Max_t	*p_maxEUTRA;	/* OPTIONAL */
		P_Max_t	*p_maxUE_FR1;	/* OPTIONAL */
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *powerCoordination_FR1;
	struct ConfigRestrictInfoSCG__servCellIndexRangeSCG {
		ServCellIndex_t	 lowBound;
		ServCellIndex_t	 upBound;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *servCellIndexRangeSCG;
	long	*maxMeasFreqsSCG_NR;	/* OPTIONAL */
	long	*maxMeasIdentitiesSCG_NR;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ConfigRestrictInfoSCG_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ConfigRestrictInfoSCG;
extern asn_SEQUENCE_specifics_t asn_SPC_ConfigRestrictInfoSCG_specs_1;
extern asn_TYPE_member_t asn_MBR_ConfigRestrictInfoSCG_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _ConfigRestrictInfoSCG_H_ */
#include <asn_internal.h>
