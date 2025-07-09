-- init.sql

-- 1. Create schema (if you want to namespace your tables)
-- CREATE SCHEMA IF NOT EXISTS sensor;

-- 2. Create the sensor_events table
CREATE TABLE IF NOT EXISTS sensor_events (
    id            SERIAL PRIMARY KEY,
    sensor_id     TEXT    NOT NULL,
    type          TEXT    NOT NULL,
    value         TEXT,
    timestamp     TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- 3. Index to speed up ORDER BY timestamp DESC
CREATE INDEX IF NOT EXISTS idx_sensor_events_timestamp
    ON sensor_events (timestamp DESC);

-- 4. Insert some sample data
INSERT INTO sensor_events (sensor_id, type, value, timestamp) VALUES
  ('sensor-1', 'motion',       'detected', NOW() - INTERVAL '5 minutes'),
  ('sensor-2', 'temperature',  '22.5',     NOW() - INTERVAL '3 minutes'),
  ('sensor-1', 'motion',       'none',     NOW() - INTERVAL '1 minute');
