#ifndef __SVGTYPES_H__
#define __SVGTYPES_H__

/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *   Andreas Becker
 *
 * Copyright (C) 2006-2008 Bob Jamison
 * Copyright (C) 2011 Andreas Becker
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * =======================================================================
 * NOTES
 *
 * This API follows:
 * http://www.w3.org/TR/SVG11/svgdom.html
 * 
 * This file contains the definitions of the non-Node SVG classes.  DOM Nodes
 * for SVG are defined in svg.h.
 */

// For access to DOM2 core
#include "dom/dom.h"

// For access to DOM2 events
#include "dom/events.h"

// For access to those parts from DOM2 CSS OM used by SVG DOM.
#include "dom/css.h"

// For access to those parts from DOM2 Views OM used by SVG DOM.
#include "dom/views.h"

// For access to the SMIL OM used by SVG DOM.
#include "dom/smil.h"

#include <math.h>

namespace org {
namespace w3c {
namespace dom {
namespace svg {

// local definitions
typedef dom::DOMString DOMString;
typedef dom::DOMException DOMException;
typedef dom::Element Element;
typedef dom::Document Document;
typedef dom::NodeList NodeList;

class SVGElement;
typedef Ptr<SVGElement> SVGElementPtr;
class SVGUseElement;
typedef Ptr<SVGUseElement> SVGUseElementPtr;
class SVGAnimatedPreserveAspectRatio;

typedef SVGList<SVGString> SVGStringList;
typedef SVGList<SVGNumber> SVGNumberList;
typedef SVGList<SVGLength> SVGLengthList;

typedef SVGAnimated<bool> SVGAnimatedBoolean;
typedef SVGAnimated<DOMString> SVGAnimatedString;
typedef SVGAnimated<unsigned short> SVGAnimatedEnumeration;
typedef SVGAnimated<long> SVGAnimatedInteger;
typedef SVGAnimated<float> SVGAnimatedNumber;
typedef SVGAnimated<SVGList<SVGNumber> > SVGAnimatedNumberList;
typedef SVGAnimated<SVGLength> SVGAnimatedLength;
typedef SVGAnimated<SVGList<SVGNumber> > SVGAnimatedLengthList;
typedef SVGAnimated<SVGAngle> SVGAnimatedAngle;
typedef SVGAnimated<SVGRect> SVGAnimatedRect;

/**
 *
 */
class SVGException
{
public:
    /**
     * A code identifying the reason why the requested operation could not be performed. The value
     * of this member will be one of the constants in the SVGException code group.
     */
    unsigned short code;

    enum
    {
        /** Raised when an object of the wrong type is passed to an operation. */
        SVG_WRONG_TYPE_ERR = 0,

        /** Raised when an invalid value is passed to an operation or assigned to an attribute. */
        SVG_INVALID_VALUE_ERR = 1,

        /** Raised when an attempt is made to invert a matrix that is not invertible. */
        SVG_MATRIX_NOT_INVERTABLE = 2
    };
};

/**
 *  In SVG, a Matrix is defined like this:
 *
 * | a  c  e |
 * | b  d  f |
 * | 0  0  1 |
 *
 */
class SVGMatrix
{
public:

    /**
     *
     */
    double getA()
    { return a; }

    /**
     *
     */
    void setA(double val) throw (DOMException)
        { a = val; }

    /**
     *
     */
    double getB()
        { return b; }

    /**
     *
     */
    void setB(double val) throw (DOMException)
        { b = val; }

    /**
     *
     */
    double getC()
        { return c; }

    /**
     *
     */
    void setC(double val) throw (DOMException)
        { c = val; }

    /**
     *
     */
    double getD()
        { return d; }

    /**
     *
     */
    void setD(double val) throw (DOMException)
        { d = val; }
    /**
     *
     */
    double getE() { return e; }

    /**
     *
     */
    void setE(double val) throw (DOMException) { e = val; }
    /**
     *
     */
    double getF() { return f; }

    /**
     *
     */
    void setF(double val) throw (DOMException)
        { f = val; }

    /**
     * Return the result of postmultiplying this matrix with another.
     */
    SVGMatrix multiply(const SVGMatrix &other)
    {
        SVGMatrix result;
        result.a = a * other.a  +  c * other.b;
        result.b = b * other.a  +  d * other.b;
        result.c = a * other.c  +  c * other.d;
        result.d = b * other.c  +  d * other.d;
        result.e = a * other.e  +  c * other.f  +  e;
        result.f = b * other.e  +  d * other.f  +  f;
        return result;
    }

    /**
     *  Calculate the inverse of this matrix
     *
     */
    SVGMatrix inverse(  ) throw( SVGException )
    {
        /*###########################################
        The determinant of a 3x3 matrix E
           (let's use our own notation for a bit)

            A  B  C
            D  E  F
            G  H  I
        is
            AEI - AFH - BDI + BFG + CDH - CEG

        Since in our affine transforms, G and H==0 and I==1,
        this reduces to:
            AE - BD
        In SVG's naming scheme, that is:  a * d - c * b .  SIMPLE!

        In a similar method of attack, SVG's adjunct matrix is:

           d  -c   cf-ed
          -b   a   eb-af
           0   0   ad-cb

        To get the inverse matrix, we divide the adjunct matrix by
        the determinant.  Notice that (ad-cb)/(ad-cb)==1.  Very cool.
        So what we end up with is this:

           a =  d/(ad-cb)  c = -c/(ad-cb)   e = (cf-ed)/(ad-cb)
           b = -b/(ad-cb)  d =  a/(ad-cb)   f = (eb-af)/(ad-cb)

        (Since this would be in all SVG-DOM implementations,
         somebody needed to document this!  ^^ )
        #############################################*/

        SVGMatrix result;
        double determinant = a * d  -  c * b;
        if (determinant < 1.0e-18)//invertible?
            {
            result.identity();//cop out
            return result;
            }

        double idet = 1.0 / determinant;
        result.a =   d * idet;
        result.b =  -b * idet;
        result.c =  -c * idet;
        result.d =   a * idet;
        result.e =  (c*f - e*d) * idet;
        result.f =  (e*b - a*f) * idet;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | 1  0  x |
     *  | 0  1  y |
     *  | 0  0  1 |
     *
     */
    SVGMatrix translate(double x, double y)
    {
        SVGMatrix result;
        result.a = a;
        result.b = b;
        result.c = c;
        result.d = d;
        result.e = a * x  +  c * y  +  e;
        result.f = b * x  +  d * y  +  f;
        return result;
    }

    /**
     * Equivalent to multiplying by:
     *  | scale  0      0 |
     *  | 0      scale  0 |
     *  | 0      0      1 |
     *
     */
    SVGMatrix scale(double scale)
    {
        SVGMatrix result;
        result.a = a * scale;
        result.b = b * scale;
        result.c = c * scale;
        result.d = d * scale;
        result.e = e;
        result.f = f;
        return result;
    }

    /**
     * Equivalent to multiplying by:
     *  | scaleX  0       0 |
     *  | 0       scaleY  0 |
     *  | 0       0       1 |
     *
     */
    SVGMatrix scaleNonUniform(double scaleX, double scaleY)
    {
        SVGMatrix result;
        result.a = a * scaleX;
        result.b = b * scaleX;
        result.c = c * scaleY;
        result.d = d * scaleY;
        result.e = e;
        result.f = f;
        return result;
    }

    /**
     * Equivalent to multiplying by:
     *  | cos(a) -sin(a)   0 |
     *  | sin(a)  cos(a)   0 |
     *  | 0       0        1 |
     *
     */
    SVGMatrix rotate (double angle)
     {
        double sina  = sin(angle);
        double msina = -sina;
        double cosa  = cos(angle);
        SVGMatrix result;
        result.a = a * cosa   +  c * sina;
        result.b = b * cosa   +  d + sina;
        result.c = a * msina  +  c * cosa;
        result.d = b * msina  +  d * cosa;
        result.e = e;
        result.f = f;
        return result;
    }

    /**
     * Equivalent to multiplying by:
     *  | cos(a) -sin(a)   0 |
     *  | sin(a)  cos(a)   0 |
     *  | 0       0        1 |
     *  In this case, angle 'a' is computed as the artangent
     *  of the slope y/x .  It is negative if the slope is negative.
     */
    SVGMatrix rotateFromVector(double x, double y)
                                      throw( SVGException )
    {
        double angle = atan(y / x);
        if (y < 0.0)
            angle = -angle;
        SVGMatrix result;
        double sina  = sin(angle);
        double msina = -sina;
        double cosa  = cos(angle);
        result.a = a * cosa   +  c * sina;
        result.b = b * cosa   +  d + sina;
        result.c = a * msina  +  c * cosa;
        result.d = b * msina  +  d * cosa;
        result.e = e;
        result.f = f;
        return result;
    }

    /**
     * Equivalent to multiplying by:
     *  | -1   0   0 |
     *  | 0    1   0 |
     *  | 0    0   1 |
     *
     */
    SVGMatrix flipX()
    {
        SVGMatrix result;
        result.a = -a;
        result.b = -b;
        result.c =  c;
        result.d =  d;
        result.e =  e;
        result.f =  f;
        return result;
    }

    /**
     * Equivalent to multiplying by:
     *  | 1   0   0 |
     *  | 0  -1   0 |
     *  | 0   0   1 |
     *
     */
    SVGMatrix flipY()
    {
        SVGMatrix result;
        result.a =  a;
        result.b =  b;
        result.c = -c;
        result.d = -d;
        result.e =  e;
        result.f =  f;
        return result;
    }

    /**
     *  | 1   tan(a)  0 |
     *  | 0   1       0 |
     *  | 0   0       1 |
     *
     */
    SVGMatrix skewX(double angle)
    {
        double tana = tan(angle);
        SVGMatrix result;
        result.a =  a;
        result.b =  b;
        result.c =  a * tana + c;
        result.d =  b * tana + d;
        result.e =  e;
        result.f =  f;
        return result;
    }

    /**
     * Equivalent to multiplying by:
     *  | 1       0   0 |
     *  | tan(a)  1   0 |
     *  | 0       0   1 |
     *
     */
    SVGMatrix skewY(double angle)
    {
        double tana = tan(angle);
        SVGMatrix result;
        result.a =  a + c * tana;
        result.b =  b + d * tana;
        result.c =  c;
        result.d =  d;
        result.e =  e;
        result.f =  f;
        return result;
    }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGMatrix()
    {
        identity();
    }

    /**
     *
     */
    SVGMatrix(double aArg, double bArg, double cArg,
              double dArg, double eArg, double fArg )
    {
        a = aArg; b = bArg; c = cArg;
        d = dArg; e = eArg; f = fArg;
    }

    /**
     * Copy constructor
     */
    SVGMatrix(const SVGMatrix &other)
    {
        a = other.a;
        b = other.b;
        c = other.c;
        d = other.d;
        e = other.e;
        f = other.f;
    }

protected:

friend class SVGTransform;

    /*
     * Set to the identify matrix
     */
   void identity()
   {
       a = 1.0;
       b = 0.0;
       c = 0.0;
       d = 1.0;
       e = 0.0;
       f = 0.0;
   }

    double a, b, c, d, e, f;
};

/**
 *
 */
class SVGTransform
{
public:

