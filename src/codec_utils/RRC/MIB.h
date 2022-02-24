/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_MIB_H_
#define	_MIB_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include <NativeEnumerated.h>
#include <NativeInteger.h>
#include "PDCCH-ConfigSIB1.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum MIB__subCarrierSpacingCommon {
	MIB__subCarrierSpacingCommon_scs15or60	= 0,
	MIB__subCarrierSpacingCommon_scs30or120	= 1
} e_MIB__subCarrierSpacingCommon;
typedef enum MIB__dmrs_TypeA_Position {
	MIB__dmrs_TypeA_Position_pos2	= 0,
	MIB__dmrs_TypeA_Position_pos3	= 1
} e_MIB__dmrs_TypeA_Position;
typedef enum MIB__cellBarred {
	MIB__cellBarred_barred	= 0,
	MIB__cellBarred_notBarred	= 1
} e_MIB__cellBarred;
typedef enum MIB__intraFreqReselection {
	MIB__intraFreqReselection_allowed	= 0,
	MIB__intraFreqReselection_notAllowed	= 1
} e_MIB__intraFreqReselection;

/* MIB */
typedef struct MIB {
	BIT_STRING_t	 systemFrameNumber;
	long	 subCarrierSpacingCommon;
	long	 ssb_SubcarrierOffset;
	long	 dmrs_TypeA_Position;
	PDCCH_ConfigSIB1_t	 pdcch_ConfigSIB1;
	long	 cellBarred;
	long	 intraFreqReselection;
	BIT_STRING_t	 spare;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MIB_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_subCarrierSpacingCommon_3;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_dmrs_TypeA_Position_7;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_cellBarred_11;	// (Use -fall-defs-global to expose) */
/* extern asn_TYPE_descriptor_t asn_DEF_intraFreqReselection_14;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_MIB;
extern asn_SEQUENCE_specifics_t asn_SPC_MIB_specs_1;
extern asn_TYPE_member_t asn_MBR_MIB_1[8];

#ifdef __cplusplus
}
#endif

#endif	/* _MIB_H_ */
#include <asn_internal.h>
