pipeline {
    agent any

    stages {
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
                        echo "skipping build stage"
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
