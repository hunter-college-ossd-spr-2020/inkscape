#ifndef SEEN_SP_STYLE_H
#define SEEN_SP_STYLE_H

/** \file
 * SPStyle - a style object for SPItem objects
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
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
class Node;
}
}

class SPCSSAttr;

class SPIFloat;
class SPIScale24;
class SPIInt;
class SPIShort;
class SPIEnum;
class SPIString;
class SPILength;
class SPIPaint;
class SPIFontSize;
class SPIBaselineShift;

/*
 * One might think that the best value for SP_SCALE24_MAX would be ((1<<24)-1), which allows the
 * greatest possible precision for fitting [0, 1] fractions into 24 bits.
 *
 * However, in practice, that gives a problem with 0.5, which falls half way between two fractions
 * of ((1<<24)-1).  What's worse is that casting double(1<<23) / ((1<<24)-1) to float on x86
 * produces wrong rounding behaviour, resulting in a fraction of ((1<<23)+2.0f) / (1<<24) rather
 * than ((1<<23)+1.0f) / (1<<24) as one would expect, let alone ((1<<23)+0.0f) / (1<<24) as one
 * would ideally like for this example.
 *
 * The value (1<<23) is thus best if one considers float conversions alone.
 *
 * The value 0xff0000 can exactly represent all 8-bit alpha channel values,
 * and can exactly represent all multiples of 0.1.  I haven't yet tested whether
 * rounding bugs still get in the way of conversions to & from float, but my instinct is that
 * it's fairly safe because 0xff fits three times inside float's significand.
 *
 * We should probably use the value 0xffff00 once we support 16 bits per channel and/or LittleCMS,
 * though that might need to be accompanied by greater use of double instead of float for
 * colours and opacities, to be safe from rounding bugs.
 */
#define SP_SCALE24_MAX (0xff0000)
#define SP_SCALE24_TO_FLOAT(v) ((double) (v) / SP_SCALE24_MAX)
#define SP_SCALE24_FROM_FLOAT(v) unsigned(((v) * SP_SCALE24_MAX) + .5)

/** Returns a scale24 for the product of two scale24 values. */
#define SP_SCALE24_MUL(_v1, _v2) unsigned((double)(_v1) * (_v2) / SP_SCALE24_MAX + .5)

#define SP_STYLE_FILL_SERVER(s) (((SPStyle *) (s))->getFillPaintServer())
#define SP_STYLE_STROKE_SERVER(s) (((SPStyle *) (s))->getStrokePaintServer())

class SVGICCColor;

/// Paint type internal to SPStyle.
class SPIPaint : public SPIXorEnum {
    SPPaintServerReference *href;
    SPColor color;

    enum {
        SP_PAINT_CURRENT_COLOR
    };

    SPIPaint() :
        href(NULL),
    {
        literalList = {
            {"none", NONE},
            {"currentColor", SP_PAINT_CURRENTCOLOR}
        };
    }

    ~SPIPaint()
    {
        if (href != NULL) {
            delete href;
        }
    }

    void readFromString(gchar *str)
    {
        SPIXorEnum::readFromString(str);
        if (status == STATUS_UNSET) {
            const guint32 rgb0 = sp_svg_read_color(str, 0xff);
            if (rgb0 != 0xff) {
                setColor(rgb0);
                status = STATUS_SET;
            }
        }
    }

    void mergeFromDyingParent(const SPIPaint &parent)
    {
        if (status == STATUS_UNSET || status == STATUS_INHERIT) {
            mergeFromParent(&parent);
            status = parent.status;
        }
    }

    void mergeFromParent(const SPIPaint &parent)
    {
        if ((paint->status == STATUS_SET && paint->value == SP_PAINT_CURRENT_COLOR) || parent->value == SP_PAINT_CURRENT_COLOR) {
            SPIStatus isset = paint->status;
            clear();
            if (isset == ) = isset;
            value = SP_PAINT_CURRENT_COLOR;
            setColor(style->color.value.color); // TODO
            return;
        }

        clear();
        if (parent->isPaintserver() == true) {
            if (parent->value.href != NULL) { // TODO
                sp_style_set_ipaint_to_uri(style, paint, parent->value.href->getURI(), parent->value.href->getOwnerDocument());
            } else {
                g_warning("Expected paint server not found.");
            }
        }
        else if (parent->isColor() ) {
            paint->setColor( parent->value.color );
        }
        else if ( parent->isNoneSet() ) {
            paint->noneSet = true;
        }
        else if (parent->isNone()) {
            //
        }
        else {
            g_assert_not_reached();
        }

    }

    // TODO sigc connection

    bool operator !=(const SPIPaint &paint)
    {
        if (a->isColor() != b->isColor()
            || a->isPaintserver() != b->isPaintserver()
            || a->status != b->status
            || (status == STATUS_LITERAL_SET && a->literalValue != b->literalValue))
        {
            return true;
        }
// TODO declare functions const!
        // TODO refactor to allow for mixed paints (rgb() *and* url(), etc)
        if (isPaintserver()) {
            return (value.href == NULL || value.href == NULL || value.href->getObject() != value.href->getObject());
        }

        if ( a->isColor() ) {
            return !( (value.color == value.color)
                     && ((value.color.icc == value.color.icc)
                         || (value.color.icc && value.color.icc
                             && (value.color.icc->colorProfile == value.color.icc->colorProfile)
                             && (value.color.icc->colors == value.color.icc->colors))));
        /* todo: Allow for epsilon differences in iccColor->colors, e.g. changes small enough not to show up
         * in the string representation. */
        }

        return false;
    }

