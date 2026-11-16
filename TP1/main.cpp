#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

// ==========================================
// 1. ESTRUCTURAS DE DATOS
// ==========================================

const int CANT_PAISES = 5;
const int CANT_AEROLINEAS = 8;
const int CANT_AIRPORTS = 15;
const int DIAS_SEMANA = 7;

// Estructura para el registro del archivo binario
struct RegistroArribo {
    int codigoPais;       // 0 a 4
    int codigoAerolinea;  // 0 a 7
    char iata[4];         // Cadena de 3 letras + '\0'
    int pasajeros;
    int diaSemana;        // 0 (lunes) a 6 (domingo)
};

// Estructura para los aeropuertos leídos del archivo de texto
struct Aeropuerto {
    string iata;
    string nombre;
    int vuelosEnviados = 0;
    int pasajerosEnviados = 0;
};

// Estructura auxiliar para el punto 5
struct DiaPasajero {
    int diaIndex;
    int pasajeros;
};

// ==========================================
// TEMPLATES Y FUNCIONES CRITERIO
// ==========================================

template <typename T, typename Criterio>
T* buscarExtremo(vector<T>& elementos, Criterio esMejor) {
    if (elementos.empty()) return nullptr;
    T* mejor = &elementos[0];
    for (size_t i = 1; i < elementos.size(); ++i) {
        if (esMejor(elementos[i], *mejor)) {
            mejor = &elementos[i];
        }
    }
    return mejor;
}
//CORRECCION 1: REEMPLAZO LOS CRITERIOS BOOLEANOS POR CRITERIOS ENTEROS
// Criterio entero para el punto 4: Menor cantidad de pasajeros enviados (> 0)
// Retorna < 0 si 'a' tiene menos pasajeros válidos que 'b', > 0 si 'b' es mejor, 0 si empatan
int criterioMenorPasajerosAeropuerto(const Aeropuerto& a, const Aeropuerto& b) {
    if (a.pasajerosEnviados == 0 && b.pasajerosEnviados == 0) return 0;
    if (a.pasajerosEnviados == 0) return 1;  // 'a' queda descartado frente a 'b'
    if (b.pasajerosEnviados == 0) return -1; // 'a' es mejor que 'b' (que tiene 0)
    return a.pasajerosEnviados - b.pasajerosEnviados;
}

// Criterio entero para el punto 5: Orden descendente por cantidad de pasajeros
int criterioMayorPasajerosDia(const DiaPasajero& a, const DiaPasajero& b) {
    return b.pasajeros - a.pasajeros;
}

// ==========================================
// 2. CARGA DE ARCHIVOS
// ==========================================

bool cargarNombres(string nombresPaises[], string nombresAerolineas[], vector<Aeropuerto>& aeropuertos) {
    ifstream archivo("nombres.txt");
    if (!archivo.is_open()) {
        cerr << "Error al abrir el archivo nombres.txt" << endl;
        return false;
    }

    string linea;

    // 1. Leer nombres de países (primeras 5 líneas)
    for (int i = 0; i < CANT_PAISES; ++i) {
        if (!getline(archivo, linea)) return false;
        nombresPaises[i] = linea;
    }

    // 2. Leer nombres de aerolíneas (siguientes 8 líneas)
    for (int i = 0; i < CANT_AEROLINEAS; ++i) {
        if (!getline(archivo, linea)) return false;
        nombresAerolineas[i] = linea;
    }

    // 3. Leer 15 líneas con código IATA, tabulador y nombre del aeropuerto
    for (int i = 0; i < CANT_AIRPORTS; ++i) {
        Aeropuerto a;
        string iataStr;
        if (!getline(archivo, iataStr, '\t')) return false;
        if (!getline(archivo, linea)) return false;
        
        a.iata = iataStr;
        a.nombre = linea;
        aeropuertos.push_back(a);
    }

    archivo.close();
    return true;
}

// Función auxiliar para buscar el índice de un aeropuerto por su código IATA
int buscarIndiceAeropuerto(const vector<Aeropuerto>& aeropuertos, const string& iata) {
    for (size_t i = 0; i < aeropuertos.size(); ++i) {
        if (aeropuertos[i].iata == iata) {
            return (int)i;
        }
    }
    return -1;
}
// CORRECCION 2: SUPLEMENTACION DE FUNCIONES PARA LA EFICACIA DEL CODIGO Y MAIN
// Funcion de lectura y acumulación del archivo binario
bool procesarArribos(int pasajerosPaisAerolinea[][CANT_AEROLINEAS],
                     int pasajerosPaisAerolineaDia[][CANT_AEROLINEAS][DIAS_SEMANA],
                     vector<Aeropuerto>& aeropuertos) {
    ifstream archBinario("arribos.bin", ios::binary);
    if (!archBinario.is_open()) {
        cerr << "Error al abrir el archivo arribos.bin" << endl;
        return false;
    }

    RegistroArribo reg;
    while (archBinario.read(reinterpret_cast<char*>(&reg), sizeof(RegistroArribo))) {
        if (reg.codigoPais >= 0 && reg.codigoPais < CANT_PAISES &&
            reg.codigoAerolinea >= 0 && reg.codigoAerolinea < CANT_AEROLINEAS) {
            
            pasajerosPaisAerolinea[reg.codigoPais][reg.codigoAerolinea] += reg.pasajeros;

            if (reg.diaSemana >= 0 && reg.diaSemana < DIAS_SEMANA) {
                pasajerosPaisAerolineaDia[reg.codigoPais][reg.codigoAerolinea][reg.diaSemana] += reg.pasajeros;
            }
        }

        int idxAeropuerto = buscarIndiceAeropuerto(aeropuertos, reg.iata);
        if (idxAeropuerto != -1) {
            aeropuertos[idxAeropuerto].vuelosEnviados++;
            aeropuertos[idxAeropuerto].pasajerosEnviados += reg.pasajeros;
        }
    }

    archBinario.close();
    return true;
}

