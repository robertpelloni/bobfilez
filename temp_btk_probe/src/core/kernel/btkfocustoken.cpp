#include <btkfocustoken.h>

#include <utility>

BtkFocusToken::BtkFocusToken()
{
}

BtkFocusToken::BtkFocusToken(QString tokenId)
   : m_tokenId(std::move(tokenId))
{
}

BtkFocusToken::BtkFocusToken(QString tokenId, QString ownerId, QString surfaceId)
   : m_tokenId(std::move(tokenId)), m_ownerId(std::move(ownerId)), m_surfaceId(std::move(surfaceId))
{
}

bool BtkFocusToken::isValid() const
{
   return ! m_tokenId.isEmpty() && ! m_ownerId.isEmpty();
}

bool BtkFocusToken::matches(const QString &ownerId, const QString &surfaceId) const
{
   return m_ownerId == ownerId && m_surfaceId == surfaceId;
}

QString BtkFocusToken::tokenId() const
{
   return m_tokenId;
}

void BtkFocusToken::setTokenId(QString tokenId)
{
   m_tokenId = std::move(tokenId);
}

QString BtkFocusToken::ownerId() const
{
   return m_ownerId;
}

void BtkFocusToken::setOwnerId(QString ownerId)
{
   m_ownerId = std::move(ownerId);
}

QString BtkFocusToken::surfaceId() const
{
   return m_surfaceId;
}

void BtkFocusToken::setSurfaceId(QString surfaceId)
{
   m_surfaceId = std::move(surfaceId);
}

BtkFocusToken::Scope BtkFocusToken::scope() const
{
   return m_scope;
}

void BtkFocusToken::setScope(Scope scope)
{
   m_scope = scope;
}

BtkFocusToken::ModalityPolicy BtkFocusToken::modalityPolicy() const
{
   return m_modalityPolicy;
}

void BtkFocusToken::setModalityPolicy(ModalityPolicy policy)
{
   m_modalityPolicy = policy;
}

int BtkFocusToken::priority() const
{
   return m_priority;
}

void BtkFocusToken::setPriority(int priority)
{
   m_priority = priority;
}

bool BtkFocusToken::isActive() const
{
   return m_active;
}

void BtkFocusToken::setActive(bool active)
{
   m_active = active;
}
