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

#include <cstring>
#include <string>
#include <stdexcept>

#include "xml/repr.h"
#include "xml/attribute-record.h"
#include "xml/rebase-hrefs.h"
#include "xml/simple-document.h"
#include "xml/text-node.h"

#include "io/sys.h"
#include "io/uristream.h"
#include "io/stringstream.h"
#include "io/gzipstream.h"

#include "extension/extension.h"

#include "attribute-rel-util.h"
#include "attribute-sort-util.h"

#include "preferences.h"


#include "repr-io.h"
#include <glibmm/miscutils.h>
#include <gio/gio.h>
#include <boost/algorithm/string.hpp>

using Inkscape::IO::Writer;
using Inkscape::Util::List;
using Inkscape::XML::Document;
using Inkscape::XML::AttributeRecord;
using Inkscape::XML::calc_abs_doc_base;

static void sp_repr_write_stream_root_element(Node *repr, Writer &out,
                                              bool add_whitespace, gchar const *default_ns,
                                              int inlineattrs, int indent,
                                              gchar const *old_href_abs_base,
                                              gchar const *new_href_abs_base);

static void sp_repr_write_stream_element(Node *repr, Writer &out,
                                         gint indent_level, bool add_whitespace,
                                         Glib::QueryQuark elide_prefix,
                                         List<AttributeRecord const> attributes,
                                         int inlineattrs, int indent,
                                         gchar const *old_href_abs_base,
                                         gchar const *new_href_abs_base);

namespace Inkscape {

struct compare_quark_ids {
    bool operator()(Glib::QueryQuark const &a, Glib::QueryQuark const &b) const {
        return a.id() < b.id();
    }
};

}

namespace {

typedef std::map<Glib::QueryQuark, Glib::QueryQuark, Inkscape::compare_quark_ids> PrefixMap;

Glib::QueryQuark qname_prefix(Glib::QueryQuark qname) {
    static PrefixMap prefix_map;
    PrefixMap::iterator iter = prefix_map.find(qname);
    if ( iter != prefix_map.end() ) {
        return (*iter).second;
    } else {
        gchar const *name_string=g_quark_to_string(qname);
        gchar const *prefix_end=strchr(name_string, ':');
        if (prefix_end) {
            Glib::Quark prefix=Glib::ustring(name_string, prefix_end);
            prefix_map.insert(PrefixMap::value_type(qname, prefix));
            return prefix;
        } else {
            return GQuark(0);
        }
    }
}

}

static void sp_repr_save_writer(Document *doc, Inkscape::IO::Writer *out,
                    gchar const *default_ns,
                    gchar const *old_href_abs_base,
                    gchar const *new_href_abs_base)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool inlineattrs = prefs->getBool("/options/svgoutput/inlineattrs");
    int indent = prefs->getInt("/options/svgoutput/indent", 2);

    /* fixme: do this The Right Way */
    out->writeString( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" );

    const gchar *str = static_cast<Node *>(doc)->attribute("doctype");
    if (str) {
        out->writeString( str );
    }

    for (Node *repr = sp_repr_document_first_child(doc);
         repr; repr = repr->next())
    {
        Inkscape::XML::NodeType const node_type = repr->type();
        if ( node_type == Inkscape::XML::ELEMENT_NODE ) {
            sp_repr_write_stream_root_element(repr, *out, TRUE, default_ns, inlineattrs, indent,
                                              old_href_abs_base, new_href_abs_base);
        } else {
            sp_repr_write_stream(repr, *out, 0, TRUE, GQuark(0), inlineattrs, indent,
                                 old_href_abs_base, new_href_abs_base);
            if ( node_type == Inkscape::XML::COMMENT_NODE ) {
                out->writeChar('\n');
            }
        }
    }
}


Glib::ustring sp_repr_save_buf(Document *doc)
{   
    Inkscape::IO::StringOutputStream souts;
    Inkscape::IO::OutputStreamWriter outs(souts);

    sp_repr_save_writer(doc, &outs, SP_INKSCAPE_NS_URI, 0, 0);

    outs.close();
    Glib::ustring buf = souts.getString();

    return buf;
}


