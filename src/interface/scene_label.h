#pragma once
#include <QLabel>
#include <vector>


class SceneLabel : public QLabel {
public:
  SceneLabel(QWidget *parent)
    : QLabel(parent)
  {}
  void setFontColor(QColor cl);
  void setFontSize(size_t pointSize);
  void setBackgroundColor(QColor cl);

private:
  // TODO: think about updating data
  std::vector<std::pair<QString, QString>> m_data;
};
