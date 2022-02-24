/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#include "DownlinkPreemption.h"

#include "INT-ConfigurationPerServingCell.h"
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_dci_PayloadSize_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 126)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_int_ConfigurationPerServingCell_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	size_t size;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	/* Determine the number of elements */
	size = _A_CSEQUENCE_FROM_VOID(sptr)->count;
	
	if((size >= 1 && size <= 32)) {
		/* Perform validation of the inner elements */
		return td->encoding_constraints.general_constraints(td, sptr, ctfailcb, app_key);
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_oer_constraints_t asn_OER_type_timeFrequencySet_constr_3 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_timeFrequencySet_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_type_int_ConfigurationPerServingCell_constr_7 CC_NOTUSED = {
	{ 0, 0 },
	-1	/* (SIZE(1..32)) */};
static asn_per_constraints_t asn_PER_type_int_ConfigurationPerServingCell_constr_7 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 5,  5,  1,  32 }	/* (SIZE(1..32)) */,
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_dci_PayloadSize_constr_6 CC_NOTUSED = {
	{ 1, 1 }	/* (0..126) */,
	-1};
static asn_per_constraints_t asn_PER_memb_dci_PayloadSize_constr_6 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 7,  7,  0,  126 }	/* (0..126) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_int_ConfigurationPerServingCell_constr_7 CC_NOTUSED = {
	{ 0, 0 },
	-1	/* (SIZE(1..32)) */};
static asn_per_constraints_t asn_PER_memb_int_ConfigurationPerServingCell_constr_7 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 5,  5,  1,  32 }	/* (SIZE(1..32)) */,
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_timeFrequencySet_value2enum_3[] = {
	{ 0,	4,	"set0" },
	{ 1,	4,	"set1" }
};
static const unsigned int asn_MAP_timeFrequencySet_enum2value_3[] = {
	0,	/* set0(0) */
	1	/* set1(1) */
};
static const asn_INTEGER_specifics_t asn_SPC_timeFrequencySet_specs_3 = {
	asn_MAP_timeFrequencySet_value2enum_3,	/* "tag" => N; sorted by tag */
	asn_MAP_timeFrequencySet_enum2value_3,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_timeFrequencySet_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_timeFrequencySet_3 = {
	"timeFrequencySet",
	"timeFrequencySet",
	&asn_OP_NativeEnumerated,
	asn_DEF_timeFrequencySet_tags_3,
	sizeof(asn_DEF_timeFrequencySet_tags_3)
		/sizeof(asn_DEF_timeFrequencySet_tags_3[0]) - 1, /* 1 */
	asn_DEF_timeFrequencySet_tags_3,	/* Same as above */
	sizeof(asn_DEF_timeFrequencySet_tags_3)
		/sizeof(asn_DEF_timeFrequencySet_tags_3[0]), /* 2 */
	{ &asn_OER_type_timeFrequencySet_constr_3, &asn_PER_type_timeFrequencySet_constr_3, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_timeFrequencySet_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_int_ConfigurationPerServingCell_7[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_INT_ConfigurationPerServingCell,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_int_ConfigurationPerServingCell_tags_7[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_int_ConfigurationPerServingCell_specs_7 = {
	sizeof(struct DownlinkPreemption__int_ConfigurationPerServingCell),
	offsetof(struct DownlinkPreemption__int_ConfigurationPerServingCell, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_int_ConfigurationPerServingCell_7 = {
	"int-ConfigurationPerServingCell",
	"int-ConfigurationPerServingCell",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_int_ConfigurationPerServingCell_tags_7,
	sizeof(asn_DEF_int_ConfigurationPerServingCell_tags_7)
		/sizeof(asn_DEF_int_ConfigurationPerServingCell_tags_7[0]) - 1, /* 1 */
	asn_DEF_int_ConfigurationPerServingCell_tags_7,	/* Same as above */
	sizeof(asn_DEF_int_ConfigurationPerServingCell_tags_7)
		/sizeof(asn_DEF_int_ConfigurationPerServingCell_tags_7[0]), /* 2 */
	{ &asn_OER_type_int_ConfigurationPerServingCell_constr_7, &asn_PER_type_int_ConfigurationPerServingCell_constr_7, SEQUENCE_OF_constraint },
	asn_MBR_int_ConfigurationPerServingCell_7,
	1,	/* Single element */
	&asn_SPC_int_ConfigurationPerServingCell_specs_7	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_DownlinkPreemption_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DownlinkPreemption, int_RNTI),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RNTI_Value,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"int-RNTI"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DownlinkPreemption, timeFrequencySet),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_timeFrequencySet_3,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"timeFrequencySet"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DownlinkPreemption, dci_PayloadSize),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_dci_PayloadSize_constr_6, &asn_PER_memb_dci_PayloadSize_constr_6,  memb_dci_PayloadSize_constraint_1 },
		0, 0, /* No default value */
		"dci-PayloadSize"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DownlinkPreemption, int_ConfigurationPerServingCell),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		0,
		&asn_DEF_int_ConfigurationPerServingCell_7,
		0,
		{ &asn_OER_memb_int_ConfigurationPerServingCell_constr_7, &asn_PER_memb_int_ConfigurationPerServingCell_constr_7,  memb_int_ConfigurationPerServingCell_constraint_1 },
		0, 0, /* No default value */
		"int-ConfigurationPerServingCell"
		},
};
static const ber_tlv_tag_t asn_DEF_DownlinkPreemption_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DownlinkPreemption_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* int-RNTI */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* timeFrequencySet */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* dci-PayloadSize */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* int-ConfigurationPerServingCell */
};
asn_SEQUENCE_specifics_t asn_SPC_DownlinkPreemption_specs_1 = {
	sizeof(struct DownlinkPreemption),
	offsetof(struct DownlinkPreemption, _asn_ctx),
	asn_MAP_DownlinkPreemption_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	4,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_DownlinkPreemption = {
	"DownlinkPreemption",
	"DownlinkPreemption",
	&asn_OP_SEQUENCE,
	asn_DEF_DownlinkPreemption_tags_1,
	sizeof(asn_DEF_DownlinkPreemption_tags_1)
		/sizeof(asn_DEF_DownlinkPreemption_tags_1[0]), /* 1 */
	asn_DEF_DownlinkPreemption_tags_1,	/* Same as above */
	sizeof(asn_DEF_DownlinkPreemption_tags_1)
		/sizeof(asn_DEF_DownlinkPreemption_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_DownlinkPreemption_1,
	4,	/* Elements count */
	&asn_SPC_DownlinkPreemption_specs_1	/* Additional specs */
};

