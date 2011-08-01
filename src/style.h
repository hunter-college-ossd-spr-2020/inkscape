#ifndef SEEN_SP_STYLE_H
#define SEEN_SP_STYLE_H

/** \file
 * SPStyle - a style object for SPItem objects
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Andreas Becker <atayoohoo@googlemail.com>
 *
 * Copyright (C) 2011 Andreas Becker
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "color.h"
#include "forward.h"
#include "sp-marker-loc.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "uri-references.h"
#include "uri.h"
#include "sp-paint-server-reference.h"

#include <stddef.h>
#include <sigc++/connection.h>

namespace Inkscape {
namespace XML {

// Often used literal properties
// TODO: merge all properties to 1 enum?
// ↓ this is not good
const int AUTO = -1;
const int NONE = -2;

/// Paint type internal to SPStyle.
class SPIPaint : public SPIXorEnum {
    SPPaintServerReference *href;
    SPColor color;

    enum {
        SP_PAINT_CURRENT_COLOR,
        SP_PAINT_COLOR_SET,
        SP_PAINT_ICCCOLOR_SET,
        SP_PAINT_IRI_SET
    };

    SPIPaint();
    ~SPIPaint();
    void readFromString(gchar *str);
    void mergeFromDyingParent(const SPIPaint &parent);
    void mergeFromParent(const SPIPaint &parent);
    bool operator !=(const SPIPaint &paint);

    void clear();

    void setColor(float r, float g, float b)
    {
        color.set(r, g, b);
        paintStatus = SP_PAINTSTATUS_COLOR_SET;
    }

    void setColor(guint32 val)
    {
        color.color.set(val);
        paintStatus = SP_PAINTSTATUS_COLOR_SET;
    }

    void setColor(SPColor const& color )
    {
        color.color = color;
        paintStatus = SP_PAINTSTATUS_COLOR_SET;
    }

    void readFromString(gchar const *str, SPStyle &style);

    

    // Win32 is a temp work-around until the emf extension is fixed:
#ifndef WIN32
private:
    SPIPaint(SPIPaint const&);
    SPIPaint &operator=(SPIPaint const &);
#endif // WIN32
};

class SPIFilter : public SPIXorEnum {
public:
    SPIFilter();
    gchar * toString();
    void readFromString(gchar *str);
    SPFilterReference *href;
};

/** The status of the CSS property. */
typedef enum {
    /** A non-literal value is set. */
    STATUS_VALUE_SET,

    /** A literal value is set. */
    STATUS_LITERAL_SET,

    /** The literal value is "inherit". */
    STATUS_INHERIT,

    /** The property is not set. */
    STATUS_UNSET
} StyleStatus;

/** CSS property that only accepts literal values. */
class SPIXorEnum {
public:
    SPIXorEnum(CSSAttribute attr);
    virtual gchar * toString() const;
    virtual void readFromString(gchar *str);
    virtual void setLiteralValue(int literal);
    virtual void mergeFromDyingParent(const SPStyle *parent);
    int getLiteralValue() const;
    StyleStatus status;
    const property;
protected:
    SPStyleEnum literalList[];

    /**
     * The literal value. The computed value and the actual value are always the same for literal values.
     */
    int literalValue;

    /** The current CSS property. */
    CSSAttribute prop;
};

class SPIString : public SPIXorEnum {
public:
    SPIString(CSSAttribute attr);
    ~SPIString();
    gchar * toString() const;
    void readFromString(gchar *str);
    void mergeFromDyingParent(SPIStyle *parent)
private:
    gchar *buffer;
};

/**
 * Represents a CSS length, a percentage or a number.
 */
class SPILengthOrNumber : public SPIXorEnum {
public:
    SPILengthOrNumber(CSSAttribute attr);
    gchar * toString() const;
    void readFromString(gchar *str);
    void mergeFromDyingParent(SPStyle *parent);
    void setLiteralValue(int literal);
    SVGLength length;
private:
    SVGLength computedLength;
    CSSAttribute prop;
}

/** none | [ underline || overline || line-through || blink ] | inherit */
class SPITextDecoration : public SPIXorEnum {
public:
    enum {
        TEXT_DECORATION_NONE         = (1 << 0),
        TEXT_DECORATION_UNDERLINE    = (1 << 2),
        TEXT_DECORATION_OVERLINE     = (1 << 3),
        TEXT_DECORATION_LINE_THROUGH = (1 << 4)
    } textDecoration;

