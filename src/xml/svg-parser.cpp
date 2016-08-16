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
#include "repr.h"

namespace Inkscape {

namespace XML {

SVGParser::SVGParser(): xmlpp::SaxParser(true), _doc(nullptr), _defaultNs("") {
    set_substitute_entities(true);
    _dummyEntity = new xmlEntity();
}

SVGParser::~SVGParser() {
    delete _dummyEntity;
}

void SVGParser::on_start_document() {
    _doc = new SimpleDocument();
}

void SVGParser::on_end_document() {
}

void SVGParser::on_start_element(const Glib::ustring& name, const AttributeList& attributes) {
    g_warning("The 'on_start_element_ns' function should be invoked");
}

void SVGParser::on_end_element(const Glib::ustring& name) {
    g_warning("The 'on_end_element_ns' function should be invoked");
}

void SVGParser::on_characters(const Glib::ustring& text) {
    if (!_context.empty()) {
        Node *t = _doc->createTextNode(text.c_str());
        _context.top()->appendChild(t);
        Inkscape::GC::release(t);
    }
}

void SVGParser::on_cdata_block(const Glib::ustring &text) {
    if (!_context.empty()) {
        Node *t = _doc->createTextNode(text.c_str(), true);
        _context.top()->appendChild(t);
        Inkscape::GC::release(t);
    }
}

void SVGParser::on_comment(const Glib::ustring& text) {
    Node *comment = _doc->createComment(text.c_str());
    if (_context.empty()) {
        _doc->appendChild(comment);
    } else {
        _context.top()->appendChild(comment);
    }
    Inkscape::GC::release(comment);
}

void SVGParser::on_start_element_ns(const Glib::ustring& name, const Glib::ustring& prefix, const Glib::ustring& uri, const NamespaceList& namespaces, const AttributeList& attributes) {
    Glib::ustring n = name;
    _promoteToNamespace(n, prefix, uri);
    Node* element = _doc->createElement(n.c_str());
    for (auto &attr: attributes) {
        Glib::ustring an = attr.name;
        if (attr.ns.prefix != "") {
            an = attr.ns.prefix + ":" + an;
        }
        element->setAttribute(an, attr.value);
    }
    if (_context.empty()) {
        _doc->appendChild(element);
    } else {
        _context.top()->appendChild(element);
    }
    _context.push(element);
    Inkscape::GC::release(element);
}

void SVGParser::on_end_element_ns(const Glib::ustring& name, const Glib::ustring& prefix, const Glib::ustring& uri) {
    _context.pop();
}

void SVGParser::on_processing_instruction(const Glib::ustring &target, const Glib::ustring &data) {
    Node *pi = _doc->createPI(target.c_str(), data.c_str());
    if (_context.empty()) {
        _doc->appendChild(pi);
    } else {
        _context.top()->appendChild(pi);
    }
    Inkscape::GC::release(pi);
}

void SVGParser::on_warning(const Glib::ustring& text) {
    Glib::ustring t = "SVGParser: " + text;
    g_warning(t.c_str());
}

void SVGParser::on_error(const Glib::ustring& text) {
    Glib::ustring t = "SVGParser: " + text;
    g_error(t.c_str());
}

void SVGParser::on_fatal_error(const Glib::ustring& text) {
    Glib::ustring t = "SVGParser: " + text;
    g_error(t.c_str());
}

xmlEntityPtr SVGParser::on_get_entity(const Glib::ustring& name)
{
    // if the name refers to external entity, result should be a nullptr
    xmlEntityPtr result =  xmlpp::SaxParser::on_get_entity(name);
    if (result) {
        return result;
    }

    return _dummyEntity;
}

void SVGParser::on_entity_declaration(const Glib::ustring& name, xmlpp::XmlEntityType type, const Glib::ustring& publicId, const Glib::ustring& systemId, const Glib::ustring& content)
{
    // allow only internal entities
    if (type == xmlpp::XmlEntityType::INTERNAL_GENERAL
        || type == xmlpp::XmlEntityType::INTERNAL_PARAMETER
        || type == xmlpp::XmlEntityType::INTERNAL_PREDEFINED) {
        xmlpp::SaxParser::on_entity_declaration(name, type, publicId, systemId, content);
    }
}

Document *SVGParser::parseFile(const Glib::ustring &filename, const Glib::ustring& defaultNs) {
    _defaultNs = defaultNs;
    parse_file(filename);
    _defaultNs = "";
    return _doc;
}

Document *SVGParser::parseBuffer(const Glib::ustring &source, const Glib::ustring& defaultNs) {
    _defaultNs = defaultNs;
    parse_memory(source);
    _defaultNs = "";
    return _doc;
}

void SVGParser::_promoteToNamespace(Glib::ustring &name, const Glib::ustring &prefix, const Glib::ustring &uri) {
    Glib::ustring p = "";
    if (prefix != "") {
        p = prefix;
    } else if (uri != "" && sp_xml_ns_uri_prefix(uri.c_str(), prefix.c_str())) {
        p = sp_xml_ns_uri_prefix(uri.c_str(), prefix.c_str());
    } else if (_defaultNs != "") {
        p = sp_xml_ns_uri_prefix(_defaultNs.c_str(), "");
    }

    if (p != "") {
        name = p + ":" + name;
    }
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
