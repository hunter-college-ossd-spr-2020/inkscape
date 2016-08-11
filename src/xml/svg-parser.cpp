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
#include "document.h"
#include "simple-document.h"

namespace Inkscape {

namespace XML {

SVGParser::SVGParser(): xmlpp::SaxParser(true), _doc(nullptr)
{
    // TODO what about entities
    set_substitute_entities(true);
}

SVGParser::~SVGParser()
{
}

void SVGParser::on_start_document()
{
    _doc = new SimpleDocument();
}

void SVGParser::on_end_document()
{
}

void SVGParser::on_start_element(const Glib::ustring& name,
                                 const AttributeList& attributes)
{
    Node* element = _doc->createElement(name.c_str());
    for (auto &attr: attributes) {
        element->setAttribute(attr.name, attr.value);
    }
    if (_context.empty()) {
        _doc->appendChild(element);
    } else {
        _context.top()->appendChild(element);
    }
    _context.push(element);
    Inkscape::GC::release(element);
}

void SVGParser::on_end_element(const Glib::ustring& name )
{
    _context.pop();
}

void SVGParser::on_characters(const Glib::ustring& text)
{
    if (!_context.empty()) {
        Node *t = _doc->createTextNode(text.c_str());
        _context.top()->appendChild(t);
        Inkscape::GC::release(t);
    }
}

void SVGParser::on_cdata_block(const Glib::ustring &text)
{
    if (!_context.empty()) {
        Node *t = _doc->createTextNode(text.c_str(), true);
        _context.top()->appendChild(t);
        Inkscape::GC::release(t);
    }
}

void SVGParser::on_comment(const Glib::ustring& text)
{
    Node *comment = _doc->createComment(text.c_str());
    if (_context.empty()) {
        _doc->appendChild(comment);
    } else {
        _context.top()->appendChild(comment);
    }
    Inkscape::GC::release(comment);
}

void SVGParser::on_warning(const Glib::ustring& text)
{
    Glib::ustring t = "SVGParser: " + text;
    g_warning(t.c_str());
}

void SVGParser::on_error(const Glib::ustring& text)
{
    Glib::ustring t = "SVGParser: " + text;
    g_error(t.c_str());
}

void SVGParser::on_fatal_error(const Glib::ustring& text)
{
    Glib::ustring t = "SVGParser: " + text;
    g_error(t.c_str());
}

Document *SVGParser::parseFile(const Glib::ustring &filename) {
    parse_file(filename);
    return _doc;
}

Document *SVGParser::parseBuffer(const Glib::ustring &source) {
    parse_memory(source);
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
