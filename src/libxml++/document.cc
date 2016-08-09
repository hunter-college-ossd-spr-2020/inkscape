/* document.cc
 * this file is part of libxml++
 *
 * copyright (C) 2003 by the libxml++ development team
 *
 * this file is covered by the GNU Lesser General Public License,
 * which should be included with libxml++ as the file COPYING.
 */

#include <libxml++/document.h>
#include <libxml++/exceptions/internal_error.h>

#include <libxml/tree.h>

namespace xmlpp
{

static const char* get_encoding_or_utf8(const Glib::ustring& encoding)
{
  if(encoding.empty())
  {
    //If we don't specify this to the xmlDocDump* functions (using nullptr instead),
    //then some other encoding is used, causing them to fail on non-ASCII characters.
    return "UTF-8";
  }
  else
    return encoding.c_str();
}


Document::Init::Init()
{
  xmlInitParser(); //Not always necessary, but necessary for thread safety.
}

Document::Init::~Init() noexcept
{
  //We don't call this because it breaks libxml generally and should only be
  //called at the very end of a process, such as at the end of a main().
  //libxml might still be used by the application, so we don't want to break
  //that.
  //This is important even here, which usually happens only when the library
  //is unloaded, because that might happen during normal application use,
  //if the application does dynamic library loading, for instance to load
  //plugins.
  //See http://xmlsoft.org/html/libxml-parser.html#xmlCleanupParser
  //xmlCleanupParser(); //As per xmlInitParser(), or memory leak will happen.
}

Document::Init Document::init_;

Document::Document(const Glib::ustring& version)
  : impl_(xmlNewDoc((const xmlChar*)version.c_str()))
{
  if (!impl_)
    throw internal_error("Could not create Document.");
  impl_->_private = this;
}

Document::Document(xmlDoc* doc)
  : impl_(doc)
{
  if (!impl_)
    throw internal_error("xmlDoc pointer cannot be nullptr");

  impl_->_private = this;
}

Document::~Document()
{
  xmlFreeDoc(impl_);
}

Glib::ustring Document::get_encoding() const
{
  Glib::ustring encoding;
  if(impl_->encoding)
    encoding = (const char*)impl_->encoding;

  return encoding;
}

Dtd* Document::get_internal_subset() const
{
  auto dtd = xmlGetIntSubset(impl_);
  if(!dtd)
    return nullptr;

  if(!dtd->_private)
    dtd->_private = new Dtd(dtd);

  return reinterpret_cast<Dtd*>(dtd->_private);
}

void Document::set_internal_subset(const Glib::ustring& name,
                                   const Glib::ustring& external_id,
                                   const Glib::ustring& system_id)
{
  auto dtd = xmlCreateIntSubset(impl_,
				   (const xmlChar*)name.c_str(),
				   external_id.empty() ? nullptr : (const xmlChar*)external_id.c_str(),
				   system_id.empty() ? nullptr : (const xmlChar*)system_id.c_str());

  if (dtd && !dtd->_private)
    dtd->_private = new Dtd(dtd);
}

void Document::set_entity_declaration(const Glib::ustring& name, XmlEntityType type,
                              const Glib::ustring& publicId, const Glib::ustring& systemId,
                              const Glib::ustring& content)
{
  auto entity = xmlAddDocEntity(impl_, (const xmlChar*)name.c_str(),
    static_cast<int>(type),
    publicId.empty() ? nullptr : (const xmlChar*)publicId.c_str(),
    systemId.empty() ? nullptr : (const xmlChar*)systemId.c_str(),
    (const xmlChar*)content.c_str());
  if (!entity)
    throw internal_error("Could not add entity declaration " + name);
}

_xmlEntity* Document::get_entity(const Glib::ustring& name)
{
  return xmlGetDocEntity(impl_, (const xmlChar*) name.c_str());
}

} //namespace xmlpp
