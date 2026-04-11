#include <QtCore/BTKCore>

int main()
{
   BtkString text = "btk";
   BtkSharedPointer<int> value(new int(42));

   return text.isEmpty() || ! value ? 1 : 0;
}
