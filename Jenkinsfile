pipeline {
    agent {
        docker {
            image 'ubuntu:latest'
            label 'docker'  // Optional: depends on your agent setup
        }
    }

    stages {
        stage('Prepare') {
            steps {
                sh 'apt-get update && apt-get install -y cat'
            }
        }

        stage('Print README.md') {
            steps {
                script {
                    if (fileExists('README.md')) {
                        sh 'cat README.md'
                    } else {
                        echo 'README.md not found'
                    }
                }
            }
        }

        stage('Say Hello') {
            steps {
                echo 'Hello, World!'
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
