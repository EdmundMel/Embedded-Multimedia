#pragma once
#include "sensor_data.h"

#include <chrono>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <memory>

// Forward libpq types
struct PGconn;
struct PGresult;

struct PGResultInterface {
  virtual ExecStatusType status() const = 0;
  virtual int ntuples() const = 0;
  virtual char* value(int row, int col) const = 0;
  virtual void clear() = 0;
  virtual ~PGResultInterface() = default;
};

struct PGConnInterface {
  virtual ExecStatusType status() const = 0;
  virtual const char* errorMessage() const = 0;
  virtual std::unique_ptr<PGResultInterface> exec(const char* query) = 0;
  virtual void finish() = 0;
  virtual ~PGConnInterface() = default;
};

class Database {
public:
  explicit Database(std::unique_ptr<PGConnInterface> conn) : conn_(std::move(conn)) {}
  std::vector<SensorEvent> getRecentSensorEvents();
private:
  std::unique_ptr<PGConnInterface> conn_;
};