    bool isSet() const { return true; /* set || colorSet*/}
    bool isSameType( SPIPaint const & other ) const {return (isPaintserver() == other.isPaintserver()) && (colorSet == other.colorSet) && (currentcolor == other.currentcolor);}

    bool isNoneSet() const {return noneSet;}

    bool isNone() const {return !currentcolor && !colorSet && !isPaintserver();} // TODO refine
    bool isColor() const {return colorSet && !isPaintserver();}
    bool isPaintserver() const {return value.href && value.href->getObject();}

    void clear();

    void setColor( float r, float g, float b ) {value.color.set( r, g, b ); colorSet = true;}
    void setColor( guint32 val ) {value.color.set( val ); colorSet = true;}
    void setColor( SPColor const& color ) {value.color = color; colorSet = true;}

    void read( gchar const *str, SPStyle &tyle, SPDocument *document = 0);

    // Win32 is a temp work-around until the emf extension is fixed:
#ifndef WIN32
private:
    SPIPaint(SPIPaint const&);
    SPIPaint &operator=(SPIPaint const &);
#endif // WIN32
};

class SPIFilter : public SPIXorEnum {
public:
    SPIFilter() :
        SPFilterReference(NULL)
    {
        literalList = {{"none", NONE}};
    }

    // Attention! This return value has to be freed!
    gchar * toString()
    {
        if (status == STATUS_INHERIT) {
            return g_strdup("inherit");
        }
        else {
            return g_strdup_printf("url(%s)", href->getURI()->toString());
        }
    }

    // TODO: "none" value?
    void readFromString(gchar *str)
    {
        SPIXorEnum::readFromString();

        if (href != NULL && href->getObject() != NULL) {
            href->detach();
        }

        if (status == STATUS_UNSET) {
            return;
        }

        if (str[0] == 'u' && str[1] == 'r' && str[2] == 'l') {
            char *uri = extract_uri(str);
            if (uri == NULL || uri[0] == '\0') {
                g_warning("Specified filter url is empty");
                status = STATUS_SET; // really?
                return;
            }

            if (href == NULL && document != NULL) {
                href = new SPFilterReference(document);
                href->changedSignal().connect(sigc::bind(sigc::ptr_fun(filterRefChanged), this));
            }

            try {
                href->attach(Inkscape::URI(uri));
            }
            catch (Inkscape::BadURIException &e) {
                g_warning("%s", e.what());
                href->detach();
            }

            g_free(uri);
        }
        else {
            status = STATUS_UNSET;
        }
    }

private:
    SPFilterReference *href;
};

#define SP_STYLE_FLAG_IFSET (1 << 0)
#define SP_STYLE_FLAG_IFDIFF (1 << 1)
#define SP_STYLE_FLAG_ALWAYS (1 << 2)

class SPIFontSize : SPILengthOrNormal {
    // is a length
    SPIFontSize()
    {
        // literallist
    }

// TODO flags
// TODO unify free requirement
    gchar * toString()
    {
        gchar *basets = SPIXorEnum::toString()[0]
        if (basets != NULL) {
            return basets;
        }
        if (fontSize.getUnit() == SVGLength::SVG_LENGTHTYPE_UNSPECIFIED) {
            // must specify px, see inkscape bug 1221626, mozilla bug 234789
            fontSize.convertToSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_PX);
        }
        return fontSize.toString();
    }

    void mergeFromDyingParent(const SPIFontSize *const parent)
    {
        
    }
SVGLength fontSize;
};

/** Baseline shift type internal to SPStyle.
 */
class SPIBaselineShift {
public:
    SPIBaselineShift() { }

    void setValue(SVGLength length, const SPIFontSize &pfont_size)
    {
        if (status == STATUS_VALUE_SET || status == STATUS_INHERIT) {
            computedValue = parent.getComputedValue();
        }
        else if (status == STATUS_LITERAL_SET) {
            switch (enumValue) {
                case SP_CSS_BASELINE_SHIFT_BASELINE:
                    computedValue.setValue(0);
                    break;
                case SP_CSS_BASELINE_SHIFT_SUPER:
                    computedValue.setValue(0.4 * pfont_size.getComputedValue());
                    break;
                case SP_CSS_BASELINE_SHIFT_SUPER:
                    computedValue.setValue(0.5 * pfont_value.getComputedValue());
                    break;
            }
        }
        else if (status == STATUS_VALUE_SET) {
            if (length.getUnitType() == SVGLength::SVG_LENGTHTYPE_PERCENTAGE) {
                computedValue = 0.5 * value * pfont_size.getComputedValue();
            }
        }
        else {
            computedValue += parent.getComputedValue();
        }
    }

    SVGLength getValue()
    {
        return length;
    }

    SVGLength getComputedValue()
    {
        return computedLength;
    }

