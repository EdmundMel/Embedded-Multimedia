pipeline {
    agent {
        docker {
            image 'danger89/cmake:bookworm-cppcheck-2.16.0'
        }
    }

    stages {
        stage('Prepare') {
            steps {
                sh '''
                    apt-get update
                    apt-get install -y libpq-dev git
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
