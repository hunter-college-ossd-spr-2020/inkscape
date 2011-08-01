#ifndef SEEN_SP_STYLE_H
#define SEEN_SP_STYLE_H

/** \file
* SPStyle - a style object for SPItem objects. For method documentation, see style.h.
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

SPIPaint::SPIPaint() :
    href(NULL),
{
    literalList = {
        {"none", NONE},
        {"currentColor", SP_PAINT_CURRENTCOLOR}
    };
}

SPIPaint::~SPIPaint()
{
    if (href != NULL) {
        delete href;
    }
}

void SPIPaint::readFromString(gchar *str)
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

void SPIPaint::mergeFromDyingParent(const SPIPaint &parent)
{
    if (status == STATUS_UNSET || status == STATUS_INHERIT) {
        mergeFromParent(&parent);
        status = parent.status;
    }
}

void SPIPaint::setToUri(const Inkscape::URI *uri)
{
    // it may be that this style's SPIPaint has not yet created its URIReference;
    // now that we have a document, we can create it here
    if (href == NULL && document != NULL) {
        href = new SPPaintServerReference(document);
    }

    if (href != NULL && paint->value.href->getObject() != NULL) {
        href->detach();
    }

    if (href != NULL) {
        try {
            href->attach(*uri);
        }
        catch (Inkscape::BadURIException &e) {
            g_warning("%s", e.what());
            href->detach();
        }
    }
}

void SPIPaint::mergeFromParent(const SPIPaint &parent)
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

char * SPIPaint::toString()
{
    char * baseTs = SPiXorEnum::toString();
    if (baseTs[0] != '\0') {
        return baseTs;
    }
    std::stringstream stream;
    
}

// TODO sigc connection

bool SPIPaint::operator !=(const SPIPaint &b)
{
    if (a->isColor() != b->isColor()
        || a->isPaintserver() != b->isPaintserver()
        || a->status != b->status
        || (status == STATUS_LITERAL_SET && a->literalValue != b->literalValue))
    {
        return true;
    }
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

bool isSameType( SPIPaint const & other ) const {return (isPaintserver() == other.isPaintserver()) && (colorSet == other.colorSet) && (currentcolor == other.currentcolor);}

bool isNone() const {return !currentcolor && !colorSet && !isPaintserver();} // TODO refine
bool isColor() const {return colorSet && !isPaintserver();}
bool isPaintserver() const {return value.href && value.href->getObject();}

void clear();

void SsetColor( float r, float g, float b ) {value.color.set( r, g, b ); colorSet = true;}
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

/* SPIFilter */

SPIFilter::SPIFilter() :
    SPFilterReference(NULL),
    literalValue(NONE)
{
    literalList = {{"none", NONE}};
}

gchar * SPIFilter::toString()
{
    gchar *baseTs = SPIXorEnum::toString();
    if (baseTs[0] != '\0') {
        return baseTs;
    }
    return g_strdup_printf("url(%s)", href->getURI()->toString());
}

