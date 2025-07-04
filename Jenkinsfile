pipeline {
    agent any

    parameters {
        booleanParam(
            name: 'INSTALL_DEPENDENCIES',
            defaultValue: false,
            description: 'Install DEPENDENCIES in Prepare stage?'
        )
    }

    environment {
        POSTGRES_CONTAINER_NAME = 'postgres'
    }

    stages {
        stage('Prepare') {
            when {
                expression { return params.INSTALL_DEPENDENCIES }
            }
            steps {
                sh '''
                    sudo apt-get update
                    sudo apt-get install -y libpq-dev git software-properties-common lsb-release docker.io

                    test -f /usr/share/doc/kitware-archive-keyring/copyright || \
                    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
                      | gpg --dearmor \
                      | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

                    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' \
                      | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

                    sudo apt-get update
                    sudo apt-get install -y kitware-archive-keyring cmake
                    cmake --version
                '''
            }
        }

        stage('Start PostgreSQL') {
            steps {
                sh '''
                    cd database
                    sudo docker compose up -d
                    # Wait until Postgres is accepting connections
                    echo "Waiting for PostgreSQL to start..."
                    for i in {1..30}; do
                      sudo docker exec $POSTGRES_CONTAINER_NAME pg_isready -U dbuser && break
                      sleep 1
                    done
                '''
            }
        }

        stage('Start Grafana') {
            steps {
                sh '''
                    cd web
                    sudo chown $(id -u):$(id -g) .
                    sudo docker compose up -d
                '''
            }
        }

        stage('Build and Run C++ Program') {
            steps {
                sh '''
                    cmake .
                    make -j$(nproc)
                    ./home-alarm-core
                '''
            }
        }

        stage('SonarQube Analysis') {
            steps {
                script {
                    def scannerHome = tool 'sonar'
                    withSonarQubeEnv() {
                        sh "${scannerHome}/bin/sonar-scanner"
                    }
                }
            }
        }
    }

    post {
        always {
            echo 'Stopping and removing PostgreSQL container if it exists...'
            sh '''
            sudo docker compose -f database/docker-compose.yml down || true
            sudo docker compose -f web/docker-compose.yml down || true
            '''
            echo 'Cleaning up workspace...'
            cleanWs()
        }
    }
}
