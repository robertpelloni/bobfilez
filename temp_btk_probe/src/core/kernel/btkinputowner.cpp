#include <btkinputowner.h>

#include <utility>

BtkInputOwner::BtkInputOwner()
   : m_capabilities(PointerCapability | KeyboardCapability)
{
}

BtkInputOwner::BtkInputOwner(QString ownerId)
   : m_ownerId(std::move(ownerId)), m_capabilities(PointerCapability | KeyboardCapability)
{
}

BtkInputOwner::BtkInputOwner(QString ownerId, QString sessionId, QString displayName)
   : m_ownerId(std::move(ownerId)), m_sessionId(std::move(sessionId)),
     m_displayName(std::move(displayName)), m_capabilities(PointerCapability | KeyboardCapability)
{
}

bool BtkInputOwner::isValid() const
{
   return ! m_ownerId.isEmpty();
}

QString BtkInputOwner::ownerId() const
{
   return m_ownerId;
}

void BtkInputOwner::setOwnerId(QString ownerId)
{
   m_ownerId = std::move(ownerId);
}

QString BtkInputOwner::sessionId() const
{
   return m_sessionId;
}

void BtkInputOwner::setSessionId(QString sessionId)
{
   m_sessionId = std::move(sessionId);
}

QString BtkInputOwner::displayName() const
{
   return m_displayName;
}

void BtkInputOwner::setDisplayName(QString displayName)
{
   m_displayName = std::move(displayName);
}

QStringList BtkInputOwner::deviceIds() const
{
   return m_deviceIds;
}

void BtkInputOwner::setDeviceIds(QStringList deviceIds)
{
   m_deviceIds = std::move(deviceIds);
}

void BtkInputOwner::addDeviceId(const QString &deviceId)
{
   if (! deviceId.isEmpty() && ! m_deviceIds.contains(deviceId)) {
      m_deviceIds.append(deviceId);
   }
}

bool BtkInputOwner::ownsDevice(const QString &deviceId) const
{
   return m_deviceIds.contains(deviceId);
}

BtkInputOwner::Capabilities BtkInputOwner::capabilities() const
{
   return m_capabilities;
}

void BtkInputOwner::setCapabilities(Capabilities capabilities)
{
   m_capabilities = capabilities;
}

bool BtkInputOwner::hasCapability(Capability capability) const
{
   return m_capabilities.testFlag(capability);
}

int BtkInputOwner::priority() const
{
   return m_priority;
}

void BtkInputOwner::setPriority(int priority)
{
   m_priority = priority;
}

bool BtkInputOwner::isLocal() const
{
   return m_local;
}

void BtkInputOwner::setLocal(bool local)
{
   m_local = local;
}

bool BtkInputOwner::collaborative() const
{
   return m_collaborative;
}

void BtkInputOwner::setCollaborative(bool collaborative)
{
   m_collaborative = collaborative;
}
