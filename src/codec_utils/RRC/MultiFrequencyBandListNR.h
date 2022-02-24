/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_MultiFrequencyBandListNR_H_
#define	_MultiFrequencyBandListNR_H_


#include <asn_application.h>

/* Including external dependencies */
#include "FreqBandIndicatorNR.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MultiFrequencyBandListNR */
typedef struct MultiFrequencyBandListNR {
	A_SEQUENCE_OF(FreqBandIndicatorNR_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MultiFrequencyBandListNR_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MultiFrequencyBandListNR;
extern asn_SET_OF_specifics_t asn_SPC_MultiFrequencyBandListNR_specs_1;
extern asn_TYPE_member_t asn_MBR_MultiFrequencyBandListNR_1[1];
extern asn_per_constraints_t asn_PER_type_MultiFrequencyBandListNR_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _MultiFrequencyBandListNR_H_ */
#include <asn_internal.h>