    /**
     * Transform Types
     */
    typedef enum
    {
        SVG_TRANSFORM_UNKNOWN   = 0,
        SVG_TRANSFORM_MATRIX    = 1,
        SVG_TRANSFORM_TRANSLATE = 2,
        SVG_TRANSFORM_SCALE     = 3,
        SVG_TRANSFORM_ROTATE    = 4,
        SVG_TRANSFORM_SKEWX     = 5,
        SVG_TRANSFORM_SKEWY     = 6,
    } TransformType;

    /**
     *
     */
    unsigned short getType()
    {
        return type;
    }

    /**
     *
     */
    SVGMatrix getMatrix()
    {
        return matrix;
    }

    /**
     *
     */
    double getAngle()
    {
        return angle;
    }


    /**
     *
     */
    void setMatrix(const SVGMatrix &matrixArg)
    {
        type = SVG_TRANSFORM_MATRIX;
        matrix = matrixArg;
    }

    /**
     *
     */
    void setTranslate(double tx, double ty)
    {
        type = SVG_TRANSFORM_TRANSLATE;
        matrix.setA(1.0);
        matrix.setB(0.0);
        matrix.setC(0.0);
        matrix.setD(1.0);
        matrix.setE(tx);
        matrix.setF(ty);
    }

    /**
     *
     */
    void setScale(double sx, double sy)
    {
        type = SVG_TRANSFORM_SCALE;
        matrix.setA(sx);
        matrix.setB(0.0);
        matrix.setC(0.0);
        matrix.setD(sy);
        matrix.setE(0.0);
        matrix.setF(0.0);
    }

    /**
     *
     */
    void setRotate (double angleArg, double cx, double cy)
    {
        angle = angleArg;
        setTranslate(cx, cy);
        type = SVG_TRANSFORM_ROTATE;
        matrix.rotate(angle);
    }

    /**
     *
     */
    void setSkewX (double angleArg)
    {
        angle = angleArg;
        type = SVG_TRANSFORM_SKEWX;
        matrix.identity();
        matrix.skewX(angle);
    }

    /**
     *
     */
    void setSkewY (double angleArg)
    {
        angle = angleArg;
        type = SVG_TRANSFORM_SKEWY;
        matrix.identity();
        matrix.skewY(angle);
    }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTransform()
    {
        type = SVG_TRANSFORM_UNKNOWN;
        angle = 0.0;
    }

    /**
     *
     */
    SVGTransform(const SVGTransform &other)
    {
        type   = other.type;
        angle  = other.angle;
        matrix = other.matrix;
    }

    /**
     *
     */
    ~SVGTransform()
    {}

protected:

    int type;
    double angle;

    SVGMatrix matrix;
};

class SVGNumber
{
public:
    SVGNumber(double val) :
        double value(val)
    {}

    /**
     * Gets the value of the given attribute.
     */
    double getValue()
    {
        return value;
    }

    /**
     * Sets the value of the given attribute.
     */
    void setValue(double val)
    {
        value = val;
    }

prrivate:
    double value;
};

template<typename T>
class SVGAnimated
{
public:
    /** Gets the base value of the given attribute before applying any animations. */
    T getBaseVal() { return baseVal; }

    /** Gets the current animated value of the given attribute. */
    T getAnimVal() { return animVal; }

private:
    T baseVal, animVal;
};

/**
 * SVGLength corresponds to the <length> basic data type.
 */
class SVGLength
{
public:
    /** Length Unit Types */
    typedef enum
    {
        SVG_LENGTHTYPE_UNKNOWN    = 0,
        SVG_LENGTHTYPE_NUMBER     = 1,
        SVG_LENGTHTYPE_PERCENTAGE = 2,
        SVG_LENGTHTYPE_EMS        = 3,
        SVG_LENGTHTYPE_EXS        = 4,
        SVG_LENGTHTYPE_PX         = 5,
        SVG_LENGTHTYPE_CM         = 6,
        SVG_LENGTHTYPE_MM         = 7,
        SVG_LENGTHTYPE_IN         = 8,
        SVG_LENGTHTYPE_PT         = 9,
        SVG_LENGTHTYPE_PC         = 10
    } SVGLengthType;

    /** Gets the unit type of the value. */
    SVGLengthType getUnitType() { return unitType; }

    /** Gets the value of the length. */
    double getValue() { return value; }

    /**
     * Sets the value of the length.
     * \exception DOMException (code NO_MODIFICATION_ALLOWED_ERR) Raised when the length
     * corresponds to a read only attribute or when the object itself is read only.
     */
    void setValue(double val)
        throw (DOMException)
    {
        value = val;
    }

    /**
     * Gets the value as a floating point value, in the units expressed by getUnitType.
     */
    double getValueInSpecifiedUnits()
    {
        return value * getConversionFactor();
    }

    /**
     * Sets the value as a floating point value, in the units expressed by getUnitType.
     */
    void setValueInSpecifiedUnits(double val)
        throw (DOMException)
    {
        value = value / getConversionFactor();
    }

    /**
     *
     */
    DOMString getValueAsString()
    {
        DOMString result;
        char buf[32];
        snprintf(buf, 31, "%f", value); // TODO improve this #*(/&*%$
        result.append(buf);
        switch (unitType) {
            case SVG_LENGTHTYPE_PERCENTAGE:
                result.append("%");
                break;
            case SVG_LENGTHTYPE_EMS:
                result.append("em");
                break;
            case SVG_LENGTHTYPE_EXS:
                result.append("ex");
                break;
            case SVG_LENGTHTYPE_PX:
                result.append("px");
                break;
            case SVG_LENGTHTYPE_CM:
                result.append("cm");
                break;
            case SVG_LENGTHTYPE_MM:
                result.append("mm");
                break;
            case SVG_LENGTHTYPE_IN:
                result.append("in");
                break;
            case SVG_LENGTHTYPE_PT:
                result.append("pt");
                break;
            case SVG_LENGTHTYPE_PC:
                result.append("pc");
                break;
        }
        return result;
    }


#define UVAL(a,b) (((unsigned int) (a) << 8) | (unsigned int) (b))

    /**
     * Sets the length value and unit from the given string.
     */
    void setValueAsString(const DOMString& val)
        throw (DOMException)
    {
        gchar *endptr;
        double dbl = g_ascii_strtod(val, &endptr);
        if (endptr[0] == '%' && endptr[1] == '\0') {
            unitType = SVG_LENGTHTYPE_PERCENT;
        }
        else if (endptr[0] == '\0' || endptr[1] == '\0' || endptr[2] != '\0') {
            unitType = SVG_LENGTHTYPE_UNSPECIFIED;
        }
        else {
            unsigned const int uval = UVAL(e[0], e[1]);
            switch (uval) {
                case UVAL('e', 'm'):
                    unitType = SVG_LENGTHTYPE_EM;
                    value = dbl*PX_PER_EM;
                    break;
                case UVAL('e', 'x'):
                    unitType = SVG_LENGTHTYPE_EX;
                    value = dbl*PX_PER_EX;
                    break;
                case UVAL('p', 'x'):
                    unitType = SVG_LENGTHTYPE_PX;
                    value = dbl;
                    break;
                case UVAL('c', 'm'):
                    unitType = SVG_LENGTHTYPE_CM;
                    value = dbl*PX_PER_CM;
                    break;
                case UVAL('m', 'm'):
                    unitType = SVG_LENGTHTYPE_MM;
                    value = dbl*PX_PER_MM;
                    break;
                case UVAL('i', 'n'):
                    unitType = SVG_LENGTHTYPE_IN;
                    value = dbl*PX_PER_IN;
                    break;
                case UVAL('p', 't'):
                    unitType = SVG_LENGTHTYPE_PT;
                    value = dbl*PX_PER_PT;
                    break;
                case UVAL('p', 'c'):
                    unitType = SVG_LENGHTTYPE_PC;
                    value = dbl*PX_PER_PC;
                    break;
            }
        }
    }

    /**
     * Reset the value as a number with an associated unitType, thereby replacing the values for
     * all of the attributes on the object.
     * \param newType 
     * \param val
     * \exception DOMException (code NOT_SUPPORTED_ERR) Raised if unitType is
     * SVG_LENGTHTYPE_UNKNOWN or not a valid unit type constant (one of the other SVG_LENGTHTYPE_*
     * constants defined on this interface).
     * \exception DOMException (code NO_MODIFICATION_ALLOWED_ERR) Raised when the length
     * corresponds to a read only attribute or when the object itself is read only.
     */
    void newValueSpecifiedUnits(SVGLengthType newType, double val)
    {
        if (newType == SVG_LENGTHTYPE_UNKNOWN) {
            throw DOMException(NOT_SUPPORTED_ERR);
        }
        unitType = newType;
        value = val;
    }

