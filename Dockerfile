FROM ubuntu:latest

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    manpages-dev \
    vim \
    nano \
    iputils-ping \
    inetutils-traceroute \
    iproute2 \
    openssh-server \
    sudo \
    curl \
    telnet \
    dnsutils \
    libenet-dev # Add this line for the enet library

# Set up the user 'student' with password and sudo privileges
RUN useradd -m student && \
    echo "student:password" | chpasswd && \
    echo "root:password" | chpasswd && \
    adduser student sudo && \
    usermod -aG sudo student && \
    echo "student ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers

# Configure SSH server
RUN mkdir /run/sshd && \
    chown root:22 /run/sshd && \
    sudo sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# Port on which the container will listen for SSH connections
EXPOSE 22

# Create a directory for lab work
USER student
RUN mkdir /home/student/lab-work

# Set the working directory and the container command
WORKDIR /home/student/lab-work
CMD sudo /usr/sbin/sshd -D && /bin/bash
