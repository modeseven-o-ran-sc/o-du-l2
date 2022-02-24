/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#include "FreqBandInformation.h"

#include "FreqBandInformationEUTRA.h"
#include "FreqBandInformationNR.h"
static asn_oer_constraints_t asn_OER_type_FreqBandInformation_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_FreqBandInformation_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_FreqBandInformation_1[] = {
	{ ATF_POINTER, 0, offsetof(struct FreqBandInformation, choice.bandInformationEUTRA),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_FreqBandInformationEUTRA,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"bandInformationEUTRA"
		},
	{ ATF_POINTER, 0, offsetof(struct FreqBandInformation, choice.bandInformationNR),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_FreqBandInformationNR,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"bandInformationNR"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_FreqBandInformation_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* bandInformationEUTRA */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* bandInformationNR */
};
asn_CHOICE_specifics_t asn_SPC_FreqBandInformation_specs_1 = {
	sizeof(struct FreqBandInformation),
	offsetof(struct FreqBandInformation, _asn_ctx),
	offsetof(struct FreqBandInformation, present),
	sizeof(((struct FreqBandInformation *)0)->present),
	asn_MAP_FreqBandInformation_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_FreqBandInformation = {
	"FreqBandInformation",
	"FreqBandInformation",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_FreqBandInformation_constr_1, &asn_PER_type_FreqBandInformation_constr_1, CHOICE_constraint },
	asn_MBR_FreqBandInformation_1,
	2,	/* Elements count */
	&asn_SPC_FreqBandInformation_specs_1	/* Additional specs */
};

