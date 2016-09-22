/*
 * Unit tests for svg serialization
 *
 * Authors:
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <src/xml/repr-io.h>
#include <gtest/gtest.h>
#include <src/xml/repr.h>
#include <src/io/stringstream.h>
#include <src/xml/simple-document.h>
#include <src/xml/svg-parser.h>

using namespace Inkscape::XML;
using namespace Inkscape::IO;

class SVGWriterTest: public ::testing::Test {
public:
    SVGWriterTest() {
        souts = new StringOutputStream();
        writer = new OutputStreamWriter(*souts);
        parser = new SVGParser();
    }
    ~SVGWriterTest() {
        delete parser;
        delete writer;
        delete souts;
    }

    StringOutputStream* souts;
    OutputStreamWriter* writer;
    SVGParser* parser;
};

TEST_F(SVGWriterTest, Basics) {
    Glib::ustring source = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<?xml-stylesheet type=\"text/xsl\" href=\"style.xsl\"?>\n"
            "<!--comment example-->\n"
            "<svg viewBox=\"0 0 96 105\">\n"
            "<g fill=\"#97C024\" stroke=\"#97C024\" stroke-linejoin=\"round\" stroke-linecap=\"round\">\n"
            "<path d=\"M14,40v24M81,40v24M38,68v24M57,68v24M28,42v31h39v-31z\" stroke-width=\"12\" />\n"
            "<path d=\"M32,5l5,10M64,5l-6,10\" stroke-width=\"2\" />\n"
            "</g>\n"
            "<path d=\"M22,35h51v10h-51zM22,33c0-31,51-31,51,0\" fill=\"#97C024\" />\n"
            "<circle cx=\"36\" cy=\"22\" r=\"2\" />\n"
            "<text>Some text</text>\n"
            "<text><![CDATA[some && cdata]]></text>\n"
            "</svg>\n";

    Document* doc = parser->parseBuffer(source);
    doc->serialize(*writer, "", 0, 0, true, true);
    writer->close();
    EXPECT_EQ(source, souts->getString());
}

TEST_F(SVGWriterTest, Indentation) {
    Glib::ustring result1 = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg>\n"
            " <g>\n"
            "  <g>\n"
            "   <g>\n"
            "    <g>\n"
            "     <g>\n"
            "      <g>\n"
            "       <g>\n"
            "        <g>\n"
            "         <g>\n"
            "          <g>\n"
            "           <g>\n"
            "            <g>\n"
            "             <g>\n"
            "              <g>\n"
            "               <g>\n"
            "               <g>\n"
            "               <g>\n"
            "               <g>\n"
            "               <g />\n"
            "               </g>\n"
            "               </g>\n"
            "               </g>\n"
            "               </g>\n"
            "              </g>\n"
            "             </g>\n"
            "            </g>\n"
            "           </g>\n"
            "          </g>\n"
            "         </g>\n"
            "        </g>\n"
            "       </g>\n"
            "      </g>\n"
            "     </g>\n"
            "    </g>\n"
            "   </g>\n"
            "  </g>\n"
            " </g>\n"
            "</svg>\n";

    Glib::ustring result2 = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg>\n"
            "  <g>\n"
            "    <g>\n"
            "      <g>\n"
            "        <g />\n"
            "      </g>\n"
            "    </g>\n"
            "  </g>\n"
            "</svg>\n";

    Document *doc1 = new SimpleDocument();
    doc1->appendChild(doc1->createElement("svg"));
    Node *node1 = doc1->root();
    for (int i = 0; i < 19; i++) {
        node1->appendChild(doc1->createElement("g"));
        node1 = node1->firstChild();
    }
    doc1->serialize(*writer, "", 1, 0, true, true);
    writer->close();
    EXPECT_EQ(result1, souts->getString());
    souts->clear();

    Document *doc2 = new SimpleDocument();
    doc2->appendChild(doc2->createElement("svg"));
    Node* node2 = doc2->root();
    for (int i = 0; i < 4; i++) {
        node2->appendChild(doc2->createElement("g"));
        node2 = node2->firstChild();
    }
    doc2->serialize(*writer, "", 2, 0, true, true);
    writer->close();
    EXPECT_EQ(result2, souts->getString());
}

TEST_F(SVGWriterTest, InlineAttributes) {
    Glib::ustring result1 = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg viewBox=\"0 0 96 105\">\n"
            "  <g fill=\"#97C024\" stroke=\"#97C024\" stroke-linejoin=\"round\" stroke-linecap=\"round\">\n"
            "    <path d=\"M32,5l5,10M64,5l-6,10\" stroke-width=\"2\" />\n"
            "  </g>\n"
            "  <circle cx=\"36\" cy=\"22\" r=\"2\" />\n"
            "  <text>Some text</text>\n"
            "</svg>\n";
    Glib::ustring result2 = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg\n"
            "   viewBox=\"0 0 96 105\">\n"
            "  <g\n"
            "     fill=\"#97C024\"\n"
            "     stroke=\"#97C024\"\n"
            "     stroke-linejoin=\"round\"\n"
            "     stroke-linecap=\"round\">\n"
            "    <path\n"
            "       d=\"M32,5l5,10M64,5l-6,10\"\n"
            "       stroke-width=\"2\" />\n"
            "  </g>\n"
            "  <circle\n"
            "     cx=\"36\"\n"
            "     cy=\"22\"\n"
            "     r=\"2\" />\n"
            "  <text>Some text</text>\n"
            "</svg>\n";

    Document *doc = new SimpleDocument();
    doc->appendChild(doc->createElement("svg"));
    Node *node = doc->root();
    node->setAttribute("viewBox", "0 0 96 105");
    node->appendChild(doc->createElement("g"));
    node->firstChild()->setAttribute("fill", "#97C024");
    node->firstChild()->setAttribute("stroke", "#97C024");
    node->firstChild()->setAttribute("stroke-linejoin", "round");
    node->firstChild()->setAttribute("stroke-linecap", "round");
    node->firstChild()->appendChild(doc->createElement("path"));
    node->firstChild()->firstChild()->setAttribute("d", "M32,5l5,10M64,5l-6,10");
    node->firstChild()->firstChild()->setAttribute("stroke-width", "2");
    node->appendChild(doc->createElement("circle"));
    node->nthChild(1)->setAttribute("cx", "36");
    node->nthChild(1)->setAttribute("cy", "22");
    node->nthChild(1)->setAttribute("r", "2");
    node->appendChild(doc->createElement("text"));
    node->nthChild(2)->appendChild(doc->createTextNode("Some text"));

    doc->serialize(*writer, "", 2, 0, true, true);
    writer->close();
    EXPECT_EQ(result1, souts->getString());
    souts->clear();

    doc->serialize(*writer, "", 2, 0, false, true);
    writer->close();
    EXPECT_EQ(result2, souts->getString());
}

TEST_F(SVGWriterTest, XmlSpaces) {
    Glib::ustring result = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg xml:space=\"preserve\">\n"
            "<text> An example text with space preserving.   </text>\n"
            "<g xml:space=\"default\">\n"
            "<text>An example text without space preserving.</text>\n"
            "</g>\n"
            "</svg>\n";

    Document* doc = new SimpleDocument();
    doc->appendChild(doc->createElement("svg"));
    Node *node = doc->root();
    node->setAttribute("xml:space", "preserve");
    node->appendChild(doc->createElement("text"));
    node->firstChild()->appendChild(doc->createTextNode(" An example text with space preserving.   "));
    node->appendChild(doc->createElement("g"));
    node->nthChild(1)->setAttribute("xml:space", "default");
    node->nthChild(1)->appendChild(doc->createElement("text"));
    node->nthChild(1)->firstChild()->appendChild(doc->createTextNode(" An example text without space preserving.   "));

    doc->serialize(*writer, "", 0, 0, true, false);
    writer->close();

    EXPECT_EQ(result, souts->getString());
}
