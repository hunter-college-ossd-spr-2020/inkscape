#!/usr/bin/env python 
'''
This extension converts a path into a dashed line using 'stroke-dasharray'
It is a modification of the file addnodes.py

Copyright (C) 2005,2007 Aaron Spike, aaron@ekips.org
Copyright (C) 2009 Alvin Penner, penner@vaxxine.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''

import inkex, cubicsuperpath, bezmisc, simplestyle

def tpoint(xxx_todo_changeme, xxx_todo_changeme1, t = 0.5):
    (x1,y1) = xxx_todo_changeme
    (x2,y2) = xxx_todo_changeme1
    return [x1+t*(x2-x1),y1+t*(y2-y1)]
def cspbezsplit(sp1, sp2, t = 0.5):
    m1=tpoint(sp1[1],sp1[2],t)
    m2=tpoint(sp1[2],sp2[0],t)
    m3=tpoint(sp2[0],sp2[1],t)
    m4=tpoint(m1,m2,t)
    m5=tpoint(m2,m3,t)
    m=tpoint(m4,m5,t)
    return [[sp1[0][:],sp1[1][:],m1], [m4,m,m5], [m3,sp2[1][:],sp2[2][:]]]
def cspbezsplitatlength(sp1, sp2, l = 0.5, tolerance = 0.001):
    bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
    t = bezmisc.beziertatlength(bez, l, tolerance)
    return cspbezsplit(sp1, sp2, t)
def cspseglength(sp1,sp2, tolerance = 0.001):
    bez = (sp1[1][:],sp1[2][:],sp2[0][:],sp2[1][:])
    return bezmisc.bezierlength(bez, tolerance)    

class SplitIt(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

    def effect(self):
        for id, node in list(self.selected.items()):
            if node.tag == inkex.addNS('path','svg'):
                dashes = []
                style = simplestyle.parseStyle(node.get('style'))
                if 'stroke-dasharray' in style:
                    if style['stroke-dasharray'].find(',') > 0:
                        dashes = [float (dash) for dash in style['stroke-dasharray'].split(',')]
                if dashes:
                    p = cubicsuperpath.parsePath(node.get('d'))
                    new = []
                    for sub in p:
                        idash = 0
                        dash = dashes[0]
                        length = 0
                        new.append([sub[0][:]])
                        i = 1
                        while i < len(sub):
                            dash = dash - length
                            length = cspseglength(new[-1][-1], sub[i])
                            while dash < length:
                                new[-1][-1], next, sub[i] = cspbezsplitatlength(new[-1][-1], sub[i], dash/length)
                                if idash % 2:           # create a gap
                                    new.append([next[:]])
                                else:                   # splice the curve
                                    new[-1].append(next[:])
                                length = length - dash
                                idash = (idash + 1) % len(dashes)
                                dash = dashes[idash]
                            if idash % 2:
                                new.append([sub[i]])
                            else:
                                new[-1].append(sub[i])
                            i+=1
                    node.set('d',cubicsuperpath.formatPath(new))

if __name__ == '__main__':
    e = SplitIt()
    e.affect()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
