/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#ifndef	_CSI_ReportPeriodicityAndOffset_H_
#define	_CSI_ReportPeriodicityAndOffset_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CSI_ReportPeriodicityAndOffset_PR {
	CSI_ReportPeriodicityAndOffset_PR_NOTHING,	/* No components present */
	CSI_ReportPeriodicityAndOffset_PR_slots4,
	CSI_ReportPeriodicityAndOffset_PR_slots5,
	CSI_ReportPeriodicityAndOffset_PR_slots8,
	CSI_ReportPeriodicityAndOffset_PR_slots10,
	CSI_ReportPeriodicityAndOffset_PR_slots16,
	CSI_ReportPeriodicityAndOffset_PR_slots20,
	CSI_ReportPeriodicityAndOffset_PR_slots40,
	CSI_ReportPeriodicityAndOffset_PR_slots80,
	CSI_ReportPeriodicityAndOffset_PR_slots160,
	CSI_ReportPeriodicityAndOffset_PR_slots320
} CSI_ReportPeriodicityAndOffset_PR;

/* CSI-ReportPeriodicityAndOffset */
typedef struct CSI_ReportPeriodicityAndOffset {
	CSI_ReportPeriodicityAndOffset_PR present;
	union CSI_ReportPeriodicityAndOffset_u {
		long	 slots4;
		long	 slots5;
		long	 slots8;
		long	 slots10;
		long	 slots16;
		long	 slots20;
		long	 slots40;
		long	 slots80;
		long	 slots160;
		long	 slots320;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CSI_ReportPeriodicityAndOffset_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CSI_ReportPeriodicityAndOffset;
extern asn_CHOICE_specifics_t asn_SPC_CSI_ReportPeriodicityAndOffset_specs_1;
extern asn_TYPE_member_t asn_MBR_CSI_ReportPeriodicityAndOffset_1[10];
extern asn_per_constraints_t asn_PER_type_CSI_ReportPeriodicityAndOffset_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _CSI_ReportPeriodicityAndOffset_H_ */
#include <asn_internal.h>
