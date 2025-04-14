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
        stage('SonarQube Analysis') {
            def scannerHome = tool 'SonarScanner';
            withSonarQubeEnv() {
            sh "${scannerHome}/bin/sonar"
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
