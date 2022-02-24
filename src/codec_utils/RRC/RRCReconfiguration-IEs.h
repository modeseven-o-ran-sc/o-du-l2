/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_RRCReconfiguration_IEs_H_
#define	_RRCReconfiguration_IEs_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct RadioBearerConfig;
struct MeasConfigRrc;
struct RRCReconfiguration_v1530_IEs;

/* RRCReconfiguration-IEs */
typedef struct RRCReconfiguration_IEs {
	struct RadioBearerConfig	*radioBearerConfig;	/* OPTIONAL */
	OCTET_STRING_t	*secondaryCellGroup;	/* OPTIONAL */
	struct MeasConfigRrc	*measConfig;	/* OPTIONAL */
	OCTET_STRING_t	*lateNonCriticalExtension;	/* OPTIONAL */
	struct RRCReconfiguration_v1530_IEs	*nonCriticalExtension;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RRCReconfiguration_IEs_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RRCReconfiguration_IEs;
extern asn_SEQUENCE_specifics_t asn_SPC_RRCReconfiguration_IEs_specs_1;
extern asn_TYPE_member_t asn_MBR_RRCReconfiguration_IEs_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _RRCReconfiguration_IEs_H_ */
#include <asn_internal.h>
