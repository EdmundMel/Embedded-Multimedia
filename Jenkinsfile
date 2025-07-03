pipeline {
    agent any

    parameters {
        booleanParam(
            name: 'INSTALL_DEPENDENCIES',
            defaultValue: false,
            description: 'Install DEPENDENCIES in Prepare stage?'
        )
    }

    stages {
        stage('Prepare') {
            when {
                expression { return params.INSTALL_DEPENDENCIES }
            }
            steps {
                sh '''
                    sudo apt-get update
                    sudo apt-get install -y libpq-dev git software-properties-common lsb-release

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

        stage('Build and Run C++ Program') {
            steps {
                sh '''
                    mkdir -p build
                    cd build
                    cmake ..
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
            echo 'Cleaning up workspace...'
            cleanWs()
        }
    }
}
