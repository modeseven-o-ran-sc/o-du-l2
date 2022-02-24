/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#include "RA-Prioritization.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static asn_oer_constraints_t asn_OER_type_powerRampingStepHighPriority_constr_2 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_powerRampingStepHighPriority_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 2,  2,  0,  3 }	/* (0..3) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_type_scalingFactorBI_constr_7 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_scalingFactorBI_constr_7 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 2,  2,  0,  3 }	/* (0..3) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_powerRampingStepHighPriority_value2enum_2[] = {
	{ 0,	3,	"dB0" },
	{ 1,	3,	"dB2" },
	{ 2,	3,	"dB4" },
	{ 3,	3,	"dB6" }
};
static const unsigned int asn_MAP_powerRampingStepHighPriority_enum2value_2[] = {
	0,	/* dB0(0) */
	1,	/* dB2(1) */
	2,	/* dB4(2) */
	3	/* dB6(3) */
};
static const asn_INTEGER_specifics_t asn_SPC_powerRampingStepHighPriority_specs_2 = {
	asn_MAP_powerRampingStepHighPriority_value2enum_2,	/* "tag" => N; sorted by tag */
	asn_MAP_powerRampingStepHighPriority_enum2value_2,	/* N => "tag"; sorted by N */
	4,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_powerRampingStepHighPriority_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_powerRampingStepHighPriority_2 = {
	"powerRampingStepHighPriority",
	"powerRampingStepHighPriority",
	&asn_OP_NativeEnumerated,
	asn_DEF_powerRampingStepHighPriority_tags_2,
	sizeof(asn_DEF_powerRampingStepHighPriority_tags_2)
		/sizeof(asn_DEF_powerRampingStepHighPriority_tags_2[0]) - 1, /* 1 */
	asn_DEF_powerRampingStepHighPriority_tags_2,	/* Same as above */
	sizeof(asn_DEF_powerRampingStepHighPriority_tags_2)
		/sizeof(asn_DEF_powerRampingStepHighPriority_tags_2[0]), /* 2 */
	{ &asn_OER_type_powerRampingStepHighPriority_constr_2, &asn_PER_type_powerRampingStepHighPriority_constr_2, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_powerRampingStepHighPriority_specs_2	/* Additional specs */
};

static const asn_INTEGER_enum_map_t asn_MAP_scalingFactorBI_value2enum_7[] = {
	{ 0,	4,	"zero" },
	{ 1,	5,	"dot25" },
	{ 2,	4,	"dot5" },
	{ 3,	5,	"dot75" }
};
static const unsigned int asn_MAP_scalingFactorBI_enum2value_7[] = {
	1,	/* dot25(1) */
	2,	/* dot5(2) */
	3,	/* dot75(3) */
	0	/* zero(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_scalingFactorBI_specs_7 = {
	asn_MAP_scalingFactorBI_value2enum_7,	/* "tag" => N; sorted by tag */
	asn_MAP_scalingFactorBI_enum2value_7,	/* N => "tag"; sorted by N */
	4,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_scalingFactorBI_tags_7[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_scalingFactorBI_7 = {
	"scalingFactorBI",
	"scalingFactorBI",
	&asn_OP_NativeEnumerated,
	asn_DEF_scalingFactorBI_tags_7,
	sizeof(asn_DEF_scalingFactorBI_tags_7)
		/sizeof(asn_DEF_scalingFactorBI_tags_7[0]) - 1, /* 1 */
	asn_DEF_scalingFactorBI_tags_7,	/* Same as above */
	sizeof(asn_DEF_scalingFactorBI_tags_7)
		/sizeof(asn_DEF_scalingFactorBI_tags_7[0]), /* 2 */
	{ &asn_OER_type_scalingFactorBI_constr_7, &asn_PER_type_scalingFactorBI_constr_7, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_scalingFactorBI_specs_7	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_RA_Prioritization_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct RA_Prioritization, powerRampingStepHighPriority),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_powerRampingStepHighPriority_2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"powerRampingStepHighPriority"
		},
	{ ATF_POINTER, 1, offsetof(struct RA_Prioritization, scalingFactorBI),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_scalingFactorBI_7,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"scalingFactorBI"
		},
};
static const int asn_MAP_RA_Prioritization_oms_1[] = { 1 };
static const ber_tlv_tag_t asn_DEF_RA_Prioritization_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_RA_Prioritization_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* powerRampingStepHighPriority */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* scalingFactorBI */
};
asn_SEQUENCE_specifics_t asn_SPC_RA_Prioritization_specs_1 = {
	sizeof(struct RA_Prioritization),
	offsetof(struct RA_Prioritization, _asn_ctx),
	asn_MAP_RA_Prioritization_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_RA_Prioritization_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_RA_Prioritization = {
	"RA-Prioritization",
	"RA-Prioritization",
	&asn_OP_SEQUENCE,
	asn_DEF_RA_Prioritization_tags_1,
	sizeof(asn_DEF_RA_Prioritization_tags_1)
		/sizeof(asn_DEF_RA_Prioritization_tags_1[0]), /* 1 */
	asn_DEF_RA_Prioritization_tags_1,	/* Same as above */
	sizeof(asn_DEF_RA_Prioritization_tags_1)
		/sizeof(asn_DEF_RA_Prioritization_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_RA_Prioritization_1,
	2,	/* Elements count */
	&asn_SPC_RA_Prioritization_specs_1	/* Additional specs */
};

