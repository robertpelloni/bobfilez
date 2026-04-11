#include <QApplication>
#include <QWidget>

#include <QtCore/BTKFocusToken>

int main(int argc, char **argv)
{
   QApplication app(argc, argv);

   QWidget widget;
   widget.setObjectName("diagnostic-widget");
   QApplication::setBtkOwnerContext(&widget, "owner-a", "surface-a");

   BtkFocusToken token("token-a", "owner-a", "surface-a");
   token.setPriority(10);
   QApplication::setBtkFocusTokens({token});

   const QStringList diagnostics = QApplication::btkFocusDiagnostics();
   const QString description = QApplication::btkDescribeFocusDecision(&widget, Qt::OtherFocusReason);

   return diagnostics.isEmpty() || description.isEmpty() ? 1 : 0;
}