void sp_repr_save_stream(Document *doc, FILE *fp, gchar const *default_ns, bool compress,
                    gchar const *const old_href_abs_base,
                    gchar const *const new_href_abs_base)
{
    Inkscape::URI dummy("x");
    Inkscape::IO::UriOutputStream bout(fp, dummy);
    Inkscape::IO::GzipOutputStream *gout = compress ? new Inkscape::IO::GzipOutputStream(bout) : NULL;
    Inkscape::IO::OutputStreamWriter *out  = compress ? new Inkscape::IO::OutputStreamWriter( *gout ) : new Inkscape::IO::OutputStreamWriter( bout );

    sp_repr_save_writer(doc, out, default_ns, old_href_abs_base, new_href_abs_base);

    delete out;
    delete gout;
}



/**
 * Returns true if file successfully saved.
 *
 * \param filename The actual file to do I/O to, which might be a temp file.
 *
 * \param for_filename The base URI [actually filename] to assume for purposes of rewriting
 *              xlink:href attributes.
 */
bool sp_repr_save_rebased_file(Document *doc, gchar const *const filename, gchar const *default_ns,
                          gchar const *old_base, gchar const *for_filename)
{
    if (!filename) {
        return false;
    }

    bool compress;
    {
        size_t const filename_len = strlen(filename);
        compress = ( filename_len > 5
                     && strcasecmp(".svgz", filename + filename_len - 5) == 0 );
    }

    Inkscape::IO::dump_fopen_call( filename, "B" );
    FILE *file = Inkscape::IO::fopen_utf8name(filename, "w");
    if (file == NULL) {
        return false;
    }

    Glib::ustring old_href_abs_base;
    Glib::ustring new_href_abs_base;
    if (for_filename) {
        old_href_abs_base = calc_abs_doc_base(old_base);
        if (Glib::path_is_absolute(for_filename)) {
            new_href_abs_base = Glib::path_get_dirname(for_filename);
        } else {
            Glib::ustring const cwd = Glib::get_current_dir();
            Glib::ustring const for_abs_filename = Glib::build_filename(cwd, for_filename);
            new_href_abs_base = Glib::path_get_dirname(for_abs_filename);
        }

        /* effic: Once we're confident that we never need (or never want) to resort
         * to using sodipodi:absref instead of the xlink:href value,
         * then we should do `if streq() { free them and set both to NULL; }'. */
    }
    sp_repr_save_stream(doc, file, default_ns, compress, old_href_abs_base.c_str(), new_href_abs_base.c_str());

    if (fclose (file) != 0) {
        return false;
    }

    return true;
}

/**
 * Returns true iff file successfully saved.
 */
bool sp_repr_save_file(Document *doc, gchar const *const filename, gchar const *default_ns)
{
    return sp_repr_save_rebased_file(doc, filename, default_ns, NULL, NULL);
}


/* (No doubt this function already exists elsewhere.) */
static void repr_quote_write (Writer &out, const gchar * val)
{
    if (val) {
        for (; *val != '\0'; val++) {
            switch (*val) {
                case '"': out.writeString( "&quot;" ); break;
                case '&': out.writeString( "&amp;" ); break;
                case '<': out.writeString( "&lt;" ); break;
                case '>': out.writeString( "&gt;" ); break;
                default: out.writeChar( *val ); break;
            }
        }
    }
}

static void repr_write_comment( Writer &out, const gchar * val, bool addWhitespace, gint indentLevel, int indent )
{
    if ( indentLevel > 16 ) {
        indentLevel = 16;
    }
    if (addWhitespace && indent) {
        for (gint i = 0; i < indentLevel; i++) {
            for (gint j = 0; j < indent; j++) {
                out.writeString(" ");
            }
        }
    }

    out.writeString("<!--");
    // WARNING out.printf() and out.writeString() are *NOT* non-ASCII friendly.
    if (val) {
        for (const gchar* cur = val; *cur; cur++ ) {
            out.writeChar(*cur);
        }
    } else {
        out.writeString(" ");
    }
    out.writeString("-->");

    if (addWhitespace) {
        out.writeString("\n");
    }
}

