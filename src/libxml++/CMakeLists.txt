set(libxmlpp_SRC
        document.cc
        dtd.cc
        keepblanks.cc
        noncopyable.cc
        exceptions/exception.cc
        exceptions/internal_error.cc
        exceptions/parse_error.cc
        exceptions/validity_error.cc
        exceptions/wrapped_exception.cc
        io/istreamparserinputbuffer.cc
        io/parserinputbuffer.cc
        parsers/parser.cc
        parsers/saxparser.cc

        # -------
        # Headers
        document.h
        dtd.h
        keepblanks.h
        libxml++config.h
        noncopyable.h
        exceptions/exception.h
        exceptions/internal_error.h
        exceptions/parse_error.h
        exceptions/validity_error.h
        exceptions/wrapped_exception.h
        io/istreamparserinputbuffer.h
        io/parserinputbuffer.h
        parsers/parser.h
        parsers/saxparser.h
        )

add_inkscape_lib(xmlpp_LIB "${libxmlpp_SRC}")
