#include <QApplication>
#include <QWidget>

#include <QtCore/BTKFocusToken>
#include <QtGui/BTKFocusOverlay>

int main(int argc, char **argv)
{
   QApplication app(argc, argv);

   QWidget widget;
   widget.setObjectName("overlay-visual-target");
   QApplication::setBtkOwnerContext(&widget, "owner-visual", "surface-visual");

   BtkFocusToken token("overlay-token", "owner-visual", "surface-visual");
   token.setPriority(25);
   QApplication::setBtkFocusTokens({token});

   BtkFocusOverlay overlay;
   overlay.setTargetWidget(&widget);
   overlay.setAutoRefreshEnabled(false);
   overlay.setPanelPreset(BtkFocusOverlay::PanelPreset::Analysis);
   overlay.cyclePanelPreset();
   overlay.setPanelPreset(BtkFocusOverlay::PanelPreset::Analysis);
   overlay.setPanelVisible(BtkFocusOverlay::PopupPanel, true);
   overlay.setPanelVisible(BtkFocusOverlay::RelationshipPanel, true);
   overlay.setPanelVisible(BtkFocusOverlay::BlockedPanel, true);
   overlay.setBlockedRoutesOnly(true);
   overlay.refreshDiagnostics();

   return overlay.diagnosticsText().isEmpty()
      || overlay.snapshot().ownerSummaries.isEmpty()
      || overlay.snapshot().relationshipSummaries.isEmpty()
      || overlay.panelPreset() != BtkFocusOverlay::PanelPreset::Analysis
      || ! overlay.isPanelVisible(BtkFocusOverlay::PopupPanel)
      || ! overlay.isPanelVisible(BtkFocusOverlay::RelationshipPanel)
      || ! overlay.isPanelVisible(BtkFocusOverlay::BlockedPanel)
      || ! overlay.blockedRoutesOnly()
      || overlay.targetRelationshipSummary().isEmpty() ? 1 : 0;
}
