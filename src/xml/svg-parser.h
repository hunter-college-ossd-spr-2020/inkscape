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

#ifndef SEEN_SVG_PARSER_H
#define SEEN_SVG_PARSER_H

#include "document.h"
#include <stack>
#include <libxml++/parsers/saxparser.h>
#include <libxml/tree.h>

namespace Inkscape {

namespace XML {

class Node;
class Document;

class SVGParser : protected xmlpp::SaxParser {
public:
    SVGParser();
    ~SVGParser() override;
    Document* parseFile(const Glib::ustring& filename, const Glib::ustring& defaultNs = "");
    Document* parseCompressedFile(const Glib::ustring& filename, const Glib::ustring& defaultNs = "");
    Document* parseBuffer(const Glib::ustring& source, const Glib::ustring& defaultNs = "");

protected:
    void on_start_document() override;
    void on_end_document() override;
    void on_start_element(const Glib::ustring& name, const AttributeList& properties) override;
    void on_end_element(const Glib::ustring& name) override;
    void on_start_element_ns(const Glib::ustring& name, const Glib::ustring& prefix, const Glib::ustring& uri, const NamespaceList& namespaces, const AttributeList& attributes) override;
    void on_end_element_ns(const Glib::ustring& name, const Glib::ustring& prefix, const Glib::ustring& uri) override;
    void on_processing_instruction(const Glib::ustring& target, const Glib::ustring& data) override;
    void on_characters(const Glib::ustring& characters) override;
    void on_comment(const Glib::ustring& text) override;
    void on_cdata_block(const Glib::ustring& text) override;
    void on_warning(const Glib::ustring& text) override;
    void on_error(const Glib::ustring& text) override;
    void on_fatal_error(const Glib::ustring& text) override;
    _xmlEntity *on_get_entity(const Glib::ustring &name) override;
    void on_entity_declaration(const Glib::ustring &name, xmlpp::XmlEntityType type, const Glib::ustring &publicId,
                               const Glib::ustring &systemId, const Glib::ustring &content) override;

private:
    AttributeRecord _promoteToNamespace(Glib::ustring &name, const Glib::ustring &prefix, const Glib::ustring &uri);

    enum XmlSpaceType {
        XML_SPACE_DEFAULT,
        XML_SPACE_PRESERVE
    };

    std::stack<Node*> _context;
    std::stack<XmlSpaceType> _currentSpaceType;
    Document* _doc;
    xmlEntityPtr _dummyEntity;
    Glib::ustring _defaultNs;

};

}

}

#endif // SEEN_SVG_PARSER_H

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
