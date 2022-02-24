/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#include "TypeI-MultiPanelCodebook.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_maxNumberResources_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 1 && value <= 64)) {
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
memb_totalNumberTxPorts_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 2 && value <= 256)) {
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
memb_maxNumberCSI_RS_PerResourceSet_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 1 && value <= 8)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_oer_constraints_t asn_OER_type_maxNumberTxPortsPerResource_constr_2 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_maxNumberTxPortsPerResource_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 2,  2,  0,  2 }	/* (0..2) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_type_supportedCodebookMode_constr_8 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_supportedCodebookMode_constr_8 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 2,  2,  0,  2 }	/* (0..2) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_type_supportedNumberPanels_constr_12 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_supportedNumberPanels_constr_12 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_maxNumberResources_constr_6 CC_NOTUSED = {
	{ 1, 1 }	/* (1..64) */,
	-1};
static asn_per_constraints_t asn_PER_memb_maxNumberResources_constr_6 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 6,  6,  1,  64 }	/* (1..64) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_totalNumberTxPorts_constr_7 CC_NOTUSED = {
	{ 2, 1 }	/* (2..256) */,
	-1};
static asn_per_constraints_t asn_PER_memb_totalNumberTxPorts_constr_7 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 8,  8,  2,  256 }	/* (2..256) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_maxNumberCSI_RS_PerResourceSet_constr_15 CC_NOTUSED = {
	{ 1, 1 }	/* (1..8) */,
	-1};
