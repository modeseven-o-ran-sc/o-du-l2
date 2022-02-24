/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_RadioBearerConfig_H_
#define	_RadioBearerConfig_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RadioBearerConfig__srb3_ToRelease {
	RadioBearerConfig__srb3_ToRelease_true	= 0
} e_RadioBearerConfig__srb3_ToRelease;

/* Forward declarations */
struct SRB_ToAddModList;
struct DRB_ToAddModList;
struct DRB_ToReleaseList;
struct SecurityConfig;

/* RadioBearerConfig */
typedef struct RadioBearerConfig {
	struct SRB_ToAddModList	*srb_ToAddModList;	/* OPTIONAL */
	long	*srb3_ToRelease;	/* OPTIONAL */
	struct DRB_ToAddModList	*drb_ToAddModList;	/* OPTIONAL */
	struct DRB_ToReleaseList	*drb_ToReleaseList;	/* OPTIONAL */
	struct SecurityConfig	*securityConfig;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RadioBearerConfig_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_srb3_ToRelease_3;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_RadioBearerConfig;
extern asn_SEQUENCE_specifics_t asn_SPC_RadioBearerConfig_specs_1;
extern asn_TYPE_member_t asn_MBR_RadioBearerConfig_1[5];

#ifdef __cplusplus
}
#endif

#endif	/* _RadioBearerConfig_H_ */
#include <asn_internal.h>