namespace {

typedef std::map<Glib::QueryQuark, gchar const *, Inkscape::compare_quark_ids> LocalNameMap;
typedef std::map<Glib::QueryQuark, Inkscape::Util::ptr_shared<char>, Inkscape::compare_quark_ids> NSMap;

gchar const *qname_local_name(Glib::QueryQuark qname) {
    static LocalNameMap local_name_map;
    LocalNameMap::iterator iter = local_name_map.find(qname);
    if ( iter != local_name_map.end() ) {
        return (*iter).second;
    } else {
        gchar const *name_string=g_quark_to_string(qname);
        gchar const *prefix_end=strchr(name_string, ':');
        if (prefix_end) {
            return prefix_end + 1;
        } else {
            return name_string;
        }
    }
}

void add_ns_map_entry(NSMap &ns_map, Glib::QueryQuark prefix) {
    using Inkscape::Util::ptr_shared;
    using Inkscape::Util::share_unsafe;

    static const Glib::QueryQuark xml_prefix("xml");

    NSMap::iterator iter=ns_map.find(prefix);
    if ( iter == ns_map.end() ) {
        if (prefix.id()) {
            gchar const *uri=sp_xml_ns_prefix_uri(g_quark_to_string(prefix));
            if (uri) {
                ns_map.insert(NSMap::value_type(prefix, share_unsafe(uri)));
            } else if ( prefix != xml_prefix ) {
                g_warning("No namespace known for normalized prefix %s", g_quark_to_string(prefix));
            }
        } else {
            ns_map.insert(NSMap::value_type(prefix, ptr_shared<char>()));
        }
    }
}

void populate_ns_map(NSMap &ns_map, Node &repr) {
    if ( repr.type() == Inkscape::XML::ELEMENT_NODE ) {
        add_ns_map_entry(ns_map, qname_prefix(repr.code()));
        for ( List<AttributeRecord const> iter=repr.attributeList() ;
              iter ; ++iter )
        {
            Glib::QueryQuark prefix=qname_prefix(iter->key);
            if (prefix.id()) {
                add_ns_map_entry(ns_map, prefix);
            }
        }
        for ( Node *child=repr.firstChild() ;
              child ; child = child->next() )
        {
            populate_ns_map(ns_map, *child);
        }
    }
}

}

static void sp_repr_write_stream_root_element(Node *repr, Writer &out,
                                  bool add_whitespace, gchar const *default_ns,
                                  int inlineattrs, int indent,
                                  gchar const *const old_href_base,
                                  gchar const *const new_href_base)
{
    using Inkscape::Util::ptr_shared;

    g_assert(repr != NULL);

    // Clean unnecessary attributes and stype properties. (Controlled by preferences.)
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool clean = prefs->getBool("/options/svgoutput/check_on_writing");
    if (clean) sp_attribute_clean_tree( repr );

    // Sort attributes in a canonical order (helps with "diffing" SVG files).
    bool sort = prefs->getBool("/options/svgoutput/sort_attributes");
    if (sort) sp_attribute_sort_tree( repr );

    Glib::QueryQuark xml_prefix=g_quark_from_static_string("xml");

    NSMap ns_map;
    populate_ns_map(ns_map, *repr);

    Glib::QueryQuark elide_prefix=GQuark(0);
    if ( default_ns && ns_map.find(GQuark(0)) == ns_map.end() ) {
        elide_prefix = g_quark_from_string(sp_xml_ns_uri_prefix(default_ns, NULL));
    }

    List<AttributeRecord const> attributes=repr->attributeList();
    for ( NSMap::iterator iter=ns_map.begin() ; iter != ns_map.end() ; ++iter )
    {
        Glib::QueryQuark prefix=(*iter).first;
        ptr_shared<char> ns_uri=(*iter).second;

        if (prefix.id()) {
            if ( prefix != xml_prefix ) {
                if ( elide_prefix == prefix ) {
                    attributes = cons(AttributeRecord(g_quark_from_static_string("xmlns"), ns_uri), attributes);
                }

                Glib::ustring attr_name="xmlns:";
                attr_name.append(g_quark_to_string(prefix));
                GQuark key = g_quark_from_string(attr_name.c_str());
                attributes = cons(AttributeRecord(key, ns_uri), attributes);
            }
        } else {
            // if there are non-namespaced elements, we can't globally
            // use a default namespace
            elide_prefix = GQuark(0);
        }
    }

    return sp_repr_write_stream_element(repr, out, 0, add_whitespace, elide_prefix, attributes,
                                        inlineattrs, indent, old_href_base, new_href_base);
}

