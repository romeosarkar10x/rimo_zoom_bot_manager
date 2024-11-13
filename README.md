# rimo_zoom_bot_manager

This project is a C++ application with a set of shell scripts to help you set up dependencies, development environment, and run the application. Follow the instructions below to make the best use of these scripts.

## Prerequisites

Ensure you have the following installed:
- Bash shell

## Setup

The `scripts/` directory contains three scripts to help you with setting up and running the project.

### 1. Installing Project Dependencies

To install all the necessary dependencies for running the project, execute:

```bash
./scripts/install_dependencies.sh
```

This will set up all required libraries and packages for the project. Run this script before attempting to build or run the project.

### 2. Installing Development Dependencies

If you are planning to contribute or develop further, you may also need additional development tools and libraries. Run:

```bash
./scripts/install_dev_dependencies.sh
```

This script installs dependencies specifically for development purposes, such as testing frameworks, debugging tools, etc.

### 3. Building and Running the Project

Once dependencies are installed, you can build and run the project with:

```bash
./scripts/build_and_run.sh
```

This script compiles the source code and starts the application.

---