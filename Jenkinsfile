pipeline {
    agent any

    stages {
        stage('Prepare') {
            steps {
                sh '''
                    sudo apt-get update
                    sudo apt-get install -y libpq-dev git software-properties-common lsb-release
                    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
                    sudo apt-add-repository "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
                    sudo apt-get update
                    sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
                    sudo apt install kitware-archive-keyring
                    sudo apt install cmake
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