void sp_repr_write_stream( Node *repr, Writer &out, gint indent_level,
                           bool add_whitespace, Glib::QueryQuark elide_prefix,
                           int inlineattrs, int indent,
                           gchar const *const old_href_base,
                           gchar const *const new_href_base)
{
    switch (repr->type()) {
        case Inkscape::XML::TEXT_NODE: {
            if( dynamic_cast<const Inkscape::XML::TextNode *>(repr)->is_CData() ) {
                // Preserve CDATA sections, not converting '&' to &amp;, etc.
                out.printf( "<![CDATA[%s]]>", repr->content() );
            } else {
                repr_quote_write( out, repr->content() );
            }
            break;
        }
        case Inkscape::XML::COMMENT_NODE: {
            repr_write_comment( out, repr->content(), add_whitespace, indent_level, indent );
            break;
        }
        case Inkscape::XML::PI_NODE: {
            out.printf( "<?%s %s?>", repr->name(), repr->content() );
            break;
        }
        case Inkscape::XML::ELEMENT_NODE: {
            sp_repr_write_stream_element( repr, out, indent_level,
                                          add_whitespace, elide_prefix,
                                          repr->attributeList(),
                                          inlineattrs, indent,
                                          old_href_base, new_href_base);
            break;
        }
        case Inkscape::XML::DOCUMENT_NODE: {
            g_assert_not_reached();
            break;
        }
        default: {
            g_assert_not_reached();
        }
    }
}


void sp_repr_write_stream_element( Node * repr, Writer & out,
                                   gint indent_level, bool add_whitespace,
                                   Glib::QueryQuark elide_prefix,
                                   List<AttributeRecord const> attributes, 
                                   int inlineattrs, int indent,
                                   gchar const *old_href_base,
                                   gchar const *new_href_base )
{
    Node *child = 0;
    bool loose = false;

    g_return_if_fail (repr != NULL);

    if ( indent_level > 16 ) {
        indent_level = 16;
    }

    if (add_whitespace && indent) {
        for (gint i = 0; i < indent_level; i++) {
            for (gint j = 0; j < indent; j++) {
                out.writeString(" ");
            }
        }
    }

    GQuark code = repr->code();
    gchar const *element_name;
    if ( elide_prefix == qname_prefix(code) ) {
        element_name = qname_local_name(code);
    } else {
        element_name = g_quark_to_string(code);
    }
    out.printf( "<%s", element_name );

    // if this is a <text> element, suppress formatting whitespace
    // for its content and children:
    gchar const *xml_space_attr = repr->attribute("xml:space");
    if (xml_space_attr != NULL && !strcmp(xml_space_attr, "preserve")) {
        add_whitespace = false;
    }

    for ( List<AttributeRecord const> iter = rebase_href_attrs(old_href_base, new_href_base,
                                                               attributes);
          iter ; ++iter )
    {
        if (!inlineattrs) {
            out.writeString("\n");
            if (indent) {
                for ( gint i = 0 ; i < indent_level + 1 ; i++ ) {
                    for ( gint j = 0 ; j < indent ; j++ ) {
                        out.writeString(" ");
                    }
                }
            }
        }
        out.printf(" %s=\"", g_quark_to_string(iter->key));
        repr_quote_write(out, iter->value);
        out.writeChar('"');
    }

    loose = TRUE;
    for (child = repr->firstChild() ; child != NULL; child = child->next()) {
        if (child->type() == Inkscape::XML::TEXT_NODE) {
            loose = FALSE;
            break;
        }
    }
    if (repr->firstChild()) {
        out.writeString( ">" );
        if (loose && add_whitespace) {
            out.writeString( "\n" );
        }
        for (child = repr->firstChild(); child != NULL; child = child->next()) {
            sp_repr_write_stream(child, out, ( loose ? indent_level + 1 : 0 ),
                                 add_whitespace, elide_prefix, inlineattrs, indent,
                                 old_href_base, new_href_base);
        }

        if (loose && add_whitespace && indent) {
            for (gint i = 0; i < indent_level; i++) {
                for ( gint j = 0 ; j < indent ; j++ ) {
                    out.writeString(" ");
                }
            }
        }
        out.printf( "</%s>", element_name );
    } else {
        out.writeString( " />" );
    }

    // text elements cannot nest, so we can output newline
    // after closing text

    if (add_whitespace || !strcmp (repr->name(), "svg:text")) {
        out.writeString( "\n" );
    }
}