    void inheritFrom(const SPIBaselineShift &parent, const SPIBaselineShift &pfont_size)
    {
        if (status == STATUS_UNSET || status == STATUS_INHERIT) {
            computedValue = parent.getComputedValue();
        }
        else if (status == STATUS_VALUE_SET) {
        }
    }

private:
    SVGLength computedLength;
    SVGLength length;
};

/**
 * color-interpolation, color-interpolation-filters, color-rendering, shape-rendering,
 * text-rendering, image-rendering, fill-rule, stroke-linecap, stroke-linejoin, display,
 * visibility
 */

class SPIXorEnum {

public:
    SPIXorEnum(SPLiteralEnum property) :
        status(ENUM_UNSET)
    {
        switch (property) {
            case SP_PROP_STROKE_LINECAP:
                value = SP_STROKE_LINECAP_BUTT;
                literalDict = {
                    {"miter", SP_STROKE_LINECAP_BUTT},
                    {"round", SP_STROKE_LINECAP_ROUND},
                    {"square", SP_STROKE_LINECAP_SQUARE},
                };
                break;
            case SP_PROP_STROKE_LINEJOIN:
                value = SP_STROKE_LINEJOIN_MITER;
                literalDict = {
                    {"miter", SP_STROKE_LINEJOIN_MITER},
                    {"round", SP_STROKE_LINEJOIN_ROUND},
                    {"bevel", SP_STROKE_LINEJOIN_BEVEL},
                };
                break;
            case SP_PROP_FILL_RULE:
                value = SP_WIND_RULE_NONZERO;
                literalDict = {
                    {"nonzero", SP_WIND_RULE_NONZERO},
                    {"evenodd", SP_WIND_RULE_EVENODD},
                };
                break;
            case SP_PROP_FONT_WEIGHT:
                value = SP_CSS_FONT_WEIGHT_NORMAL;
                literalDict = {
                    {"100", SP_CSS_FONT_WEIGHT_100},
                    {"200", SP_CSS_FONT_WEIGHT_200},
                    {"300", SP_CSS_FONT_WEIGHT_300},
                    {"400", SP_CSS_FONT_WEIGHT_400},
                    {"500", SP_CSS_FONT_WEIGHT_500},
                    {"600", SP_CSS_FONT_WEIGHT_600},
                    {"700", SP_CSS_FONT_WEIGHT_700},
                    {"800", SP_CSS_FONT_WEIGHT_800},
                    {"900", SP_CSS_FONT_WEIGHT_900},
                    {"normal", SP_CSS_FONT_WEIGHT_NORMAL},
                    {"bold", SP_CSS_FONT_WEIGHT_BOLD},
                    {"lighter", SP_CSS_FONT_WEIGHT_LIGHTER},
                    {"bolder", SP_CSS_FONT_WEIGHT_BOLDER},
                };
                break;
            case SP_PROP_FONT_VARIANT:
                value = SP_CSS_FONT_VARIANT_NORMAL;
                literalDict = {
                    {"normal", SP_CSS_FONT_VARIANT_NORMAL},
                    {"small-caps", SP_CSS_FONT_VARIANT_SMALL_CAPS},
                };
                break;
            case SP_PROP_FONT_STRETCH:
                value = SP_CSS_FONT_STRETCH_NORMAL;
                literalDict = {
                    {"ultra-condensed", SP_CSS_FONT_STRETCH_ULTRA_CONDENSED},
                    {"extra-condensed", SP_CSS_FONT_STRETCH_EXTRA_CONDENSED},
                    {"condensed", SP_CSS_FONT_STRETCH_CONDENSED},
                    {"semi-condensed", SP_CSS_FONT_STRETCH_SEMI_CONDENSED},
                    {"normal", SP_CSS_FONT_STRETCH_NORMAL},
                    {"semi-expanded", SP_CSS_FONT_STRETCH_SEMI_EXPANDED},
                    {"expanded", SP_CSS_FONT_STRETCH_EXPANDED},
                    {"extra-expanded", SP_CSS_FONT_STRETCH_EXTRA_EXPANDED},
                    {"ultra-expanded", SP_CSS_FONT_STRETCH_ULTRA_EXPANDED},
                    {"narrower", SP_CSS_FONT_STRETCH_NARROWER},
                    {"wider", SP_CSS_FONT_STRETCH_WIDER},
                };
                break;
            case SP_PROP_SHAPE_RENDERING:
                literalList = {
                    {"auto", 0},
                    {"optimizeSpeed", 0},
                    {"crispEdges", 0},
                    {"geometricPrecision", 0}
                };
                break;
            case SP_PROP_DISPLAY:
                literalList = {
                    {"none",      SP_CSS_DISPLAY_NONE},
                    {"inline",    SP_CSS_DISPLAY_INLINE},
                    {"block",     SP_CSS_DISPLAY_BLOCK},
                    {"list-item", SP_CSS_DISPLAY_LIST_ITEM},
                    {"run-in",    SP_CSS_DISPLAY_RUN_IN},
                    {"compact",   SP_CSS_DISPLAY_COMPACT},
                    {"marker",    SP_CSS_DISPLAY_MARKER},
                    {"table",     SP_CSS_DISPLAY_TABLE},
                    {"inline-table",  SP_CSS_DISPLAY_INLINE_TABLE},
                    {"table-row-group",    SP_CSS_DISPLAY_TABLE_ROW_GROUP},
                    {"table-header-group", SP_CSS_DISPLAY_TABLE_HEADER_GROUP},
                    {"table-footer-group", SP_CSS_DISPLAY_TABLE_FOOTER_GROUP},
                    {"table-row",     SP_CSS_DISPLAY_TABLE_ROW},
                    {"table-column-group", SP_CSS_DISPLAY_TABLE_COLUMN_GROUP},
                    {"table-column",  SP_CSS_DISPLAY_TABLE_COLUMN},
                    {"table-cell",    SP_CSS_DISPLAY_TABLE_CELL},
                    {"table-caption", SP_CSS_DISPLAY_TABLE_CAPTION}
                };
                break;
            case SP_PROP_ENABLE_BACKGROUND:
                literalList = {
                    {"accumulate", SP_CSS_BACKGROUND_ACCUMULATE},
                    {"new", SP_CSS_BACKGROUND_NEW}
                };
                break;
            case SP_PROP_CLIP_RULE:
                literalList = {
                    {"nonzero", SP_WIND_RULE_NONZERO},
                    {"evenodd", SP_WIND_RULE_EVENODD}
                };
                break;
            case SP_PROP_OVERFLOW:
                literalList = {
                    {"visible", SP_CSS_OVERFLOW_VISIBLE},
                    {"hidden", SP_CSS_OVERFLOW_HIDDEN},
                    {"scroll", SP_CSS_OVERFLOW_SCROLL},
                    {"auto", SP_CSS_OVERFLOW_AUTO}
                };
                break;
            case SP_PROP_VISIBILITY:
                literalList = {
                    {"hidden", SP_CSS_VISIBILITY_HIDDEN},
                    {"collapse", SP_CSS_VISIBILITY_COLLAPSE},
                    {"visible", SP_CSS_VISIBILITY_VISIBLE}
                };
                break;
            case SP_PROP_BASELINE_SHIFT:
                literalList = {
                    {"baseline", SP_CSS_BASELINE_SHIFT_BASELINE},
                    {"sub",      SP_CSS_BASELINE_SHIFT_SUB},
                    {"super",    SP_CSS_BASELINE_SHIFT_SUPER}
                };
                break;
            case SP_PROP_FONT_STYLE:
                literalList = {
                    {"normal", SP_CSS_FONT_STYLE_NORMAL},
                    {"italic", SP_CSS_FONT_STYLE_ITALIC},
                    {"oblique", SP_CSS_FONT_STYLE_OBLIQUE}
                };
                break;
            case SP_PROP_FONT_SIZE:
                literalList = {
                    {"xx-small", SP_CSS_FONT_SIZE_XX_SMALL},
                    {"x-small", SP_CSS_FONT_SIZE_X_SMALL},
                    {"small", SP_CSS_FONT_SIZE_SMALL},
                    {"medium", SP_CSS_FONT_SIZE_MEDIUM},
                    {"large", SP_CSS_FONT_SIZE_LARGE},
                    {"x-large", SP_CSS_FONT_SIZE_X_LARGE},
                    {"xx-large", SP_CSS_FONT_SIZE_XX_LARGE},
                    {"smaller", SP_CSS_FONT_SIZE_SMALLER},
                    {"larger", SP_CSS_FONT_SIZE_LARGER}
                };
                break;
            case SP_PROP_TEXT_ALIGN:
                literalList = {
                    {"start", SP_CSS_TEXT_ALIGN_START},
                    {"end", SP_CSS_TEXT_ALIGN_END},
                    {"left", SP_CSS_TEXT_ALIGN_LEFT},
                    {"right", SP_CSS_TEXT_ALIGN_RIGHT},
                    {"center", SP_CSS_TEXT_ALIGN_CENTER},
                    {"justify", SP_CSS_TEXT_ALIGN_JUSTIFY}
                };
                break;
            case SP_PROP_TEXT_TRANSFORM:
                literalList = {
                    {"capitalize", SP_CSS_TEXT_TRANSFORM_CAPITALIZE},
                    {"uppercase", SP_CSS_TEXT_TRANSFORM_UPPERCASE},
                    {"lowercase", SP_CSS_TEXT_TRANSFORM_LOWERCASE},
                    {"none", SP_CSS_TEXT_TRANSFORM_NONE}
                };
                break;
            case SP_PROP_TEXT_ANCHOR:
                literalList = {
                    {"start", SP_CSS_TEXT_ANCHOR_START},
                    {"middle", SP_CSS_TEXT_ANCHOR_MIDDLE},
                    {"end", SP_CSS_TEXT_ANCHOR_END}
                };
                break;
            case SP_PROP_DIRECTION:
                literalList = {
                    {"ltr", SP_CSS_DIRECTION_LTR},
                    {"rtl", SP_CSS_DIRECTION_RTL}
                };
                break;
            case SP_PROP_BLOCK_PROGRESSION:
                literalList = {
                    {"tb", SP_CSS_BLOCK_PROGRESSION_TB},
                    {"rl", SP_CSS_BLOCK_PROGRESSION_RL},
                    {"lr", SP_CSS_BLOCK_PROGRESSION_LR}
                };
                break;
         }
    }

