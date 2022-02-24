/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#include "CSI-RS-CellMobility.h"

#include "CSI-RS-Resource-Mobility.h"
/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_startPRB_constraint_3(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 2169)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static int
memb_csi_rs_ResourceList_Mobility_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
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
	
	if((size >= 1 && size <= 96)) {
		/* Perform validation of the inner elements */
		return td->encoding_constraints.general_constraints(td, sptr, ctfailcb, app_key);
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_oer_constraints_t asn_OER_type_nrofPRBs_constr_4 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_nrofPRBs_constr_4 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 3,  3,  0,  4 }	/* (0..4) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_startPRB_constr_10 CC_NOTUSED = {
	{ 2, 1 }	/* (0..2169) */,
	-1};
static asn_per_constraints_t asn_PER_memb_startPRB_constr_10 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 12,  12,  0,  2169 }	/* (0..2169) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_type_density_constr_11 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_density_constr_11 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 1,  1,  0,  1 }	/* (0..1) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_type_csi_rs_ResourceList_Mobility_constr_14 CC_NOTUSED = {
	{ 0, 0 },
	-1	/* (SIZE(1..96)) */};
static asn_per_constraints_t asn_PER_type_csi_rs_ResourceList_Mobility_constr_14 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 7,  7,  1,  96 }	/* (SIZE(1..96)) */,
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_csi_rs_ResourceList_Mobility_constr_14 CC_NOTUSED = {
	{ 0, 0 },
	-1	/* (SIZE(1..96)) */};
