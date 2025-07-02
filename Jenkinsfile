pipeline {
    agent any

    stages {
        stage('Install Dependencies') {
            steps {
                script {
                    sh '''
                        echo "Updating package lists and installing dependencies..."
                        sudo apt-get update
                        sudo apt-get install -y \
                            build-essential \
                            cmake \
                            libpq-dev \
                            git \
                            wget \
                            libssl-dev \
                            pkg-config
                    '''
                }
            }
        }

        stage('Build and Run C++ Program') {
            steps {
                script {
                    sh '''
                        echo "Configuring and building the project..."
                        mkdir -p build
                        cd build
                        cmake ..
                        make -j$(nproc)

                        echo "Running the program..."
                        ./home-alarm-core
                    '''
                }
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
