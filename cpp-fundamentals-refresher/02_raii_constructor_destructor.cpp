#include <memory>
#include <iostream>

//Creo una clase para ver como se manifiesta en el main, se crea y destruye
class Centinela {

public:
    Centinela(){

        std::cout << "Centinela construido O" << std::endl;

    }

    ~Centinela(){

        std::cout << "Centinela destruido X" << std:: endl;

    }

};

int main() {

    //Declaramos una clase dentro de un bucle para ver como funciona
    std::cout << "Antes del bucle" << std::endl;
    for(int i=1;i<3;i++){
        
        std::cout << "Iteración: " << i << std::endl;
        Centinela centinela;
    }
    std::cout << "Después del bucle" << std::endl;
    
    
    return 0;

   
}