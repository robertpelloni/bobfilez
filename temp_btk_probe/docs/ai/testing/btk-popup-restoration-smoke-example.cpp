#include <QApplication>
#include <QWidget>

int main(int argc, char **argv)
{
   QApplication app(argc, argv);

   QWidget popupA1;
   popupA1.setObjectName("popup-a-1");
   QApplication::setBtkOwnerContext(&popupA1, "owner-a", "popup-a-1");

   QWidget popupA2;
   popupA2.setObjectName("popup-a-2");
   QApplication::setBtkOwnerContext(&popupA2, "owner-a", "popup-a-2");

   const QString descriptionA1 = QApplication::btkDescribeWidgetContext(&popupA1);
   const QString descriptionA2 = QApplication::btkDescribeWidgetContext(&popupA2);

   return descriptionA1.isEmpty() || descriptionA2.isEmpty() ? 1 : 0;
}
