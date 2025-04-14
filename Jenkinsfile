pipeline {
    agent any
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Print ReadME and Hello World') {
            steps {
                script {
                    sh "cat README.md; echo 'Hello, World!'"
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
