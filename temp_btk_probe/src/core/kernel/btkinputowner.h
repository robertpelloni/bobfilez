/***********************************************************************
*
* BTK multi-user ownership primitives.
* These types establish a first framework-level vocabulary for local and
* networked collaborative input ownership without yet rewriting the full
* event delivery stack.
*
***********************************************************************/

#ifndef BTK_INPUT_OWNER_H
#define BTK_INPUT_OWNER_H

#include <qflags.h>
#include <qglobal.h>
#include <qstring.h>
#include <qstringlist.h>

class Q_CORE_EXPORT BtkInputOwner
{
 public:
   enum Capability {
      NoCapability        = 0x0000,
      PointerCapability   = 0x0001,
      KeyboardCapability  = 0x0002,
      TouchCapability     = 0x0004,
      ClipboardCapability = 0x0008,
      WindowCapability    = 0x0010,
      CollaborationCapability = 0x0020,
      AutomationCapability    = 0x0040,
      AllCapabilities = 0xFFFF
   };

   Q_DECLARE_FLAGS(Capabilities, Capability)

   BtkInputOwner();
   explicit BtkInputOwner(QString ownerId);
   BtkInputOwner(QString ownerId, QString sessionId, QString displayName = QString());

   bool isValid() const;

   QString ownerId() const;
   void setOwnerId(QString ownerId);

   QString sessionId() const;
   void setSessionId(QString sessionId);

   QString displayName() const;
   void setDisplayName(QString displayName);

   QStringList deviceIds() const;
   void setDeviceIds(QStringList deviceIds);
   void addDeviceId(const QString &deviceId);
   bool ownsDevice(const QString &deviceId) const;

   Capabilities capabilities() const;
   void setCapabilities(Capabilities capabilities);
   bool hasCapability(Capability capability) const;

   int priority() const;
   void setPriority(int priority);

   bool isLocal() const;
   void setLocal(bool local);

   bool collaborative() const;
   void setCollaborative(bool collaborative);

 private:
   QString m_ownerId;
   QString m_sessionId;
   QString m_displayName;
   QStringList m_deviceIds;
   Capabilities m_capabilities;
   int m_priority = 0;
   bool m_local = true;
   bool m_collaborative = true;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BtkInputOwner::Capabilities)

#endif