    virtual gchar * toString()
    {
        if (status == STATUS_UNSET) {
            return "";
        }
        else if (status == STATUS_INHERIT) {
            return "inherit";
        }
        else {
            unsigned int i;
            for (unsigned int i = 0; i < G_N_ELEMENTS(literalList); ++i) {
                if (literalList[i][1] == value) {
                    return literalList[i][0];
                }
            }
        }
    }

    virtual void readFromString(gchar *str)
    {
        if (str[0] == '\0') {
            status = STATUS_UNSET;
            value = defaultValue;
        }
        else if (strcmp(str, "inherit") == 0) {
            status = STATUS_INHERIT;
        }
        else {
            for (unsigned int i = 0; i < G_N_ELEMENTS(literalList); ++i) {
                if (strcmp(literalList[i][0] == str) == 0) {
                    status = STATUS_LITERAL_SET;
                    value = literalList[i][1];
                }
            }
        }
    }

    virtual void inheritFrom(SPIXorEnum *parent)
    {
        if (parent->status == STATUS_LITERAL_SET || parent->status == STATUS_INHERIT) {
            computedValue = parent.getComputedValue()
        }
    }

    void readFromNode(Inkscape::XML::Node *repr)
    {
        // sp_attribute_name in attributes.cpp
        const char *name = sp_attribute_name(prop);
        const gchar *val = repr->attribute(name);
        readFromString(val);
    }

