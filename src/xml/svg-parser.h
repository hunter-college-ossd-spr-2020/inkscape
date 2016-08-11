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

#include <libxml++/parsers/saxparser.h>
#include <stack>
#include "document.h"

namespace Inkscape {

namespace XML {

class Node;
class Document;

class SVGParser : protected xmlpp::SaxParser
{
public:
    SVGParser();
    ~SVGParser() override;
    Document* parseFile(const Glib::ustring& filename);
    Document* parseBuffer(const Glib::ustring& source);

protected:
    void on_start_document() override;
    void on_end_document() override;
    void on_start_element(const Glib::ustring& name, const AttributeList& properties) override;
    void on_end_element(const Glib::ustring& name) override;
    void on_characters(const Glib::ustring& characters) override;
    void on_comment(const Glib::ustring& text) override;
    void on_cdata_block(const Glib::ustring& text) override;
    void on_warning(const Glib::ustring& text) override;
    void on_error(const Glib::ustring& text) override;
    void on_fatal_error(const Glib::ustring& text) override;

private:
    std::stack<Node*> _context;
    Document* _doc;
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