    /**
     * Preserve the same underlying stored value, but reset the stored unit identifier to the given
     * unitType.
     * \param newType The unit type to switch to.
     * \exception DOMException (code NOT_SUPPORTED_ERR) Raised if unitType is
    *  SVG_LENGTHTYPE_UNKNOWN.
     */
    void convertToSpecifiedUnits(SVGLengthType newType)
    {
        if (newType == SVG_LENGTHTYPE_UNSPECIFIED) {
            throw DOMException(NOT_SUPPORTED_ERR);
        }
        unitType = newType;
    }

    /**
     * Default constructor, sets the value to zero and the unit type to unspecified.
     */
    SVGLength() :
        value(0),
        unitType(SVG_LENGTHTYPE_UNSPECIFIED)
    {}

    /**
     * Constructor that calls newValueSpecifiedArguments with the given arguments.
     */
    SVGLength(SVGLengthType newType, double val)
    {
        newValueSpecifiedUnits(newType, val);
    }

private:
    /**
     * Gets the conversion factor from px to the current unit.
     */
    double getConversionFactor()
    {
        // http://www.w3.org/TR/SVG/coords.html#Units
        // http://www.w3.org/TR/CSS2/syndata.html#length-units
        // TODO move dpi to another place or make it even configurable in UI
        // w3c proposes 90 dpi
        int dpi = 90;
        switch (unitType) {
            case SVG_LENGTHTYPE_CM:
                return 2.54 / dpi;
            case SVG_LENGTHTYPE_IN:
                return dpi;
            case SVG_LENGTHTYPE_MM:
                return 25.4 / dpi;
            case SVG_LENGTHTYPE_PT:
                return 72 / dpi;
            case SVG_LENGTHTYPE_PC:
                return 6 / dpi; // 72 / 12 = 6
        }

        // for all relative units or px or unspecified:
        return 1;
    }

    /** The unit type of the length. */
    SVGLengthType unitType;

    /** The actual length in pixels (px). */
    double value;
};

/**
 * Defines a list of DOM objects.
 */
template<typename T>
class SVGList
{
public:
    /**
     * The number of items in the list.
     */
    unsigned long getNumberOfItems()
    {
        return items.size();
    }

    /**
     * Clears all existing current items from the list, with the result being an empty list.
     * \exception DOMException Raised when the list corresponds to a read only attribute or when the
     * object itself is read only.
     */
    void clear()
        throw(DOMException)
    {
        items.clear();
    }

    /**
     * Clears all existing current items from the list and re-initializes the list to hold the
     * single item specified by the parameter. If the inserted item is already in a list, it is
     * removed from its previous list before it is inserted into this list. The inserted item is
     * the item itself and not a copy.
     * \param newItem The item which should become the only member of the list.
     * \return The item being inserted into the list.
     * \exception DOMException Raised when the list corresponds to a read only attribute or when the
     * object itself is read only.
     */
    T & initialize(const T &newItem)
        throw(DOMException)
    {
        items.clear();
        items.push_back(newItem);
        return newItem;
    }

    /**
     * Returns the specified item from the list. The returned item is the item itself and not a
     * copy. Any changes made to the item are immediately reflected in the list.
     * \param index The index of the item from the list which is to be returned. The first item is
     * number 0.
     * \return The selected item.
     * \exception DOMException Raised when the list corresponds to a read only attribute or when the
     * object itself is read only.
     */
    T & getItem(unsigned long index)
        throw(DOMException)
    {
        if (index >= items.size())
        {
            SVGLength ret;
            return ret;
        }
        return items[index];
    }

    /**
     * Inserts a new item into the list at the specified position. The first item is number 0. If
     * newItem is already in a list, it is removed from its previous list before it is inserted
     * into this list. The inserted item is the item itself and not a copy. If the item is already
     * in this list, note that the index of the item to insert before is before the removal of the
     * item.
     * \param newItem The item which is to be inserted into the list.
     * \param index The index of the item before which the new item is to be inserted. The first
     * item is number 0. If the index is equal to 0, then the new item is inserted at the front of
     * the list. If the index is greater than or equal to numberOfItems, then the new item is
     * appended to the end of the list.
     * \return The inserted item.
     * \exception DOMException Raised when the list corresponds to a read only attribute or when
     * the object itself is read only.
     */
    T & insertItemBefore(const T &newItem, unsigned long index)
        throw(DOMException)
    {
        if (index >= items.size())
        {
            items.push_back(newItem);
        }
        else
        {// TODO
            std::vector<T>::iterator iter = items.begin() + index;
            items.insert(iter, newItem);
        }
        return newItem;
    }

    /**
     * Replaces an existing item in the list with a new item. If newItem is already in a list, it
     * is removed from its previous list before it is inserted into this list. The inserted item is
     * the item itself and not a copy. If the item is already in this list, note that the index of
     * the item to replace is before the removal of the item.
     * \param newItem The item which is to be inserted into the list.
     * \param index The index of the item which is to be replaced. The first item is number 0.
     * \return The inserted item.
     * \exception DOMException (code NO_MODIFICATION_ALLOWED_ERR) Raised when the list corresponds
     * to a read only attribute or when the object itself is read only.
     * the object itself is read only.
     * \exception DOMException (code INDEX_SIZE_ERR) Raised if the index number is greater than or
     * equal to numberOfItems.
     */
    T & replaceItem(const T &newItem,
                          unsigned long index)
        throw(DOMException)
    {
        if (index >= items.size())
        {
            throw DOMException(INDEX_SIZE_ERR);
        }
        std::vector<T>::iterator iter = items.begin() + index;
        *iter = newItem;
        return newItem;
    }

    /**
     * Removes an existing item from the list.
     * \param index The index of the item which is to be removed. The first item is number 0.
     * \return The removed item.
     * \exception DOMException (code NO_MODIFICATION_ALLOWED_ERR) Raised when the list corresponds
     * to a read only attribute or when the object itself is read only.
     * the object itself is read only.
     * \exception DOMException (code INDEX_SIZE_ERR) Raised if the index number is greater than or
     * equal to numberOfItems.
     */
    T removeItem(unsigned long index)
        throw(DOMException)
    {
        if (index >= items.size())
        {
            throw DOMException(INDEX_SIZE_ERR);
        }
        std::vector<T>::iterator iter = items.begin() + index;
        SVGLength oldval = *iter;
        items.erase(iter);
        return oldval;
    }

    /**
     * Inserts a new item at the end of the list. If newItem is already in a list, it is removed
     * from its previous list before it is inserted into this list. The inserted item is the item
     * itself and not a copy.
     * \param newItem The item which is to be inserted. The first item is number 0.
     * \return The inserted item.
     * \exception DOMException (code NO_MODIFICATION_ALLOWED_ERR) Raised when the list corresponds
     * to a read only attribute or when the object itself is read only.
     */
    T appendItem (const T &newItem)
        throw(DOMException)
    {
        items.push_back(newItem);
        return newItem;
    }

private:
    std::vector<T> items;
};

/**
 *
 */
class SVGAngle
{
public:
    /**
     * Angle Unit Types
     */
    typedef enum
    {
        SVG_ANGLETYPE_UNKNOWN     = 0,
        SVG_ANGLETYPE_UNSPECIFIED = 1,
        SVG_ANGLETYPE_DEG         = 2,
        SVG_ANGLETYPE_RAD         = 3,
        SVG_ANGLETYPE_GRAD        = 4
    } SVGAngleUnitType;

    /** Gets the type of the value. */
    SVGAngleUnitType getUnitType() { return unitType; }

    /** Gets the angle value as floating point value, in degrees. */
    double getValue() { return value; }

    /**
     *
     */
    void setValue(double val)
        throw (DOMException)
    {
        value = val;
    }

    /**
     * Gets the angle value as a floating point value, in the units expressed by unitType.
     */
    double getValueInSpecifiedUnits()
    {
        if (unitType == SVG_ANGLETYPE_RAD) {
                return value/180*M_PI;
        }
        if (unitType == SVG_ANGLETYPE_GRAD) {
                return value/9*10;
        }
        return value;
    }

    /**
     * Sets the angle value as a floating point value, in the units expressed by unitType.
     * \exception DOMException (code NO_MODIFICATION_ALLOWED_ERR) Raised when the angle
     * corresponds to a read only attribute or when the object itself is read only.
     */
    void setValueInSpecifiedUnits(double val)
        throw (DOMException)
    {
        if (unitType == SVG_ANGLETYPE_RAD) {
            value = val/M_PI*180;
        }
        else if (unitType == SVG_ANGLETYPE_GRAD) {
            value = val/10*9;
        }
        else {
            value = val;
        }
    }

    /**
     * Gets the angle value as a string. Example output: '4.2deg', '42'.
     */
    DOMString getValueAsString()
    {
        DOMString result;
        char buf[32];
        snprintf(buf, 31, "%f", value);
        result.append(buf);
        switch (unitType) {
            case SVG_ANGLETYPE_DEG:
                result.append("deg");
                break;
            case SVG_ANGLETYPE_RAD:
                result.append("rad");
                break;
            case SVG_ANGLE_TYPE_GRAD:
                result.append("grad");
                break
        }
        return result;
    }

