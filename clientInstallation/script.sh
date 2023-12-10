#!/bin/bash

# Instalar el servidor SSH
sudo apt-get update -y
sudo apt-get install -y libfltk1.3-dev
sudo apt-get install -y openssh-server
sudo apt install -y curl

# Editar el archivo de configuración SSH
sudo sed -i 's/#Port 22/Port 443/' /etc/ssh/sshd_config

# Iniciar el servicio SSH
sudo service ssh start

# Crear un nuevo usuario
sudo useradd -m ClientP2P

# Establecer la contraseña del nuevo usuario
echo 'ClientP2P:S3cur3P4ssw0rd!!!' | sudo chpasswd

# Crear el directorio .ssh para el nuevo usuario
sudo mkdir -p "/home/ClientP2P/.ssh"
sudo touch /home/ClientP2P/.ssh/authorized_keys

# Cambiar los permisos del directorio .ssh
sudo chown ClientP2P:ClientP2P /home/ClientP2P/.ssh
sudo chmod 700 /home/ClientP2P/.ssh

# Agregar el usuario al grupo sudoers
sudo usermod -aG sudo ClientP2P

# DNS Resolution

# URL del archivo raw en GitHub
github_raw_url="https://raw.githubusercontent.com/lluisalonsop/TFG/main/utils/ServerIp.txt"

# Descarga del contenido desde la URL
raw_content=$(curl -sSL "$github_raw_url")

# Extrae la dirección IP usando AWK
ip=$(echo "$raw_content" | awk '/Server Ip:/ {print $NF}')

# Verifica si la entrada ya existe en /etc/hosts
if ! grep -qF "$ip p2pProxyService.com" /etc/hosts; then
  # Agrega la entrada al archivo /etc/hosts
  echo "$ip p2pProxyService.com" | sudo tee -a /etc/hosts
  echo "La entrada se agregó correctamente a /etc/hosts."
else
  echo "La entrada ya existe en /etc/hosts. No se agregó."
fi

sudo mkdir -p "/home/ClientP2P/utils/"


sudo bash -c '
  if [ ! -f "/home/ClientP2P/utils/cert.pem" ]; then
    sudo wget -q -P "/home/ClientP2P/utils" "https://raw.githubusercontent.com/lluisalonsop/TFG/main/utils/cert.pem"
  fi
'

echo "El servidor SSH se ha instalado y configurado correctamente."