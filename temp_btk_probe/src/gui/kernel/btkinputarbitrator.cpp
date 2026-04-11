#include <btkinputarbitrator.h>

#include <utility>

void BtkInputArbitrator::clear()
{
   m_tokens.clear();
}

void BtkInputArbitrator::setFocusTokens(QList<BtkFocusToken> tokens)
{
   m_tokens = std::move(tokens);
}

QList<BtkFocusToken> BtkInputArbitrator::focusTokens() const
{
   return m_tokens;
}

void BtkInputArbitrator::addFocusToken(const BtkFocusToken &token)
{
   if (token.isValid()) {
      m_tokens.append(token);
   }
}

bool BtkInputArbitrator::wouldBlock(const QString &ownerId, const QString &surfaceId) const
{
   BtkInputRouteRequest request;
   request.ownerId = ownerId;
   request.surfaceId = surfaceId;

   return ! route(request).accepted();
}

BtkInputRouteResult BtkInputArbitrator::route(const BtkInputRouteRequest &request) const
{
   BtkInputRouteResult result;
   result.resolvedOwnerId = request.ownerId;

   const BtkFocusToken *token = highestPriorityActiveToken();
   if (token == nullptr) {
      result.decision = BtkInputRouteResult::Decision::Share;
      return result;
   }

   if (token->ownerId() == request.ownerId) {
      result.decision = token->surfaceId() == request.surfaceId
         ? BtkInputRouteResult::Decision::Share
         : BtkInputRouteResult::Decision::Transfer;
      return result;
   }

   switch (token->modalityPolicy()) {
      case BtkFocusToken::ModalityPolicy::Shared:
         result.decision = BtkInputRouteResult::Decision::Share;
         return result;

      case BtkFocusToken::ModalityPolicy::OwnerExclusive:
      case BtkFocusToken::ModalityPolicy::WindowExclusive:
      case BtkFocusToken::ModalityPolicy::ApplicationExclusive:
      case BtkFocusToken::ModalityPolicy::SystemExclusive:
         result.decision = BtkInputRouteResult::Decision::Reject;
         result.blockingOwnerId = token->ownerId();
         result.blockingSurfaceId = token->surfaceId();
         return result;
   }

   return result;
}

const BtkFocusToken *BtkInputArbitrator::highestPriorityActiveToken() const
{
   const BtkFocusToken *retval = nullptr;

   for (const auto &token : m_tokens) {
      if (! token.isActive()) {
         continue;
      }

      if (retval == nullptr || token.priority() > retval->priority()) {
         retval = &token;
      }
   }

   return retval;
}
