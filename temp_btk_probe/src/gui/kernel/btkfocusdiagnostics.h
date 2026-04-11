/***********************************************************************
*
* BTK developer-facing focus diagnostics groundwork.
* This is a lightweight structured reporting surface that can later feed
* richer overlays, inspectors, and multiplayer debugging tools.
*
***********************************************************************/

#ifndef BTK_FOCUS_DIAGNOSTICS_H
#define BTK_FOCUS_DIAGNOSTICS_H

#include <qglobal.h>
#include <qstring.h>
#include <qstringlist.h>

class QWidget;

class Q_GUI_EXPORT BtkFocusDiagnosticsSnapshot
{
 public:
   QString activePopupOwnerId;
   QString activeModalOwnerId;
   QString focusWidgetDescription;
   QString focusWidgetPath;
   QString focusPopupRelationship;
   QString focusOwnerId;
   QString focusSurfaceId;
   QString currentStateText;
   QString targetDecisionSummary;
   QStringList popupStackSummaries;
   QStringList ownerSummaries;
   QStringList tokenSummaries;
   QStringList blockedRouteSummaries;
   QStringList blockerSummaries;
   QStringList relationshipSummaries;
   QStringList lines;

   int popupCount() const {
      return popupStackSummaries.size();
   }

   int ownerCount() const {
      return ownerSummaries.size();
   }

   int tokenCount() const {
      return tokenSummaries.size();
   }

   int blockedRouteCount() const {
      return blockedRouteSummaries.size();
   }

   int blockerCount() const {
      return blockerSummaries.size();
   }

   int relationshipCount() const {
      return relationshipSummaries.size();
   }

   bool isEmpty() const;
   QString toString() const;
};

class Q_GUI_EXPORT BtkFocusDiagnostics
{
 public:
   static BtkFocusDiagnosticsSnapshot snapshot();
   static QString describeWidgetTreePath(const QWidget *widget);
   static QString describeCurrentState();
};

#endif
