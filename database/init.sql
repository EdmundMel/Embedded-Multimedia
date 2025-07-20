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

