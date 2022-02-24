/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NR-RRC-Definitions"
 * 	found in "../../../rrc_15.3_asn.asn1"
 * 	`asn1c -D ./25_02_2022_RRC/ -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-example`
 */

#include "PCI-RangeElement.h"

asn_TYPE_member_t asn_MBR_PCI_RangeElement_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PCI_RangeElement, pci_RangeIndex),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PCI_RangeIndex,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"pci-RangeIndex"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PCI_RangeElement, pci_Range),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PCI_Range,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"pci-Range"
		},
};
static const ber_tlv_tag_t asn_DEF_PCI_RangeElement_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PCI_RangeElement_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* pci-RangeIndex */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* pci-Range */
};
asn_SEQUENCE_specifics_t asn_SPC_PCI_RangeElement_specs_1 = {
	sizeof(struct PCI_RangeElement),
	offsetof(struct PCI_RangeElement, _asn_ctx),
	asn_MAP_PCI_RangeElement_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_PCI_RangeElement = {
	"PCI-RangeElement",
	"PCI-RangeElement",
	&asn_OP_SEQUENCE,
	asn_DEF_PCI_RangeElement_tags_1,
	sizeof(asn_DEF_PCI_RangeElement_tags_1)
		/sizeof(asn_DEF_PCI_RangeElement_tags_1[0]), /* 1 */
	asn_DEF_PCI_RangeElement_tags_1,	/* Same as above */
	sizeof(asn_DEF_PCI_RangeElement_tags_1)
		/sizeof(asn_DEF_PCI_RangeElement_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_PCI_RangeElement_1,
	2,	/* Elements count */
	&asn_SPC_PCI_RangeElement_specs_1	/* Additional specs */
};

