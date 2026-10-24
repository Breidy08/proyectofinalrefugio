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
    bool activo
    Animal* siguiente;

    Animal(int _id, string _nombre, string _especie, int _edad) {
        id = _id; nombre = _nombre; especie = _especie; edadMeses = _edad;
        activo = true;
        siguiente = nullptr;
    }
};

const int TAMANO_TABLA = 100;

// TABLA HASH (Registro e Inventario) ---
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
            if (temp->id == id && temp->activo) return temp;
            temp = temp->siguiente;
        }
        return nullptr;
    }

    bool eliminar(int id) {
        Animal* a = buscarPorId(id);
        if (a) {
            a->activo = false; 
            return true;
        }
        return false;
    }

    crow::json::wvalue obtenerJSON() {
        crow::json::wvalue::list listaJson;
        for (int i = 0; i < TAMANO_TABLA; i++) {
            Animal* temp = tabla[i];
            while (temp != nullptr) {
                if(temp->activo) {
                    crow::json::wvalue animalJson;
                    animalJson["id"] = temp->id;
                    animalJson["nombre"] = temp->nombre;
                    animalJson["especie"] = temp->especie;
                    animalJson["edad"] = temp->edadMeses;
                    listaJson.push_back(animalJson);
                }
                temp = temp->siguiente;
            }
        }
        return crow::json::wvalue(listaJson);
    }
};

// --- 2. ÁRBOL BINARIO DE BÚSQUEDA (BST - Ordenamiento) ---
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
            if (nodo->animal->activo) {
                crow::json::wvalue aJson;
                aJson["id"] = nodo->animal->id;
                aJson["nombre"] = nodo->animal->nombre;
                aJson["especie"] = nodo->animal->especie;
                aJson["edad"] = nodo->animal->edadMeses;
                lista.push_back(aJson);
            }
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

// --- 3. GRAFO (Registro Histórico de Adopciones) ---
struct NodoAdopcion {
    string nombreAdoptante;
    string nombreAnimal; 
    NodoAdopcion* siguiente;
    NodoAdopcion(string nAdop, string nAni) : nombreAdoptante(nAdop), nombreAnimal(nAni), siguiente(nullptr) {}
};

class GrafoAdopciones {
private:
    NodoAdopcion* adyacencia[TAMANO_TABLA]; 

public:
    GrafoAdopciones() {
        for (int i = 0; i < TAMANO_TABLA; i++) adyacencia[i] = nullptr;
    }

    void registrarAdopcion(int idAnimal, string nombreAdoptante, string nombreAnimal) {
        NodoAdopcion* nuevo = new NodoAdopcion(nombreAdoptante, nombreAnimal);
        nuevo->siguiente = adyacencia[idAnimal];
        adyacencia[idAnimal] = nuevo;
    }

    crow::json::wvalue obtenerHistorial() {
        crow::json::wvalue::list listaJson;
        for (int i = 0; i < TAMANO_TABLA; i++) {
            NodoAdopcion* temp = adyacencia[i];
            while (temp != nullptr) {
                crow::json::wvalue adop;
                adop["idAnimal"] = i;
                adop["nombreAnimal"] = temp->nombreAnimal;
                adop["nombreAdoptante"] = temp->nombreAdoptante;
                listaJson.push_back(adop);
                temp = temp->siguiente;
            }
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

    bool eliminarCitaPorIndice(int indice) {
        if (frente == nullptr || indice < 0) return false;
        
        if (indice == 0) return desencolar();

        Cita* actual = frente;
        Cita* previo = nullptr;
        int i = 0;
        
        while (actual != nullptr && i < indice) {
            previo = actual;
            actual = actual->siguiente;
            i++;
        }

        if (actual == nullptr) return false;

        previo->siguiente = actual->siguiente;
        if (actual == finalCola) finalCola = previo;
        delete actual;
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
    GrafoAdopciones grafoAdopciones;
    ColaMedica agenda;

    CROW_ROUTE(app, "/")([]() {
        crow::response res; res.set_static_file_info("index.html"); return res;
    });

    CROW_ROUTE(app, "/api/animales").methods(crow::HTTPMethod::Get)([&inventario]() { return inventario.obtenerJSON(); });

    CROW_ROUTE(app, "/api/registrar").methods(crow::HTTPMethod::Post)([&inventario, &arbolEdades](const crow::request& req) {
        auto datos = crow::json::load(req.body);
        if (!datos) return crow::response(400);
        Animal* nuevo = inventario.registrar(datos["nombre"].s(), datos["especie"].s(), datos["edad"].i());
        arbolEdades.insertar(nuevo);
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/buscar/<int>").methods(crow::HTTPMethod::Get)([&inventario](int id) {
        Animal* encontrado = inventario.buscarPorId(id);
        if (encontrado) {
            crow::json::wvalue res;
            res["id"] = encontrado->id; res["nombre"] = encontrado->nombre;
            res["especie"] = encontrado->especie; res["edad"] = encontrado->edadMeses;
            return crow::response(res);
        }
        return crow::response(404);
    });

    CROW_ROUTE(app, "/api/borrar/<int>").methods(crow::HTTPMethod::Delete)([&inventario](int id) {
        if(inventario.eliminar(id)) return crow::response(200);
        return crow::response(404);
    });

    CROW_ROUTE(app, "/api/edades").methods(crow::HTTPMethod::Get)([&arbolEdades]() {
        return arbolEdades.obtenerOrdenadoPorEdad();
    });

    CROW_ROUTE(app, "/api/adoptar").methods(crow::HTTPMethod::Post)([&inventario, &grafoAdopciones](const crow::request& req) {
        auto datos = crow::json::load(req.body);
        if (!datos) return crow::response(400, "JSON Invalido");
        
        int id = datos["idAnimal"].i();
        string adoptante = datos["nombreAdoptante"].s();

        Animal* a = inventario.buscarPorId(id);
        if (a != nullptr && a->activo) {
            grafoAdopciones.registrarAdopcion(id, adoptante, a->nombre);
            a->activo = false; 
            return crow::response(200);
        }
        return crow::response(404, "El animal no existe en memoria.");
    });

    CROW_ROUTE(app, "/api/adopciones").methods(crow::HTTPMethod::Get)([&grafoAdopciones]() {
        return grafoAdopciones.obtenerHistorial();
    });

    CROW_ROUTE(app, "/api/agenda").methods(crow::HTTPMethod::Get)([&agenda]() { return agenda.obtenerJSON(); });

    CROW_ROUTE(app, "/api/encolar").methods(crow::HTTPMethod::Post)([&agenda](const crow::request& req) {
        auto datos = crow::json::load(req.body);
        if (!datos) return crow::response(400);
        agenda.encolar(datos["nombre"].s(), datos["motivo"].s());
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/atender").methods(crow::HTTPMethod::Post)([&agenda]() {
        if (agenda.desencolar()) return crow::response(200);
        return crow::response(400);
    });

    CROW_ROUTE(app, "/api/agenda/borrar/<int>").methods(crow::HTTPMethod::Get)([&agenda](int indice) {
        if(agenda.eliminarCitaPorIndice(indice)) return crow::response(200);
        return crow::response(404);
    });

    app.port(18080).multithreaded().run();
}
