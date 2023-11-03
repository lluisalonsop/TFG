#!/bin/bash

# Instalar el servidor SSH
sudo apt-get update
sudo apt-get install -y libfltk1.3-dev
sudo apt-get install -y openssh-server
sudo apt-get install libssh-dev

# Editar el archivo de configuración SSH
sudo sed -i 's/#Port 22/Port 443/' /etc/ssh/sshd_config

# Iniciar el servicio SSH
sudo service ssh start

# Crear un nuevo usuario
sudo useradd -m ClientP2P

# Establecer la contraseña del nuevo usuario
echo 'ClientP2P:S3cur3P4ssw0rd!!!' | sudo chpasswd

# Crear el directorio .ssh para el nuevo usuario
sudo mkdir /home/ClientP2P/.ssh
sudo touch /home/ClientP2P/.ssh/authorized_keys

# Cambiar los permisos del directorio .ssh
sudo chown ClientP2P:ClientP2P /home/ClientP2P/.ssh
sudo chmod 700 /home/ClientP2P/.ssh

# Agregar el usuario al grupo sudoers
sudo usermod -aG sudo ClientP2P

echo "El servidor SSH se ha instalado y configurado correctamente."