    enum {
        STATUS_VALUE_SET,
        STATUS_LITERAL_SET,
        STATUS_INHERIT,
        STATUS_UNSET
    } status;

    SPStyleEnum literalDict[];
    unsigned int value;
    unsigned int computedValue;
    CSSAttribute prop;
};

class SPIString : public SPIXorEnum {
public:
    SPIString(gchar *str)
    {
        buffer = g_strdup(str);
    }

    ~SPIString()
    {
        if (buffer != NULL) { // omit check?
            g_free(buffer);
        }
    }

// const gchar * better
    gchar * toString()
    {
        return buffer;
    }

    void readFromString(gchar *str)
    {
        delete buffer;
        buffer = g_strdup(str);
    }

    void mergeFromDyingParent(SPIString *parent)
    {
        if (status == STATUS_UNSET || status == STATUS_INHERIT) {
            g_free(buffer);
            buffer = g_strdup(parent.toString());
            status = parent.status;
        }
    }

private:
    gchar *buffer;
};

// also used for SVGNumber (just with unit SVG_LENGTHTYPE_UNSPECIFIED)
class SPILengthOrNumber : public SPIXorEnum {
public:
    SPILengthOrNumber(CSSAttribute attr) :
        SVGLength(0)
    {
        switch (prop) {
            case SP_PROP_STROKE_OPACITY:
            case SP_PROP_OPACITY:
            case SP_PROP_FILL_OPACITY:
            case SP_PROP_STROKE_WIDTH:
                length = SVGLength(1);
                break;
            // case SP_PROP_DASH_OFFSET:
                // default length stays 0
            //    break;
            case SP_PROP_LETTER_SPACING:
            case SP_PROP_WORD_SPACING:
                value = NORMAL;
                enumList = {{"normal", NORMAL}};
            case SP_PROP_KERNING:
                value = AUTO;
                enumList = {{"auto", AUTO}};
                break;
        }
    }

    gchar * toString()
    {
        return length.getValueAsString();
    }

    void readFromString(gchar *str)
    {
        length.setValueAsString(str);

        // check if unit is unspecified when data type is actually SVGNumber
        if (length.getUnit() != SVG_LENGTHTYPE_UNSPECIFIED && (
            prop == SP_PROP_STROKE_OPACITY ||
            prop == SP_PROP_FILL_OPACITY ||
            prop == SP_PROP_OPACITY))
        {
            // invalid type
            status = STATUS_VALUE_UNSET;
            return;
        }

        // clamping to range 0...1, according to w3c
        if (prop == SP_PROP_STROKE_OPACITY ||
            prop == SP_PROP_FILL_OPACITY ||
            prop == SP_PROP_OPACITY)
        {
            if (length.getValue() > 1) {
                length.setValue(1);
            }
            else if (length.getValue() < 0) {
                length.setValue(0);
            }
        }

        status = STATUS_VALUE_SET;
    }

    // TODO maybe tostring and tooptimizedstring?

