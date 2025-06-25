# DB Specification

This document contains the specification for a lightweight time-series database in Go or C.

---

## 1. System Overview

```text
┌───────────────┐       TCP (JSON)        ┌───────────────────┐      HTTP     ┌─────────────┐
│ Data Client   │ ───────────────────────▶│ DB Server         │◀─────────────▶│ Grafana     │
│ (IoT Sensor,  │                         │ (Go/C)            │               │ (Simple JSON│
│   Script…)    │                         │                   │               │   Plugin)   │
└───────────────┘                         └───────────────────┘               └─────────────┘
```

- **TCP Ingestion**: Port 5433
- **Storage**: WAL + segmented data files + B-tree index
- **Query API**: HTTP endpoint `/query` for Grafana

---

## 2. Components in Detail

### 2.1 TCP Server & Authentication

- **Languages**

  - **Go**: `net` package, `golang.org/x/crypto/bcrypt`
  - **C**: BSD sockets, OpenSSL/TLS, `libbcrypt`

- **Startup**

  - Configurable port (default: 5433)

- **Authentication Frame**

  - Client → Server (newline-delimited JSON):

    ```json
    {
      "type": "auth",
      "username": "user1",
      "password": "secret"
    }
    ```

  - Server → Client:

    ```json
    { "status": "ok" }
    ```

    or

    ```json
    { "status": "error", "message": "Invalid credentials" }
    ```

  - Data frames are only accepted after `"status": "ok"`.

- **Continuous Operation**

  - Connection remains open and data frames arrive continuously (approximately every 10 seconds).

### 2.2 Ingestion Protocol & Data Format

- **Frame Types** (newline-delimited JSON):

  1. **Auth**

     ```json
     { "type": "auth", "username": "...", "password": "..." }
     ```

  2. **Data**

     ```json
     {
       "type": "data",
       "metric": "temperature",
       "timestamp": "2025-06-25T14:12:03Z",
       "value": 23.7,
       "tags": {
         "location": "room-1",
         "unit": "C"
       }
     }
     ```

  3. **Close** (optional)

     ```json
     { "type": "close" }
     ```

### 2.3 Storage Engine

#### 2.3.1 Write-Ahead Log (WAL)

- **Append-Only**: Each data frame is immediately appended as a new line in the `wal/` directory (e.g., `0000001.wal`, `0000002.wal`, etc.).
- **Durability**: After an fsync acknowledgment to the client, the entry is considered persisted.

#### 2.3.2 Segmented Data Files

- **Periodic Checkpoints** (e.g., every 60 seconds or at 64 MB WAL size)
  → Consolidate WAL entries into a new segment file (`data/segment0001.dat`) and remove old WAL files.
- **Segment Format**:

  - Binary: `[MetricID (u32)][Timestamp (i64)][Value (f64)][TagBlockLen (u32)][TagBlock (JSON)]`
  - Segments are immutable.

#### 2.3.3 Index (B-Tree)

- **On-Disk B-Tree** per metric

  - Key: Timestamp
  - Value: Offset within the segment file

- **In-Memory Cache** of upper nodes for faster lookups; serialized on shutdown.

---

## 3. Configuration (`config.yaml`)

```yaml
server:
  tcp_port: 5433

auth:
  users:
    - username: "user1"
      password_hash: "$2a$10$..."

storage:
  data_dir: "./data"
  wal_dir: "./wal"
  wal_segment_size_mb: 64
  checkpoint_interval_s: 60

query:
  http_port: 8181
```

---

## 4. HTTP Query API for Grafana

- **Path**: `/query`

- **Method**: `GET`

- **Response** (`application/json`):

  ```json
  {
    "metric": "temperature",
    "values": [
      ["2025-06-25T14:00:00Z", 22.8],
      ["2025-06-25T14:10:00Z", 23.1]
    ]
  }
  ```

- **Implementation**:

  - **Go**: `net/http`, parse query params, B-tree range lookup, JSON marshal
  - **C**: `libmicrohttpd`, JSON library (e.g., cJSON)

---

## 5. Grafana Integration

1. **Plugin**: Simple JSON Data Source (or Infinity)
2. **Configuration**:

   - URL: `http://<DB_HOST>:8181`

3. **Panel Query**:

   ```
   /query?metric=temperature&from=$__from&to=$__to
   ```

4. **Visualization**:

   - X-axis: Time
   - Y-axis: Value

---

## 6. Project Structure

```text
├── cmd
│   └── db
│       └── main.go        # or main.c
├── pkg
│   ├── server
│   │   ├── listener.go    # TCP setup
│   │   └── auth.go        # bcrypt authentication
│   ├── ingestion
│   │   └── protocol.go    # JSON parser, frame dispatcher
│   ├── storage
│   │   ├── wal.go
│   │   ├── segment.go
│   │   └── index.go       # B-tree implementation
│   └── http
│       └── query.go       # /query handler
├── config.yaml
└── README.md
```

---

## 7. Summary

- **Persistent TCP connection** with authentication
- **Newline-delimited JSON** every 10 seconds
- **WAL + segmented data files + B-tree index** for efficiency
- **HTTP endpoint** `/query` for Grafana time-series
- **Configurable** via YAML
