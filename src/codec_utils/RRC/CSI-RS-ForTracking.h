/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_CSI_RS_ForTracking_H_
#define	_CSI_RS_ForTracking_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CSI-RS-ForTracking */
typedef struct CSI_RS_ForTracking {
	long	 burstLength;
	long	 maxSimultaneousResourceSetsPerCC;
	long	 maxConfiguredResourceSetsPerCC;
	long	 maxConfiguredResourceSetsAllCC;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CSI_RS_ForTracking_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CSI_RS_ForTracking;
extern asn_SEQUENCE_specifics_t asn_SPC_CSI_RS_ForTracking_specs_1;
extern asn_TYPE_member_t asn_MBR_CSI_RS_ForTracking_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _CSI_RS_ForTracking_H_ */
#include <asn_internal.h>
