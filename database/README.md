# DB Specification

This document outlines the architecture and deployment pipeline of a time-series sensor event system using **PostgreSQL**, **C++** and **Grafana**.

---

## 1. System Overview

```text
┌───────────────┐                        ┌────────────────────┐              ┌─────────────┐
│ Data Client   │ ─────────────────────▶ │ C++ Program        │              │ Grafana     │
│ (Sensor, etc) │                        │ (home-alarm-core)  │              │ (PostgreSQL │
│               │                        │                    │              │  Plugin)    │
└───────────────┘                        └────────┬───────────┘              └────┬────────┘
                                                 ▼                                 │
                                       ┌────────────────────┐                     │
                                       │ PostgreSQL (Docker)│◀────────────────────┘
                                       └────────────────────┘
```

- **TCP Ingestion**: Implemented in C++
- **Storage**: Sensor events are persisted in PostgreSQL (`sensor_events` table)
- **Query**: Grafana directly queries PostgreSQL

---

## 2. C++ Program: Sensor Event Collection

### 2.1 Database Access

The C++ code connects to PostgreSQL using `libpq`:

```cpp
const char* conninfo = "host=localhost port=5432 dbname=sensordb user=dbuser password=secret";
```

### 2.2 Table Schema (`sensor_events`)

```sql
CREATE TABLE IF NOT EXISTS sensor_events (
    id            SERIAL PRIMARY KEY,
    sensor_id     TEXT    NOT NULL,
    value         TEXT,
    timestamp     TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

### 2.3 Sample Query (C++)

```sql
SELECT sensor_id, value, timestamp
FROM sensor_events
ORDER BY timestamp DESC
LIMIT 10;
```

---

## 3. SQL Init Script (`init.sql`)

```sql
-- init.sql
-- 1. Create the sensor_events table
CREATE TABLE IF NOT EXISTS sensor_events (
    id            SERIAL PRIMARY KEY,
    sensor_id     TEXT    NOT NULL,
    value         TEXT,
    timestamp     TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 2. Index to speed up ORDER BY timestamp DESC
CREATE INDEX IF NOT EXISTS idx_sensor_events_timestamp
    ON sensor_events (timestamp DESC);

```
