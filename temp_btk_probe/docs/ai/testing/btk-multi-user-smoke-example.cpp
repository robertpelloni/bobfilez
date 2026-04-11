#include <QtCore/BTKFocusToken>
#include <QtCore/BTKInputOwner>
#include <QtGui/BTKInputArbitrator>

int main()
{
   BtkInputOwner ownerA("owner-a", "session-main", "Alice");
   ownerA.addDeviceId("mouse-1");

   BtkInputOwner ownerB("owner-b", "session-main", "Bob");
   ownerB.addDeviceId("mouse-2");

   BtkFocusToken token("focus-1", ownerA.ownerId(), "dialog-1");
   token.setModalityPolicy(BtkFocusToken::ModalityPolicy::OwnerExclusive);
   token.setPriority(10);

   BtkInputArbitrator arbitrator;
   arbitrator.addFocusToken(token);

   BtkInputRouteRequest request;
   request.ownerId = ownerB.ownerId();
   request.surfaceId = "dialog-1";

   BtkInputRouteResult result = arbitrator.route(request);
   return result.accepted() ? 1 : 0;
}
