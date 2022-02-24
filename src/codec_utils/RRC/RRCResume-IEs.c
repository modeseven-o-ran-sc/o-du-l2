/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#include "RRCResume-IEs.h"

#include "RadioBearerConfig.h"
#include "MeasConfigRrc.h"
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_masterCellGroup_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const OCTET_STRING_t *st = (const OCTET_STRING_t *)sptr;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	
	if(1 /* No applicable constraints whatsoever */) {
		(void)st; /* Unused variable */
		/* Nothing is here. See below */
	}
	
	return td->encoding_constraints.general_constraints(td, sptr, ctfailcb, app_key);
}

static asn_oer_constraints_t asn_OER_type_fullConfig_constr_5 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_fullConfig_constr_5 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 0,  0,  0,  0 }	/* (0..0) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_masterCellGroup_constr_3 CC_NOTUSED = {
	{ 0, 0 },
	-1	/* (SIZE(0..MAX)) */};
static asn_per_constraints_t asn_PER_memb_masterCellGroup_constr_3 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_SEMI_CONSTRAINED,	-1, -1,  0,  0 }	/* (SIZE(0..MAX)) */,
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_fullConfig_value2enum_5[] = {
	{ 0,	4,	"true" }
};
static const unsigned int asn_MAP_fullConfig_enum2value_5[] = {
	0	/* true(0) */
};
static const asn_INTEGER_specifics_t asn_SPC_fullConfig_specs_5 = {
	asn_MAP_fullConfig_value2enum_5,	/* "tag" => N; sorted by tag */
	asn_MAP_fullConfig_enum2value_5,	/* N => "tag"; sorted by N */
	1,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_fullConfig_tags_5[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_fullConfig_5 = {
	"fullConfig",
	"fullConfig",
	&asn_OP_NativeEnumerated,
	asn_DEF_fullConfig_tags_5,
	sizeof(asn_DEF_fullConfig_tags_5)
		/sizeof(asn_DEF_fullConfig_tags_5[0]) - 1, /* 1 */
	asn_DEF_fullConfig_tags_5,	/* Same as above */
	sizeof(asn_DEF_fullConfig_tags_5)
		/sizeof(asn_DEF_fullConfig_tags_5[0]), /* 2 */
	{ &asn_OER_type_fullConfig_constr_5, &asn_PER_type_fullConfig_constr_5, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_fullConfig_specs_5	/* Additional specs */
};

static const ber_tlv_tag_t asn_DEF_nonCriticalExtension_tags_8[] = {
	(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SEQUENCE_specifics_t asn_SPC_nonCriticalExtension_specs_8 = {
	sizeof(struct RRCResume_IEs__nonCriticalExtension),
	offsetof(struct RRCResume_IEs__nonCriticalExtension, _asn_ctx),
	0,	/* No top level tags */
	0,	/* No tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_nonCriticalExtension_8 = {
	"nonCriticalExtension",
	"nonCriticalExtension",
	&asn_OP_SEQUENCE,
	asn_DEF_nonCriticalExtension_tags_8,
	sizeof(asn_DEF_nonCriticalExtension_tags_8)
		/sizeof(asn_DEF_nonCriticalExtension_tags_8[0]) - 1, /* 1 */
	asn_DEF_nonCriticalExtension_tags_8,	/* Same as above */
	sizeof(asn_DEF_nonCriticalExtension_tags_8)
		/sizeof(asn_DEF_nonCriticalExtension_tags_8[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	0, 0,	/* No members */
	&asn_SPC_nonCriticalExtension_specs_8	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_RRCResume_IEs_1[] = {
	{ ATF_POINTER, 6, offsetof(struct RRCResume_IEs, radioBearerConfig),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RadioBearerConfig,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"radioBearerConfig"
		},
	{ ATF_POINTER, 5, offsetof(struct RRCResume_IEs, masterCellGroup),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ &asn_OER_memb_masterCellGroup_constr_3, &asn_PER_memb_masterCellGroup_constr_3,  memb_masterCellGroup_constraint_1 },
		0, 0, /* No default value */
		"masterCellGroup"
		},
	{ ATF_POINTER, 4, offsetof(struct RRCResume_IEs, measConfig),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MeasConfigRrc,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"measConfig"
		},
	{ ATF_POINTER, 3, offsetof(struct RRCResume_IEs, fullConfig),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_fullConfig_5,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"fullConfig"
		},
	{ ATF_POINTER, 2, offsetof(struct RRCResume_IEs, lateNonCriticalExtension),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"lateNonCriticalExtension"
		},
	{ ATF_POINTER, 1, offsetof(struct RRCResume_IEs, nonCriticalExtension),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		0,
		&asn_DEF_nonCriticalExtension_8,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"nonCriticalExtension"
		},
};
static const int asn_MAP_RRCResume_IEs_oms_1[] = { 0, 1, 2, 3, 4, 5 };
static const ber_tlv_tag_t asn_DEF_RRCResume_IEs_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_RRCResume_IEs_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* radioBearerConfig */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* masterCellGroup */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* measConfig */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* fullConfig */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* lateNonCriticalExtension */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* nonCriticalExtension */
};
asn_SEQUENCE_specifics_t asn_SPC_RRCResume_IEs_specs_1 = {
	sizeof(struct RRCResume_IEs),
	offsetof(struct RRCResume_IEs, _asn_ctx),
	asn_MAP_RRCResume_IEs_tag2el_1,
	6,	/* Count of tags in the map */
	asn_MAP_RRCResume_IEs_oms_1,	/* Optional members */
	6, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_RRCResume_IEs = {
	"RRCResume-IEs",
	"RRCResume-IEs",
	&asn_OP_SEQUENCE,
	asn_DEF_RRCResume_IEs_tags_1,
	sizeof(asn_DEF_RRCResume_IEs_tags_1)
		/sizeof(asn_DEF_RRCResume_IEs_tags_1[0]), /* 1 */
	asn_DEF_RRCResume_IEs_tags_1,	/* Same as above */
	sizeof(asn_DEF_RRCResume_IEs_tags_1)
		/sizeof(asn_DEF_RRCResume_IEs_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_RRCResume_IEs_1,
	6,	/* Elements count */
	&asn_SPC_RRCResume_IEs_specs_1	/* Additional specs */
};