    /**
     * Sets the angle value from a string value. The unit type might change, see
     * http://www.w3.org/Graphics/SVG/WG/track/issues/2216.
     * \par str String containing the angle information. Example: '4.2deg', '42'.
     */
    void setValueAsString(const DOMString & str)
        throw (DOMException)
    {
        gchar *e;
        const float dbl = g_ascii_strtod(str, &e);
        if (e == str) {
            // cannot convert str to a number
            unitType = SVG_ANGLETYPE_UNSPECIFIED;
            value = 0F;
        }

        if (e[0] == 'd' && e[1] == 'e' && e[2] == 'g' && e[3] == '\0') {
            unitType = SVG_ANGLETYPE_DEG;
            value = dbl;
        }
        else if (e[0] == 'r' && e[1] == 'a' && e[2] == 'd' && e[3] == '\0') {
            unitType = SVG_ANGLETYPE_RAD;
            value = dbl/M_PI*180;
        }
        else if (e[0] == 'g' && e[1] == 'r' && e[2] == 'a' && e[3] == 'd' && e[4] == '\0') {
            unitType = SVG_ANGLETYPE_GRAD;
            value = dbl/10*9;
        }
        else {
            unitType = SVG_ANGLETYPE_UNSPECIFIED;
            value = dbl;
        }
    }


    /**
     * Reset the value as a number with an associated unitType, thereby replacing the values for
     * all of the attributes on the object.
     * \param newType The unit type for the value.
     * \param val The angle value.
     * \except DOMException (code NOT_SUPPORTED_ERR) Raised if unitType is SVG_ANGLETYPE_UNKNOWN or
     * not a valid unit type constant (one of the other SVG_ANGLETYPE_* constants defined on this
     * interface).
     * \except DOMException (code NO_MODIFICATION_ALLOWED_ERR) Raised when the angle corresponds to
     * a read only attribute or when the object itself is read only.
     */
    void newValueSpecifiedUnits(SVGAngleUnitType newType, double val)
    {
        if (newType == SVG_ANGLETYPE_UNKNOWN) {
            throw DOMException(NOT_SUPPORTED_ERR);
        }
        unitType = newType;
        setValueSpecifiedUnits(val);
    }

    /**
     * Preserve the same underlying stored value, but reset the stored unit identifier to the given
     * unitType. Object attributes unitType, valueInSpecifiedUnits and valueAsString might be
     * modified as a result of this method.
     * \param newType The unit type to switch to.
     */
    void convertToSpecifiedUnits(SVGAngleUnitType newType)
    {
        if (newType == SVG_ANGLETYPE_UNKNOWN) {
            throw DOMException(NOT_SUPPORTED_ERR);
        }
        unitType = newType;
    }

    /**
     * Default constructor, sets length to zero and unit to unspecified.
     */
    SVGAngle() :
        value(0,)
        unitType(SVG_ANGLETYPE_UNSPECIFIED)
    {
    }

private:
    /** Unit type of the angle value. */
    SVGAngleUnitType unitType;

    /** Angle stored as DEG. */
    double value;
};

/**
 *
 */
class SVGColor : public css::CSSValue
{
public:
    /**
     * Color Types
     */
    typedef enum
        {
        SVG_COLORTYPE_UNKNOWN           = 0,
        SVG_COLORTYPE_RGBCOLOR          = 1,
        SVG_COLORTYPE_RGBCOLOR_ICCCOLOR = 2,
        SVG_COLORTYPE_CURRENTCOLOR      = 3
        } ColorType;


    /** Gets the color type. */
    SVGColorType getColorType()
    {
        return colorType;
    }

    /**
     *
     */
    css::RGBColor getRgbColor()
    {
        css::RGBColor col;
        return col;
    }

    /**
     *
     */
    SVGICCColor getIccColor()
        {
        SVGICCColor col;
        return col;
        }


    /**
     *
     */
    void setRGBColor (const DOMString& /*rgbColor*/ )
                              throw( SVGException )
        {
        }

    /**
     *
     */
    void setRGBColorICCColor (const DOMString& /*rgbColor*/,
                                      const DOMString& /*iccColor*/ )
                                      throw( SVGException )
        {
        }

    /**
     *
     */
    void setColor (unsigned short /*colorType*/,
                           const DOMString& /*rgbColor*/,
                           const DOMString& /*iccColor*/ )
                           throw( SVGException )
        {
        }

protected:
    int colorType;
};

/** Represents a rectangle. */
class SVGRect
{
public:

    /** Gets the x coordinate of the rectangle. */
    double getX() { return x; }

    /** Sets the x coordinate of the rectangle. */
    void setX(double val)
        throw (DOMException)
    {
        x = val;
    }

    /** Gets the y coordinate of the rectangle. */
    double getY() { return y; }

    /** Sets the y coordinate of the rectangle. */
    void setY(double val)
        throw (DOMException)
    {
        y = val;
    }

    /** Gets the width of the rectangle. */
    double getWidth() { return width; }

    /** Sets the width of the rectangle. */
    void setWidth(double val)
        throw (DOMException)
    {
        width = val;
    }

    /** Gets the height of the rectangle. */
    double getHeight() { return height; }

    /** Sets the height of the rectangle. */
    void setHeight(double val)
        throw (DOMException)
    {
        height = val;
    }

private:
    double x, y, width, height;
};

class SVGUnitTypes
{
public:
    /**
     * Unit Types
     */
    typedef enum
    {
        SVG_UNIT_TYPE_UNKNOWN           = 0,
        SVG_UNIT_TYPE_USERSPACEONUSE    = 1,
        SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2
    } UnitType;
};

/**
 * SVGStylable is implemented on all objects corresponding to SVG elements that can have 'style'
 * and 'class' attributes specified on them.
 */
class SVGStylable
{
public:

    /** Gets the class name attribute. */
    SVGAnimatedString getClassName() { return className; }

    /** Gets the style attribute. */
    css::CSSStyleDeclaration getStyle() { return style; }

private:
    SVGAnimatedString className;
    css::CSSStyleDeclaration style;
};

/**
 *
 */
class SVGLocatable
{
public:

    /**
     *
     */
    SVGElementPtr getNearestViewportElement()
        {
        SVGElementPtr result;
        return result;
        }

    /**
     *
     */
    SVGElementPtr getFarthestViewportElement()
        {
        SVGElementPtr result;
        return result;
        }

    /**
     *
     */
    SVGRect getBBox (  )
        {
        return bbox;
        }

    /**
     *
     */
    SVGMatrix getCTM (  )
        {
        return ctm;
        }

    /**
     *
     */
    SVGMatrix getScreenCTM (  )
        {
        return screenCtm;
        }

    /**
     *
     */
    SVGMatrix getTransformToElement (const SVGElement &/*element*/)
                    throw( SVGException )
        {
        SVGMatrix result;
        //do calculations
        return result;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLocatable() {}

    /**
     *
     */
    SVGLocatable(const SVGLocatable &/*other*/)
        {
        }

    /**
     *
     */
    ~SVGLocatable() {}

protected:

    SVGRect bbox;
    SVGMatrix ctm;
    SVGMatrix screenCtm;

};






/*#########################################################################
## SVGTransformable
#########################################################################*/

/**
 *
 */
class SVGTransformable : public SVGLocatable
{
public:


    /**
     *
     */
    SVGAnimatedTransformList &getTransform()
        {
        return transforms;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTransformable() {}

    /**
     *
     */
    SVGTransformable(const SVGTransformable &other) : SVGLocatable(other)
        {
        transforms = other.transforms;
        }

    /**
     *
     */
    ~SVGTransformable() {}

protected:

    SVGAnimatedTransformList transforms;
};






/*#########################################################################
## SVGTests
#########################################################################*/

/**
 *
 */
class SVGTests
{
public:


    /**
     *
     */
    SVGStringList &getRequiredFeatures()
        {
        return requiredFeatures;
        }

    /**
     *
     */
    SVGStringList &getRequiredExtensions()
        {
        return requiredExtensions;
        }

    /**
     *
     */
    SVGStringList &getSystemLanguage()
        {
        return systemLanguage;
        }


    /**
     *
     */
    bool hasExtension (const DOMString& /*extension*/ )
        {
        return false;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTests() {}

    /**
     *
     */
    SVGTests(const SVGTests &other)
        {
        requiredFeatures   = other.requiredFeatures;
        requiredExtensions = other.requiredExtensions;
        systemLanguage     = other.systemLanguage;
        }

    /**
     *
     */
    ~SVGTests() {}

protected:

    SVGStringList requiredFeatures;
    SVGStringList requiredExtensions;
    SVGStringList systemLanguage;

};






/*#########################################################################
## SVGLangSpace
#########################################################################*/

/**
 *
 */
class SVGLangSpace
{
public:


    /**
     *
     */
    DOMString getXmllang()
        {
        return xmlLang;
        }

    /**
     *
     */
    void setXmllang(const DOMString &val)
                                     throw (DOMException)
        {
        xmlLang = val;
        }

    /**
     *
     */
    DOMString getXmlspace()
        {
        return xmlSpace;
        }

    /**
     *
     */
    void setXmlspace(const DOMString &val)
                                     throw (DOMException)
        {
        xmlSpace = val;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLangSpace() {}

    /**
     *
     */
    SVGLangSpace(const SVGLangSpace &other)
        {
        xmlLang  = other.xmlLang;
        xmlSpace = other.xmlSpace;
        }

    /**
     *
     */
    ~SVGLangSpace() {}

protected:

    DOMString xmlLang;
    DOMString xmlSpace;

};






/*#########################################################################
## SVGExternalResourcesRequired
#########################################################################*/

/**
 *
 */
class SVGExternalResourcesRequired
{
public:


    /**
     *
     */
    SVGAnimatedBoolean getExternalResourcesRequired()
        { return required; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGExternalResourcesRequired()
        {  }


    /**
     *
     */
    SVGExternalResourcesRequired(const SVGExternalResourcesRequired &other)
        {
        required = other.required;
        }

    /**
     *
     */
    ~SVGExternalResourcesRequired() {}

protected:

    SVGAnimatedBoolean required;

};






/*#########################################################################
## SVGPreserveAspectRatio
#########################################################################*/

/**
 *
 */
class SVGPreserveAspectRatio
{
public:


