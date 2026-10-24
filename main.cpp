#include "crow_all.h"
#include <iostream>
#include <string>

using namespace std;

// --- ESTRUCTURA BÁSICA ---
struct Animal {
    int id;
    string nombre;
    string especie;
    int edadMeses;
    Animal* siguiente;

    Animal(int _id, string _nombre, string _especie, int _edad) {
        id = _id; nombre = _nombre; especie = _especie; edadMeses = _edad;
        siguiente = nullptr;
    }
};

const int TAMANO_TABLA = 100;

// --- 1. TABLA HASH (Registro O(1)) ---
class TablaHashAnimales {
private:
    Animal* tabla[TAMANO_TABLA];
    int contadorId;

public:
    TablaHashAnimales() {
        contadorId = 1;
        for (int i = 0; i < TAMANO_TABLA; i++) tabla[i] = nullptr;
    }

    int funcionHash(int id) { return id % TAMANO_TABLA; }

    Animal* registrar(string nombre, string especie, int edad) {
        int id = contadorId++;
        Animal* nuevoAnimal = new Animal(id, nombre, especie, edad);
        int indice = funcionHash(id);
        
        nuevoAnimal->siguiente = tabla[indice];
        tabla[indice] = nuevoAnimal;
        return nuevoAnimal;
    }

    Animal* buscarPorId(int id) {
        int indice = funcionHash(id);
        Animal* temp = tabla[indice];
        while (temp != nullptr) {
            if (temp->id == id) return temp;
            temp = temp->siguiente;
        }
        return nullptr;
    }

    crow::json::wvalue obtenerJSON() {
        crow::json::wvalue::list listaJson;
        for (int i = 0; i < TAMANO_TABLA; i++) {
            Animal* temp = tabla[i];
            while (temp != nullptr) {
                crow::json::wvalue animalJson;
                animalJson["id"] = temp->id;
                animalJson["nombre"] = temp->nombre;
                animalJson["especie"] = temp->especie;
                animalJson["edad"] = temp->edadMeses;
                listaJson.push_back(animalJson);
                temp = temp->siguiente;
            }
        }
        return crow::json::wvalue(listaJson);
    }
};

// --- 2. ÁRBOL BINARIO DE BÚSQUEDA (BST - Ordenamiento O(log n)) ---
struct NodoArbol {
    Animal* animal;
    NodoArbol* izquierdo;
    NodoArbol* derecho;
    NodoArbol(Animal* a) { animal = a; izquierdo = derecho = nullptr; }
};

class ArbolEdad {
private:
    NodoArbol* raiz;

    NodoArbol* insertarRec(NodoArbol* nodo, Animal* animal) {
        if (nodo == nullptr) return new NodoArbol(animal);
        if (animal->edadMeses <= nodo->animal->edadMeses)
            nodo->izquierdo = insertarRec(nodo->izquierdo, animal);
        else
            nodo->derecho = insertarRec(nodo->derecho, animal);
        return nodo;
    }

    void inOrdenRec(NodoArbol* nodo, crow::json::wvalue::list& lista) {
        if (nodo != nullptr) {
            inOrdenRec(nodo->izquierdo, lista);
            
            crow::json::wvalue aJson;
            aJson["id"] = nodo->animal->id;
            aJson["nombre"] = nodo->animal->nombre;
            aJson["especie"] = nodo->animal->especie;
            aJson["edad"] = nodo->animal->edadMeses;
            lista.push_back(aJson);

            inOrdenRec(nodo->derecho, lista);
        }
    }

public:
    ArbolEdad() { raiz = nullptr; }
    void insertar(Animal* animal) { raiz = insertarRec(raiz, animal); }
    
    crow::json::wvalue obtenerOrdenadoPorEdad() {
        crow::json::wvalue::list listaJson;
        inOrdenRec(raiz, listaJson);
        return crow::json::wvalue(listaJson);
    }
};

// --- 3. GRAFO (Compatibilidad de Adopciones / Red) ---
struct NodoAdyacencia {
    int idDestino;
    int afinidad; // Peso de la arista
    NodoAdyacencia* siguiente;
    NodoAdyacencia(int dest, int afi) : idDestino(dest), afinidad(afi), siguiente(nullptr) {}
};

class GrafoAdopciones {
private:
    NodoAdyacencia* listaAdyacencia[TAMANO_TABLA];

public:
    GrafoAdopciones() {
        for (int i = 0; i < TAMANO_TABLA; i++) listaAdyacencia[i] = nullptr;
    }

    void agregarConexion(int id1, int id2, int afinidad) {
        NodoAdyacencia* nuevo1 = new NodoAdyacencia(id2, afinidad);
        nuevo1->siguiente = listaAdyacencia[id1];
        listaAdyacencia[id1] = nuevo1;

        NodoAdyacencia* nuevo2 = new NodoAdyacencia(id1, afinidad);
        nuevo2->siguiente = listaAdyacencia[id2];
        listaAdyacencia[id2] = nuevo2;
    }

