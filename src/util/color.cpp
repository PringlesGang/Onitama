#include "color.h"

Color operator~(const Color& orig) {
  return orig == Color::Red ? Color::Blue : Color::Red;
}
