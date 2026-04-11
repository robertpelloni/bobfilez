#include <QApplication>
#include <QWidget>

#include <QtCore/BTKFocusToken>

int main(int argc, char **argv)
{
   QApplication app(argc, argv);

   QWidget ownerAWidget;
   ownerAWidget.setObjectName("owner-a-widget");
   QApplication::setBtkOwnerContext(&ownerAWidget, "owner-a", "surface-a");

   BtkFocusToken token("focus-owner-a", "owner-a", "surface-a");
   token.setModalityPolicy(BtkFocusToken::ModalityPolicy::OwnerExclusive);
   token.setPriority(100);

   QApplication::setBtkFocusTokens({token});

   return QApplication::btkWouldBlockFocusChange(&ownerAWidget) ? 1 : 0;
}
