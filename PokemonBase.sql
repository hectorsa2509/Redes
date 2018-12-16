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



INSERT INTO Usuario VALUES (1,"Hector","kaab");
INSERT INTO Usuario VALUES (2,"Juan","kaab");
INSERT INTO Usuario VALUES (3,"Luis","kaab");


INSERT INTO Pokemon VALUES (1,"Bulbasor","PLACEHOLDER");
INSERT INTO Pokemon VALUES (2,"Ivysaur","PLACEHOLDER");
INSERT INTO Pokemon VALUES (3,"Venusaur","PLACEHOLDER");
INSERT INTO Pokemon VALUES (4,"Charmander","PLACEHOLDER");
INSERT INTO Pokemon VALUES (5,"Charmeleon","PLACEHOLDER");
INSERT INTO Pokemon VALUES (6,"Charizard","PLACEHOLDER");
INSERT INTO Pokemon VALUES (7,"Squirtle","PLACEHOLDER");
INSERT INTO Pokemon VALUES (8,"Wartortle","PLACEHOLDER");
INSERT INTO Pokemon VALUES (9,"Blastoise","PLACEHOLDER");