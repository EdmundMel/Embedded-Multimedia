#!/bin/bash
set -e

# Update system packages
sudo apt update && sudo apt upgrade -y

# Install required system packages
sudo apt install -y libgpiod-dev python3-venv python3-pip curl git libpq-dev git software-properties-common lsb-release docker.io

# --- Install Docker ---
# sudo apt remove -y docker docker-engine docker.io containerd runc || true
# curl -fsSL https://get.docker.com -o get-docker.sh
# sh get-docker.sh
# rm get-docker.sh
# sudo usermod -aG docker "$USER"

# --- Install latest Go ---
GO_VERSION=$(curl -s https://go.dev/VERSION?m=text)
GO_TAR="$GO_VERSION.linux-amd64.tar.gz"
curl -LO "https://go.dev/dl/${GO_TAR}"
sudo rm -rf /usr/local/go
sudo tar -C /usr/local -xzf "$GO_TAR"
rm "$GO_TAR"
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.profile
export PATH=$PATH:/usr/local/go/bin

# --- Install httplib.h (cpp-httplib) ---
HTTPLIB_VERSION="0.23.1"
mkdir -p include/cpp-httplib
curl -L "https://raw.githubusercontent.com/yhirose/cpp-httplib/v${HTTPLIB_VERSION}/httplib.h" \
  -o include/cpp-httplib/httplib.h

# --- Setup Python venv and install lgpio ---
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip
pip install lgpio

echo "✅ Installation completed. You may need to restart your terminal for Docker, Go, and group changes to take effect."