    // TODO fix this
    void mergeFromDyingParent(const SPILength &parent, const double parent_child_em_ratio)
    {
        if ((status == STATUS_VALUE_SET || status == STATUS_INHERIT)
             && parent.status == STATUS_VALUE_SET && parent.status != STATUS_INHERIT)
        {
            length = parent.length;
            status = parent.status; // correct?
            switch (parent.getValue().getUnit()) {
                case SVGLength::SVG_LENGTHTYPE_EM:
                case SVGLength::SVG_LENGTHTYPE_EX:
                    length.setValue(length.getValue() * parent_child_em_ratio);
                    /** \todo
                     * fixme: Have separate ex ratio parameter.
                     * Get x height from libnrtype or pango.
                     */
                    if (!IS_FINITE(length.getValue())) {
                        length.setValue(computedLength.getValue());
                    }
                    break;
            }
    }

    SVGLength length;
    SVGLength computedLength;
private:
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
        textDecoration(TEXT_DECORATION_NONE)
    {}

    gchar * toString()
    {
        Inkscape::CSSOStringStream os;
        if (textDecoration & TEXT_DECORATION_INHERIT) {
            os << "inherit";
        }
        else if {
            if (textDecoration & TEXT_DECORATION_UNDERLINE) {
                os << " underline";
            }
            if (textDecoration & TEXT_DECORATION_OVERLINE) {
                os << " overline";
            }
            if (textDecoration & TEXT_DECORATION_LINE_THROUGH) {
                os << " line-through";
            }
        }

        return os.str().c_str();
    }

    void readFromString(const gchar *str)
    {
        if (strcmp(str, "inherit") == 0) {
            status = STATUS_INHERIT;
        }
        else if (strcmp(str, "none") == 0) {
            textDecoration = TEXT_DECORATION_NONE;
        }
        else {
            textDecoration = TEXT_DECORATION_UNSET;
            while (*str != '\0') {
                if (*str == ' ') {
                    continue;
                }
                if (strneq(str, "underline", 9) && (str[9] == ' ' || str[9] == '\0')) {
                    textDecoration |= TEXT_DECORATION_UNDERLINE;
                    str += 9;
                }
                else if (strneq(str, "overline", 8) && (str[8] == ' ' || str[8] == '\0')) {
                    textDecoration |= TEXT_DECORATION_OVERLINE;
                    str += 8;
                }
                else if (strneq(str, "line-through", 12) && (str[12] == ' ' || str[12] == '\0')) {
                    textDecoration |= TEXT_DECORATION_LINE_THROUGH;
                    str += 12;
                }
                else {
                    status = STATUS_UNSET;
                    return;  // invalid value
                }
                ++str;
            }
        }
    }
};

class AttributeSplitter {
public:
    // Free the return value with freeAttributeSplit.
    static std::vector<gchar *> splitAttributeString(gchar *str, bool commaIsSeparator)
    {
        // split after comma or whitespace
        // tokStartIndex: start position of the split item
        // if -1, we currently have separators
        unsigned int tokStartIndex = 0;
        std::vector<gchar *> result;
        for (unsigned int i = 0; str[i] != '\0'; ++i) {
            if (str[i] == ' ' || (commaIsSeparator == true && str[i] == ',') ||
                str[i] == '\n' || str[i] == '\r' || str[i] == '\t') {
                if (tokStartIndex == 0) {
                    // leading whitespaces or so
                    continue;
                }

                // true, if we this is the first separator character
                if (tokStartIndex != -1) {
                    gchar *split = g_malloc0(i - tokStartIndex + 2);
                    g_utf8_strncpy(output, &str[tokStartIndex], i - tokStartIndex + 1);
                    result.push_back(split);
                    tokStartIndex = -1;
                }
            }
            else {
                tokStartIndex = i;
            }
        }
        return result;
    }

    static void freeAttributeSplit(std::vector<gchar *> split)
    {
        for (int i = 0; i < split.size(); ++i) {
            g_free(split[i]);
        }
    }
};

class SPIStrokeDashArray : public SPIXorEnum {
public:
    gchar * toString()
    {
        Inkscape::CSSOStringStream os;
        unsigned long noi = dashes.getNumberOfItems();
        for (unsigned long i = 0; i < noi; ++i) {
            os << dashes.getItem(i).getValueAsString();
            if (i != noi - 1) {
                os << ",";
            }
        }
        return os.str().c_str();
    }

    void readFromString(gchar *str)
    {
        std::vector<gchar *> split = AttributeSplitter::splitAttributeString(str);
        for (unsigned int i = 0; i < split.size(); ++i) {
            SVGLength *item = new SVGLength();
            item.setValueAsString(split[i]);
            dashes.appendItem(const_cast<const SVGLength>(item));
        }
        AttributeSplitter::freeAttributeSplit(split);
    }

    ~SPIStrokeDashArray()
    {
        for (unsigned int i = 0; i < dashes.getNumberOfItems(); ++i) {
            delete dashes.getItem(i);
        }
        delete dashes;
    }

private:
    SVGLengthList dashes;
}

class SPTextStyle;

/// An SVG style object.
struct SPStyle {
    int refcount;

    /** Object we are attached to */
    SPObject *object;

    /** Document we are associated with */
    SPDocument *document;

    /** Our text style component */
    SPTextStyle *text;
    unsigned text_private : 1;

    /** Size of the font */
    SPIFontSize font_size;

    /** text decoration (css2 16.3.1) */
    SPITextDecoration text_decoration;

    /** Baseline shift (svg1.1 10.9.2) */
    SPIBaselineShift baseline_shift;

    SPIDashArray stroke_dasharray;

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

    SPStyle(SPDocument *doc) :
        cloned(false),
        object(NULL),
        document(doc)
    {
        refcount = 1;
        for (unsigned int i = 0; i < G_N_ELEMENTS(lengthProperties); ++i) {
            literal[lengthProperties[i]] = SPILength(lengthProperties[i]);
        }

        for (unsigned int i = 0; i < G_N_ELEMENTS(literalProperties); ++i) {
            length[literProperties[i]] = SPIXorEnum(literalProperties[i]);
        }
    }

    SPStyle(SPObject *obj)
    {
        SPStyle(obj->document);
        object = object;
        release_connection = obj->connectRelease(sgc::bind<1>(sigc::ptr_fun(&objectRelease), style));
        if (object->cloned == true) {
            style->cloned = true;
        }
    }

