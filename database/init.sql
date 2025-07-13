-- init.sql

-- 1. Create schema (if you want to namespace your tables)
-- CREATE SCHEMA IF NOT EXISTS sensor;

-- 2. Create the sensor_events table
CREATE TABLE IF NOT EXISTS sensor_events (
    id            SERIAL PRIMARY KEY,
    sensor_id     TEXT    NOT NULL,
    value         TEXT,
    timestamp     TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 3. Index to speed up ORDER BY timestamp DESC
CREATE INDEX IF NOT EXISTS idx_sensor_events_timestamp
    ON sensor_events (timestamp DESC);

-- 4. Insert some sample data
INSERT INTO sensor_events (sensor_id, value, timestamp) VALUES
  ('sensor-1', '0', NOW() - INTERVAL '5 minutes'),  
  ('sensor-2', '0', NOW() - INTERVAL '12 minutes'),
  ('sensor-1', '0', NOW() - INTERVAL '6 minute'),
  ('sensor-1', '1', NOW() - INTERVAL '7 minutes'),
  ('sensor-2', '1', NOW() - INTERVAL '13 minutes'),
  ('sensor-1', '1', NOW() - INTERVAL '11 minute');

