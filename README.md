# Embedded-Multimedia Home Alarm System

Ein umfassendes Home Alarm System mit C++ Core, Go Telegram Bot, PostgreSQL Database und Grafana Monitoring.

## 📋 Voraussetzungen

- **Linux Server** (ARM64 oder x86_64 Architecture)
- **Docker & Docker Compose** installiert
- **Git** installiert
- **Sudo-Rechte** für Installation

## 🚀 Manuelle Installation

### 1. Repository Setup

```bash
# Clone das Repository
git clone https://github.com/EdmundMel/Embedded-Multimedia.git
cd Embedded-Multimedia
```

### 2. System Dependencies installieren

```bash
# Install requirements script ausführen
sudo sh ./install_requirements.sh

# CMake Repository hinzufügen (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y wget gpg
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
  | gpg --dearmor \
  | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' \
  | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

# Core Development Tools
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    postgresql-server-dev-all \
    libpq-dev \
    golang-go \
    kitware-archive-keyring
```

### 3. Docker Services starten

#### PostgreSQL Database

```bash
cd database
sudo docker compose up -d

# Warten bis PostgreSQL bereit ist
echo "Waiting for PostgreSQL to start..."
for i in {1..30}; do
  sudo docker exec postgres pg_isready -U dbuser && break
  sleep 1
done
cd ..
```

#### Grafana Dashboard (Optional)

```bash
cd web
mkdir -p data
sudo docker compose up -d
cd ..
```

### 4. C++ Projekt builden

```bash
# CMake konfigurieren mit Compile Commands für Analyse-Tools
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .

# Projekt kompilieren
make -j$(nproc)

# Unit Tests ausführen
ctest --output-on-failure

# Test-Coverage generieren (optional)
./db_access_test

# Coverage-Dateien generieren
cd CMakeFiles/db_access.dir/src
gcov -b -c db_access.cpp.gcno
mv *.gcov ../../../src/
cd ../../..
```

### 5. Go Projekt testen

```bash
cd home-alarm-bot
go test ./... -coverprofile=coverage.out -covermode=set
cd ..
```

### 6. Services starten

#### Home Alarm Core (C++)

```bash
# Core Service im Hintergrund starten
./home-alarm-core &
CORE_PID=$!

# Process ID für später speichern
echo $CORE_PID > home-alarm-core.pid

# Service stoppen:
# kill $(cat home-alarm-core.pid)
```

#### Telegram Bot (Go)

```bash
cd home-alarm-bot
go run cmd/bot/main.go &
BOT_PID=$!
echo $BOT_PID > ../telegram-bot.pid
cd ..

# Bot stoppen:
# kill $(cat telegram-bot.pid)
```

## 🧪 Entwicklung & Testing

### C++ Development

```bash
# Einzelne Tests ausführen
./db_access_test
```

### Go Development

```bash
cd home-alarm-bot

# Tests mit verbose output
go test -v ./...

# Race condition detection
go test -race ./...

# Benchmark tests
go test -bench=. ./...

# Code formatieren
go fmt ./...

# Go modules aufräumen
go mod tidy

cd ..
```

### Database Management

```bash
# PostgreSQL Container Status prüfen
sudo docker compose -f database/docker-compose.yml ps

# Database Logs anschauen
sudo docker compose -f database/docker-compose.yml logs

# Database Shell
sudo docker exec -it postgres psql -U dbuser -d sensordb

# Database Backup
sudo docker exec postgres pg_dump -U dbuser sensordb > backup.sql

# Database Restore
cat backup.sql | sudo docker exec -i postgres psql -U dbuser -d sensordb
```

## 🐳 Docker Services Übersicht

### PostgreSQL

- **Port:** 5432
- **Database:** `sensordb`
- **User:** `dbuser`
- **Container Name:** `postgres`

### Grafana (Optional)

- **Port:** 3000
- **Admin:** admin/admin
- **Data Directory:** `web/data`
- **Container Name:** `grafana`

## 🔧 GPIO Configuration (Raspberry Pi)

```bash
# Pin 16 als Input mit Pull-up konfigurieren
pinctrl set 16 ip pu

# GPIO Status prüfen
pinctrl get 16

# Weitere GPIO Operationen
pinctrl set <pin> op    # Output
pinctrl set <pin> ip    # Input
pinctrl set <pin> ip pd # Input with pull-down
```

## 🔍 Troubleshooting

### Häufige Probleme

#### 1. Dependencies fehlen

```bash
# Dependencies manuell installieren
sudo apt-get update
sudo apt-get install -y build-essential cmake libpq-dev golang-go
```

#### 2. Docker Permission Denied

```bash
# Aktuellen User zu docker Gruppe hinzufügen
sudo usermod -aG docker $USER
# Neu anmelden oder:
newgrp docker
```

#### 3. PostgreSQL Connection Failed

```bash
# Container Status prüfen
sudo docker ps | grep postgres
sudo docker logs postgres

# Container neu starten
sudo docker compose -f database/docker-compose.yml restart
```

#### 4. Port bereits in Verwendung

```bash
# Prozess auf Port finden
sudo netstat -tlnp | grep :5432
sudo lsof -i :5432

# Prozess beenden
sudo kill <PID>
```

#### 5. Build Fehler

```bash
# Build-Verzeichnis komplett neu erstellen
rm -rf CMakeFiles/ CMakeCache.txt
cmake .
make clean
make -j$(nproc)
```

## 🧹 Cleanup & Shutdown

### Services stoppen

```bash
# Alle Background-Services stoppen
if [ -f home-alarm-core.pid ]; then
    kill $(cat home-alarm-core.pid) 2>/dev/null || true
    rm home-alarm-core.pid
fi

if [ -f telegram-bot.pid ]; then
    kill $(cat telegram-bot.pid) 2>/dev/null || true
    rm telegram-bot.pid
fi

# Docker Services stoppen
sudo docker compose -f database/docker-compose.yml down
sudo docker compose -f web/docker-compose.yml down || true
```

### Build-Dateien aufräumen

```bash
# Build Artifacts löschen
make clean
rm -rf CMakeFiles/ CMakeCache.txt
rm -rf bw-output/
rm -f *.gcov src/*.gcov
rm -f home_alarm_bot_coverage.out
rm -f home-alarm-core db_access_test
```

### Docker komplett aufräumen

```bash
# Alle Container, Images und Volumes löschen (Vorsicht!)
sudo docker system prune -a --volumes
```

## 📊 Monitoring & Logs

### Log-Verzeichnisse

- **Application Logs:** Siehe Stdout/Stderr der Services
- **Docker Logs:** `sudo docker logs <container-name>`
- **System Logs:** `/var/log/syslog`

### Performance Monitoring

```bash
# System Resources
htop
iostat -x 1
free -h
df -h

# Docker Resources
sudo docker stats

# Network
sudo netstat -tlnp
sudo ss -tlnp
```

---