    crow::json::wvalue obtenerConexiones(int id, TablaHashAnimales& inv) {
        crow::json::wvalue::list listaJson;
        NodoAdyacencia* temp = listaAdyacencia[id];
        while (temp != nullptr) {
            Animal* a = inv.buscarPorId(temp->idDestino);
            if(a) {
                crow::json::wvalue cJson;
                cJson["id"] = a->id;
                cJson["nombre"] = a->nombre;
                cJson["afinidad"] = temp->afinidad;
                listaJson.push_back(cJson);
            }
            temp = temp->siguiente;
        }
        return crow::json::wvalue(listaJson);
    }
};

// --- 4. COLA (Agenda Médica FIFO) ---
struct Cita {
    string nombreAnimal;
    string motivo;
    Cita* siguiente;
    Cita(string _nombre, string _motivo) { nombreAnimal = _nombre; motivo = _motivo; siguiente = nullptr; }
};

class ColaMedica {
private:
    Cita* frente;
    Cita* finalCola;

public:
    ColaMedica() { frente = nullptr; finalCola = nullptr; }

    void encolar(string nombre, string motivo) {
        Cita* nuevaCita = new Cita(nombre, motivo);
        if (finalCola == nullptr) { frente = finalCola = nuevaCita; return; }
        finalCola->siguiente = nuevaCita;
        finalCola = nuevaCita;
    }

    bool desencolar() {
        if (frente == nullptr) return false;
        Cita* temp = frente;
        frente = frente->siguiente;
        if (frente == nullptr) finalCola = nullptr;
        delete temp;
        return true;
    }

    crow::json::wvalue obtenerJSON() {
        crow::json::wvalue::list listaJson;
        Cita* temp = frente;
        while (temp != nullptr) {
            crow::json::wvalue citaJson;
            citaJson["nombre"] = temp->nombreAnimal;
            citaJson["motivo"] = temp->motivo;
            listaJson.push_back(citaJson);
            temp = temp->siguiente;
        }
        return crow::json::wvalue(listaJson);
    }
};

// --- SERVIDOR Y ENDPOINTS ---
int main() {
    crow::SimpleApp app;
    TablaHashAnimales inventario;
    ArbolEdad arbolEdades;
    GrafoAdopciones grafoCompatibilidad;
    ColaMedica agenda;

    CROW_ROUTE(app, "/")([]() {
        crow::response res; res.set_static_file_info("index.html"); return res;
    });

    CROW_ROUTE(app, "/api/animales").methods(crow::HTTPMethod::GET)([&inventario]() { return inventario.obtenerJSON(); });

    CROW_ROUTE(app, "/api/registrar").methods(crow::HTTPMethod::POST)([&inventario, &arbolEdades](const crow::request& req) {
        auto datos = crow::json::load(req.body);
        if (!datos) return crow::response(400);
        Animal* nuevo = inventario.registrar(datos["nombre"].s(), datos["especie"].s(), datos["edad"].i());
        arbolEdades.insertar(nuevo);
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/buscar/<int>").methods(crow::HTTPMethod::GET)([&inventario](int id) {
        Animal* encontrado = inventario.buscarPorId(id);
        if (encontrado) {
            crow::json::wvalue res;
            res["id"] = encontrado->id; res["nombre"] = encontrado->nombre;
            res["especie"] = encontrado->especie; res["edad"] = encontrado->edadMeses;
            return crow::response(res);
        }
        return crow::response(404);
    });

    CROW_ROUTE(app, "/api/edades").methods(crow::HTTPMethod::GET)([&arbolEdades]() {
        return arbolEdades.obtenerOrdenadoPorEdad();
    });

    CROW_ROUTE(app, "/api/grafo/conectar").methods(crow::HTTPMethod::POST)([&grafoCompatibilidad](const crow::request& req) {
        auto datos = crow::json::load(req.body);
        if (!datos) return crow::response(400);
        grafoCompatibilidad.agregarConexion(datos["id1"].i(), datos["id2"].i(), datos["afinidad"].i());
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/grafo/<int>").methods(crow::HTTPMethod::GET)([&grafoCompatibilidad, &inventario](int id) {
        return grafoCompatibilidad.obtenerConexiones(id, inventario);
    });

    CROW_ROUTE(app, "/api/agenda").methods(crow::HTTPMethod::GET)([&agenda]() { return agenda.obtenerJSON(); });

    CROW_ROUTE(app, "/api/encolar").methods(crow::HTTPMethod::POST)([&agenda](const crow::request& req) {
        auto datos = crow::json::load(req.body);
        if (!datos) return crow::response(400);
        agenda.encolar(datos["nombre"].s(), datos["motivo"].s());
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/atender").methods(crow::HTTPMethod::POST)([&agenda]() {
        if (agenda.desencolar()) return crow::response(200);
        return crow::response(400);
    });

    app.port(18080).multithreaded().run();
}
