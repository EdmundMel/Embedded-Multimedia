## 1. Systemübersicht

```text
┌───────────────┐       TCP (JSON)        ┌───────────────────┐      HTTP     ┌─────────────┐
│ Mess-Client   │ ───────────────────────▶│ DB-Server         │◀─────────────▶│ Grafana     │
│ (IoT-Sensor,  │                         │ (Go/C)            │               │ (Simple JSON│
│   Skript…)    │                         │                   │               │   Plugin)   │
└───────────────┘                         └───────────────────┘               └─────────────┘

```

- **TCP-Ingestion**: Port 5433
- **Storage**: WAL + segmentierte Data-Files + B-Baum-Index
- **Query-API**: HTTP-Endpoint `/query` für Grafana

---

## 2. Komponenten im Detail

### 2.1 TCP-Server & Authentifizierung

- **Sprachen**

  - **Go**: `net`-Package, `golang.org/x/crypto/bcrypt`
  - **C**: BSD-Sockets, OpenSSL/TLS, `libbcrypt`

- **Start**

  - Konfigurierbarer Port (Standard: `5433`)

- **Authentifizierungs-Frame**

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

    oder

    ```json
    { "status": "error", "message": "Invalid credentials" }
    ```

  - Erst nach `"status":"ok"` werden Data-Frames akzeptiert.

- **Dauerbetrieb**

  - Verbindung bleibt offen, Daten-Frames kommen kontinuierlich (alle ~10 Sekunden).

### 2.2 Ingestion-Protokoll & Datenformat

- **Frame-Typen** (newline-delimited JSON):

  1.  **Auth**

      ```json
      { "type": "auth", "username": "...", "password": "..." }
      ```

  2.  **Data**

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

  3.  **Close** (optional)

      ```json
      { "type": "close" }
      ```

### 2.3 Storage-Engine

#### 2.3.1 Write-Ahead Log (WAL)

- **Append-Only**: Jeder Data-Frame wird sofort zeilenweise in `wal/` geschrieben (z. B. `0000001.wal`, `0000002.wal` …).
- **Sicherheit**: Nach fsync-Ack an den Client gilt die Zeile als persisted.

#### 2.3.2 Segmentierte Data-Files

- **Periodisch** (z. B. alle 60 Sek. oder bei 64 MB WAL-Größe)  
  → Konsolidierung: WAL-Einträge in ein neues Segment (`data/segment0001.dat`) übernehmen, WAL-Dateien löschen.
- **Format der Segmente**:

  - Binär: `[MetricID (u32)][Timestamp (i64)][Value (f64)]`
  - Segmente bleiben unveränderlich.

#### 2.3.3 Index (B-Baum)

- **On-Disk-B-Baum** für jede Metrik

  - Schlüssel: Timestamp
  - Wert: Offset im jeweiligen Segment

- **In-Memory-Cache** der obersten Nodes zum Beschleunigen; bei Shutdown serialisiert.

---

## 3. Konfiguration (`config.yaml`)

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

## 4. HTTP-Query-API für Grafana

- **Pfad**: `/query`
- **Methode**: `GET`

- **Antwort** (`application/json`):

  ```json
  {
    "metric": "temperature",
    "values": [
      ["2025-06-25T14:00:00Z", 22.8],
      ["2025-06-25T14:10:00Z", 23.1]
      // ...
    ]
  }
  ```

- **Implementierung**:

  - **Go**: `net/http`, parse Query-Params, B-Baum-Suche von `from` bis `to`, JSON-Marshall
  - **C**: `libmicrohttpd`, JSON-Bibliothek (z. B. cJSON)

---

## 5. Grafana-Integration

1.  **Plugin**: Simple JSON Data Source (oder Infinity)
2.  **Konfiguration**:

    - URL: `http://<DB_HOST>:8181`

3.  **Panel-Query**:

    ```
    /query?metric=temperature&from=$__from&to=$__to

    ```

4.  **Visualisierung**:

    - X-Achse: Zeit
    - Y-Achse: Wert

---

## 6. Projektstruktur

```text
├── cmd
│   └── db
│       └── main.go        # oder main.c
├── pkg
│   ├── server
│   │   ├── listener.go    # TCP Setup
│   │   └── auth.go        # bcrypt-Verifikation
│   ├── ingestion
│   │   └── protocol.go    # JSON-Parser, Frame-Dispatcher
│   ├── storage
│   │   ├── wal.go
│   │   ├── segment.go
│   │   └── index.go       # B-Baum Implementation
│   └── http
│   │   └── query.go       # /query-Handler
├── config.yaml
└── README.md

```