    void readFromObject() {
        Inkscape::XML::Node *repr = object->getRepr();
        readStyle(repr);
    }

    void readStyle(Inkscape::XML::Node *repr)
    {
        for (unsigned int i = 0; i < G_N_ELEMENTS(lengthProperties); ++i) {
            if (literal[lengthProperties[i]].status == STATUS_UNSET) {
                literal[lengthProperties[i]].readFromNode(repr);
            }
        }

        for (unsigned int i = 0; i < G_N_ELEMENTS(literalProperties); ++i) {
            if (literal[lengthProperties[i]].status == STATUS_UNSET) {
                length[literProperties[i]].readFromNode(repr);
            }
        }

        if (fill.status == STATUS_UNSET) {
            fill.readFromNode(repr);
        }

        if (color.status == STATUS_UNSET) {
            color.readFromNode(repr);
        }

        if (stroke.status == STATUS_UNSET) {
            stroke.readFromNode(repr);
        }

        if (text_decoration.status == STATUS_UNSET) {
            text_decoration.readFromNode(repr);
        }

        if (baseline_shift.status == STATUS_UNSET) {
            baseline_shift.readFromNode(repr);
        }

        if (font_size.status == STATUS_UNSET) {
            font_size.readFromNode(repr);
        }

        if (marker_start.status == STATUS_UNSET) {
            marker_start.readFromNode(repr);
        }

        if (marker_mid.status == STATUS_UNSET) {
            marker_mid.readFromNode(repr);
        }

        if (marker_end.status == STATUS_UNSET) {
            marker_end.readFromNode(repr);
        }

        if (stroke_dasharray.status == STATUS_UNSET) {
            stroke_dasharray.readFromNode(repr);
        }
    }

// TODO parameter for STATUS_UNSET

    void mergeProperty(CSSAttribute prop, const gchar *val)
    {
        for (unsigned int i = 0; i < G_N_ELEMENTS(lengthProperties); ++i) {
            if (prop == lengthProperties[i]) {
                // prop is a length!
                length[prop].readFromString(val);
                return;
            }
        }

        for (unsigned int i = 0; i < G_N_ELEMENTS(literalProperties); ++i) {
            if (prop == literal[i]) {
                literal[prop].readFromString(val);
                return;
            }
        }

        switch (prop) {
            case SP_PROP_STROKE_DASHARRAY:
                stroke_dasharray.readFromString(val, true);
                break;
            case SP_PROP_FONT_SIZE:
                font_size.readFromString(val, true);
                break;
            case SP_PROP_COLOR:
                color.readFromString(val, true);
                break;
            case SP_PROP_FILL:
                fill.readFromString(val, true);
                break;
            case SP_PROP_STROKE:
                stroke.readFromString(val, true);
                break;
            case SP_PROP_MARKER_START:
                marker_start.readFromString(val, true):
                break;
            case SP_PROP_MARKER_MID:
                marker_mid.readFromString(val, true);
                break;
            case SP_PROP_MARKER_END:
                marker_end.readFromString(val, true);
                break;
            case SP_PROP_FONT_FONT_FAMILY:
                font_family.readFromString(val, true);
                break;
            default:
                g_warning("Unimplemented style property with id %d", val);
        }

    }

    void mergeFromStyleString(const gchar *const p)
    {
        CRDeclaration *const decl_list =
            cr_declaration_parse_list_from_bug(reinterpret_cast<guchar const *>(p), CR_UTF_B);
        if (decl_list != NULL) {
            mergeFromDeclList(decl_list);
            cr_declaration_destroy(decl_list);
        }
    }

    void mergeFromDeclList(const CRDeclaration *const decl_list)
    {
        // read the decls from end to start, using head recursion, so that latter declarations override
        // (Ref: http://www.w3.org/TR/REC-CSS2/cascade.html#cascading-order point 4.)
        // because sp_style_merge_style_from_decl only sets properties that are unset
        if (decl_list->next != NULL) {
           mergeFromDeclList(decl_list->next);
        }
        mergeFromDecl(decl_list);
    }

    void mergeFromDeclList(const CRDeclaration *const decl)
    {
        /** \todo Ensure that property is lcased, as per
         * http://www.w3.org/TR/REC-CSS2/syndata.html#q4.
         * Should probably be done in libcroco.
         */
        unsigned const prop_idx = sp_attribute_lookup(decl->property->stryng->str);
        if (prop_idx != SP_ATTR_INVALID) {
            /** \todo
             * effic: Test whether the property is already set before trying to
             * convert to string. Alternatively, set from CRTerm directly rather
             * than converting to string.
             */
            guchar *const str_value_unsigned = cr_term_to_string(decl->value);
            gchar *const str_value = reinterpret_cast<gchar *>(str_value_unsigned);
            mergeProperty(prop_idx, str_value);
            g_free(str_value);
        }
    }

    void mergeFromProps(CRPropList props)
    {
        if (props != NULL) {
            mergeFromProps(cr_prop_list_get_next(props));
            CRDeclaration *decl = NULL;
            cr_prop_list_get_decl(props, &decl);
            mergeFromDecl(decl);
        }
    }