static asn_per_constraints_t asn_PER_memb_maxNumberCSI_RS_PerResourceSet_constr_15 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 3,  3,  1,  8 }	/* (1..8) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_maxNumberTxPortsPerResource_value2enum_2[] = {
	{ 0,	2,	"p8" },
	{ 1,	3,	"p16" },
	{ 2,	3,	"p32" }
};
static const unsigned int asn_MAP_maxNumberTxPortsPerResource_enum2value_2[] = {
	1,	/* p16(1) */
	2,	/* p32(2) */
	0	/* p8(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_maxNumberTxPortsPerResource_specs_2 = {
	asn_MAP_maxNumberTxPortsPerResource_value2enum_2,	/* "tag" => N; sorted by tag */
	asn_MAP_maxNumberTxPortsPerResource_enum2value_2,	/* N => "tag"; sorted by N */
	3,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_maxNumberTxPortsPerResource_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_maxNumberTxPortsPerResource_2 = {
	"maxNumberTxPortsPerResource",
	"maxNumberTxPortsPerResource",
	&asn_OP_NativeEnumerated,
	asn_DEF_maxNumberTxPortsPerResource_tags_2,
	sizeof(asn_DEF_maxNumberTxPortsPerResource_tags_2)
		/sizeof(asn_DEF_maxNumberTxPortsPerResource_tags_2[0]) - 1, /* 1 */
	asn_DEF_maxNumberTxPortsPerResource_tags_2,	/* Same as above */
	sizeof(asn_DEF_maxNumberTxPortsPerResource_tags_2)
		/sizeof(asn_DEF_maxNumberTxPortsPerResource_tags_2[0]), /* 2 */
	{ &asn_OER_type_maxNumberTxPortsPerResource_constr_2, &asn_PER_type_maxNumberTxPortsPerResource_constr_2, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_maxNumberTxPortsPerResource_specs_2	/* Additional specs */
};

static const asn_INTEGER_enum_map_t asn_MAP_supportedCodebookMode_value2enum_8[] = {
	{ 0,	5,	"mode1" },
	{ 1,	5,	"mode2" },
	{ 2,	4,	"both" }
};
static const unsigned int asn_MAP_supportedCodebookMode_enum2value_8[] = {
	2,	/* both(2) */
	0,	/* mode1(0) */
	1	/* mode2(1) */
};
static const asn_INTEGER_specifics_t asn_SPC_supportedCodebookMode_specs_8 = {
	asn_MAP_supportedCodebookMode_value2enum_8,	/* "tag" => N; sorted by tag */
	asn_MAP_supportedCodebookMode_enum2value_8,	/* N => "tag"; sorted by N */
	3,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_supportedCodebookMode_tags_8[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_supportedCodebookMode_8 = {
	"supportedCodebookMode",
	"supportedCodebookMode",
	&asn_OP_NativeEnumerated,
	asn_DEF_supportedCodebookMode_tags_8,
	sizeof(asn_DEF_supportedCodebookMode_tags_8)
		/sizeof(asn_DEF_supportedCodebookMode_tags_8[0]) - 1, /* 1 */
	asn_DEF_supportedCodebookMode_tags_8,	/* Same as above */
	sizeof(asn_DEF_supportedCodebookMode_tags_8)
		/sizeof(asn_DEF_supportedCodebookMode_tags_8[0]), /* 2 */
	{ &asn_OER_type_supportedCodebookMode_constr_8, &asn_PER_type_supportedCodebookMode_constr_8, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_supportedCodebookMode_specs_8	/* Additional specs */
};

static const asn_INTEGER_enum_map_t asn_MAP_supportedNumberPanels_value2enum_12[] = {
	{ 0,	2,	"n2" },
	{ 1,	2,	"n4" }
};
static const unsigned int asn_MAP_supportedNumberPanels_enum2value_12[] = {
	0,	/* n2(0) */
	1	/* n4(1) */
};
static const asn_INTEGER_specifics_t asn_SPC_supportedNumberPanels_specs_12 = {
	asn_MAP_supportedNumberPanels_value2enum_12,	/* "tag" => N; sorted by tag */
	asn_MAP_supportedNumberPanels_enum2value_12,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_supportedNumberPanels_tags_12[] = {
	(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_supportedNumberPanels_12 = {
	"supportedNumberPanels",
	"supportedNumberPanels",
	&asn_OP_NativeEnumerated,
	asn_DEF_supportedNumberPanels_tags_12,
	sizeof(asn_DEF_supportedNumberPanels_tags_12)
		/sizeof(asn_DEF_supportedNumberPanels_tags_12[0]) - 1, /* 1 */
	asn_DEF_supportedNumberPanels_tags_12,	/* Same as above */
	sizeof(asn_DEF_supportedNumberPanels_tags_12)
		/sizeof(asn_DEF_supportedNumberPanels_tags_12[0]), /* 2 */
	{ &asn_OER_type_supportedNumberPanels_constr_12, &asn_PER_type_supportedNumberPanels_constr_12, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_supportedNumberPanels_specs_12	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_TypeI_MultiPanelCodebook_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct TypeI_MultiPanelCodebook, maxNumberTxPortsPerResource),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_maxNumberTxPortsPerResource_2,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"maxNumberTxPortsPerResource"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TypeI_MultiPanelCodebook, maxNumberResources),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_maxNumberResources_constr_6, &asn_PER_memb_maxNumberResources_constr_6,  memb_maxNumberResources_constraint_1 },
		0, 0, /* No default value */
		"maxNumberResources"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TypeI_MultiPanelCodebook, totalNumberTxPorts),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_totalNumberTxPorts_constr_7, &asn_PER_memb_totalNumberTxPorts_constr_7,  memb_totalNumberTxPorts_constraint_1 },
		0, 0, /* No default value */
		"totalNumberTxPorts"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TypeI_MultiPanelCodebook, supportedCodebookMode),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_supportedCodebookMode_8,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"supportedCodebookMode"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TypeI_MultiPanelCodebook, supportedNumberPanels),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_supportedNumberPanels_12,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"supportedNumberPanels"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TypeI_MultiPanelCodebook, maxNumberCSI_RS_PerResourceSet),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_maxNumberCSI_RS_PerResourceSet_constr_15, &asn_PER_memb_maxNumberCSI_RS_PerResourceSet_constr_15,  memb_maxNumberCSI_RS_PerResourceSet_constraint_1 },
		0, 0, /* No default value */
		"maxNumberCSI-RS-PerResourceSet"
		},
};
static const ber_tlv_tag_t asn_DEF_TypeI_MultiPanelCodebook_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_TypeI_MultiPanelCodebook_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* maxNumberTxPortsPerResource */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* maxNumberResources */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* totalNumberTxPorts */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* supportedCodebookMode */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* supportedNumberPanels */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* maxNumberCSI-RS-PerResourceSet */
};
asn_SEQUENCE_specifics_t asn_SPC_TypeI_MultiPanelCodebook_specs_1 = {
	sizeof(struct TypeI_MultiPanelCodebook),
	offsetof(struct TypeI_MultiPanelCodebook, _asn_ctx),
	asn_MAP_TypeI_MultiPanelCodebook_tag2el_1,
	6,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_TypeI_MultiPanelCodebook = {
	"TypeI-MultiPanelCodebook",
	"TypeI-MultiPanelCodebook",
	&asn_OP_SEQUENCE,
	asn_DEF_TypeI_MultiPanelCodebook_tags_1,
	sizeof(asn_DEF_TypeI_MultiPanelCodebook_tags_1)
		/sizeof(asn_DEF_TypeI_MultiPanelCodebook_tags_1[0]), /* 1 */
	asn_DEF_TypeI_MultiPanelCodebook_tags_1,	/* Same as above */
	sizeof(asn_DEF_TypeI_MultiPanelCodebook_tags_1)
		/sizeof(asn_DEF_TypeI_MultiPanelCodebook_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_TypeI_MultiPanelCodebook_1,
	6,	/* Elements count */
	&asn_SPC_TypeI_MultiPanelCodebook_specs_1	/* Additional specs */
};