namespace Inkscape {

namespace XML {

using namespace Inkscape::IO;

SVGParser IO::parser;

static void clean_attributes(Document *doc) {
    // Clean unnecessary attributes and style properties from SVG documents (controlled by preferences)
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool clean = prefs->getBool("/options/svgoutput/check_on_reading");
    if(clean) {
        sp_attribute_clean_tree(doc->root());
    }
}

Document* IO::read_svg_file(const Glib::ustring& filename, const bool& isInternal, const Glib::ustring& defaultNs) {
    if (!Inkscape::IO::file_test(filename.c_str(), G_FILE_TEST_EXISTS)) {
        g_warning("Can't open file: %s (doesn't exist)", filename.c_str());
        return nullptr;
    }
    GError *error;
    GFile *file = g_file_new_for_path (filename.c_str());
    GFileInfo *file_info = g_file_query_info (file, "standard::*", G_FILE_QUERY_INFO_NONE, nullptr, &error);

    const char *content_type = g_file_info_get_content_type (file_info);

    Document *doc;
    if(strcmp(content_type, "image/svg+xml") == 0
       || strcmp(content_type, "application/xml") == 0
       || strcmp(content_type, "text/html") == 0) {
        doc = parser.parseFile(filename, defaultNs);
    } else if (strcmp(content_type, "image/svg+xml-compressed") == 0
               || strcmp(content_type, "application/x-gzip") == 0) {
        doc = parser.parseCompressedFile(filename, defaultNs);
    } else {
        g_warning("Wrong content type (%s) to open.", content_type);
        return nullptr;
    }

    if(!isInternal) {
        clean_attributes(doc);
    }
    return doc;
}

Document* IO::read_svg_buffer(const Glib::ustring& source, const bool& isInternal, const Glib::ustring& defaultNs) {
    Document *doc = parser.parseBuffer(source, defaultNs);
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

    find_all_prefixes(doc, doc->root());
    if (defaultNs != "") {
        doc->root()->setNamespace("xmlns", defaultNs);
    }
    Glib::ustring defaultPrefix = sp_xml_ns_uri_prefix(defaultNs.c_str(), "");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool inlineAttributes = prefs->getBool("/options/svgoutput/inlineattrs");
    int indent = prefs->getInt("/options/svgoutput/indent", 2);

    // Clean unnecessary attributes and stype properties. (Controlled by preferences.)
    bool clean = prefs->getBool("/options/svgoutput/check_on_writing");
    if (clean) {
        sp_attribute_clean_tree(doc->root());
    }
    // Sort attributes in a canonical order (helps with "diffing" SVG files).
    bool sort = prefs->getBool("/options/svgoutput/sort_attributes");
    if (sort) {
        sp_attribute_sort_tree(doc->root());
    }

    doc->serialize(writer, defaultPrefix, indent, 0, inlineAttributes, false);
}

Glib::ustring IO::save_svg_buffer(Document *doc, const Glib::ustring& defaultNs, const Glib::ustring& oldBase, const Glib::ustring& newBase) {
    StringOutputStream stream;
    OutputStreamWriter writer(stream);

    serialize(writer, doc, defaultNs, oldBase, newBase);

    writer.close();
    return stream.getString();
}

bool IO::save_svg_file(Document *doc, const Glib::ustring &filename, const Glib::ustring& defaultNs, const Glib::ustring& oldBase, const Glib::ustring& newBase) {
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
