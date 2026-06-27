#include <memory>
#include <iostream>

int main() {

    //Declaramos un smart pointer de tipo unique_ptr que nos asegura la construccion y destruccion de un objeto de tipo int
    std::unique_ptr <int> puntero_inteligente = std::make_unique <int>(42);

    //Mostramos su valor
    std::cout << "Valor del puntero inteligente: " << *puntero_inteligente << "\n";
    std::cout << "Valor del puntero inteligente (esta versiion usa std::endl): " << *puntero_inteligente << std::endl;
    return 0;

    std::cout << "Última línea del programa. " << std::endl;
}