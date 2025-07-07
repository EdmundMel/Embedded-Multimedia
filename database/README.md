# DB Specification

This document outlines the architecture and deployment pipeline of a time-series sensor event system using **PostgreSQL**, **C++**, **Jenkins**, and **Grafana**.

---

## 1. System Overview

```text
┌───────────────┐       TCP (JSON)       ┌────────────────────┐              ┌─────────────┐
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
    id          SERIAL PRIMARY KEY,
    sensor_id   TEXT NOT NULL,
    type        TEXT NOT NULL,
    timestamp   TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

### 2.3 Sample Query (C++)

```sql
SELECT sensor_id, type, timestamp
FROM sensor_events
ORDER BY timestamp DESC
LIMIT 10;
```

---

## 3. Grafana Integration

- **Data Source**: PostgreSQL Plugin

- **Host**: `localhost`, Port: `5432`

- **Database**: `sensordb`

- **User**: `dbuser`, Password: `secret`

- **Panel Query**:

  ```sql
  SELECT
    timestamp AS "time",
    value_column AS "value"
  FROM
    sensor_events
  WHERE
    $__timeFilter(timestamp)
  ```

- **Visualization**:

  - X-axis: `timestamp`
  - Y-axis: measurement (e.g., temperature)

---

## 4. SQL Init Script (`init.sql`)

```sql
CREATE TABLE IF NOT EXISTS sensor_events (
    id SERIAL PRIMARY KEY,
    sensor_id TEXT NOT NULL,
    type TEXT NOT NULL,
    timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_sensor_events_timestamp
    ON sensor_events (timestamp DESC);

-- Sample data
INSERT INTO sensor_events (sensor_id, type, timestamp) VALUES
  ('sensor-1', 'motion', NOW() - INTERVAL '5 minutes'),
  ('sensor-2', 'temperature', NOW() - INTERVAL '3 minutes'),
  ('sensor-3', 'door', NOW() - INTERVAL '1 minute');
```
