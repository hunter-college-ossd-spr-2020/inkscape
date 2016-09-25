/*
 * IO functions for svg files.
 *
 * Authors:
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "io/uristream.h"
#include "io/stringstream.h"
#include "io/gzipstream.h"
#include "attribute-rel-util.h"
#include "attribute-sort-util.h"
#include "preferences.h"
#include "repr-io.h"
#include "rebase-hrefs.h"
#include "simple-node.h"
#include "util/list.h"
#include "svg-parser.h"
#include "xml/attribute-record.h"
#include <boost/algorithm/string.hpp>
#include <io/sys.h>
#include <glibmm/miscutils.h>
#include <giomm/file.h>

using namespace Inkscape::Util;

namespace Inkscape {

namespace XML {

using namespace Inkscape::IO;

static SVGParser _parser;

static void clean_attributes(Document *doc) {
    // Clean unnecessary attributes and style properties from SVG documents (controlled by preferences)
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool clean = prefs->getBool("/options/svgoutput/check_on_reading");
    if(clean) {
        sp_attribute_clean_tree(doc->root());
    }
}

Document* read_svg_file(const Glib::ustring& filename, const bool& isInternal, const Glib::ustring& defaultNs) {
    if (!Inkscape::IO::file_test(filename.c_str(), G_FILE_TEST_EXISTS)) {
        g_warning("Can't open file: %s (doesn't exist)", filename.c_str());
        return nullptr;
    }

    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filename);
    Glib::RefPtr<Gio::FileInfo> info = file->query_info();
    std::string content_type = info->get_content_type();

    Document *doc;
    if(content_type == "image/svg+xml" || content_type == "application/xml" || content_type == "text/html") {
        doc = _parser.parseFile(filename, defaultNs);
    } else if (content_type == "image/svg+xml-compressed" || content_type == "application/x-gzip") {
        doc = _parser.parseCompressedFile(filename, defaultNs);
    } else {
        g_warning("Wrong content type (%s) to open.", content_type.c_str());
        return nullptr;
    }

    if(!isInternal) {
        clean_attributes(doc);
    }
    return doc;
}

Document* read_svg_buffer(const Glib::ustring& source, const bool& isInternal, const Glib::ustring& defaultNs) {
    Document *doc = _parser.parseBuffer(source, defaultNs);
    if(!isInternal) {
        clean_attributes(doc);
    }
    return doc;
}

static void rebase_attributes(Node* node, const Glib::ustring& old_href_base, const Glib::ustring& new_href_base) {
    for(auto iter = rebase_href_attrs(old_href_base.c_str(), new_href_base.c_str(), node->attributeList()); iter; ++iter) {
        node->setAttribute(g_quark_to_string(iter->key), iter->value);
    }

    for (Node *child = node->firstChild(); child != nullptr; child = child->next()) {
        if (child->type() == ELEMENT_NODE) {
            rebase_attributes(child, old_href_base, new_href_base);
        }
    }
}

static void add_namespace_to_doc(Document* doc, const Glib::ustring& nodeName) {
    if (nodeName.find(":") != Glib::ustring::npos) {
        Glib::ustring prefix = nodeName.substr(0, nodeName.find(":"));
        Glib::ustring extendedPrefix = "xmlns:" + prefix;
        SimpleNode* root = dynamic_cast<SimpleNode*>(doc->root());
        if (root->namespaces().find(g_quark_from_string(extendedPrefix.c_str())) == root->namespaces().end()
                && prefix != "xml") {
            root->setNamespace(extendedPrefix, sp_xml_ns_prefix_uri(prefix.c_str()));
        }
    }
}

static void find_all_prefixes(Document* doc, Node* node) {
    add_namespace_to_doc(doc, node->name());
    for (List<AttributeRecord const> iter = node->attributeList(); iter; ++iter) {
        add_namespace_to_doc(doc, g_quark_to_string(iter->key));
    }
    for (Node *child = node->firstChild(); child != nullptr; child = child->next()) {
        if (child->type() == ELEMENT_NODE) {
            find_all_prefixes(doc, child);
        }
    }
}

static void serialize(Writer& writer, Document* doc, const Glib::ustring& defaultNs, const Glib::ustring& old_base, const Glib::ustring& new_base) {
    if (old_base != "") {
        Glib::ustring old_href_abs_base = calc_abs_doc_base(old_base.c_str());
        Glib::ustring new_href_abs_base;
        if (Glib::path_is_absolute(new_base)) {
            new_href_abs_base = Glib::path_get_dirname(new_base);
        } else {
            Glib::ustring const cwd = Glib::get_current_dir();
            Glib::ustring const for_abs_filename = Glib::build_filename(cwd, new_base);
            new_href_abs_base = Glib::path_get_dirname(for_abs_filename);
        }
        rebase_attributes(doc->root(), old_href_abs_base, new_href_abs_base);
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    // Clean unnecessary attributes and stype properties. (Controlled by preferences.)
    bool clean = prefs->getBool("/options/svgoutput/check_on_writing");
    if (clean) {
        sp_attribute_clean_tree(doc->root());
    }

    // find all used prefixes
    find_all_prefixes(doc, doc->root());
    if (defaultNs != "") {
        doc->root()->setNamespace("xmlns", defaultNs);
    }
    Glib::ustring defaultPrefix = sp_xml_ns_uri_prefix(defaultNs.c_str(), "");

    bool inlineAttributes = prefs->getBool("/options/svgoutput/inlineattrs");
    int indent = prefs->getInt("/options/svgoutput/indent", 2);

    // Sort attributes in a canonical order (helps with "diffing" SVG files).
    bool sort = prefs->getBool("/options/svgoutput/sort_attributes");
    if (sort) {
        sp_attribute_sort_tree(doc->root());
    }

    doc->serialize(writer, defaultPrefix, indent, 0, inlineAttributes, false);
}

Glib::ustring save_svg_buffer(Document *doc, const Glib::ustring& defaultNs, const Glib::ustring& oldBase, const Glib::ustring& newBase) {
    StringOutputStream stream;
    OutputStreamWriter writer(stream);

    serialize(writer, doc, defaultNs, oldBase, newBase);

    writer.close();
    return stream.getString();
}

bool save_svg_file(Document *doc, const Glib::ustring &filename, const Glib::ustring& defaultNs, const Glib::ustring& oldBase, const Glib::ustring& newBase) {
    FILE* file = fopen_utf8name(filename.c_str(), "w");

    if (file == nullptr) {
        return false;
    }

    Inkscape::URI dummy("x");
    std::unique_ptr<OutputStream> stream(new UriOutputStream(file, dummy));
    std::unique_ptr<OutputStream> decorator;
    if (filename.find(".svgz") == filename.size() - 5) {
        decorator = std::unique_ptr<OutputStream>(new GzipOutputStream(*stream));
    }
    OutputStreamWriter writer(decorator ? *decorator : *stream);

    serialize(writer, doc, defaultNs, oldBase, newBase);

    writer.close();
    return fclose(file) == 0;

}

} // namespace XML

} // namespace Inkscape



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
