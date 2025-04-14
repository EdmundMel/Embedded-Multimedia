pipeline {
    agent any

    environment {
        CONTAINER_NAME = "temp-ubuntu-container"
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Run Docker Container') {
            steps {
                script {
                    sh """
                    docker run --name \$CONTAINER_NAME --rm \
                        -v "\$(pwd)":/workspace \
                        -w /workspace \
                        ubuntu bash -c "apt-get update && apt-get install -y cat && cat README.md || echo 'README.md not found'; echo 'Hello, World!'"
                    """
                }
            }
        }
    }

    post {
        always {
            node {
                echo 'Cleaning up workspace...'
                cleanWs()
            }
        }
    }
}
