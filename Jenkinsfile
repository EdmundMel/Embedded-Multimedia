pipeline {
    agent any
    stages {
        stage('Print ReadME and Hello World') {
            steps {
                script {
                    sh "cat README.md; echo 'Hello, World!'"
                }
            }
        }
        stage('Build and Run C Program') {
            steps {
                script {
                    // Assume main.c exists in the repo
                    sh 'gcc main.c -o main'
                    sh './main'
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
