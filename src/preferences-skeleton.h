#define __PREFERENCES_SKELETON_H__

static const char preferences_skeleton[] =
"<inkscape version=\"" VERSION "\""
"  xmlns:sodipodi=\"http://inkscape.sourceforge.net/DTD/sodipodi-0.dtd\""
"  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\">"
"  <interface id=\"toolboxes\">"
"    <interface id=\"file\" state=\"1\"/>"
"    <interface id=\"edit\" state=\"1\"/>"
"    <interface id=\"object\" state=\"1\"/>"
"    <interface id=\"selection\" state=\"0\"/>"
"    <interface id=\"draw\" state=\"1\"/>"
"    <interface id=\"zoom\" state=\"0\"/>"
"    <interface id=\"node\" state=\"0\"/>"
"  </interface>"

"  <group id=\"documents\">"
"    <group id=\"recent\"/>"
"  </group>"

"  <group id=\"template\">"
"    <sodipodi:namedview"
"       id=\"base\""
"       pagecolor=\"#ffffff\""
"       bordercolor=\"#666666\""
"       borderopacity=\"1.0\""
"       inkscape:pageopacity=\"0.0\""
"       inkscape:pageshadow=\"2\""
"       inkscape:zoom=\"0.43415836\""
"       inkscape:cx=\"305.259528\""
"       inkscape:cy=\"417.849475\""
"       inkscape:window-width=\"640\""
"       inkscape:window-height=\"480\" />"
"  </group>"

"  <group id=\"tools\""
"         style=\"fill:none;fill-opacity:0.75;"
"                 stroke:black;stroke-opacity:1;stroke-width:1pt;stroke-linejoin:miter;stroke-linecap:butt;\">"
"    <group id=\"shapes\" style=\"fill-rule:evenodd;\">"
"      <eventcontext id=\"rect\" style=\"fill:blue;\"/>"
"      <eventcontext id=\"arc\" style=\"fill:red;\"/>"
"      <eventcontext id=\"star\" magnitude=\"5\" style=\"fill:yellow;\"/>"
"      <eventcontext id=\"spiral\" style=\"fill:none;stroke:black;\"/>"
"    </group>"
"    <group id=\"freehand\""
"         style=\"fill:none;fill-rule:evenodd;stroke:black;stroke-opacity:1;stroke-width:1pt;stroke-linejoin:miter;stroke-linecap:butt;\">"
"      <eventcontext id=\"pencil\" tolerance=\"10.0\"/>"
"      <eventcontext id=\"pen\" mode=\"drag\"/>"
"    </group>"
"    <eventcontext id=\"calligraphic\" style=\"fill:black;fill-rule:nonzero;stroke:none;\""
"                       mass=\"0.3\" drag=\"0.5\" angle=\"30\" width=\"0.2\"/>"
"    <eventcontext id=\"text\""
"                  style=\"fill:black;fill-opacity:1;stroke:none;font-family:helvetica;font-style:normal;font-weight:normal;font-size:12px;\"/>"
"    <eventcontext id=\"nodes\"/>"
"    <eventcontext id=\"zoom\"/>"
"    <eventcontext id=\"dropper\"/>"
"    <eventcontext id=\"select\"/>"
"  </group>"
"  <group id=\"palette\">"
"    <group id=\"dashes\">"
"      <dash id=\"solid\" style=\"stroke-dasharray:none;\"/>"
"      <dash id=\"dash-1-1\" style=\"stroke-dasharray:1 1;\"/>"
"      <dash id=\"dash-1-2\" style=\"stroke-dasharray:1 2;\"/>"
"      <dash id=\"dash-1-4\" style=\"stroke-dasharray:1 4;\"/>"
"      <dash id=\"dash-2-1\" style=\"stroke-dasharray:2 1;\"/>"
"      <dash id=\"dash-4-1\" style=\"stroke-dasharray:4 1;\"/>"
"      <dash id=\"dash-2-2\" style=\"stroke-dasharray:2 2;\"/>"
"      <dash id=\"dash-4-4\" style=\"stroke-dasharray:4 4;\"/>"
"      <dash id=\"dash-4-2-1-2\" style=\"stroke-dasharray:4 2 1 2;\"/>"
"    </group>"
"  </group>"
"  <group id=\"dialogs\">"
"    <group id=\"toolbox\"/>"
"    <group id=\"fillstroke\"/>"
"    <group id=\"textandfont\"/>"
"    <group id=\"transformation\"/>"
"    <group id=\"align\"/>"
"    <group id=\"xml\"/>"
"    <group id=\"documentoptions\"/>"
"    <group id=\"preferences\"/>"
"    <group id=\"gradienteditor\"/>"
"    <group id=\"export\">"
"      <group id=\"exportarea\"/>"
"      <group id=\"defaultxdpi\"/>"
"    </group>"
"  </group>"
"  <group id=\"printing\">"
"    <settings id=\"ps\"/>"
"  </group>"

"  <group id=\"options\">"
"    <group id=\"nudgedistance\" value=\"2.8346457\"/>"
"    <group id=\"rotationsnapsperpi\" value=\"12\"/>"
"    <group id=\"cursortolerance\" value=\"8.0\"/>"
"    <group id=\"dragtolerance\" value=\"4.0\"/>"
"    <group id=\"savewindowgeometry\" value=\"1\"/>"
"    <group id=\"defaultoffsetwidth\" value=\"2\"/>"
"    <group id=\"showrulers\" value=\"1\"/>"
"    <group id=\"showscrollbars\" value=\"1\"/>"
"    <group id=\"maxrecentdocuments\" value=\"20\"/>"
"    <group id=\"zoomincrement\" value=\"1.414213562\"/>"
"    <group id=\"keyscroll\" value=\"10\"/>"
"    <group id=\"wheelscroll\" value=\"40\"/>"
"    <group id=\"transientpolicy\" value=\"1\"/>"
"    <group id=\"scrollingacceleration\" value=\"0.35\"/>"
"    <group id=\"autoscrollspeed\" value=\"0.5\"/>"
"  </group>"

"</inkscape>";

#define PREFERENCES_SKELETON_SIZE (sizeof (preferences_skeleton) - 1)

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
