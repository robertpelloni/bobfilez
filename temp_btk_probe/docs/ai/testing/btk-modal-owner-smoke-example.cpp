#include <QApplication>
#include <QWidget>

#include <QtCore/BTKFocusToken>

int main(int argc, char **argv)
{
   QApplication app(argc, argv);

   QWidget ownerAWindow;
   ownerAWindow.setObjectName("owner-a-window");
   QApplication::setBtkOwnerContext(&ownerAWindow, "owner-a", "window-a");

   QWidget ownerAPopup;
   ownerAPopup.setObjectName("owner-a-popup");
   QApplication::setBtkOwnerContext(&ownerAPopup, "owner-a", "popup-a");

   BtkFocusToken token("owner-a-exclusive", "owner-a", "popup-a");
   token.setModalityPolicy(BtkFocusToken::ModalityPolicy::OwnerExclusive);
   token.setPriority(50);
   QApplication::setBtkFocusTokens({token});

   return QApplication::btkWouldBlockFocusChange(&ownerAWindow, Qt::PopupFocusReason) ? 1 : 0;
}
