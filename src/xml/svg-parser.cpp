/*
 * New SAX parser for svg files.
 *
 * Authors:
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg-parser.h"
#include "node.h"
#include "document.h"

namespace Inkscape {

namespace XML {

SVGParser::SVGParser(): xmlpp::SaxParser(), _doc(nullptr)
{
}

SVGParser::~SVGParser()
{
}

void SVGParser::on_start_document()
{
}

void SVGParser::on_end_document()
{
}

void SVGParser::on_start_element(const Glib::ustring& name, const AttributeList& attributes)
{
}

void SVGParser::on_end_element(const Glib::ustring& /* name */)
{
}

void SVGParser::on_characters(const Glib::ustring& text)
{
}

void SVGParser::on_comment(const Glib::ustring& text)
{
}

void SVGParser::on_warning(const Glib::ustring& text)
{
}

void SVGParser::on_error(const Glib::ustring& text)
{
}

void SVGParser::on_fatal_error(const Glib::ustring& text)
{
}

void SVGParser::on_cdata_block(const Glib::ustring &text)
{

}

Document *SVGParser::getDocument() {
    return _doc;
}

}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
