# Redes
Repositorio para el segundo proyecto de redes.

Integrantes:

->Luis Brandon Flores Gonzalez

->Juan Carlos Garduño Vargas

->Hector Santaella Marin


#Manejador de Base de Datos , Instalación de la Base de Datos.

Se utilizó como manejador de base de datos Mysql, por lo que la instalación del manejador dependera del sistema operativo.
Para instalar la base de datos, se debe de instalar mysql y correr el comando:

En Ubuntu/Debian:
sudo apt-get install libmysqlclient-dev
En fedora:
sudo yum install mysql-devel

Para instalar la base de datos:
mysql -u root -p <PokemonBase.sql  

Por default en la base de datos tenemos usuarios ya creados con nombre de usuario y contraseña, como también los pokemon que se utilizaran.

**NOTA**
Cambiar en el archivo PokemonClient.c el usuario y contraseña de MySQL del usuario, y en caso de que no tenga password, asignarlo con el valor de "NULL".

Ejecuciones:

Para compilar la aplicacion primero debe entrar al directorio del proyecto y ejecutar el comando: make


Despues para la ejecucion del Servidor: ./TCPMainServer-Thread

Y por ultimo la ejecucion del Cliente: ./clientPokemon 127.0.0.1 9999