// ==========================================
// 3, 4 y 5. FUNCIONES DE INFORMES
// ==========================================

void mostrarListadoTotales(const string nombresPaises[], const string nombresAerolineas[],
                           const int pasajerosPaisAerolinea[][CANT_AEROLINEAS]) {
    cout << "Listado de totales de pasajeros\n";
    cout << "País     Aerolínea             Cant. Pasajeros\n";

    for (int p = 0; p < CANT_PAISES; ++p) {
        for (int a = 0; a < CANT_AEROLINEAS; ++a) {
            cout << nombresPaises[p] << "\t" << nombresAerolineas[a] << "\t\t" << pasajerosPaisAerolinea[p][a] << "\n";
        }
    }
    cout << "\n";
}

void informarAeropuertoMenorPasajeros(vector<Aeropuerto>& aeropuertos) {
    Aeropuerto* minAeropuerto = buscarExtremo(aeropuertos, criterioMenorPasajerosAeropuerto);
    if (minAeropuerto != nullptr && minAeropuerto->pasajerosEnviados > 0) {
        cout << "Aeropuerto que envió menor cantidad de pasajeros: " << minAeropuerto->iata << ": " << minAeropuerto->nombre << "\n";
        cout << "Pasajeros enviados: " << minAeropuerto->pasajerosEnviados << " en " << minAeropuerto->vuelosEnviados << " vuelos.\n\n";
    } else {
        cout << "No se registran aeropuertos con pasajeros enviados.\n\n";
    }
}

void informarCombinacionMaxima(const string nombresPaises[], const string nombresAerolineas[],
                               const int pasajerosPaisAerolinea[][CANT_AEROLINEAS],
                               const int pasajerosPaisAerolineaDia[][CANT_AEROLINEAS][DIAS_SEMANA]) {
    int maxPasajeros = -1;
    int mejorPais = -1;
    int mejorAerolinea = -1;

    for (int p = 0; p < CANT_PAISES; ++p) {
        for (int a = 0; a < CANT_AEROLINEAS; ++a) {
            if (pasajerosPaisAerolinea[p][a] > maxPasajeros) {
                maxPasajeros = pasajerosPaisAerolinea[p][a];
                mejorPais = p;
                mejorAerolinea = a;
            }
        }
    }

    if (maxPasajeros > 0) {
        cout << "La mayor cantidad de pasajeros (" << maxPasajeros << ") se dio para la combinación " 
             << nombresPaises[mejorPais] << " con aerolínea " << nombresAerolineas[mejorAerolinea] 
             << " según el siguiente detalle diario:\n";
        cout << "Día       Cantidad\n";
        
        vector<DiaPasajero> distribucionDias(DIAS_SEMANA);
        string nombresDias[DIAS_SEMANA] = {"Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado", "Domingo"};
        
        for (int d = 0; d < DIAS_SEMANA; ++d) {
            distribucionDias[d].diaIndex = d;
            distribucionDias[d].pasajeros = pasajerosPaisAerolineaDia[mejorPais][mejorAerolinea][d];
        }

        for (int i = 0; i < DIAS_SEMANA; ++i) {
            cout << nombresDias[distribucionDias[i].diaIndex] << "\t\t" << distribucionDias[i].pasajeros << "\n";
        }
    } else {
        cout << "No hay registros de pasajeros.\n";
    }
}

// ==========================================
// PROGRAMA PRINCIPAL
// ==========================================

int main() {
    string nombresPaises[CANT_PAISES];
    string nombresAerolineas[CANT_AEROLINEAS];
    vector<Aeropuerto> aeropuertos;

    if (!cargarNombres(nombresPaises, nombresAerolineas, aeropuertos)) {
        cerr << "Fallo en la lectura del archivo de nombres." << endl;
        return 1;
    }

    int pasajerosPaisAerolinea[CANT_PAISES][CANT_AEROLINEAS] = { {0} };
    int pasajerosPaisAerolineaDia[CANT_PAISES][CANT_AEROLINEAS][DIAS_SEMANA] = { { {0} } };

    if (!procesarArribos(pasajerosPaisAerolinea, pasajerosPaisAerolineaDia, aeropuertos)) {
        return 1;
    }

    mostrarListadoTotales(nombresPaises, nombresAerolineas, pasajerosPaisAerolinea);
    informarAeropuertoMenorPasajeros(aeropuertos);
    informarCombinacionMaxima(nombresPaises, nombresAerolineas, pasajerosPaisAerolinea, pasajerosPaisAerolineaDia);
    return 0;
}