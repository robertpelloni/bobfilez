/***********************************************************************
*
* BTK multi-focus token model.
* The goal is to move from a single implicit focus owner toward explicit,
* owner-scoped focus tokens that can later drive arbitration in the GUI
* event routing layer.
*
***********************************************************************/

#ifndef BTK_FOCUS_TOKEN_H
#define BTK_FOCUS_TOKEN_H

#include <qglobal.h>
#include <qstring.h>

class Q_CORE_EXPORT BtkFocusToken
{
 public:
   enum class Scope {
      Widget,
      Window,
      Application,
      Workspace,
      Global
   };

   enum class ModalityPolicy {
      Shared,
      OwnerExclusive,
      WindowExclusive,
      ApplicationExclusive,
      SystemExclusive
   };

   BtkFocusToken();
   explicit BtkFocusToken(QString tokenId);
   BtkFocusToken(QString tokenId, QString ownerId, QString surfaceId);

   bool isValid() const;
   bool matches(const QString &ownerId, const QString &surfaceId) const;

   QString tokenId() const;
   void setTokenId(QString tokenId);

   QString ownerId() const;
   void setOwnerId(QString ownerId);

   QString surfaceId() const;
   void setSurfaceId(QString surfaceId);

   Scope scope() const;
   void setScope(Scope scope);

   ModalityPolicy modalityPolicy() const;
   void setModalityPolicy(ModalityPolicy policy);

   int priority() const;
   void setPriority(int priority);

   bool isActive() const;
   void setActive(bool active);

 private:
   QString m_tokenId;
   QString m_ownerId;
   QString m_surfaceId;
   Scope m_scope = Scope::Widget;
   ModalityPolicy m_modalityPolicy = ModalityPolicy::Shared;
   int m_priority = 0;
   bool m_active = true;
};

#endif
