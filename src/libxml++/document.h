/* document.h
 * this file is part of libxml++
 *
 * parts of the code copyright (C) 2003 by Stefan Seefeld
 * others copyright (C) 2003 by libxml++ developer's team
 *
 * this file is covered by the GNU Lesser General Public License,
 * which should be included with libxml++ as the file COPYING.
 */

#ifndef __LIBXMLPP_DOCUMENT_H
#define __LIBXMLPP_DOCUMENT_H

#include <libxml++/dtd.h>

#include <string>
#include <ostream>

/* std::string or Glib::ustring in function prototypes in libxml++?
 *
 * If it's propagated to a libxml2 function that takes a xmlChar*, it's
 * UTF-8 encoded, and Glib::ustring is the right choice.
 *
 * If it's propagated to a libxml2 function that takes a char*, it's not
 * necessarily UTF-8 encoded, and std::string is usually the right choice.
 * Most of these strings are filenames or URLs.
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern "C" {
  struct _xmlDoc;
  struct _xmlEntity;
}
#endif //DOXYGEN_SHOULD_SKIP_THIS

namespace xmlpp
{

// xmlpp::XmlEntityType is similar to xmlEntityType in libxml2.
/** The valid entity types.
 */
enum class XmlEntityType
{
  INTERNAL_GENERAL = 1,
  EXTERNAL_GENERAL_PARSED = 2,
  EXTERNAL_GENERAL_UNPARSED = 3,
  INTERNAL_PARAMETER = 4,
  EXTERNAL_PARAMETER = 5,
  INTERNAL_PREDEFINED = 6
};

//TODO: Make Document inherit from Node, when we can break ABI one day?
//
//libxml might intend xmlDoc to derive (theoretically) from xmlNode.
//This is suggested because the xmlNodeSet returned by xmlXPathEval (see the
//Node::find() implementation) can contain either xmlNode or xmlDocument elements.
// See https://bugzilla.gnome.org/show_bug.cgi?id=754673#c8 for an explanation
// why it has not been done in libxml++ 3.0.
/**
 * Represents an XML document in the DOM model.
 */
class Document : public NonCopyable
{
  //Ensure that libxml is properly initialised:
  class Init
  {
  public:
    Init();
    ~Init() noexcept;
  };

  friend class SaxParser;

public:
  /** Create a new document.
   * @param version XML version.
   * @throws xmlpp::internal_error If memory allocation fails.
   */
  explicit Document(const Glib::ustring& version = "1.0");

  /** Create a new C++ wrapper for an xmlDoc struct.
   * The created xmlpp::Document takes ownership of the xmlDoc.
   * When the Document is deleted, so is the xmlDoc and all its nodes.
   * @param doc A pointer to an xmlDoc struct. Must not be <tt>nullptr</tt>.
   * @throws xmlpp::internal_error If @a doc is <tt>nullptr</tt>.
   */
  explicit Document(_xmlDoc* doc);

  ~Document() override;

  /** @return The encoding used in the source from which the document has been loaded.
   */
  Glib::ustring get_encoding() const;

  /** Get the internal subset of this document.
   * @returns A pointer to the DTD, or <tt>nullptr</tt> if not found.
   */
  Dtd* get_internal_subset() const;

  /** Create the internal subset of this document.
   * If the document already has an internal subset, a new one is not created.
   * @param name The DTD name.
   * @param external_id The external (PUBLIC) ID, or an empty string.
   * @param system_id The system ID, or an empty string.
   */
  void set_internal_subset(const Glib::ustring& name,
                           const Glib::ustring& external_id,
                           const Glib::ustring& system_id);

  /** Add an Entity declaration to the document.
   * @param name The name of the entity that will be used in an entity reference.
   * @param type The type of entity.
   * @param publicId The public ID of the subset.
   * @param systemId The system ID of the subset.
   * @param content The value of the Entity. In entity reference substitutions, this
   * is the replacement value.
   * @throws xmlpp::internal_error
   */
  virtual void set_entity_declaration(const Glib::ustring& name, XmlEntityType type,
                                      const Glib::ustring& publicId, const Glib::ustring& systemId,
                                      const Glib::ustring& content);

protected:
  /** Retrieve an Entity.
   * The entity can be from an external subset or internally declared.
   * @param name The name of the entity to get.
   * @returns A pointer to the libxml2 entity structure, or <tt>nullptr</tt> if not found.
   */
  _xmlEntity* get_entity(const Glib::ustring& name);

private:
  static Init init_;

  _xmlDoc* impl_;
};

} //namespace xmlpp

#endif //__LIBXMLPP_DOCUMENT_H
