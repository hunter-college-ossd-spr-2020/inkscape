/*
 * Unit tests for repr IO functions
 *
 * Authors:
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtest/gtest.h>
#include <glibmm/ustring.h>
#include <src/xml/repr-io.h>
#include <src/xml/simple-document.h>
#include <src/xml/repr.h>
#include <src/extension/extension.h>
#include <algorithm>

using namespace Inkscape::XML;

TEST(ReprIOTest, Namespaces) {
    std::string expected1 = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg\n"
            "   xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
            "   xmlns:extension=\"http://www.inkscape.org/namespace/inkscape/extension\"\n"
            "   xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
            "   xmlns=\"http://www.w3.org/2000/svg\">\n"
            "  <extension:ext\n"
            "     xlink:href=\"a.jpg\" />\n"
            "</svg>";
    std::string expected2 = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg:svg\n"
            "   xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
            "   xmlns:extension=\"http://www.inkscape.org/namespace/inkscape/extension\"\n"
            "   xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
            "   xmlns=\"http://www.inkscape.org/namespace/inkscape/extension\">\n"
            "  <ext\n"
            "     xlink:href=\"a.jpg\" />\n"
            "</svg:svg>";
    Document *doc = new SimpleDocument();
    doc->appendChild(doc->createElement("svg:svg"));
    Node *child = doc->createElement("extension:ext");
    child->setAttribute("xlink:href", "a.jpg");
    doc->firstChild()->appendChild(child);
    Glib::ustring d1 = IO::save_svg_buffer(doc, SP_SVG_NS_URI);
    std::string actual1 = d1.raw();
    actual1.erase(std::remove_if(actual1.begin(), actual1.end(), isspace), actual1.end());
    expected1.erase(std::remove_if(expected1.begin(), expected1.end(), isspace), expected1.end());
    Glib::ustring d2 = IO::save_svg_buffer(doc, INKSCAPE_EXTENSION_URI);
    std::string actual2 = d2.raw();
    actual2.erase(std::remove_if(actual2.begin(), actual2.end(), isspace), actual2.end());
    expected2.erase(std::remove_if(expected2.begin(), expected2.end(), isspace), expected2.end());
    EXPECT_EQ(expected1, actual1);
    EXPECT_EQ(expected2, actual2);
}

TEST(ReprIOTest, Rebasing) {
    std::string expected = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<svg xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
            "<image xlink:href=\"user/a.jpg\" />"
            "</svg>";
    Document *doc = new SimpleDocument();
    doc->appendChild(doc->createElement("svg"));
    Node *child = doc->createElement("image");
    child->setAttribute("xlink:href", "a.jpg");
    doc->firstChild()->appendChild(child);
    Glib::ustring d = IO::save_svg_buffer(doc, "", "/home/user", "/home/a.svg");
    std::string actual = d.raw();
    actual.erase(std::remove_if(actual.begin(), actual.end(), isspace), actual.end());
    expected.erase(std::remove_if(expected.begin(), expected.end(), isspace), expected.end());
    EXPECT_EQ(expected, actual);
}
