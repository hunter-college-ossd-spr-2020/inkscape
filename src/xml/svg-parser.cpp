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

#include <io/sys.h>
#include <io/gzipstream.h>
#include <io/uristream.h>
#include <boost/algorithm/string/trim.hpp>
#include <util/share.h>
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
    _currentSpaceType.push(XML_SPACE_DEFAULT);
}

void SVGParser::on_end_document() {
    std::stack<Node*> emptyContext;
    std::stack<XmlSpaceType> emptySpaceTypes;
    _context.swap(emptyContext);
    _currentSpaceType.swap(emptySpaceTypes);
}

void SVGParser::on_start_element(const Glib::ustring& name, const AttributeList& attributes) {
    g_warning("The 'on_start_element_ns' function should be invoked");
}

void SVGParser::on_end_element(const Glib::ustring& name) {
    g_warning("The 'on_end_element_ns' function should be invoked");
}

void SVGParser::on_characters(const Glib::ustring& text) {
    std::string t = text.raw();
    if(_currentSpaceType.top() == XML_SPACE_DEFAULT) {
        boost::trim(t);
    }
    if (!_context.empty() && t != "") {
        Node *node = _doc->createTextNode(t.c_str());
        _context.top()->appendChild(node);
        Inkscape::GC::release(node);
    }
}

void SVGParser::on_cdata_block(const Glib::ustring &text) {
    std::string t = text.raw();
    if(_currentSpaceType.top() == XML_SPACE_DEFAULT) {
        boost::trim(t);
    }
    if (!_context.empty() && t != "") {
        Node *node = _doc->createTextNode(t.c_str(), true);
        _context.top()->appendChild(node);
        Inkscape::GC::release(node);
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
    AttributeRecord ar = _promoteToNamespace(n, prefix, uri);
    Node* element = _doc->createElement(n.c_str());
    for (auto &ns: namespaces) {
        Glib::ustring nsn = "xmlns";
        if (ns.prefix != "") {
            nsn += ":" + ns.prefix;
        }
        element->setNamespace(nsn, ns.uri);
    }
    for (auto &attr: attributes) {
        Glib::ustring an = attr.name;
        if (attr.ns.prefix != "") {
            an = attr.ns.prefix + ":" + an;
        }
        // must call (const char*, const char*) function due to overloading
        element->setAttribute(an.c_str(), attr.value.c_str());
    }
    if (_context.empty()) {
        _doc->appendChild(element);
    } else {
        _context.top()->appendChild(element);
    }
    const char* a = element->attribute("xml:space");
    if (a) {
        if (strcmp(a, "preserve") == 0) {
            _currentSpaceType.push(XML_SPACE_PRESERVE);
        } else {
            _currentSpaceType.push(XML_SPACE_DEFAULT);
        }
    }
    if (ar.key != 0) {
        _doc->root()->setNamespace(g_quark_to_string(ar.key), ar.value.pointer());
    }
    _context.push(element);
    Inkscape::GC::release(element);
}

void SVGParser::on_end_element_ns(const Glib::ustring& name, const Glib::ustring& prefix, const Glib::ustring& uri) {
    const char* a = _context.top()->attribute("xml:space");
    if (a) {
        _currentSpaceType.pop();
    }
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
    g_warning("SVGParser: %s", text.c_str());
}

void SVGParser::on_error(const Glib::ustring& text) {
    // should be g_error, but unfortunately g_error kills the program
    g_warning("SVGParser: %s", text.c_str());
}

void SVGParser::on_fatal_error(const Glib::ustring& text) {
    // should be g_error, but unfortunately g_error kills the program
    g_warning("SVGParser: %s", text.c_str());
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
    try {
        parse_file(filename);
    } catch (const xmlpp::exception &ex) {
        g_warning(ex.what());
    }
    return _doc;
}


Document *SVGParser::parseCompressedFile(const Glib::ustring &filename, const Glib::ustring &defaultNs) {
    _defaultNs = defaultNs;
    const int bufferSize = 1024;
    FILE* fp = Inkscape::IO::fopen_utf8name(filename.c_str(), "r");
    URI uri;
    auto instr = new Inkscape::IO::UriInputStream(fp, uri);
    auto gzin = new Inkscape::IO::GzipInputStream(*instr);
    char buffer[bufferSize];
    char ch;
    try {
        while(true) {
            std::memset(buffer, 0, bufferSize);
            int i;
            for (i = 0; i < bufferSize; i++) {
                ch = (char) gzin->get();
                if (ch < 0) {
                    break;
                }
                buffer[i] = ch;
            }
            Glib::ustring input(buffer, (Glib::ustring::size_type) i);
            parse_chunk(input);
            if (i < bufferSize) {
                break;
            }
        }
    } catch (const xmlpp::exception &ex) {
        g_warning(ex.what());
    }
    finish_chunk_parsing();

    return _doc;
}


Document *SVGParser::parseBuffer(const Glib::ustring &source, const Glib::ustring& defaultNs) {
    _defaultNs = defaultNs;
    try {
        parse_memory(source);
    } catch (const xmlpp::exception &ex) {
        g_warning(ex.what());
    }
    return _doc;
}

AttributeRecord SVGParser::_promoteToNamespace(Glib::ustring &name, const Glib::ustring &prefix, const Glib::ustring &uri) {
    AttributeRecord ar(0, Inkscape::Util::ptr_shared<char>());
    Glib::ustring p = "";
    if (prefix != "") {
        p = prefix;
        ar.value = Inkscape::Util::share_string(uri.c_str());
    } else if (uri != "" && sp_xml_ns_uri_prefix(uri.c_str(), prefix.c_str())) {
        p = sp_xml_ns_uri_prefix(uri.c_str(), prefix.c_str());
        ar.value = Inkscape::Util::share_string(uri.c_str());
    } else if (_defaultNs != "") {
        p = sp_xml_ns_uri_prefix(_defaultNs.c_str(), "");
        ar.value = Inkscape::Util::share_string(_defaultNs.c_str());
    }

    if (p != "") {
        name = p + ":" + name;
        p = "xmlns:" + p;
        ar.key = g_quark_from_string(p.c_str());
    }
    return ar;
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
