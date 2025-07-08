pipeline {
    agent any

    parameters {
        booleanParam(
            name: 'INSTALL_DEPENDENCIES',
            defaultValue: false,
            description: 'Install DEPENDENCIES in Prepare stage?'
        ),
        booleanParam(
            name: 'START_HOME_ALARM_CORE',
            defaultValue: false,
            description: 'Start Home Alarm Core service?'
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
            when {
                expression { return params.START_HOME_ALARM_CORE }
            }
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
            when {
                expression { return params.START_HOME_ALARM_CORE }
            }
            steps {
                sh '''
                    cd web
                    mkdir -p data
                    # Ensure the data directory is writable by the current user
                    sudo docker compose up -d
                '''
            }
        }

        stage('Install SonarQube Scanner') {
            environment {
                SONAR_SCANNER_VERSION = '7.0.2.4839'
                SONAR_DIR = "${HOME}/workspace/.sonar"
            }
                steps {
                    sh '''
                        set -e

                        mkdir -p $SONAR_DIR

                        # Download and unzip SonarQube scanner
                        curl -fSL -o $SONAR_DIR/sonar-scanner.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_SCANNER_VERSION}-linux-aarch64.zip
                        unzip -o $SONAR_DIR/sonar-scanner.zip -d $SONAR_DIR

                        # Download and unzip build-wrapper
                        curl -fSL -o $SONAR_DIR/build-wrapper.zip https://sonarcloud.io/static/cpp/build-wrapper-linux-aarch64.zip
                        unzip -o $SONAR_DIR/build-wrapper.zip -d $SONAR_DIR
                    '''
                }
            }

        stage('Build C++ Program') {
            steps {
                sh '''
                    export PATH=$HOME/workspace/.sonar/build-wrapper-linux-aarch64:$PATH
                    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
                    build-wrapper-linux-aarch64 --out-dir bw-output make -j$(nproc)
                '''
            }
        }

        stage('Run C++ Program') {
            when {
                expression { return params.START_HOME_ALARM_CORE }
            }
            steps {
                sh '''
                    ./home-alarm-core &
                    PID=$!

                    sleep 600

                    if ! kill -0 $PID 2>/dev/null; then
                        echo "home-alarm-core exited early. Failing pipeline."
                        exit 1
                    fi

                    echo "home-alarm-core is still running. Terminating..."
                    kill $PID
                    wait $PID || true
                '''
            }
        }

        stage('SonarQube Analysis') {
            environment {
                SONAR_SCANNER_VERSION = '7.0.2.4839'
                SONAR_SCANNER_HOME = "${HOME}/workspace/.sonar/sonar-scanner-${SONAR_SCANNER_VERSION}-linux-aarch64"
            }
            steps {
                withCredentials([string(credentialsId: 'sonar-token', variable: 'SONAR_TOKEN')]) {
                    sh '''
                        export PATH=$SONAR_SCANNER_HOME/bin:$HOME/workspace/.sonar/build-wrapper-linux-aarch64:$PATH
                        export SONAR_SCANNER_OPTS="-server"

                        sonar-scanner \
                        -Dsonar.inclusions=**/*.c,**/*.cpp,**/*.h \
                        -Dsonar.cfamily.compile-commands=bw-output/compile_commands.json \
                        -Dsonar.token=$SONAR_TOKEN
                    '''
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
