#pragma once
#include "soko/solver.h"
#include <QObject>

// TODO: either name solver thread or rename solver wrapper

class SolverThread
  : public QObject
  , public soko::Solver {
  Q_OBJECT
public:
  SolverThread() = default;

  double time() const noexcept { return m_time; }
  void runThread(const soko::Map &m) noexcept;
  const soko::Map &map() const noexcept { return m_map; }

signals:
  void finished();

private:
  soko::Map m_map;
  double m_time = 0.;
};