    SPITextDecoration() :
    gchar * toString() const;
    void readFromString(const gchar *str);
    void mergeFromDyingParent(SPStyle *parent);
};

class SPIStrokeDashArray : public SPIXorEnum {
public:
    gchar * toString() const;
    void readFromString(gchar *str);
    void mergeFromDyingParent(SPStyle *parent);
    void mergeFromParent(SPStyle *parent);
    ~SPIStrokeDashArray();

    /** Converts the dash lengths to an array, used for the livarot/cairo renderer. */
    double * getDashArray();
    std::vector<SPILength *> dashes;
}


class SPStyle {
    /** Object we are attached to */
    SPObject *object;

    /** Document we are associated with */
    SPDocument *document;

    /** Our text style component */
    SPTextStyle *text;

    /** Size of the font */
    SPIFontSize font_size;

    /** text decoration (css2 16.3.1) */
    SPITextDecoration text_decoration;

    /** Baseline shift (svg1.1 10.9.2) */
    SPIBaselineShift baseline_shift;

    SPIStrokeDashArray stroke_dasharray;

    CSSAttribute literalProperties[] = {
        SP_PROP_COLOR,
        SP_PROP_DISPLAY,
        SP_PROP_OVERFLOW,
        SP_PROP_VISIBILITY,
        SP_PROP_CLIP_RULE,
        SP_PROP_TEXT_ANCHOR,
        SP_PROP_TEXT_ALIGN,
        SP_PROP_TEXT_TRANSFORM,
        SP_PROP_DIRECTION,
        SP_PROP_BLOCK_PROGRESSION,
        SP_PROP_WRITING_MODE,
        SP_PROP_FONT_STYLE,
        SP_PROP_FONT_VARIANT,
        SP_PROP_FONT_WEIGHT,
        SP_PROP_FONT_STRETCH,
        SP_PROP_STROKE_LINECAP,
        SP_PROP_STROKE_LINEJOIN,
        SP_PROP_FILL_RULE,
        SP_PROP_OVERFLOW,
        SP_PROP_DISPLAY,
        SP_PROP_CLIP_RULE
        SP_PROP_ENABLE_BACKGROUND
        SP_PROP_FILTER_BLEND_MODE
    };

    CSSAttribute lengthProperties[] = {
        SP_PROP_OPACITY,
        SP_PROP_FILL_OPACITY,
        SP_PROP_STROKE_OPACITY,
        SP_PROP_STROKE_WIDTH,
        SP_PROP_TEXT_INDENT,
        SP_PROP_WORD_SPACING,
        SP_PROP_LETTER_SPACING,
        SP_PROP_LINE_HEIGHT,
        SP_PROP_STROKE_MITERLIMIT
    };

    std::map<CSSAttribute, SPIXorEnum> length;

    std::map<CSSAttribute, SPILength> literal;

    SPStyle(SPDocument *doc);

    SPStyle(SPObject *obj)