    /**
     * Alignment Types
     */
    typedef enum
        {
        SVG_PRESERVEASPECTRATIO_UNKNOWN  = 0,
        SVG_PRESERVEASPECTRATIO_NONE     = 1,
        SVG_PRESERVEASPECTRATIO_XMINYMIN = 2,
        SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3,
        SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4,
        SVG_PRESERVEASPECTRATIO_XMINYMID = 5,
        SVG_PRESERVEASPECTRATIO_XMIDYMID = 6,
        SVG_PRESERVEASPECTRATIO_XMAXYMID = 7,
        SVG_PRESERVEASPECTRATIO_XMINYMAX = 8,
        SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9,
        SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10
        } AlignmentType;


    /**
     * Meet-or-slice Types
     */
    typedef enum
        {
        SVG_MEETORSLICE_UNKNOWN  = 0,
        SVG_MEETORSLICE_MEET     = 1,
        SVG_MEETORSLICE_SLICE    = 2
        } MeetOrSliceType;


    /**
     *
     */
    unsigned short getAlign()
        { return align; }

    /**
     *
     */
    void setAlign(unsigned short val) throw (DOMException)
        { align = val; }

    /**
     *
     */
    unsigned short getMeetOrSlice()
        { return meetOrSlice; }

    /**
     *
     */
    void setMeetOrSlice(unsigned short val) throw (DOMException)
        { meetOrSlice = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPreserveAspectRatio()
        {
        align       = SVG_PRESERVEASPECTRATIO_UNKNOWN;
        meetOrSlice = SVG_MEETORSLICE_UNKNOWN;
        }

    /**
     *
     */
    SVGPreserveAspectRatio(const SVGPreserveAspectRatio &other)
        {
        align       = other.align;
        meetOrSlice = other.meetOrSlice;
        }

    /**
     *
     */
    ~SVGPreserveAspectRatio() {}

protected:

    unsigned short align;
    unsigned short meetOrSlice;

};

/**
 *
 */
class SVGFitToViewBox
{
public:

    /**
     *
     */
    SVGAnimatedRect getViewBox()
        { return viewBox; }

    /**
     *
     */
    SVGAnimatedPreserveAspectRatio getPreserveAspectRatio()
        { return preserveAspectRatio; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGFitToViewBox()
        {}

    /**
     *
     */

    SVGFitToViewBox(const SVGFitToViewBox &other)
        {
        viewBox = other.viewBox;
        preserveAspectRatio = other.preserveAspectRatio;
        }

    /**
     *
     */
    ~SVGFitToViewBox() {}

protected:

    SVGAnimatedRect viewBox;

    SVGAnimatedPreserveAspectRatio preserveAspectRatio;

};


/*#########################################################################
## SVGZoomAndPan
#########################################################################*/

/**
 *
 */
class SVGZoomAndPan
{
public:


    /**
     * Zoom and Pan Types
     */
    typedef enum
        {
        SVG_ZOOMANDPAN_UNKNOWN = 0,
        SVG_ZOOMANDPAN_DISABLE = 1,
        SVG_ZOOMANDPAN_MAGNIFY = 2
        } ZoomAndPanType;


    /**
     *
     */
    unsigned short getZoomAndPan()
        { return zoomAndPan; }

    /**
     *
     */
    void setZoomAndPan(unsigned short val) throw (DOMException)
        { zoomAndPan = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGZoomAndPan()
        { zoomAndPan = SVG_ZOOMANDPAN_UNKNOWN; }

    /**
     *
     */
    SVGZoomAndPan(const SVGZoomAndPan &other)
        { zoomAndPan = other.zoomAndPan; }

    /**
     *
     */
    ~SVGZoomAndPan() {}

protected:

    unsigned short zoomAndPan;

};






/*#########################################################################
## SVGViewSpec
#########################################################################*/

/**
 *
 */
class SVGViewSpec : public SVGZoomAndPan,
                    public SVGFitToViewBox
{
public:

    /**
     *
     */
    SVGTransformList getTransform()
        { return transform; }

    /**
     *
     */
    SVGElementPtr getViewTarget()
        { return viewTarget; }

    /**
     *
     */
    DOMString getViewBoxString()
        {
        DOMString ret;
        return ret;
        }

    /**
     *
     */
    DOMString getPreserveAspectRatioString()
        {
        DOMString ret;
        return ret;
        }

    /**
     *
     */
    DOMString getTransformString()
        {
        DOMString ret;
        return ret;
        }

    /**
     *
     */
    DOMString getViewTargetString()
        {
        DOMString ret;
        return ret;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGViewSpec()
        {
        viewTarget = NULL;
        }

    /**
     *
     */
    SVGViewSpec(const SVGViewSpec &other) : SVGZoomAndPan(other), SVGFitToViewBox(other)
        {
        viewTarget = other.viewTarget;
        transform  = other.transform;
        }

    /**
     *
     */
    ~SVGViewSpec() {}

protected:

    SVGElementPtr viewTarget;
    SVGTransformList transform;
};




/*#########################################################################
## SVGURIReference
#########################################################################*/

/**
 *
 */
class SVGURIReference
{
public:

    /**
     *
     */
    SVGAnimatedString getHref()
        { return href; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGURIReference() {}

    /**
     *
     */
    SVGURIReference(const SVGURIReference &other)
        {
        href = other.href;
        }

    /**
     *
     */
    ~SVGURIReference() {}

protected:

    SVGAnimatedString href;

};






/*#########################################################################
## SVGCSSRule
#########################################################################*/

/**
 *
 */
class SVGCSSRule : public css::CSSRule
{
public:


    /**
     * Additional CSS RuleType to support ICC color specifications
     */
    typedef enum
        {
        COLOR_PROFILE_RULE = 7
        } ColorProfileRuleType;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGCSSRule()
        { type = COLOR_PROFILE_RULE; }

    /**
     *
     */
    SVGCSSRule(const SVGCSSRule &other) : css::CSSRule(other)
        { type = COLOR_PROFILE_RULE; }

    /**
     *
     */
    ~SVGCSSRule() {}

};



/*#########################################################################
## SVGRenderingIntent
#########################################################################*/

/**
 *
 */
class SVGRenderingIntent
{
public:

    /**
     * Rendering Intent Types
     */
    typedef enum
        {
        RENDERING_INTENT_UNKNOWN               = 0,
        RENDERING_INTENT_AUTO                  = 1,
        RENDERING_INTENT_PERCEPTUAL            = 2,
        RENDERING_INTENT_RELATIVE_COLORIMETRIC = 3,
        RENDERING_INTENT_SATURATION            = 4,
        RENDERING_INTENT_ABSOLUTE_COLORIMETRIC = 5
        } RenderingIntentType;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGRenderingIntent()
        {
        renderingIntentType = RENDERING_INTENT_UNKNOWN;
        }

    /**
     *
     */
    SVGRenderingIntent(const SVGRenderingIntent &other)
        {
        renderingIntentType = other.renderingIntentType;
        }

    /**
     *
     */
    ~SVGRenderingIntent() {}

protected:

    unsigned short renderingIntentType;
};


static char const *const pathSegLetters[] =
{
    "@", // PATHSEG_UNKNOWN,
    "z", // PATHSEG_CLOSEPATH
    "M", // PATHSEG_MOVETO_ABS
    "m", // PATHSEG_MOVETO_REL,
    "L", // PATHSEG_LINETO_ABS
    "l", // PATHSEG_LINETO_REL
    "C", // PATHSEG_CURVETO_CUBIC_ABS
    "c", // PATHSEG_CURVETO_CUBIC_REL
    "Q", // PATHSEG_CURVETO_QUADRATIC_ABS,
    "q", // PATHSEG_CURVETO_QUADRATIC_REL
    "A", // PATHSEG_ARC_ABS
    "a", // PATHSEG_ARC_REL,
    "H", // PATHSEG_LINETO_HORIZONTAL_ABS,
    "h", // PATHSEG_LINETO_HORIZONTAL_REL
    "V", // PATHSEG_LINETO_VERTICAL_ABS
    "v", // PATHSEG_LINETO_VERTICAL_REL
    "S", // PATHSEG_CURVETO_CUBIC_SMOOTH_ABS
    "s", // PATHSEG_CURVETO_CUBIC_SMOOTH_REL
    "T", // PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS
    "t"  // PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL
};

/*#########################################################################
## SVGPathSeg
#########################################################################*/

/**
 *
 */
class SVGPathSeg
{
public:



    /**
     *  Path Segment Types
     */
    typedef enum
        {
        PATHSEG_UNKNOWN                      = 0,
        PATHSEG_CLOSEPATH                    = 1,
        PATHSEG_MOVETO_ABS                   = 2,
        PATHSEG_MOVETO_REL                   = 3,
        PATHSEG_LINETO_ABS                   = 4,
        PATHSEG_LINETO_REL                   = 5,
        PATHSEG_CURVETO_CUBIC_ABS            = 6,
        PATHSEG_CURVETO_CUBIC_REL            = 7,
        PATHSEG_CURVETO_QUADRATIC_ABS        = 8,
        PATHSEG_CURVETO_QUADRATIC_REL        = 9,
        PATHSEG_ARC_ABS                      = 10,
        PATHSEG_ARC_REL                      = 11,
        PATHSEG_LINETO_HORIZONTAL_ABS        = 12,
        PATHSEG_LINETO_HORIZONTAL_REL        = 13,
        PATHSEG_LINETO_VERTICAL_ABS          = 14,
        PATHSEG_LINETO_VERTICAL_REL          = 15,
        PATHSEG_CURVETO_CUBIC_SMOOTH_ABS     = 16,
        PATHSEG_CURVETO_CUBIC_SMOOTH_REL     = 17,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = 18,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = 19
        } PathSegmentType;

    /**
     *
     */
    unsigned short getPathSegType()
        { return type; }

    /**
     *
     */
    DOMString getPathSegTypeAsLetter()
        {
        int typ = type;
        if (typ<0 || typ>PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL)
            typ = PATHSEG_UNKNOWN;
        char const *ch = pathSegLetters[typ];
        DOMString letter = ch;
        return letter;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSeg()
        { type = PATHSEG_UNKNOWN; }

    /**
     *
     */
    SVGPathSeg(const SVGPathSeg &other)
        {
        type = other.type;
        }

    /**
     *
     */
    ~SVGPathSeg() {}

protected:

    int type;

};






/*#########################################################################
## SVGPathSegClosePath
#########################################################################*/

/**
 *
 */
class SVGPathSegClosePath : public SVGPathSeg
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegClosePath()
        {
        type = PATHSEG_CLOSEPATH;
        }

    /**
     *
     */
    SVGPathSegClosePath(const SVGPathSegClosePath &other) : SVGPathSeg(other)
        {
        type = PATHSEG_CLOSEPATH;
        }

    /**
     *
     */
    ~SVGPathSegClosePath() {}

};




/*#########################################################################
## SVGPathSegMovetoAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegMovetoAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegMovetoAbs()
        {
        type = PATHSEG_MOVETO_ABS;
        x = y = 0.0;
        }

    /**
     *
     */
    SVGPathSegMovetoAbs(double xArg, double yArg)
        {
        type = PATHSEG_MOVETO_ABS;
        x = xArg; y = yArg;
        }

    /**
     *
     */
    SVGPathSegMovetoAbs(const SVGPathSegMovetoAbs &other) : SVGPathSeg(other)
        {
        type = PATHSEG_MOVETO_ABS;
        x = other.x; y = other.y;
        }

    /**
     *
     */
    ~SVGPathSegMovetoAbs() {}

protected:

    double x,y;

};






/*#########################################################################
## SVGPathSegMovetoRel
#########################################################################*/

/**
 *
 */
class SVGPathSegMovetoRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegMovetoRel()
        {
        type = PATHSEG_MOVETO_REL;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegMovetoRel(double xArg, double yArg)
        {
        type = PATHSEG_MOVETO_REL;
        x = xArg; y = yArg;
        }

    /**
     *
     */
    SVGPathSegMovetoRel(const SVGPathSegMovetoRel &other) : SVGPathSeg(other)
        {
        type = PATHSEG_MOVETO_REL;
        x = other.x; y = other.y;
        }

    /**
     *
     */
    ~SVGPathSegMovetoRel() {}

protected:

    double x,y;
};






/*#########################################################################
## SVGPathSegLinetoAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoAbs()
        {
        type = PATHSEG_LINETO_ABS;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoAbs(double xArg, double yArg)
        {
        type = PATHSEG_LINETO_ABS;
        x = xArg; y = yArg;
        }

    /**
     *
     */
    SVGPathSegLinetoAbs(const SVGPathSegLinetoAbs &other) : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_ABS;
        x = other.x; y = other.y;
        }