    void mergeFromObjectStylesheet(const SPObject *const object)
    {
        static CRSelEng *sel_eng // TODO static?! change this
        if (sel_eng == NULL) {
            sel_end = sp_repr_sel_eng();
        }

        // XML Tree being directly used here while it shouldn't be.
        CRStatus status = cr_sel_eng_get_matched_properties_from_cascade(sel_eng,
                                                                         object->document->style_cascade,
                                                                         object->getRepr(),
                                                                         &props);
        g_return_if_fail(status == CR_OK);
        /// \todo Check what errors can occur, and handle them properly.
        if (props) {
            sp_style_merge_from_props(style, props);
            cr_prop_list_destroy(props);
        }
        RPropList *props = NULL;
    }

    void readFromPrefs(const Glib::ustring &path)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        // not optimal: we reconstruct the node based on the prefs, then pass it to
        // sp_style_read for actual processing.
        Inkscape::XML::SimpleDocument *tempdoc = new Inkscape::XML::SimpleDocument;
        Inkscape::XML::Node *tempnode = tempdoc->createElement("temp");

        std::vector<Inkscape::Preferences::Entry> attrs = prefs->getAllEntries(path);
        for (std::vector<Inkscape::Preferences::Entry>::iterator i = attrs.begin(); i != attrs.end(); ++i) {
            tempnode->setAttribute(i->getEntryName().data(), i->getString().data());
        }

// object = NULL to bool
        readStyle(tempnode);

        Inkscape::GC::release(tempnode);
        Inkscape::GC::release(tempdoc);
        delete tempdoc;
    }

    void ref()
    {
        ++refcount;
    }

    // TODO remove this, since hardly used
    void unref()
    {
        --refcount;
        if (refcount < 1) {
            ~SPStyle();
        }
    }

    /** color */
    SPIPaint color;

    /** fill */
    SPIPaint fill;

    /** stroke */
    SPIPaint stroke;

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

    SPObject *getFilter() { return (filter.href) ? filter.href->getObject() : 0; }
    SPObject const *getFilter() const { return (filter.href) ? filter.href->getObject() : 0; }
    gchar const *getFilterURI() const { return (filter.href) ? filter.href->getURI()->toString() : 0; }

    SPPaintServer *getFillPaintServer() { return (fill.value.href) ? fill.value.href->getObject() : 0; }
    SPPaintServer const *getFillPaintServer() const { return (fill.value.href) ? fill.value.href->getObject() : 0; }
    gchar const *getFillURI() const { return (fill.value.href) ? fill.value.href->getURI()->toString() : 0; }

    SPPaintServer *getStrokePaintServer() { return (stroke.value.href) ? stroke.value.href->getObject() : 0; }
    SPPaintServer const *getStrokePaintServer() const { return (stroke.value.href) ? stroke.value.href->getObject() : 0; }
    gchar const  *getStrokeURI() const { return (stroke.value.href) ? stroke.value.href->getURI()->toString() : 0; }
};

SPStyle *sp_style_new(SPDocument *document);

SPStyle *sp_style_new_from_object(SPObject *object);

SPStyle *sp_style_ref(SPStyle *style);

SPStyle *sp_style_unref(SPStyle *style);

void sp_style_read_from_object(SPStyle *style, SPObject *object);

void sp_style_read_from_prefs(SPStyle *style, Glib::ustring const &path);

void sp_style_merge_from_style_string(SPStyle *style, gchar const *p);

void sp_style_merge_from_parent(SPStyle *style, SPStyle const *parent);

void sp_style_merge_from_dying_parent(SPStyle *style, SPStyle const *parent);

gchar *sp_style_write_string(SPStyle const *style, guint flags = SP_STYLE_FLAG_IFSET);

gchar *sp_style_write_difference(SPStyle const *from, SPStyle const *to);

void sp_style_set_to_uri_string (SPStyle *style, bool isfill, const gchar *uri);

/* SPTextStyle */

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

/// An SPTextStyle has a refcount, a font family, and a font name.
struct SPTextStyle {
    int refcount;

    /* CSS font properties */
    SPIString font_family;

    /* Full font name, as font_factory::ConstructFontSpecification would give */
    SPIString font_specification;

    /** \todo fixme: The 'font' property is ugly, and not working (lauris) */
    SPIString font;
};

class SPTextStyle : public SPIXorEnum {
public:
    // TODO possible to merge 3 strdup to one?
    SPTextStyle() :
        font_family(g_strdup("Sans")),
        font_specification(g_strdup("Sans")),
        font(g_strdup("Sans"))
        refcount(0) // 0 or 1?
    {
        // no, separated
    }

private:
    gchar *font_family;
    gchar *font_specification;
    gchar* font;
}

SPCSSAttr *sp_css_attr_from_style (SPStyle const *const style, guint flags);
SPCSSAttr *sp_css_attr_from_object(SPObject *object, guint flags = SP_STYLE_FLAG_IFSET);
SPCSSAttr *sp_css_attr_unset_text(SPCSSAttr *css);
SPCSSAttr *sp_css_attr_unset_uris(SPCSSAttr *css);
SPCSSAttr *sp_css_attr_scale(SPCSSAttr *css, double ex);

void sp_style_unset_property_attrs(SPObject *o);

void sp_style_set_property_url (SPObject *item, gchar const *property, SPObject *linked, bool recursive);

