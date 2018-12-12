/* Se crea base de datos */
CREATE DATABASE PokemonBase;
USE PokemonBase;

DROP TABLE IF EXISTS Pokedex;
DROP TABLE IF EXISTS Usuario;

CREATE TABLE Usuario(
	idUsuario int(11) PRIMARY KEY NOT NULL AUTO_INCREMENT,
	nombre_usuario varchar(50) NOT NULL ,
	password varchar(100) NOT NULL
);

DROP TABLE IF EXISTS Pokemon;
CREATE TABLE Pokemon(
	idPokemon int(11) PRIMARY KEY NOT NULL AUTO_INCREMENT,
	nombre varchar(50) NOT NULL ,
	imagen varchar(100) NOT NULL
);

CREATE TABLE Pokedex(
	idPokedex int(11) PRIMARY KEY NOT NULL AUTO_INCREMENT,
	idUsuario int(11) NOT NULL ,
	idPokemon int(11) NOT NULL
);

ALTER TABLE Pokedex 
ADD CONSTRAINT fk_id_usuario
FOREIGN KEY (idUsuario) 
REFERENCES Usuario(idUsuario) ON DELETE CASCADE ON UPDATE CASCADE; 

ALTER TABLE Pokedex 
ADD CONSTRAINT fk_id_poke
FOREIGN KEY (idPokemon) 
REFERENCES Pokemon(idPokemon) ON DELETE CASCADE ON UPDATE CASCADE; 