    /**
     *
     */
    ~SVGPathSegLinetoAbs() {}

protected:

    double x,y;
};






/*#########################################################################
## SVGPathSegLinetoRel
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoRel()
        {
        type = PATHSEG_LINETO_REL;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoRel(double xArg, double yArg)
        {
        type = PATHSEG_LINETO_REL;
        x = xArg; y = yArg;
        }

    /**
     *
     */
    SVGPathSegLinetoRel(const SVGPathSegLinetoRel &other) : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_REL;
        x = other.x; y = other.y;
        }

    /**
     *
     */
    ~SVGPathSegLinetoRel() {}

protected:

    double x,y;
};






/*#########################################################################
## SVGPathSegCurvetoCubicAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    double getX1()
        { return x1; }

    /**
     *
     */
    void setX1(double val) throw (DOMException)
        { x1 = val; }

    /**
     *
     */
    double getY1()
        { return y1; }

    /**
     *
     */
    void setY1(double val) throw (DOMException)
        { y1 = val; }


    /**
     *
     */
    double getX2()
        { return x2; }

    /**
     *
     */
    void setX2(double val) throw (DOMException)
        { x2 = val; }

    /**
     *
     */
    double getY2()
        { return y2; }

    /**
     *
     */
    void setY2(double val) throw (DOMException)
        { y2 = val; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegCurvetoCubicAbs()
        {
        type = PATHSEG_CURVETO_CUBIC_ABS;
        x = y = x1 = y1 = x2 = y2 = 0.0;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicAbs(double xArg,  double yArg,
                              double x1Arg, double y1Arg,
                              double x2Arg, double y2Arg)
        {
        type = PATHSEG_CURVETO_CUBIC_ABS;
        x  = xArg;   y  = yArg;
        x1 = x1Arg;  y1 = y1Arg;
        x2 = x2Arg;  y2 = y2Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicAbs(const SVGPathSegCurvetoCubicAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_CUBIC_ABS;
        x  = other.x;  y  = other.y;
        x1 = other.x1; y1 = other.y1;
        x2 = other.x2; y2 = other.y2;
        }

    /**
     *
     */
    ~SVGPathSegCurvetoCubicAbs() {}

protected:

    double x, y, x1, y1, x2, y2;

};






/*#########################################################################
## SVGPathSegCurvetoCubicRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    double getX1()
        { return x1; }

    /**
     *
     */
    void setX1(double val) throw (DOMException)
        { x1 = val; }

    /**
     *
     */
    double getY1()
        { return y1; }

    /**
     *
     */
    void setY1(double val) throw (DOMException)
        { y1 = val; }


    /**
     *
     */
    double getX2()
        { return x2; }

    /**
     *
     */
    void setX2(double val) throw (DOMException)
        { x2 = val; }

    /**
     *
     */
    double getY2()
        { return y2; }

    /**
     *
     */
    void setY2(double val) throw (DOMException)
        { y2 = val; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegCurvetoCubicRel()
        {
        type = PATHSEG_CURVETO_CUBIC_REL;
        x = y = x1 = y1 = x2 = y2 = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoCubicRel(double xArg,  double yArg,
                              double x1Arg, double y1Arg,
                              double x2Arg, double y2Arg)
        {
        type = PATHSEG_CURVETO_CUBIC_REL;
        x  = xArg;   y  = yArg;
        x1 = x1Arg;  y1 = y1Arg;
        x2 = x2Arg;  y2 = y2Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicRel(const SVGPathSegCurvetoCubicRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_CUBIC_REL;
        x  = other.x;  y  = other.y;
        x1 = other.x1; y1 = other.y1;
        x2 = other.x2; y2 = other.y2;
        }

    /**
     *
     */
    ~SVGPathSegCurvetoCubicRel() {}

protected:

    double x, y, x1, y1, x2, y2;

};






/*#########################################################################
## SVGPathSegCurvetoQuadraticAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    double getX1()
        { return x1; }

    /**
     *
     */
    void setX1(double val) throw (DOMException)
        { x1 = val; }

    /**
     *
     */
    double getY1()
        { return y1; }

    /**
     *
     */
    void setY1(double val) throw (DOMException)
        { y1 = val; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegCurvetoQuadraticAbs()
        {
        type = PATHSEG_CURVETO_QUADRATIC_ABS;
        x = y = x1 = y1 = 0.0;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticAbs(double xArg,  double yArg,
                              double x1Arg, double y1Arg)
        {
        type = PATHSEG_CURVETO_QUADRATIC_ABS;
        x  = xArg;   y  = yArg;
        x1 = x1Arg;  y1 = y1Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticAbs(const SVGPathSegCurvetoQuadraticAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_QUADRATIC_ABS;
        x  = other.x;  y  = other.y;
        x1 = other.x1; y1 = other.y1;
        }

    /**
     *
     */
    ~SVGPathSegCurvetoQuadraticAbs() {}

protected:

    double x, y, x1, y1;

};






/*#########################################################################
## SVGPathSegCurvetoQuadraticRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    double getX1()
        { return x1; }

    /**
     *
     */
    void setX1(double val) throw (DOMException)
        { x1 = val; }

    /**
     *
     */
    double getY1()
        { return y1; }

    /**
     *
     */
    void setY1(double val) throw (DOMException)
        { y1 = val; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegCurvetoQuadraticRel()
        {
        type = PATHSEG_CURVETO_QUADRATIC_REL;
        x = y = x1 = y1 = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoQuadraticRel(double xArg,  double yArg,
                                  double x1Arg, double y1Arg)
        {
        type = PATHSEG_CURVETO_QUADRATIC_REL;
        x  = xArg;   y  = yArg;
        x1 = x1Arg;  y1 = y1Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticRel(const SVGPathSegCurvetoQuadraticRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_QUADRATIC_REL;
        x  = other.x;  y  = other.y;
        x1 = other.x1; y1 = other.y1;
        }

    /**
     *
     */
    ~SVGPathSegCurvetoQuadraticRel() {}

protected:

    double x, y, x1, y1;

};






/*#########################################################################
## SVGPathSegArcAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegArcAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    double getR1()
        { return r1; }

    /**
     *
     */
    void setR1(double val) throw (DOMException)
        { r1 = val; }

    /**
     *
     */
    double getR2()
        { return r2; }

    /**
     *
     */
    void setR2(double val) throw (DOMException)
        { r2 = val; }

    /**
     *
     */
    double getAngle()
        { return angle; }

    /**
     *
     */
    void setAngle(double val) throw (DOMException)
        { angle = val; }

    /**
     *
     */
    bool getLargeArcFlag()
        { return largeArcFlag; }

    /**
     *
     */
    void setLargeArcFlag(bool val) throw (DOMException)
        { largeArcFlag = val; }

    /**
     *
     */
    bool getSweepFlag()
        { return sweepFlag; }

    /**
     *
     */
    void setSweepFlag(bool val) throw (DOMException)
        { sweepFlag = val; }

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegArcAbs()
        {
        type = PATHSEG_ARC_ABS;
        x = y = r1 = r2 = angle = 0.0;
        largeArcFlag = sweepFlag = false;
        }

    /**
     *
     */
    SVGPathSegArcAbs(double xArg,  double yArg,
                     double r1Arg, double r2Arg,
                     double angleArg,
                     bool largeArcFlagArg,
                     bool sweepFlagArg )

        {
        type = PATHSEG_ARC_ABS;
        x  = xArg;   y  = yArg;
        r1 = r1Arg;  r2 = r2Arg;
        angle        = angleArg;
        largeArcFlag = largeArcFlagArg;
        sweepFlag    = sweepFlagArg;
        }

    /**
     *
     */
    SVGPathSegArcAbs(const SVGPathSegArcAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_ARC_ABS;
        x  = other.x;  y  = other.y;
        r1 = other.r1; r2 = other.r2;
        angle        = other.angle;
        largeArcFlag = other.largeArcFlag;
        sweepFlag    = other.sweepFlag;
        }

    /**
     *
     */
    ~SVGPathSegArcAbs() {}

protected:

    double x, y, r1, r2, angle;
    bool largeArcFlag;
    bool sweepFlag;

};



/*#########################################################################
## SVGPathSegArcRel
#########################################################################*/

/**
 *
 */
class SVGPathSegArcRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    double getR1()
        { return r1; }

    /**
     *
     */
    void setR1(double val) throw (DOMException)
        { r1 = val; }

    /**
     *
     */
    double getR2()
        { return r2; }

    /**
     *
     */
    void setR2(double val) throw (DOMException)
        { r2 = val; }

    /**
     *
     */
    double getAngle()
        { return angle; }

    /**
     *
     */
    void setAngle(double val) throw (DOMException)
        { angle = val; }

    /**
     *
     */
    bool getLargeArcFlag()
        { return largeArcFlag; }

    /**
     *
     */
    void setLargeArcFlag(bool val) throw (DOMException)
        { largeArcFlag = val; }

    /**
     *
     */
    bool getSweepFlag()
        { return sweepFlag; }

    /**
     *
     */
    void setSweepFlag(bool val) throw (DOMException)
        { sweepFlag = val; }

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegArcRel()
        {
        type = PATHSEG_ARC_REL;
        x = y = r1 = r2 = angle = 0.0;
        largeArcFlag = sweepFlag = false;
        }


    /**
     *
     */
    SVGPathSegArcRel(double xArg, double yArg,
                     double r1Arg, double r2Arg,
                     double angleArg,
                     bool largeArcFlagArg,
                     bool sweepFlagArg )

        {
        type = PATHSEG_ARC_REL;
        x  = xArg;   y  = yArg;
        r1 = r1Arg;  r2 = r2Arg;
        angle        = angleArg;
        largeArcFlag = largeArcFlagArg;
        sweepFlag    = sweepFlagArg;
        }

    /**
     *
     */
    SVGPathSegArcRel(const SVGPathSegArcRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_ARC_REL;
        x  = other.x;  y  = other.y;
        r1 = other.r1; r2 = other.r2;
        angle        = other.angle;
        largeArcFlag = other.largeArcFlag;
        sweepFlag    = other.sweepFlag;
        }

    /**
     *
     */
    ~SVGPathSegArcRel() {}

protected:

    double x, y, r1, r2, angle;
    bool largeArcFlag;
    bool sweepFlag;

};






/*#########################################################################
## SVGPathSegLinetoHorizontalAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoHorizontalAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoHorizontalAbs()
        {
        type = PATHSEG_LINETO_HORIZONTAL_ABS;
        x = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoHorizontalAbs(double xArg)
        {
        type = PATHSEG_LINETO_HORIZONTAL_ABS;
        x = xArg;
        }

    /**
     *
     */
    SVGPathSegLinetoHorizontalAbs(const SVGPathSegLinetoHorizontalAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_HORIZONTAL_ABS;
        x = other.x;
        }

    /**
     *
     */
    ~SVGPathSegLinetoHorizontalAbs() {}

protected:

    double x;

};

/**
 *
 */
class SVGPathSegLinetoHorizontalRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoHorizontalRel()
        {
        type = PATHSEG_LINETO_HORIZONTAL_REL;
        x = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoHorizontalRel(double xArg)
        {
        type = PATHSEG_LINETO_HORIZONTAL_REL;
        x = xArg;
        }

    /**
     *
     */
    SVGPathSegLinetoHorizontalRel(const SVGPathSegLinetoHorizontalRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_HORIZONTAL_REL;
        x = other.x;
        }

    /**
     *
     */
    ~SVGPathSegLinetoHorizontalRel() {}

protected:

    double x;

};

/**
 *
 */
class SVGPathSegLinetoVerticalAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoVerticalAbs()
        {
        type = PATHSEG_LINETO_VERTICAL_ABS;
        y = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoVerticalAbs(double yArg)
        {
        type = PATHSEG_LINETO_VERTICAL_ABS;
        y = yArg;
        }

    /**
     *
     */
    SVGPathSegLinetoVerticalAbs(const SVGPathSegLinetoVerticalAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_VERTICAL_ABS;
        y = other.y;
        }

    /**
     *
     */
    ~SVGPathSegLinetoVerticalAbs() {}

protected:

    double y;

};

/**
 *
 */
class SVGPathSegLinetoVerticalRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoVerticalRel()
        {
        type = PATHSEG_LINETO_VERTICAL_REL;
        y = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoVerticalRel(double yArg)
        {
        type = PATHSEG_LINETO_VERTICAL_REL;
        y = yArg;
        }

    /**
     *
     */
    SVGPathSegLinetoVerticalRel(const SVGPathSegLinetoVerticalRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_VERTICAL_REL;
        y = other.y;
        }

    /**
     *
     */
    ~SVGPathSegLinetoVerticalRel() {}

protected:

    double y;

};

/*#########################################################################
## SVGPathSegCurvetoCubicSmoothAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicSmoothAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    double getX2()
        { return x2; }

    /**
     *
     */
    void setX2(double val) throw (DOMException)
        { x2 = val; }

    /**
     *
     */
    double getY2()
        { return y2; }

    /**
     *
     */
    void setY2(double val) throw (DOMException)
        { y2 = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothAbs()
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_ABS;
        x = y = x2 = y2 = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothAbs(double xArg,   double yArg,
                                    double x2Arg, double y2Arg)
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_ABS;
        x  = xArg;    y  = yArg;
        x2 = x2Arg;   y2 = y2Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothAbs(const SVGPathSegCurvetoCubicSmoothAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_ABS;
        x  = other.x;  y  = other.y;
        x2 = other.x2; y2 = other.y2;
        }

    /**
     *
     */
    ~SVGPathSegCurvetoCubicSmoothAbs() {}

protected:

    double x, y, x2, y2;

};

/**
 *
 */
class SVGPathSegCurvetoCubicSmoothRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    double getX2()
        { return x2; }

    /**
     *
     */
    void setX2(double val) throw (DOMException)
        { x2 = val; }

    /**
     *
     */
    double getY2()
        { return y2; }

    /**
     *
     */
    void setY2(double val) throw (DOMException)
        { y2 = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothRel()
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_REL;
        x = y = x2 = y2 = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothRel(double xArg,   double yArg,
                                    double x2Arg, double y2Arg)
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_REL;
        x  = xArg;    y  = yArg;
        x2 = x2Arg;   y2 = y2Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothRel(const SVGPathSegCurvetoCubicSmoothRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_REL;
        x  = other.x;  y  = other.y;
        x2 = other.x2; y2 = other.y2;
        }

    /**
     *
     */
    ~SVGPathSegCurvetoCubicSmoothRel() {}

protected:

    double x, y, x2, y2;

};

/*#########################################################################
## SVGPathSegCurvetoQuadraticSmoothAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticSmoothAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothAbs()
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothAbs(double xArg, double yArg)
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS;
        x = xArg;     y = yArg;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothAbs(const SVGPathSegCurvetoQuadraticSmoothAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS;
        x = y = 0.0;
        }

    /**
     *
     */
    ~SVGPathSegCurvetoQuadraticSmoothAbs() {}

protected:

    double x, y;

};






/*#########################################################################
## SVGPathSegCurvetoQuadraticSmoothRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticSmoothRel : public SVGPathSeg
{
public:

    /**
     *
     */
    double getX()
        { return x; }

    /**
     *
     */
    void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    double getY()
        { return y; }

    /**
     *
     */
    void setY(double val) throw (DOMException)
        { y = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothRel()
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothRel(double xArg, double yArg)
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL;
        x = xArg;     y = yArg;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothRel(const SVGPathSegCurvetoQuadraticSmoothRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL;
        x = y = 0.0;
        }

    /**
     *
     */
    ~SVGPathSegCurvetoQuadraticSmoothRel() {}

protected:

    double x, y;

};

/**
 *
 */
class SVGAnimatedPathData
{
public:

    /**
     *
     */
    SVGPathSegList getPathSegList()
        {
        SVGPathSegList list;
        return list;
        }

    /**
     *
     */
    SVGPathSegList getNormalizedPathSegList()
        {
        SVGPathSegList list;
        return list;
        }

    /**
     *
     */
    SVGPathSegList getAnimatedPathSegList()
        {
        SVGPathSegList list;
        return list;
        }

    /**
     *
     */
    SVGPathSegList getAnimatedNormalizedPathSegList()
        {
        SVGPathSegList list;
        return list;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
   SVGAnimatedPathData()
        {}

    /**
     *
     */
   SVGAnimatedPathData(const SVGAnimatedPathData &/*other*/)
        {
        }

    /**
     *
     */
    ~SVGAnimatedPathData() {}

};






/*#########################################################################
## SVGAnimatedPoints
#########################################################################*/

/**
 *
 */
class SVGAnimatedPoints
{
public:

    /**
     *
     */
    SVGPointList getPoints()
        { return points; }

    /**
     *
     */
    SVGPointList getAnimatedPoints()
        { return animatedPoints; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedPoints() {}

    /**
     *
     */
    SVGAnimatedPoints(const SVGAnimatedPoints &other)
        {
        points         = other.points;
        animatedPoints = other.animatedPoints;
        }

    /**
     *
     */
    ~SVGAnimatedPoints() {}

protected:

    SVGPointList points;
    SVGPointList animatedPoints;

};

/**
 * See http://www.w3.org/TR/SVG/painting.html#InterfaceSVGPaint.
 */
class SVGPaint : public SVGColor
{
public:
    /**
     * Paint Types
     */
    typedef enum
    {
        SVG_PAINTTYPE_UNKNOWN               = 0,
        SVG_PAINTTYPE_RGBCOLOR              = 1,
        SVG_PAINTTYPE_RGBCOLOR_ICCCOLOR     = 2,
        SVG_PAINTTYPE_NONE                  = 101,
        SVG_PAINTTYPE_CURRENTCOLOR          = 102,
        SVG_PAINTTYPE_URI_NONE              = 103,
        SVG_PAINTTYPE_URI_CURRENTCOLOR      = 104,
        SVG_PAINTTYPE_URI_RGBCOLOR          = 105,
        SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR = 106,
        SVG_PAINTTYPE_URI                   = 107
    } SVGPaintType;


    /** Gets the type of paint. */
    SVGPaintType getPaintType()
    {
        return paintType;
    }

    /** Gets the paint uri or null, if no uri is set. */
    DOMString getUri()
    {
        if (paintType == SVG_PAINTTYPE_URI_NONE ||
            paintType == SVG_PAINTTYPE_URI_CURRENTCOLOR ||
            paintType == SVG_PAINTTYPE_URI_RGBCOLOR ||
            paintType == SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR ||
            paintType == SVG_PAINTTYPE_URI)
        {
            return uri;
        }
        return NULL;
    }

    /** Sets the paintType to SVG_PAINTTYPE_URI_NONE and sets uri to the specified value. */
    void setUri(const DOMString& uriArg)
    {
        paintType = SVG_PAINTTYPE_URI_NONE;
        uri = uriArg;
    }

    /**
     * 
     */
    void setPaint(SVGPaintType paintTypeArg,
                  DOMString& uriArg,
                  DOMString& rgbColor,
                  DOMString& iccColor)
        throw(SVGException)
     {
        if (paintType == SVG_PAINTTYPE_UNKNOWN) {
            throw new SVGException(SVG_INVALID_VALUE_ERR);
        }

        if (uriArg == NULL && (
            paintType == SVG_PAINTTYPE_URI_NONE ||
            paintType == SVG_PAINTTYPE_URI_CURRENTCOLOR ||
            paintType == SVG_PAINTTYPE_URI_RGBCOLOR ||
            paintType == SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR ||
            paintType == SVG_PAINTTYPE_URI))
        {
            throw new SVGException(SVG_INVALID_VALUE_ERR);
        }

        if (rgbColor == NULL && (
            paintType == SVG_PAINTTYPE_RGBCOLOR ||
            paintType == SVG_PAINTTYPE_RGBCOLOR_ICCCOLOR ||
            paintType == SVG_PAINTTYPE_URI_RGBCOLOR ||
            paintType == SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR))
        {
            throw new SVGException(SVG_INVALID_VALUE_ERR);
        }

        paintType = paintTypeArg;
        uri       = uriArg;
        DOMString rgbColor
        DOMString iccColor;
    }

protected:
    SVGPaintType paintType;
    DOMString uri;
    DOMString 
};




/*#########################################################################
## SVGColorProfileRule
#########################################################################*/

/**
 *
 */
class SVGColorProfileRule : public SVGCSSRule,
                            public SVGRenderingIntent
{

public:
   /**
     *
     */
    DOMString getSrc()
        { return src; }

    /**
     *
     */
    void setSrc(const DOMString &val) throw (DOMException)
        { src = val; }

    /**
     *
     */
    DOMString getName()
        { return name; }

    /**
     *
     */
    void setName(const DOMString &val) throw (DOMException)
        { name = val; }

    /**
     *
     */
    unsigned short getRenderingIntent()
        { return renderingIntent; }

    /**
     *
     */
    void setRenderingIntent(unsigned short val) throw (DOMException)
        { renderingIntent = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGColorProfileRule() {}

    /**
     *
     */
    SVGColorProfileRule(const SVGColorProfileRule &other)
               : SVGCSSRule(other), SVGRenderingIntent(other)
        {
        renderingIntent = other.renderingIntent;
        src             = other.src;
        name            = other.name;
        }

    /**
     *
     */
    ~SVGColorProfileRule() {}

protected:

    unsigned short renderingIntent;
    DOMString src;
    DOMString name;

};



/*#########################################################################
## SVGFilterPrimitiveStandardAttributes
#########################################################################*/

/**
 *
 */
class SVGFilterPrimitiveStandardAttributes : public SVGStylable
{
public:



    /**
     *
     */
    SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    SVGAnimatedLength getHeight()
        { return height; }

    /**
     *
     */
    SVGAnimatedString getResult()
        { return result; }



    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGFilterPrimitiveStandardAttributes()
        {}

    /**
     *
     */
    SVGFilterPrimitiveStandardAttributes(const SVGFilterPrimitiveStandardAttributes &other)
                                 : SVGStylable(other)
        {
        x      = other.x;
        y      = other.y;
        width  = other.width;
        height = other.height;
        result = other.result;
        }

    /**
     *
     */
    ~SVGFilterPrimitiveStandardAttributes() {}

protected:

    SVGAnimatedLength x;
    SVGAnimatedLength y;
    SVGAnimatedLength width;
    SVGAnimatedLength height;
    SVGAnimatedString result;

};











/*#########################################################################
## SVGEvent
#########################################################################*/

/**
 *
 */
class SVGEvent : events::Event
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGEvent() {}

    /**
     *
     */
    SVGEvent(const SVGEvent &other) : events::Event(other)
        {}

    /**
     *
     */
    ~SVGEvent() {}

};




/*#########################################################################
## SVGZoomEvent
#########################################################################*/

/**
 *
 */
class SVGZoomEvent : events::UIEvent
{
public:

    /**
     *
     */
    SVGRect getZoomRectScreen()
        { return zoomRectScreen; }

    /**
     *
     */
    double getPreviousScale()
        { return previousScale; }

    /**
     *
     */
    SVGPoint getPreviousTranslate()
        { return previousTranslate; }

    /**
     *
     */
    double getNewScale()
        { return newScale; }

   /**
     *
     */
    SVGPoint getNewTranslate()
        { return newTranslate; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGZoomEvent()
        {}

    /**
     *
     */
    SVGZoomEvent(const SVGZoomEvent &other) : events::Event(other),
                                              events::UIEvent(other)
        {
        zoomRectScreen    = other.zoomRectScreen;
        previousScale     = other.previousScale;
        previousTranslate = other.previousTranslate;
        newScale          = other.newScale;
        newTranslate      = other.newTranslate;
        }

    /**
     *
     */
    ~SVGZoomEvent() {}

protected:

    SVGRect  zoomRectScreen;
    double   previousScale;
    SVGPoint previousTranslate;
    double   newScale;
    SVGPoint newTranslate;

};



/*#########################################################################
## SVGElementInstance
#########################################################################*/

/**
 *
 */
class SVGElementInstance : public events::EventTarget
{
public:

    /**
     *
     */
    SVGElementPtr getCorrespondingElement()
        { return correspondingElement; }

    /**
     *
     */
    SVGUseElementPtr getCorrespondingUseElement()
        { return correspondingUseElement; }

    /**
     *
     */
    SVGElementInstance getParentNode()
        {
        SVGElementInstance ret;
        return ret;
        }

    /**
     *  Since we are using stack types and this is a circular definition,
     *  we will instead implement this as a global function below:
     *   SVGElementInstanceList getChildNodes(const SVGElementInstance instance);
     */
    //virtual SVGElementInstanceList getChildNodes();

    /**
     *
     */
    SVGElementInstance getFirstChild()
        {
        SVGElementInstance ret;
        return ret;
        }

    /**
     *
     */
    SVGElementInstance getLastChild()
        {
        SVGElementInstance ret;
        return ret;
        }

    /**
     *
     */
    SVGElementInstance getPreviousSibling()
        {
        SVGElementInstance ret;
        return ret;
        }

    /**
     *
     */
    SVGElementInstance getNextSibling()
        {
        SVGElementInstance ret;
        return ret;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGElementInstance() {}

    /**
     *
     */
    SVGElementInstance(const SVGElementInstance &other)
                        : events::EventTarget(other)
        {
        }

    /**
     *
     */
    ~SVGElementInstance() {}

protected:

    SVGElementPtr      correspondingElement;
    SVGUseElementPtr   correspondingUseElement;

};






/*#########################################################################
## SVGElementInstanceList
#########################################################################*/

/**
 *
 */
class SVGElementInstanceList
{
public:


    /**
     *
     */
    unsigned long getLength()
        { return items.size(); }

    /**
     *
     */
    SVGElementInstance item (unsigned long index )
        {
        if (index >= items.size())
            {
            SVGElementInstance ret;
            return ret;
            }
        return items[index];
        }

    /**
     *  This static method replaces the circular definition of:
     *        SVGElementInstanceList SVGElementInstance::getChildNodes()
     *
     */
    static SVGElementInstanceList getChildNodes(const SVGElementInstance &/*instance*/)
        {
        SVGElementInstanceList list;
        return list;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGElementInstanceList() {}

    /**
     *
     */
    SVGElementInstanceList(const SVGElementInstanceList &other)
        {
        items = other.items;
        }

    /**
     *
     */
    ~SVGElementInstanceList() {}

protected:
    std::vector<SVGElementInstance> items;

};

}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif /* __SVGTYPES_H__ */

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

