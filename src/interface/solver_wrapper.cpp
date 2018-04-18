#include "interface/solver_wrapper.h"
#include "future"

#include <QTime>

namespace
{

double futureFunction(soko::Solver &solver, const soko::Map &m)
{
  QTime now = QTime::currentTime();
  solver.solve(m);
  QTime finish = QTime::currentTime();
  double secs = now.msecsTo(finish) / 1000.0;
  return secs;
}

} // namespace

void SolverThread::runThread(const soko::Map &m) noexcept
{
  m_map = m;
  auto fn = std::thread([&]() {
    this->m_time = futureFunction(*this, m);
    emit finished();
  });
  fn.detach();
}