void SPIFilter::readFromString(gchar *str)
{
    SPIXorEnum::readFromString();
    if (status != STATUS_UNSET) {
        return;
    }

    if (href != NULL && href->getObject() != NULL) {
        href->detach();
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

/* SPIXorEnum */

SPIXorEnum::SPIXorEnum(CSSAttribute attr) :
    property(attr),
    status(ENUM_UNSET)
{
    switch (attr) {
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
        case SP_PROP_FONT_STYLE:
            literalList = {
                {"normal", SP_CSS_FONT_STYLE_NORMAL},
                {"italic", SP_CSS_FONT_STYLE_ITALIC},
                {"oblique", SP_CSS_FONT_STYLE_OBLIQUE}
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

virtual gchar * SPIXorEnum::toString()
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

virtual void SPIXorEnum::setLiteralValue(int literal)
{
    literalValue = literal;
}

virtual void SPIXorEnum::readFromString(gchar *str)
{
    else if (strcmp(str, "inherit") == 0) {
        status = STATUS_INHERIT;
    }
    else {
        for (unsigned int i = 0; i < G_N_ELEMENTS(literalList); ++i) {
            if (strcmp(literalList[i][0] == str) == 0) {
                status = STATUS_LITERAL_SET;
                setLiteralValue(literalList[i][1]);
                return;
            }
        }
        status = STATUS_UNSET;
        literalValue = defaultValue;
    }
}

virtual void SPIXorEnum::inheritFrom(SPIXorEnum *parent)
{
    if (parent->status == STATUS_LITERAL_SET || parent->status == STATUS_INHERIT) {
        computedValue = parent.getComputedValue();
    }
}

SPIString::SPIString(CSSAttribute attr) :
    property(attr),
    buffer(NULL)
{}

SPIString::~SPIString()
{
    if (buffer != NULL) {
        g_free(buffer);
    }
}

gchar * SPIString::toString()
{
    return buffer;
}

void SPIString::readFromString(gchar *str)
{
    if (buffer != NULL) {
        delete buffer;
    }
    buffer = g_strdup(str);
}

void SPIString::mergeFromDyingParent(SPIString *parent)
{
    if (status == STATUS_UNSET || status == STATUS_INHERIT) {
        g_free(buffer);
        buffer = g_strdup(parent.toString());
        status = parent.status;
    }
}

void SPIString::mergeFromParent(SPIString *parent)
{
    if ((status == STATUS_UNSET || status == STATUS_INHERIT)
        && parent->status != STATUS_UNSET &&
        parent->status != STATUS_INHERIT)
    {
        g_free(buffer);
        buffer = g_strdup(parent.toString());
        status = parent.status;
    }
}

/* SPILengthOrNumber */

SPILengthOrNumber(CSSAttribute attr) :
    property(attr),
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
            literalValue = NORMAL;
            enumList = {{"normal", NORMAL}};
            break;
        case SP_PROP_KERNING:
            literalValue = AUTO;
            enumList = {{"auto", AUTO}};
            break;
        case SP_PROP_BASELINE_SHIFT:
            literalList = {
                {"baseline", SP_CSS_BASELINE_SHIFT_BASELINE},
                {"sub", SP_CSS_BASELINE_SHIFT_SUB},
                {"super", SP_CSS_BASELINE_SHIFT_SUPER}
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
    }
}

char * SPILengthOrNumber::toString()
{
    char *baseTs = SPIXorEnum::toString():
    if (baseTs[0] != '\0') {
        return baseTs;
    }
    if (property == SP_PROP_FONT_SIZE && length.getUnit() == SVGLength::SVG_LENGTHTYPE_UNSPECIFIED) {
        // must specify px, see inkscape bug 1221626, mozilla bug 234789
        length.convertToSpecifiedUnits(SVGLength::SVG_LENGTHTYPE_PX);
    }
    return length.getValueAsString();
}

void SPILengthOrNumber::setLiteralValue(int literal)
{
    literalValue = literal;
    status = STATUS_LITERAL_SET;
    if (property == SP_PROP_BASELINE_SHIFT) {
        switch (literal) {
            case SP_CSS_BASELINE_SHIFT_BASELINE:
                computedValue.setValue(0);
                break;
            case SP_CSS_BASELINE_SHIFT_SUB:
                computedValue.setValue(-0.2 * pfont_size.getComputedValue());
                break;
            case SP_CSS_BASELINE_SHIFT_SUPER:
                computedValue.setValue(0.4 * pfont_value.getComputedValue());
                break;
    }
    else if (property == SP_PROP_FONT_FONT_SIZE) {
        switch (literal) {
            case SP_FONT_SIZE_XX_SMALL:
                computedLength.setValue(6.0);
                break;
            case SP_FONT_SIZE_X_SMALL:
                computedLength.setValue(8.0);
                break;
            case SP_FONT_SIZE_SMALL:
                computedLength.setValue(10.0);
                break;
            case SP_FONT_SIZE_MEDIUM:
                computedLength.setValue(12.0);
                break;
            case SP_FONT_SIZE_LARGE:
                computedLength.setValue(14.0);
                break;
            case SP_FONT_SIZE_X_LARGE:
                computedLength.setValue(18.0);
                break;
            case SP_FONT_SIZE_XX_LARGE:
                computedLength.setValue(24.0);
                break;
            case SP_FONT_SIZE_SMALLER:
                // "In CSS2, the suggested scaling factor for a computer screen
                // between adjacent indexes was 1.2"
                computedLength.setValue(parent.getComputedLength() / 1.2);
                break;
            case SP_FONT_SIZE_LARGER:
                computedLength.setValue(parent.getComputedValue() * 1.2);
                break;
        }
    }
}

void SPILengthOrNumber::readFromString(char *str)
{
    SPIXorEnum::readFromString(str);
    if (status != STATUS_UNSET) {
        // was a literal value
        }
        return;
    }

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

    // clamp to range 0...1, according to w3c
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

    // set computedLength
    mergeFromParent();
}

double SPILengthOrNumber::getComputedValue()
{
    return computedLength.getValue();
}

void SPILengthOrNumber::mergeFromParent(SPILength *parent)
{
    if (status == STATUS_UNSET || status == STATUS_INHERIT) {
        literalValue = parent->getLiteralValue();
        computedLength = parent->getComputedLength();
    }

    if (status != STATUS_VALUE_SET) {
        return;
    }

    if (isRelative(length.getUnit())) {
        switch (length.getUnit()) {
            case SVGLength::SVG_LENGTHTYPE_PERCENT:
                computedValue =
                break;
            case SVGLength::SVG_LENGTHTYPE_EMS:
                computedValue = parent->getComputedValue() * pxPerEm;
                break;
            case SVGLength::SVG_LENGTHTYPE_EXS:
                computedValue = parent->getComputedValue() * pxPerExc;
                break;
        }
    }
    else {
        computedLength = parent.getComputedLength();
    }
}

// TODO maybe tostring and tooptimizedstring?

// TODO fix this
void SPILengthOrNumber::mergeFromDyingParent(SPIStyle *style, SPILength *parent)
{
    if (parent->status == STATUS_SET && parent->status != STATUS_INHERIT) {
        if (status != STATUS_UNSET || status == STATUS_INHERIT) {
            length = parent->getComputedValue();
        }
        else if (length.getUnit() == SVGLength::SVG_LENGTHTYPE_EM ||
                 length.getUnit() == SVGLength::SVG_LENGTHTYPE_EX ||
                 length.getUnit() == SVGLength::SVG_LENGTHTYPE_PERCENTAGE)
        {
            status = STATUS_VALUE_SET;
            computedLength.setValue(parent->length * getRelativeFontSizeFrac());
        }
        else if ()
        // * Parent has relative 
    }
}

double SPILengthOrNumber::getRelativeFontSizeFrac()
{
    if (status == STATUS_LITERAL_SET) {
        switch (length.getUnit()) {
            case SP_CSS_FONT_SIZE_SMALLER:
                return 1.2;
            case SP_CSS_FONT_SIZE_LARGER:
                return 1/1.2;
        }
    }

    switch (length.getUnit()) {
        case SVGLength::SVG_LENGTHTYPE_PERCENTAGE:
            return length.getValue() / 100;
        case SVGLength::SVG_LENGTHTYPE_EM:
            return length.getValue();
        case SVGLength::SVG_LENGTHTYPE_EX:
            // TODO get ex size from pango
            return length.getValue() * 0.5;
    }
}

SPITextDecoration::SPITextDecoration() :
    property(SP_PROP_TEXT_DECORATION),
    textDecoration(TEXT_DECORATION_NONE)
{}

char * SPITextDecoration::toString()
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

void SPITextDecoration::readFromString(const gchar *str)
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

static AttributeSplitter::std::vector<gchar *> splitAttributeString(gchar *str, bool commaIsSeparator)
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

static void AttributeSplitter::freeAttributeSplit(std::vector<gchar *> split)
{
    for (int i = 0; i < split.size(); ++i) {
        g_free(split[i]);
    }
}

gchar * SPIStrokeDashArray::toString()
{
    Inkscape::CSSOStringStream os;
    unsigned long noi = dashes.getNumberOfItems();
    for (unsigned long i = 0; i < noi; ++i) {
        os << dashes.getItem(i).getValueAsString();
        if (i != noi - 1) {
            os << ',';
        }
    }
    return os.str().c_str();
}

double * SPIStrokeDashArray::getDashArray()
{
    int noe = dashes.getNumberOfElements();
    double *result = new double[noe];
    for (int i = 0; i < noe; ++i) {
        result[i] = dashes[i].getComputedLength();
    }
    return result;
}

void SPIStrokeDashArray::readFromString(gchar *str)
{
    std::vector<gchar *> split = AttributeSplitter::splitAttributeString(str);
    for (unsigned int i = 0; i < split.size(); ++i) {
        SPILength *item = new SPILength();
        item.readFromString(split[i]);
        dashes.push_back(item);
    }
    AttributeSplitter::freeAttributeSplit(split);
}

void SPIStrokeDashArray::mergeFromParent(SPIStyle *parent)
{
    for (unsigned int i = 0; i < split.size(); ++i) {
        dashes[i].mergeFromParent(parent);
    }
}

void SPIStrokeDashArray::mergeFromDyingParent(SPIStyle *parent)
{
    for (unsigned int i = 0; i < split.size(); ++i) {
        dashes[i].mergeFromDyingParent(parent);
    }
}

SPIStrokeDashArray::~SPIStrokeDashArray()
{
    for (unsigned int i = 0; i < dashes.getNumberOfItems(); ++i) {
        delete dashes.getItem(i);
    }
    delete dashes;
}

/* SPStyle */

SPStyle::SPStyle(SPDocument *doc) :
    cloned(false),
    object(NULL),
    document(doc)
{
    // use pointer to avoid object slicing
    base = { &fill_opacity, &stroke_opacity, };
    for (unsigned int i = 0; i < G_N_ELEMENTS(lengthProperties); ++i) {
        literal[lengthProperties[i]] = SPILength(lengthProperties[i]);
    }

    for (unsigned int i = 0; i < G_N_ELEMENTS(literalProperties); ++i) {
        length[literProperties[i]] = SPIXorEnum(literalProperties[i]);
    }
}

SPStyle::SPStyle(SPObject *obj)
{
    SPStyle(obj->document);
    object = object;
    release_connection = obj->connectRelease(sgc::bind<1>(sigc::ptr_fun(&objectRelease), style));
    if (object->cloned == true) {
        style->cloned = true;
    }
}

void SPStyle::readFromObject()
{
    Inkscape::XML::Node *repr = object->getRepr();
    readStyle(repr);
}

void SPStyle::readStyle(Inkscape::XML::Node *repr)
{
    for (int i = 0; i < G_N_ELEMENTS(base); ++i) {
        if (base[i].status == STATUS_UNSET) {
            const char *name = sp_attribute_name(base[i].property);
            char *value = repr->attribute(name);
            base[i].readFromString(value);
        }
    }
}

void SPStyle::mergeFromParent(const SPStyle *parent)
{
    for (int i = 0; i < G_N_ELEMENTS(base); ++i) {
        if (base[i]->status == STATUS_UNSET || length[prop].status == STATUS_INHERIT) {
            base[i]->mergeFromParent(parent);
        }
    }
}

void SPStyle::mergeFromDyingParent(const SPStyle *parent)
{
    for (int i = 0; i < G_N_ELEMENTS(base); ++i) {
        if (base[i]->status == STATUS_UNSET || length[prop].status == STATUS_INHERIT) {
            base[i].mergeFromDyingParent(parent);
        }
    }
}

void SPStyle::mergeProperty(CSSAttribute prop, const gchar *val)
{
    for (int i = 0; i < G_N_ELEMENTS(base); ++i) {
        if (base[i].prop == prop) {
            prop.readFromString(val, true);
        }
    }
    g_warning("Unimplemented style property with id %d", static_cast<int>(prop));
}

void SPStyle::mergeFromStyleString(const gchar *const p)
{
    CRDeclaration *const decl_list =
        cr_declaration_parse_list_from_bug(reinterpret_cast<guchar const *>(p), CR_UTF_B);
    if (decl_list != NULL) {
        mergeFromDeclList(decl_list);
        cr_declaration_destroy(decl_list);
    }
}

void SPStyle::mergeFromDeclList(const CRDeclaration *const decl_list)
{
    // read the decls from end to start, using head recursion, so that latter declarations override
    // (Ref: http://www.w3.org/TR/REC-CSS2/cascade.html#cascading-order point 4.)
    // because sp_style_merge_style_from_decl only sets properties that are unset
    if (decl_list->next != NULL) {
       mergeFromDeclList(decl_list->next);
    }
    mergeFromDecl(decl_list);
}

void SPStyle::mergeFromDeclList(const CRDeclaration *const decl)
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

void SPStyle::mergeFromProps(CRPropList props)
{
    if (props != NULL) {
        mergeFromProps(cr_prop_list_get_next(props));
        CRDeclaration *decl = NULL;
        cr_prop_list_get_decl(props, &decl);
        mergeFromDecl(decl);
    }
}
// TODO important! use cssostringstream in in svglengh since it provides precision handling

char * SPStyle::toString()
{
    ostringstream result;
    SPIXorEnum *base = ...;
    for (int i = 0; i < G_N_ELEMENTS(base); ++i) {
        const gchar *name = sp_attribute_name(base[i]->property);
        char *value = base[i].toString();
        // TODO replace g_strdup_printf because we do not need gchar, but char
        gchar *propstr = g_strdup_printf("%s%s:%s;", result, name, value);
        result << propstr;
        g_free(value);
        g_free(propstr);
    }
    return result.str().c_str();
}

void SPStyle::mergeFromObjectStylesheet(const SPObject *const object)
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

void SPStyle::readFromPrefs(const Glib::ustring &path)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    std::vector<Inkscape::Preferences::Entry> attrs = prefs->getAllEntries(path);
    for (std::vector<Inkscape::Preferences::Entry>::iterator i = attrs.begin(); i != attrs.end(); ++i) {
        int prop = sp_attribute_lookup(i->getEntryName().data());
        mergeProperty(prop, i->getString().data());
    }
}

void SPStyle::paintServerRefChanged(SPObject *oldref, SPObject *ref, sigc::connection *ps)
{
    if (oldref != NULL) {
        ps->disconnect();
    :w}
    if (SP_IS_PAINT_SERVER(ref)) {
        ps = ref->connectModified(sigc::bind(sigc::ptf_fun(&SPStyle::paintServerRefModified)));
    }
    paintServerRefModified(ref, NULL);
}

void SPStyle::filterRefModified(SPObject *obj, guint flags)
{
    SPFilter *filter = static_cast<SPFilter *>(obj);
    if (getFilter() == filter) {
        if (object != NULL) {
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    }
}

void SPStyle::filterRefChanged(SPObject *oldref, SPObject *ref)
{
    if (oldref != NULL) {
        filter_modified_connection
    }
}

void SPStyle::paintServerRefModified(SPObject *ref)
{
    SPPaintServer *server = static_cast<SPPaintServer *>(obj);
    if (fill.isPaintserver() == true) {
        if (object != NULL) {
            /** TODO
             * fixme: I do not know, whether it is optimal - we are
             * forcing reread of everything (Lauris)
             */
            /** TODO
             * fixme: We have to use object_modified flag, because parent
             * flag is only available downstreams.
             */
            object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    }
    else if (stroke.isPaintServer() == true && getStrokePaintserver == server) {
        object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
    }
    else if (server != NULL) {
        g_assert_not_reached();
    }
}

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

