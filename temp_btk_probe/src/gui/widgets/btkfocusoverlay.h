/***********************************************************************
*
* BTK visual developer overlay scaffold for owner-aware focus diagnostics.
* This widget renders the current BTK focus diagnostics in a lightweight
* always-on-top debug surface that can later evolve into a richer HUD.
*
***********************************************************************/

#ifndef BTK_FOCUS_OVERLAY_H
#define BTK_FOCUS_OVERLAY_H

#include <btkfocusdiagnostics.h>
#include <qflags.h>
#include <qwidget.h>

class Q_GUI_EXPORT BtkFocusOverlay : public QWidget
{
   GUI_CS_OBJECT(BtkFocusOverlay)

 public:
   enum Panel {
      SummaryPanel = 0x0001,
      FocusPanel   = 0x0002,
      OwnerPanel        = 0x0004,
      PopupPanel        = 0x0008,
      TokenPanel        = 0x0010,
      TargetPanel       = 0x0020,
      BlockedPanel      = 0x0040,
      RawPanel          = 0x0080,
      RelationshipPanel = 0x0100,
      AllPanels         = 0xFFFF
   };

   enum class PanelPreset {
      Compact,
      OwnerCentric,
      Analysis,
      Full
   };

   Q_DECLARE_FLAGS(PanelFlags, Panel)

   explicit BtkFocusOverlay(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::ToolTip);

   BtkFocusOverlay(const BtkFocusOverlay &) = delete;
   BtkFocusOverlay &operator=(const BtkFocusOverlay &) = delete;

   ~BtkFocusOverlay();

   void setTargetWidget(QWidget *widget);
   QWidget *targetWidget() const;

   void setAutoRefreshEnabled(bool enabled);
   bool autoRefreshEnabled() const;

   void setRefreshInterval(int msec);
   int refreshInterval() const;

   void refreshDiagnostics();
   QString diagnosticsText() const;
   BtkFocusDiagnosticsSnapshot snapshot() const;

   void setVisiblePanels(PanelFlags panels);
   PanelFlags visiblePanels() const;
   void setPanelVisible(Panel panel, bool visible);
   bool isPanelVisible(Panel panel) const;
   void setPanelPreset(PanelPreset preset);
   PanelPreset panelPreset() const;
   void cyclePanelPreset();
   void setBlockedRoutesOnly(bool enabled);
   bool blockedRoutesOnly() const;
   QString targetRelationshipSummary() const;

   QSize sizeHint() const override;

 protected:
   void paintEvent(QPaintEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void timerEvent(QTimerEvent *event) override;

 private:
   void updateTimer();
   QString buildDisplayText() const;
   bool shouldRenderPanel(Panel panel) const;

   QWidget *m_targetWidget = nullptr;
   BtkFocusDiagnosticsSnapshot m_snapshot;
   QString m_diagnosticsText;
   int m_refreshInterval = 250;
   int m_timerId = 0;
   bool m_autoRefreshEnabled = true;
   bool m_blockedRoutesOnly = false;
   PanelFlags m_visiblePanels = AllPanels;
   PanelPreset m_panelPreset = PanelPreset::Full;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BtkFocusOverlay::PanelFlags)

#endif
