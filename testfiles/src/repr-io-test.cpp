/*
 * Unit tests for repr IO
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
#include <glibmm/ustring.h>
#include <src/xml/repr.h>

using namespace Inkscape::XML;

TEST(XmlIOTest, XmlSpaces) {
    Glib::ustring source1 = "<svg xml:space=\"preserve\">"
            "<text> An example text with space preserving.   </text>"
            "</svg>";
    Glib::ustring source2 = "<svg>"
        "<text> An example text without space preserving.   </text>"
        "</svg>";
    Glib::ustring source3 = "<svg xml:space=\"preserve\">"
        "<text> An example text with space preserving.   </text>"
        "<g xml:space=\"default\">"
        "<text> An example text without space preserving.   </text>"
        "</g>"
        "</svg>";
    Document* doc1 = IO::read_svg_buffer(source1);
    EXPECT_STREQ("text", doc1->root()->firstChild()->name());
    EXPECT_STREQ(" An example text with space preserving.   ", doc1->root()->firstChild()->firstChild()->content());
    Document* doc2 = IO::read_svg_buffer(source2);
    EXPECT_STREQ("An example text without space preserving.", doc2->root()->firstChild()->firstChild()->content());
    EXPECT_STREQ("text", doc2->root()->firstChild()->name());
    Document* doc3 = IO::read_svg_buffer(source3);
    EXPECT_STREQ("text", doc3->root()->firstChild()->name());
    EXPECT_STREQ(" An example text with space preserving.   ", doc3->root()->firstChild()->firstChild()->content());
    EXPECT_STREQ("text", doc3->root()->lastChild()->firstChild()->name());
    EXPECT_STREQ("An example text without space preserving.", doc3->root()->lastChild()->firstChild()->firstChild()->content());
}
