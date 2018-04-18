#include "interface/scene_label.h"

void SceneLabel::setFontColor(QColor cl)
{
  auto pl = palette();
  pl.setColor(foregroundRole(), cl);
  setPalette(pl);
}

void SceneLabel::setFontSize(size_t pointSize)
{
  auto f = font();
  f.setPointSize(pointSize);
  setFont(f);
}
