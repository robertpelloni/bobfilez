#include <QApplication>
#include <QWidget>

#include <QtGui/BTKFocusDiagnostics>

int main(int argc, char **argv)
{
   QApplication app(argc, argv);

   QWidget widget;
   widget.setObjectName("overlay-probe");
   QApplication::setBtkOwnerContext(&widget, "owner-overlay", "surface-overlay");

   const BtkFocusDiagnosticsSnapshot snapshot = BtkFocusDiagnostics::snapshot();
   const QString rendered = BtkFocusDiagnostics::describeCurrentState();
   const QString path = BtkFocusDiagnostics::describeWidgetTreePath(&widget);

   return rendered.isEmpty() || path.isEmpty() || snapshot.lines.isEmpty() ? 1 : 0;
}
