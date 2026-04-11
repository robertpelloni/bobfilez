/***********************************************************************
*
* BTK GUI-side input arbitration scaffolding.
* This is an initial policy layer for deciding whether an incoming input
* request should be shared, transferred, queued, or rejected based on the
* currently active owner-scoped focus tokens.
*
***********************************************************************/

#ifndef BTK_INPUT_ARBITRATOR_H
#define BTK_INPUT_ARBITRATOR_H

#include <btkfocustoken.h>

#include <qglobal.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qstring.h>

class Q_GUI_EXPORT BtkInputRouteRequest
{
 public:
   enum class Kind {
      Pointer,
      Keyboard,
      Touch,
      Wheel,
      Shortcut,
      System
   };

   QString ownerId;
   QString surfaceId;
   QString deviceId;
   Kind kind = Kind::Pointer;
   Qt::FocusReason reason = Qt::OtherFocusReason;
};

class Q_GUI_EXPORT BtkInputRouteResult
{
 public:
   enum class Decision {
      Reject,
      Share,
      Transfer,
      Queue
   };

   Decision decision = Decision::Share;
   QString resolvedOwnerId;
   QString blockingOwnerId;
   QString blockingSurfaceId;

   bool accepted() const {
      return decision != Decision::Reject;
   }
};

class Q_GUI_EXPORT BtkInputArbitrator
{
 public:
   void clear();
   void setFocusTokens(QList<BtkFocusToken> tokens);
   QList<BtkFocusToken> focusTokens() const;
   void addFocusToken(const BtkFocusToken &token);

   bool wouldBlock(const QString &ownerId, const QString &surfaceId) const;
   BtkInputRouteResult route(const BtkInputRouteRequest &request) const;

 private:
   const BtkFocusToken *highestPriorityActiveToken() const;
   QList<BtkFocusToken> m_tokens;
};

#endif
