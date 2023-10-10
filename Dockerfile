# Usar una imagen base que incluya el compilador de C++ y las herramientas necesarias
FROM ubuntu:latest

# Actualizar el sistema e instalar las dependencias necesarias
RUN apt-get update && apt-get install -y g++ make cmake libcpprest-dev libpqxx-dev libssl-dev libcpprest-dev

# Establecer el directorio de trabajo en el que se compilará la API
WORKDIR /api

# Copiar los archivos de código fuente de la API al contenedor
COPY main.cpp /api/

# Compilar la API (ajusta el comando según tus necesidades)
RUN g++ -o my_api main.cpp -std=c++11 -lpqxx -lpq -lssl -lcrypto -lcpprest

# Exponer el puerto en el que se ejecutará la API
EXPOSE 8080

# Comando por defecto para ejecutar la API (ajusta el comando según tu aplicación)
CMD ["./my_api"]

#Instalacion del contenedor para la base de datos, en este caso será postgres
# docker pull postgres #Descarga de la imagen mas actual para el contenedor

#Corremos el contenedor con la imagen descargada
#sudo docker run --name BD-postgres -e POSTGRES_PASSWORD=batman2505 -d -p 5432:5432 postgres

#Instrucción para construir la imagen de este docker file
#sudo docker build -t my_api_image .

#Instrucción para correr la imagen previamente creada
#sudo docker run --name api_contenedor --network mi_red -p 8080:8080 -d my_api_image
