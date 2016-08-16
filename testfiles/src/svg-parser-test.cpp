/*
 * Unit tests for SVGParser
 *
 * Authors:
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtest/gtest.h>
#include <src/xml/svg-parser.h>
#include <src/xml/repr.h>
#include <src/xml/text-node.h>
#include <algorithm>

using namespace Inkscape::XML;

class SVGParserTest: public ::testing::Test {
public:
    SVGParserTest() {
        parser = new SVGParser();
    }
    ~SVGParserTest() {
        delete parser;
    }

    SVGParser* parser;
};

TEST_F(SVGParserTest, Basics) {
    Glib::ustring source = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
        "<?xml-stylesheet type=\"text/xsl\" href=\"style.xsl\"?>\n"
        "<!--comment example-->\n"
        "<svg viewBox=\"0 0 96 105\">"
            "<g fill=\"#97C024\" stroke=\"#97C024\" stroke-linejoin=\"round\" stroke-linecap=\"round\">"
                "<path d=\"M14,40v24M81,40v24M38,68v24M57,68v24M28,42v31h39v-31z\" stroke-width=\"12\"/>"
                "<path d=\"M32,5l5,10M64,5l-6,10 \" stroke-width=\"2\"/>"
            "</g>"
            "<path d=\"M22,35h51v10h-51zM22,33c0-31,51-31,51,0\" fill=\"#97C024\"/>"
            "<circle cx=\"36\" cy=\"22\" r=\"2\"/>"
            "<text>Some text</text>"
            "<text><![CDATA[some && cdata]]></text>"
        "</svg>";

    Document* doc = parser->parseBuffer(source);
    EXPECT_EQ(NodeType::DOCUMENT_NODE, doc->type());
    EXPECT_TRUE(doc->root());
    EXPECT_EQ(3, doc->childCount());
    EXPECT_EQ(NodeType::PI_NODE, doc->firstChild()->type());
    EXPECT_STREQ("xml-stylesheet", doc->firstChild()->name());
    EXPECT_EQ(NodeType::COMMENT_NODE, doc->nthChild(1)->type());
    EXPECT_STREQ("comment", doc->nthChild(1)->name());
    EXPECT_STREQ("comment example", doc->nthChild(1)->content());
    EXPECT_EQ(NodeType::ELEMENT_NODE, doc->lastChild()->type());
    EXPECT_STREQ("svg", doc->lastChild()->name());
    EXPECT_EQ(5, doc->lastChild()->childCount());
    Node* root = doc->root();
    EXPECT_EQ(NodeType::ELEMENT_NODE, root->nthChild(0)->type());
    EXPECT_STREQ("g", root->nthChild(0)->name());
    EXPECT_EQ(2, root->nthChild(0)->childCount());
    EXPECT_EQ(NodeType::ELEMENT_NODE, root->nthChild(0)->firstChild()->type());
    EXPECT_STREQ("path", root->nthChild(0)->firstChild()->name());
    EXPECT_EQ(NodeType::ELEMENT_NODE, root->nthChild(0)->lastChild()->type());
    EXPECT_STREQ("path", root->nthChild(0)->lastChild()->name());
    EXPECT_EQ(NodeType::ELEMENT_NODE, root->nthChild(1)->type());
    EXPECT_STREQ("path", root->nthChild(1)->name());
    EXPECT_EQ(NodeType::ELEMENT_NODE, root->nthChild(2)->type());
    EXPECT_STREQ("circle", root->nthChild(2)->name());
    EXPECT_EQ(NodeType::ELEMENT_NODE, root->nthChild(3)->type());
    EXPECT_STREQ("text", root->nthChild(3)->name());
    EXPECT_EQ(1, root->nthChild(3)->childCount());
    EXPECT_EQ(NodeType::TEXT_NODE, root->nthChild(3)->firstChild()->type());
    EXPECT_STREQ("Some text", root->nthChild(3)->firstChild()->content());
    EXPECT_EQ(NodeType::ELEMENT_NODE, root->nthChild(4)->type());
    EXPECT_STREQ("text", root->nthChild(4)->name());
    EXPECT_EQ(1, root->nthChild(4)->childCount());
    EXPECT_EQ(NodeType::TEXT_NODE, root->nthChild(4)->firstChild()->type());
    EXPECT_TRUE(dynamic_cast<TextNode*>(root->nthChild(4)->firstChild())->is_CData());
    EXPECT_STREQ("some && cdata", root->nthChild(4)->firstChild()->content());
}

TEST_F(SVGParserTest, Namespaces) {
    Glib::ustring source1 = "<svg><text>sample</text></svg>";
    Glib::ustring source2 = "<svg xmlns=\"http://www.w3.org/2000/svg\"><text>sample</text></svg>";
    Document* doc1 = parser->parseBuffer(source1);
    EXPECT_EQ(1, doc1->childCount());
    EXPECT_STREQ("svg", doc1->firstChild()->name());
    Document* doc2 = parser->parseBuffer(source1, SP_INKSCAPE_NS_URI);
    EXPECT_EQ(1, doc2->childCount());
    EXPECT_STREQ("inkscape:svg", doc2->firstChild()->name());
    Document* doc3 = parser->parseBuffer(source2);
    EXPECT_EQ(1, doc3->childCount());
    EXPECT_STREQ("svg:svg", doc3->firstChild()->name());
    Document* doc4 = parser->parseBuffer(source2, SP_INKSCAPE_NS_URI);
    EXPECT_EQ(1, doc4->childCount());
    EXPECT_STREQ("svg:svg", doc4->firstChild()->name());
}

TEST_F(SVGParserTest, Entities) {
    Glib::ustring source = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
        "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20000303 Stylable//EN\" \"http://www.w3.org/TR/2000/03/WD-SVG-20000303/DTD/svg-20000303-stylable.dtd\" [\n"
        "<!ENTITY a \"AaAaaAaaa\">\n"
        "<!ENTITY b \"20px\">\n"
        "<!ENTITY public PUBLIC \"-//W3C//TEXT copyright//EN\" \"http://www.w3.org/xmlspec/copyright.xml\">\n"
        "<!ENTITY xxe SYSTEM \"file:///etc/hostname\">\n"
        "]>\n"
        "<svg>"
        "<rect width=\"&b;\" />"
        "<text>&a;</text>"
        "<text>Text with xxe vulnerability &xxe;</text>"
        "<text>Text with public resource &public;</text>"
        "</svg>";

    Document *doc = parser->parseBuffer(source);
    Node* root = doc->root();
    EXPECT_EQ(4, root->childCount());
    EXPECT_STREQ("rect", root->nthChild(0)->name());
    EXPECT_STREQ("20px", root->nthChild(0)->attribute("width"));
    EXPECT_STREQ("text", root->nthChild(1)->name());
    EXPECT_STREQ("AaAaaAaaa", root->nthChild(1)->firstChild()->content());
    EXPECT_STREQ("text", root->nthChild(2)->name());
    EXPECT_STREQ("Text with xxe vulnerability ", root->nthChild(2)->firstChild()->content());
    EXPECT_STREQ("text", root->nthChild(3)->name());
    EXPECT_STREQ("Text with public resource ", root->nthChild(3)->firstChild()->content());
}

TEST_F(SVGParserTest, AdobeIlustratorSvg) {
    // svg example from
    // https://git.gnome.org/browse/libxml++/plain/examples/sax_parser_build_dom/example.xml
    Glib::ustring source = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
        "<!-- Generator: Adobe Illustrator 9.0, SVG Export Plug-In  -->\n"
        "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20000303 Stylable//EN\"   \"http://www.w3.org/TR/2000/03/WD-SVG-20000303/DTD/svg-20000303-stylable.dtd\" [\n"
        "\t<!ENTITY st0 \"fill-rule:nonzero;clip-rule:nonzero;fill:#FFFFFF;stroke:#000000;stroke-miterlimit:4;\">\n"
        "\t<!ENTITY st1 \"fill:#3466DE;stroke:#3466DE;stroke-width:2.9783;\">\n"
        "\t<!ENTITY st2 \"stroke:none;\">\n"
        "]>\n"
        "<svg width=\"23pt\" height=\"23pt\" viewBox=\"0 0 30 30\" xml:space=\"preserve\">\n"
        "\t<g id=\"Layer_x0020_1\" style=\"&st0;\">\n"
        "\t\t<g style=\"&st2;\">\n"
        "\t\t\t<g>\n"
        "\t\t\t\t<g>\n"
        "\t\t\t\t\t<path style=\"&st1;\" d=\"M14.364,27.242c7.111,0,12.876-5.766,12.876-12.877c0-7.111-5.765-12.876-12.876-12.876S1.487,7.253,1.487,14.365c0,7.112,5.766,12.877,12.877,12.877z\"/>\n"
        "\t\t\t\t</g>\n"
        "\t\t\t</g>\n"
        "\t\t\t<g>\n"
        "\t\t\t\t<path d=\"M6.757,18.987c-0.795-0.797-2.715-2.47-1.207-3.979c1.428-1.427,3.211,0.438,4.035,1.263l2.635,2.634c0.986,0.988,2.387,2.415,1.014,3.787c-1.316,1.317-2.771,0-3.621-0.851l-2.855-2.854z M6.675,22.856c2.744,2.744,4.953,4.541,8.109,1.386\n"
        "\t\t\t\t\tc3.156-3.156,1.359-5.364-1.385-8.109l-1.469-1.468c-2.703-2.538-4.857-4.28-7.932-1.208c-3.072,3.074-1.33,5.229,1.209,7.932l1.467,1.468z\"/>\n"
        "\t\t\t\t<path d=\"M19.614,18.918L9.323,8.628l1.922-1.921l4.568,4.569l0.027-0.027c-0.508-1.111-0.918-2.512-1.262-3.678l-0.975-3.225l2.168-2.168l1.879,6.957l8.535,3.21l-2.168,2.168l-5.873-2.25l0.439,1.784l2.949,2.949l-1.92,1.921z\"/>\n"
        "\t\t\t</g>\n"
        "\t\t</g>\n"
        "\t</g>\n"
        "\t<cdata_test><![CDATA[& is not protected here]]></cdata_test>\n"
        "</svg>";
    std::string expected = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
        "<!-- Generator: Adobe Illustrator 9.0, SVG Export Plug-In  -->\n"
        "<svg width=\"23pt\" height=\"23pt\" viewBox=\"0 0 30 30\" xml:space=\"preserve\">\n"
        "\t<g id=\"Layer_x0020_1\" style=\"fill-rule:nonzero;clip-rule:nonzero;fill:#FFFFFF;stroke:#000000;stroke-miterlimit:4;\">\n"
        "\t\t<g style=\"stroke:none;\">\n"
        "\t\t\t<g>\n"
        "\t\t\t\t<g>\n"
        "\t\t\t\t\t<path style=\"fill:#3466DE;stroke:#3466DE;stroke-width:2.9783;\" d=\"M14.364,27.242c7.111,0,12.876-5.766,12.876-12.877c0-7.111-5.765-12.876-12.876-12.876S1.487,7.253,1.487,14.365c0,7.112,5.766,12.877,12.877,12.877z\"/>\n"
        "\t\t\t\t</g>\n"
        "\t\t\t</g>\n"
        "\t\t\t<g>\n"
        "\t\t\t\t<path d=\"M6.757,18.987c-0.795-0.797-2.715-2.47-1.207-3.979c1.428-1.427,3.211,0.438,4.035,1.263l2.635,2.634c0.986,0.988,2.387,2.415,1.014,3.787c-1.316,1.317-2.771,0-3.621-0.851l-2.855-2.854z M6.675,22.856c2.744,2.744,4.953,4.541,8.109,1.386\n"
        "\t\t\t\t\tc3.156-3.156,1.359-5.364-1.385-8.109l-1.469-1.468c-2.703-2.538-4.857-4.28-7.932-1.208c-3.072,3.074-1.33,5.229,1.209,7.932l1.467,1.468z\"/>\n"
        "\t\t\t\t<path d=\"M19.614,18.918L9.323,8.628l1.922-1.921l4.568,4.569l0.027-0.027c-0.508-1.111-0.918-2.512-1.262-3.678l-0.975-3.225l2.168-2.168l1.879,6.957l8.535,3.21l-2.168,2.168l-5.873-2.25l0.439,1.784l2.949,2.949l-1.92,1.921z\"/>\n"
        "\t\t\t</g>\n"
        "\t\t</g>\n"
        "\t</g>\n"
        "\t<cdata_test><![CDATA[& is not protected here]]></cdata_test>\n"
        "</svg>";
    Document* doc = parser->parseBuffer(source);
    Glib::ustring d = sp_repr_save_buf(doc);
    std::string actual = d.raw();
    actual.erase(std::remove_if(actual.begin(), actual.end(), isspace), actual.end());
    expected.erase(std::remove_if(expected.begin(), expected.end(), isspace), expected.end());
    EXPECT_EQ(expected, actual);
}
