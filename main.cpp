#include "maincircuito.h"
#include <QApplication>

//Autores:  Luisa de Moura Galvão Mathias
//          Marcos Paulo Barbosa

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainCircuito w;

    w.show();

    return a.exec();
}
