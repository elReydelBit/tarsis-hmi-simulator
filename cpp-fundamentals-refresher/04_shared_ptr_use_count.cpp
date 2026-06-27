#include <memory>
#include <iostream>

/* 

EJERCICIO RAII 3:
-----------------------------------------------------------------------------------------------------------------------
1.- Crea un std::shared_ptr<int> con make_shared, e imprime use_count() justo después.
2.- Dentro de un bloque { } anidado, cópialo a una segunda variable, e imprime use_count() otra vez (debería subir a 2).
3.- Después de cerrar ese bloque, imprime use_count() una tercera vez (debería volver a 1). 

*/



int main(){

    //Creamos un smart pointer de tipo shared_ptr  y vemos su funcionalidad con use_count()
    std::shared_ptr <int> puntero_compartido = std::make_shared <int> (42);
    
    //imprimimos use_count() justo después de crearlo
    std::cout << "Contador de 'use_count()' despues de crearlo: " << puntero_compartido.use_count() << std::endl;
    

    //Creamos un scope para comprobar como va su funcionamiento 
    {
        std:: shared_ptr <int> puntero_aux=puntero_compartido;

        //imprimimos use_count() justo después de compartirlo
        std::cout << "Contador de 'use_count()' despues de compartirlo: " << puntero_compartido.use_count() << std::endl;
    }

    //imprimimos use_count() justo después de salir del scope donde se compartió el puntero
    std::cout << "Contador de 'use_count()' despues de salir del scope: " << puntero_compartido.use_count() << std::endl;

    return 0;
}