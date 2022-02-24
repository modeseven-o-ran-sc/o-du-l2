/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_RRCResumeRequest_H_
#define	_RRCResumeRequest_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RRCResumeRequest-IEs.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RRCResumeRequest */
typedef struct RRCResumeRequest {
	RRCResumeRequest_IEs_t	 rrcResumeRequest;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RRCResumeRequest_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RRCResumeRequest;
extern asn_SEQUENCE_specifics_t asn_SPC_RRCResumeRequest_specs_1;
extern asn_TYPE_member_t asn_MBR_RRCResumeRequest_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _RRCResumeRequest_H_ */
#include <asn_internal.h>