    void readFromObject() {

    void readStyle(Inkscape::XML::Node *repr);
// TODO parameter for STATUS_UNSET

    void mergeFromParent(SPStyle *parent);

    void mergeFromDyingParent(SPStyle *parent);

    void mergeFromObjectStylesheet(const SPObject *const object);

    void mergeProperty(CSSAttribute prop, const gchar *val);

    void mergeFromStyleString(const gchar *const p);

    void mergeFromDeclList(const CRDeclaration *const decl_list);

    void mergeFromDeclList(const CRDeclaration *const decl);

    void mergeFromProps(CRPropList props);

    void mergeFromObjectStylesheet(const SPObject *const object);

    void readFromPrefs(const Glib::ustring &path);

    void toString();

    gchar * toStringByDifference(SPStyle *from, SPStyle *to);

    void setToUriString(bool isfill, const gchar *uri);

    /** color */
    SPIPaint color;

    /** fill */
    SPIPaint fill;

    /** stroke */
    SPIPaint stroke;

    SPI

    /** Marker list */
    SPIString marker_start;

    SPIString marker_mid;

    SPIString marker_end;

    /** Filter effect */
    SPIFilter filter;

   /** normally not usedut duplicates the Gaussian blur deviation (if any) from the attached
        filter when the style is used for querying */
    SPILength filter_gaussianBlur_deviation;

    /// style belongs to a cloned object
    bool cloned;

    sigc::connection release_connection;

    sigc::connection filter_modified_connection;
    sigc::connection fill_ps_modified_connection;
    sigc::connection stroke_ps_modified_connection;
};


enum SPCSSFontSize {
    SP_CSS_FONT_SIZE_XX_SMALL,
    SP_CSS_FONT_SIZE_X_SMALL,
    SP_CSS_FONT_SIZE_SMALL,
    SP_CSS_FONT_SIZE_MEDIUM,
    SP_CSS_FONT_SIZE_LARGE,
    SP_CSS_FONT_SIZE_X_LARGE,
    SP_CSS_FONT_SIZE_XX_LARGE,
    SP_CSS_FONT_SIZE_SMALLER,
    SP_CSS_FONT_SIZE_LARGER
};

enum SPCSSFontStyle {
    SP_CSS_FONT_STYLE_NORMAL,
    SP_CSS_FONT_STYLE_ITALIC,
    SP_CSS_FONT_STYLE_OBLIQUE
};

enum SPCSSFontVariant {
    SP_CSS_FONT_VARIANT_NORMAL,
    SP_CSS_FONT_VARIANT_SMALL_CAPS
};

enum SPCSSFontWeight {
    SP_CSS_FONT_WEIGHT_100,
    SP_CSS_FONT_WEIGHT_200,
    SP_CSS_FONT_WEIGHT_300,
    SP_CSS_FONT_WEIGHT_400,
    SP_CSS_FONT_WEIGHT_500,
    SP_CSS_FONT_WEIGHT_600,
    SP_CSS_FONT_WEIGHT_700,
    SP_CSS_FONT_WEIGHT_800,
    SP_CSS_FONT_WEIGHT_900,
    SP_CSS_FONT_WEIGHT_NORMAL,
    SP_CSS_FONT_WEIGHT_BOLD,
    SP_CSS_FONT_WEIGHT_LIGHTER,
    SP_CSS_FONT_WEIGHT_BOLDER
};

enum SPCSSFontStretch {
    SP_CSS_FONT_STRETCH_ULTRA_CONDENSED,
    SP_CSS_FONT_STRETCH_EXTRA_CONDENSED,
    SP_CSS_FONT_STRETCH_CONDENSED,
    SP_CSS_FONT_STRETCH_SEMI_CONDENSED,
    SP_CSS_FONT_STRETCH_NORMAL,
    SP_CSS_FONT_STRETCH_SEMI_EXPANDED,
    SP_CSS_FONT_STRETCH_EXPANDED,
    SP_CSS_FONT_STRETCH_EXTRA_EXPANDED,
    SP_CSS_FONT_STRETCH_ULTRA_EXPANDED,
    SP_CSS_FONT_STRETCH_NARROWER,
    SP_CSS_FONT_STRETCH_WIDER
};

enum SPCSSTextAlign {
    SP_CSS_TEXT_ALIGN_START,
    SP_CSS_TEXT_ALIGN_END,
    SP_CSS_TEXT_ALIGN_LEFT,
    SP_CSS_TEXT_ALIGN_RIGHT,
    SP_CSS_TEXT_ALIGN_CENTER,
    SP_CSS_TEXT_ALIGN_JUSTIFY
    // also <string> is allowed, but only within table calls
};

enum SPCSSTextTransform {
    SP_CSS_TEXT_TRANSFORM_CAPITALIZE,
    SP_CSS_TEXT_TRANSFORM_UPPERCASE,
    SP_CSS_TEXT_TRANSFORM_LOWERCASE,
    SP_CSS_TEXT_TRANSFORM_NONE
};

enum SPCSSDirection {
    SP_CSS_DIRECTION_LTR,
    SP_CSS_DIRECTION_RTL
};

enum SPCSSBlockProgression {
    SP_CSS_BLOCK_PROGRESSION_TB,
    SP_CSS_BLOCK_PROGRESSION_RL,
    SP_CSS_BLOCK_PROGRESSION_LR
};

enum SPCSSWritingMode {
    SP_CSS_WRITING_MODE_LR_TB,
    SP_CSS_WRITING_MODE_RL_TB,
    SP_CSS_WRITING_MODE_TB_RL,
    SP_CSS_WRITING_MODE_TB_LR
};

enum SPTextAnchor {
    SP_CSS_TEXT_ANCHOR_START,
    SP_CSS_TEXT_ANCHOR_MIDDLE,
    SP_CSS_TEXT_ANCHOR_END
};

enum SPCSSBaselineShift {
  SP_CSS_BASELINE_SHIFT_BASELINE,
  SP_CSS_BASELINE_SHIFT_SUB,
  SP_CSS_BASELINE_SHIFT_SUPER
};

enum SPVisibility {
    SP_CSS_VISIBILITY_HIDDEN,
    SP_CSS_VISIBILITY_COLLAPSE,
    SP_CSS_VISIBILITY_VISIBLE
};

enum SPOverflow {
    SP_CSS_OVERFLOW_VISIBLE,
    SP_CSS_OVERFLOW_HIDDEN,
    SP_CSS_OVERFLOW_SCROLL,
    SP_CSS_OVERFLOW_AUTO
};

/// \todo more display types
enum SPCSSDisplay {
    SP_CSS_DISPLAY_NONE,
    SP_CSS_DISPLAY_INLINE,
    SP_CSS_DISPLAY_BLOCK,
    SP_CSS_DISPLAY_LIST_ITEM,
    SP_CSS_DISPLAY_RUN_IN,
    SP_CSS_DISPLAY_COMPACT,
    SP_CSS_DISPLAY_MARKER,
    SP_CSS_DISPLAY_TABLE,
    SP_CSS_DISPLAY_INLINE_TABLE,
    SP_CSS_DISPLAY_TABLE_ROW_GROUP,
    SP_CSS_DISPLAY_TABLE_HEADER_GROUP,
    SP_CSS_DISPLAY_TABLE_FOOTER_GROUP,
    SP_CSS_DISPLAY_TABLE_ROW,
    SP_CSS_DISPLAY_TABLE_COLUMN_GROUP,
    SP_CSS_DISPLAY_TABLE_COLUMN,
    SP_CSS_DISPLAY_TABLE_CELL,
    SP_CSS_DISPLAY_TABLE_CAPTION
};

enum SPEnableBackground {
    SP_CSS_BACKGROUND_ACCUMULATE,
    SP_CSS_BACKGROUND_NEW
};

// <unused>
enum SPAlignmentBaseline {
    SP_ALIGNMENT_BASELINE_BASELINE,
    SP_ALIGNMENT_BASELINE_BEFORE_EDGE,
    SP_ALIGNMENT_BASELINE_TEXT_BEFORE_EDGE,
    SP_ALIGNMENT_BASELINE_MIDDLE,
    SP_ALIGNMENT_BASELINE_CENTRAL,
    SP_ALIGNMENT_BASELINE_AFTER_EDGE,
    SP_ALIGNMENT_BASELINE_TEXT_AFTER_EDGE,
    SP_ALIGNMENT_BASELINE_IDEOGRAPHIC,
    SP_ALIGNMENT_BASELINE_ALPHABETIC,
    SP_ALIGNMENT_BASELINE_HANGING,
    SP_ALIGNMENT_BASELINE_MATHEMATICAL
};

enum SPDominantBaseline {
    SP_DOMINANT_BASELINE_USE_SCRIPT,
    SP_DOMINANT_BASELINE_NO_CHANGE,
    SP_DOMINANT_BASELINE_RESET_SIZE,
    SP_DOMINANT_BASELINE_IDEOGRAPHIC,
    SP_DOMINANT_BASELINE_ALPHABETIC,
    SP_DOMINANT_BASELINE_HANGING,
    SP_DOMINANT_BASELINE_MATHEMATICAL,
    SP_DOMINANT_BASELINE_CENTRAL,
    SP_DOMINANT_BASELINE_MIDDLE,
    SP_DOMINANT_BASELINE_TEXT_AFTER_EDGE,
    SP_DOMINANT_BASELINE_TEXT_BEFORE_EDGE
};
// </unused>

SPCSSAttr *sp_css_attr_from_style (SPStyle const *const style, guint flags);
SPCSSAttr *sp_css_attr_from_object(SPObject *object, guint flags = SP_STYLE_FLAG_IFSET);
SPCSSAttr *sp_css_attr_unset_text(SPCSSAttr *css);
SPCSSAttr *sp_css_attr_unset_uris(SPCSSAttr *css);
SPCSSAttr *sp_css_attr_scale(SPCSSAttr *css, double ex);

void sp_style_unset_property_attrs(SPObject *o);

void sp_style_set_property_url (SPObject *item, gchar const *property, SPObject *linked, bool recursive);

#endif