static asn_per_constraints_t asn_PER_memb_csi_rs_ResourceList_Mobility_constr_14 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 7,  7,  1,  96 }	/* (SIZE(1..96)) */,
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_nrofPRBs_value2enum_4[] = {
	{ 0,	6,	"size24" },
	{ 1,	6,	"size48" },
	{ 2,	6,	"size96" },
	{ 3,	7,	"size192" },
	{ 4,	7,	"size264" }
};
static const unsigned int asn_MAP_nrofPRBs_enum2value_4[] = {
	3,	/* size192(3) */
	0,	/* size24(0) */
	4,	/* size264(4) */
	1,	/* size48(1) */
	2	/* size96(2) */
};
static const asn_INTEGER_specifics_t asn_SPC_nrofPRBs_specs_4 = {
	asn_MAP_nrofPRBs_value2enum_4,	/* "tag" => N; sorted by tag */
	asn_MAP_nrofPRBs_enum2value_4,	/* N => "tag"; sorted by N */
	5,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_nrofPRBs_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_nrofPRBs_4 = {
	"nrofPRBs",
	"nrofPRBs",
	&asn_OP_NativeEnumerated,
	asn_DEF_nrofPRBs_tags_4,
	sizeof(asn_DEF_nrofPRBs_tags_4)
		/sizeof(asn_DEF_nrofPRBs_tags_4[0]) - 1, /* 1 */
	asn_DEF_nrofPRBs_tags_4,	/* Same as above */
	sizeof(asn_DEF_nrofPRBs_tags_4)
		/sizeof(asn_DEF_nrofPRBs_tags_4[0]), /* 2 */
	{ &asn_OER_type_nrofPRBs_constr_4, &asn_PER_type_nrofPRBs_constr_4, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_nrofPRBs_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_csi_rs_MeasurementBW_3[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CSI_RS_CellMobility__csi_rs_MeasurementBW, nrofPRBs),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_nrofPRBs_4,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"nrofPRBs"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CSI_RS_CellMobility__csi_rs_MeasurementBW, startPRB),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_startPRB_constr_10, &asn_PER_memb_startPRB_constr_10,  memb_startPRB_constraint_3 },
		0, 0, /* No default value */
		"startPRB"
		},
};
static const ber_tlv_tag_t asn_DEF_csi_rs_MeasurementBW_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_csi_rs_MeasurementBW_tag2el_3[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nrofPRBs */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* startPRB */
};
static asn_SEQUENCE_specifics_t asn_SPC_csi_rs_MeasurementBW_specs_3 = {
	sizeof(struct CSI_RS_CellMobility__csi_rs_MeasurementBW),
	offsetof(struct CSI_RS_CellMobility__csi_rs_MeasurementBW, _asn_ctx),
	asn_MAP_csi_rs_MeasurementBW_tag2el_3,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_csi_rs_MeasurementBW_3 = {
	"csi-rs-MeasurementBW",
	"csi-rs-MeasurementBW",
	&asn_OP_SEQUENCE,
	asn_DEF_csi_rs_MeasurementBW_tags_3,
	sizeof(asn_DEF_csi_rs_MeasurementBW_tags_3)
		/sizeof(asn_DEF_csi_rs_MeasurementBW_tags_3[0]) - 1, /* 1 */
	asn_DEF_csi_rs_MeasurementBW_tags_3,	/* Same as above */
	sizeof(asn_DEF_csi_rs_MeasurementBW_tags_3)
		/sizeof(asn_DEF_csi_rs_MeasurementBW_tags_3[0]), /* 2 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_csi_rs_MeasurementBW_3,
	2,	/* Elements count */
	&asn_SPC_csi_rs_MeasurementBW_specs_3	/* Additional specs */
};

static const asn_INTEGER_enum_map_t asn_MAP_density_value2enum_11[] = {
	{ 0,	2,	"d1" },
	{ 1,	2,	"d3" }
};
static const unsigned int asn_MAP_density_enum2value_11[] = {
	0,	/* d1(0) */
	1	/* d3(1) */
};
static const asn_INTEGER_specifics_t asn_SPC_density_specs_11 = {
	asn_MAP_density_value2enum_11,	/* "tag" => N; sorted by tag */
	asn_MAP_density_enum2value_11,	/* N => "tag"; sorted by N */
	2,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_density_tags_11[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_density_11 = {
	"density",
	"density",
	&asn_OP_NativeEnumerated,
	asn_DEF_density_tags_11,
	sizeof(asn_DEF_density_tags_11)
		/sizeof(asn_DEF_density_tags_11[0]) - 1, /* 1 */
	asn_DEF_density_tags_11,	/* Same as above */
	sizeof(asn_DEF_density_tags_11)
		/sizeof(asn_DEF_density_tags_11[0]), /* 2 */
	{ &asn_OER_type_density_constr_11, &asn_PER_type_density_constr_11, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_density_specs_11	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_csi_rs_ResourceList_Mobility_14[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_CSI_RS_Resource_Mobility,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_csi_rs_ResourceList_Mobility_tags_14[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_csi_rs_ResourceList_Mobility_specs_14 = {
	sizeof(struct CSI_RS_CellMobility__csi_rs_ResourceList_Mobility),
	offsetof(struct CSI_RS_CellMobility__csi_rs_ResourceList_Mobility, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_csi_rs_ResourceList_Mobility_14 = {
	"csi-rs-ResourceList-Mobility",
	"csi-rs-ResourceList-Mobility",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_csi_rs_ResourceList_Mobility_tags_14,
	sizeof(asn_DEF_csi_rs_ResourceList_Mobility_tags_14)
		/sizeof(asn_DEF_csi_rs_ResourceList_Mobility_tags_14[0]) - 1, /* 1 */
	asn_DEF_csi_rs_ResourceList_Mobility_tags_14,	/* Same as above */
	sizeof(asn_DEF_csi_rs_ResourceList_Mobility_tags_14)
		/sizeof(asn_DEF_csi_rs_ResourceList_Mobility_tags_14[0]), /* 2 */
	{ &asn_OER_type_csi_rs_ResourceList_Mobility_constr_14, &asn_PER_type_csi_rs_ResourceList_Mobility_constr_14, SEQUENCE_OF_constraint },
	asn_MBR_csi_rs_ResourceList_Mobility_14,
	1,	/* Single element */
	&asn_SPC_csi_rs_ResourceList_Mobility_specs_14	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_CSI_RS_CellMobility_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CSI_RS_CellMobility, cellId),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PhysCellId,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"cellId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CSI_RS_CellMobility, csi_rs_MeasurementBW),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		0,
		&asn_DEF_csi_rs_MeasurementBW_3,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"csi-rs-MeasurementBW"
		},
	{ ATF_POINTER, 1, offsetof(struct CSI_RS_CellMobility, density),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_density_11,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"density"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CSI_RS_CellMobility, csi_rs_ResourceList_Mobility),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		0,
		&asn_DEF_csi_rs_ResourceList_Mobility_14,
		0,
		{ &asn_OER_memb_csi_rs_ResourceList_Mobility_constr_14, &asn_PER_memb_csi_rs_ResourceList_Mobility_constr_14,  memb_csi_rs_ResourceList_Mobility_constraint_1 },
		0, 0, /* No default value */
		"csi-rs-ResourceList-Mobility"
		},
};
static const int asn_MAP_CSI_RS_CellMobility_oms_1[] = { 2 };
static const ber_tlv_tag_t asn_DEF_CSI_RS_CellMobility_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_CSI_RS_CellMobility_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* cellId */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* csi-rs-MeasurementBW */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* density */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* csi-rs-ResourceList-Mobility */
};
asn_SEQUENCE_specifics_t asn_SPC_CSI_RS_CellMobility_specs_1 = {
	sizeof(struct CSI_RS_CellMobility),
	offsetof(struct CSI_RS_CellMobility, _asn_ctx),
	asn_MAP_CSI_RS_CellMobility_tag2el_1,
	4,	/* Count of tags in the map */
	asn_MAP_CSI_RS_CellMobility_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_CSI_RS_CellMobility = {
	"CSI-RS-CellMobility",
	"CSI-RS-CellMobility",
	&asn_OP_SEQUENCE,
	asn_DEF_CSI_RS_CellMobility_tags_1,
	sizeof(asn_DEF_CSI_RS_CellMobility_tags_1)
		/sizeof(asn_DEF_CSI_RS_CellMobility_tags_1[0]), /* 1 */
	asn_DEF_CSI_RS_CellMobility_tags_1,	/* Same as above */
	sizeof(asn_DEF_CSI_RS_CellMobility_tags_1)
		/sizeof(asn_DEF_CSI_RS_CellMobility_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_CSI_RS_CellMobility_1,
	4,	/* Elements count */
	&asn_SPC_CSI_RS_CellMobility_specs_1	/* Additional specs */
};

