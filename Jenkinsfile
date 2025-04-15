pipeline {
    agent any

    stages {
        stage('Install Dependencies') {
            steps {
                script {
                    sh '''
                        echo "Updating packages and installing build essentials..."
                        sudo apt-get update
                        sudo apt-get install -y build-essential
                    '''
                }
            }
        }

        stage('Print README and Hello World') {
            steps {
                script {
                    sh "cat README.md || echo 'No README.md found'"
                    echo 'Hello, World!'
                }
            }
        }

        stage('Build and Run C Program') {
            steps {
                script {
                    sh '''
                        gcc main.c -o main
                        ./main
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
