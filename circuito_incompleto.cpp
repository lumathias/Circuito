#include <fstream>
#include <utility> // para std::swap
#include "circuito.h"

//Autores:  Luisa de Moura Galvão Mathias
//          Marcos Paulo Barbosa

/// As strings que definem os tipos de porta ///

// Funcao auxiliar que testa se uma string com nome de porta eh valida
// Caso necessario, converte os caracteres da string para maiusculas
bool validType(std::string& Tipo)
{
    if (Tipo.size()!=2) return false;
    Tipo.at(0) = toupper(Tipo.at(0));
    Tipo.at(1) = toupper(Tipo.at(1));
    if (Tipo=="NT" ||
            Tipo=="AN" || Tipo=="NA" ||
            Tipo=="OR" || Tipo=="NO" ||
            Tipo=="XO" || Tipo=="NX") return true;
    return false;
}

// Funcao auxiliar que retorna um ponteiro que aponta para uma porta alocada dinamicamente
// O tipo da porta alocada depende do parametro string de entrada (AN, OR, etc.)
// Caso o tipo nao seja nenhum dos validos, retorna nullptr
// Pode ser utilizada nas funcoes: Circuito::setPort, Circuito::digitar e Circuito::ler
ptr_Port allocPort(std::string& Tipo)
{
    if (!validType(Tipo)) return nullptr;

    if (Tipo=="NT") return new Port_NOT;
    if (Tipo=="AN") return new Port_AND;
    if (Tipo=="NA") return new Port_NAND;
    if (Tipo=="OR") return new Port_OR;
    if (Tipo=="NO") return new Port_NOR;
    if (Tipo=="XO") return new Port_XOR;
    if (Tipo=="NX") return new Port_NXOR;

    // Nunca deve chegar aqui...
    return nullptr;
}

///
/// CLASSE CIRCUITO
///

/// ***********************
/// Inicializacao e finalizacao
/// ***********************
Circuito::Circuito(): Nin(0), id_out(), out_circ(), ports(){} //construtor default

Circuito::Circuito(const Circuito& C):Nin(C.Nin){ //construtor por copia
    id_out.resize(C.id_out.size());
    out_circ.resize(C.out_circ.size());

    for(unsigned i=0; i<C.id_out.size(); i++){
        id_out.at(i) = C.id_out.at(i);
        out_circ.at(i) = C.out_circ.at(i);
    }

    for(unsigned i=0; i<C.ports.size(); i++){
        ports.push_back(ports.at(i)->clone());
    }
}

Circuito::Circuito(Circuito&& C):Nin(C.Nin){ //construtor por movimento
    this->clear();
    id_out.resize(C.id_out.size());
    out_circ.resize(C.out_circ.size());

    for(unsigned i=0; i<C.id_out.size(); i++){
        id_out.at(i) = C.id_out.at(i);
        out_circ.at(i) = C.out_circ.at(i);
    }

    for(unsigned i=0; i<C.ports.size(); i++){
        ports.push_back(ports.at(i)->clone());
    }

    C.clear();
}
Circuito::~Circuito(){ clear(); } //destrutor

// Limpa todo o conteudo do circuito. Faz Nin <- 0 e
// utiliza o metodo STL clear para limpar os vetores id_out, out_circ e ports
// ATENCAO: antes de dar um clear no vetor ports, tem que liberar (delete) as areas
// de memoria para as quais cada ponteiro desse vetor aponta.
void Circuito::clear(){
    Nin = 0;
    id_out.clear();
    out_circ.clear();

    for(unsigned i = 0; i < ports.size(); i++) delete ports.at(i);
    ports.clear();
}

// Operador de atribuicao por copia
// Atribui (faz copia) de Nin e dos vetores id_out e out_circ
// ATENCAO: antes de alterar o vetor ports, tem que liberar (delete) as areas
// de memoria para as quais cada ponteiro desse vetor aponta.
// O vetor ports terah a mesma dimensao do equivalente no Circuit C
// Serah necessario utilizar a funcao virtual clone para criar copias das portas
void Circuito::operator=(const Circuito& C){
    clear();
    Nin = C.Nin;
    id_out.resize(C.id_out.size());
    out_circ.resize(C.out_circ.size());

    for(unsigned i=0; i<C.id_out.size(); i++){
        id_out.at(i) = C.id_out.at(i);
        out_circ.at(i) = C.out_circ.at(i);
    }

    for(unsigned i=0; i<C.ports.size(); i++){
        ports.push_back(ports.at(i)->clone());
    }
}

// Operador de atribuicao por movimento
// Move Nin e os vetores id_out, out_circ e ports
// ATENCAO: antes de mover o vetor ports, tem que liberar (delete) as areas
// de memoria anteriores para as quais cada ponteiro desse vetor aponta.
void Circuito::operator=(Circuito&& C){
    Nin = C.Nin;
    id_out.resize(C.id_out.size());
    out_circ.resize(C.out_circ.size());

    for(unsigned i=0; i<C.id_out.size(); i++){
        id_out.at(i) = C.id_out.at(i);
        out_circ.at(i) = C.out_circ.at(i);
    }

    for(unsigned i=0; i<C.ports.size(); i++){
        ports.push_back(ports.at(i)->clone());
    }

    C.clear();
}

// Redimensiona o circuito para passar a ter NI entradas, NO saidas e NP ports
// Inicialmente checa os parametros. Caso sejam validos,
// depois de limpar conteudo anterior (clear), altera Nin; os vetores tem as dimensoes
// alteradas (resize) e sao inicializados com valores iniciais neutros ou invalidos:
// id_out[i] <- 0
// out_circ[i] <- UNDEF
// ports[i] <- nullptr
void Circuito::resize(int NI, int NO, int NP){
    if(NI>0 && NO>0 && NP>0){
        clear();
        Nin = NI;

        id_out.resize(NO, 0);
        out_circ.resize(NO, bool3S::UNDEF);
        ports.resize(NP, nullptr);
    }
}

/// ***********************
/// Funcoes de testagem
/// ***********************

// Retorna true se IdInput eh uma id de entrada do circuito valida (entre -1 e -NInput)
bool Circuito::validIdInput(int IdInput) const
{
    return (IdInput<=-1 && IdInput>=-getNumInputs());
}

// Retorna true se IdOutput eh uma id de saida do circuito valida (entre 1 e NOutput)
bool Circuito::validIdOutput(int IdOutput) const
{
    return (IdOutput>=1 && IdOutput<=getNumOutputs());
}

// Retorna true se IdPort eh uma id de porta do circuito valida (entre 1 e NPort)
bool Circuito::validIdPort(int IdPort) const
{
    return (IdPort>=1 && IdPort<=getNumPorts());
}

// Retorna true se IdOrig eh uma id valida para a origem do sinal de uma entrada de porta ou
// para a origem de uma saida do circuito (podem vir de uma entrada do circuito ou de uma porta)
// validIdOrig == validIdInput OR validIdPort
bool Circuito::validIdOrig(int IdOrig) const
{
    return validIdInput(IdOrig) || validIdPort(IdOrig);
}

// Retorna true se IdPort eh uma id de porta valida (validIdPort) e
// a porta estah definida (estah alocada, ou seja, != nullptr)
bool Circuito::definedPort(int IdPort) const
{
    if (!validIdPort(IdPort)) return false;
    if (ports.at(IdPort-1)==nullptr) return false;
    return true;
}

// Retorna true se IdPort eh uma porta existente (definedPort) e
// todas as entradas da porta com Id de origem valida (usa getId_inPort e validIdOrig)
bool Circuito::validPort(int IdPort) const
{
    if (!definedPort(IdPort)) return false;
    for (int j=0; j<getNumInputsPort(IdPort); j++)
    {
        if (!validIdOrig(getId_inPort(IdPort,j))) return false;
    }
    return true;
}

// Retorna true se o circuito eh valido (estah com todos os dados corretos):
// - numero de entradas, saidas e portas valido (> 0)
// - todas as portas validas (usa validPort)
// - todas as saidas com Id de origem validas (usa getIdOutput e validIdOrig)
// Essa funcao deve ser usada antes de salvar ou simular um circuito
bool Circuito::valid() const
{
    if (getNumInputs()<=0) return false;
    if (getNumOutputs()<=0) return false;
    if (getNumPorts()<=0) return false;
    for (int i=0; i<getNumPorts(); i++)
    {
        if (!validPort(i+1)) return false;
    }
    for (int i=0; i<getNumOutputs(); i++)
    {
        if (!validIdOrig(getIdOutput(i+1))) return false;
    }
    return true;
}

/// ***********************
/// Funcoes de consulta
/// ***********************

// Caracteristicas do circuito

// Retorna o numero de entradas Nin
int Circuito::getNumInputs() const{
    return Nin;
}
// Retorna o tamanho (size) dos vetores correspondentes:
// id_out e ports, respectivamente
int Circuito::getNumOutputs() const{
    return id_out.size();
}
int Circuito::getNumPorts() const{
    return ports.size();
}

// Caracteristicas das saidas do circuito

// Retorna a origem (a id) do sinal de saida cuja id eh IdOutput
// Depois de testar o parametro (validIdOutput), retorna id_out[IdOutput-1]
// ou 0 se parametro invalido
int Circuito::getIdOutput(int IdOutput) const{
    if(validIdOutput(IdOutput)) return id_out[IdOutput-1];
    else return 0;
}

// Retorna o valor logico atual da saida cuja id eh IdOutput
// Depois de testar o parametro (validIdOutput), retorna out_circ[IdOutput-1]
// ou bool3S::UNDEF se parametro invalido
bool3S Circuito::getOutput(int IdOutput) const{
    if(validIdOutput(IdOutput)) return out_circ[IdOutput-1];
    else return bool3S::UNDEF;
}

// Retorna o nome da porta: AN, NX, etc
// Depois de testar se a porta existe (definedPort),
// retorna ports.at(IdPort-1)->getName()
// ou "??" se parametro invalido
std::string Circuito::getNamePort(int IdPort) const{
    if(definedPort(IdPort)) return ports.at(IdPort-1)->getName();
    else return "??";
}

// Retorna o numero de entradas da porta
// Depois de testar se a porta existe (definedPort),
// retorna ports[IdPort-1]->getNumInputs()
// ou 0 se parametro invalido
int Circuito::getNumInputsPort(int IdPort) const{
    if(definedPort(IdPort)) return ports.at(IdPort-1)->getNumInputs();
    else return 0;
}

// Retorna a origem (a id) da I-esima entrada da porta cuja id eh IdPort
// Depois de testar se a porta existe (definedPort) e o indice da entrada I,
// retorna ports[IdPort-1]->getId_in(I)
// ou 0 se parametro invalido
int Circuito::getId_inPort(int IdPort, int I) const{
    if(definedPort(IdPort) && ports.at(IdPort-1)->validIndex(I)) return ports.at(IdPort-1)->getId_in(I);
    else return 0;
}

/// ***********************
/// Funcoes de modificacao
/// ***********************

// Caracteristicas das saidas

// Altera a origem da saida de id "IdOut", que passa a ser "IdOrig"
// Depois de testar os parametros (validIdOutput,validIdOrig),
// faz: id_out[IdOut-1] <- IdOrig
void Circuito::setIdOutput(int IdOut, int IdOrig){
    IdOut = IdOrig;
    if(validIdOutput(IdOut) && validIdOrig(IdOrig)){
        id_out[IdOut-1] = IdOrig;
    }
}

// Caracteristicas das ports

// A porta cuja id eh IdPort passa a ser do tipo Tipo (NT, AN, etc.), com NIn entradas
// Depois de varios testes (Id, tipo, num de entradas), faz:
// 1) Libera a antiga area de memoria: delete ports[IdPort-1]
// 2) Cria a nova porta: ports[IdPort-1] <- new ... (de acordo com tipo)
// 3) Fixa o numero de entrada: ports[IdPort-1]->setNumInputs(NIn)
void Circuito::setPort(int IdPort, std::string Tipo, int NIn){
    if(validIdPort(IdPort) && validType(Tipo)){ //&& ports[IdPort-1]->validNumInputs(NIn) testar isso tbm?
        delete ports.at(IdPort-1);

        ports.at(IdPort-1) = allocPort(Tipo);
        ports.at(IdPort-1)->setNumInputs(NIn);
    }
}

// Altera a origem da I-esima entrada da porta cuja id eh IdPort, que passa a ser "IdOrig"
// Depois de VARIOS testes (definedPort, validIndex, validIdOrig)
// faz: ports[IdPort-1]->setId_in(I,Idorig)
void Circuito::setId_inPort(int IdPort, int I, int IdOrig) const{
    if(definedPort(IdPort) && ports.at(IdPort-1)->validIndex(I) && validIdOrig(IdOrig)){
        ports.at(IdPort-1)->setId_in(I, IdOrig);
    }
}

/// ***********************
/// E/S de dados
/// ***********************

// Entrada dos dados de um circuito via teclado
// O usuario digita o numero de entradas, saidas e portas
// apos o que, se os valores estiverem corretos (>0), redimensiona o circuito
// Em seguida, para cada porta o usuario digita o tipo (NT,AN,NA,OR,NO,XO,NX) que eh conferido
// Apos criada dinamicamente (new) a porta do tipo correto, chama a
// funcao digitar na porta recem-criada. A porta digitada eh conferida (validPort).
// Em seguida, o usuario digita as ids de todas as saidas, que sao conferidas (validIdOrig).
// Se o usuario digitar um dado invalido, o metodo deve pedir que ele digite novamente
// Deve utilizar o metodo digitar da classe Port
void Circuito::digitar(){
    unsigned entrada(0), saida(0), portas(0);
    std::string tipo;

    std::cout << "Digite o numero de entradas: ";
    while(entrada <= 0){
        std::cin >> entrada;
    }
    std::cout << "Digite o numero de saidas: ";
    while(saida <= 0){
        std::cin >> saida;
    }
    std::cout << "Digite o numero de portas: ";
    while(portas <= 0){
        std::cin >> portas;
    }

    resize(entrada, saida, portas);

    for(unsigned i=0; i<portas; i++){


        std::cout << "Digite o tipo da porta " << i+1 << std::endl;
        std::cout << "Opcoes: NT, AN, NA, OR, NO, XO, NX" << std::endl;
        std::cin >> tipo;
        while(!validType(tipo)){
            std::cout << "Tipo invalido!" << std::endl;
            std::cout << "Digite o tipo da porta " << i+1<< ":";
            std::cin >> tipo;
        }
        if(ports.at(i) != nullptr) delete ports.at(i);  ///lembrar de fazer  nos outros
        ports.at(i) = allocPort(tipo);
        do{
            ports.at(i)->digitar();
        }while(!validPort(i+1));


    }
    for(unsigned i = 0; i <saida; i++){
        int idOut;
        std::cout << "Digite o Id da saida " << i+1 << ":";
        std::cin >> idOut;
        while(!validIdOrig(idOut)){
            std::cout << "Id Invalido!" << std::endl;
            std::cout << "Digite o Id da saida " << i+1 << ":";
            std::cin >> idOut;
        }
        id_out.at(i) = (idOut);
    }
}

// Entrada dos dados de um circuito via arquivo
// Leh do arquivo o cabecalho com o numero de entradas, saidas e portas
// Em seguida, para cada porta leh e confere a id e o tipo (validType)
// Apos criada dinamicamente (allocPort) a porta do tipo correto, chama a
// funcao ler na porta recem-criada. O retorno da leitura da porta eh conferido
// bem como se a porta lida eh valida (validPort) para o circuito.
// Em seguida, leh as ids de todas as saidas, que sao conferidas (validIdOrig)
// Retorna true se deu tudo OK; false se deu erro.
// Deve utilizar o metodo ler da classe Port
bool Circuito::ler(const std::string& arq){
    std::ifstream arqv(arq.c_str());

    try{
        if (!arqv.is_open()) throw 1;

        std::string prov, prov1;
        unsigned in, out, portas;
        unsigned idprov;
        char test;

        arqv >> prov >> in >> out >> portas >> prov1;

        if (!arqv.good() || prov != "CIRCUITO" || in<=0 || out<=0 || portas<=0 || prov1 != "PORTAS") throw 2;
        resize(in,out,portas);

        for(unsigned i=0 ; i<portas; i++){

            std::string nomePorta;
            arqv >> idprov >> test;

            if(idprov != i+1 || test != ')') throw 3;

            arqv >> nomePorta;

            if(!validType(nomePorta)) throw 4;
            ptr_Port portaProv  = allocPort(nomePorta);
            if(!portaProv->ler(arqv)) throw 5;
            ports.at(i) = portaProv;
        }

        arqv >> prov;
        if(prov != "SAIDAS") throw 6;
        for(unsigned i=0 ; i<out; i++){
            int saida;
            arqv >> idprov >> test;
            if(idprov != i+1 || test != ')') throw 7;
            arqv >> saida;
            id_out.at(i) = saida;
        }

        bool circ_valid = valid();
        if(!circ_valid) throw 8;

    }
    catch (int i)
    {
        clear();
        return false;
    }

    return true;
}

// Saida dos dados de um circuito (em tela ou arquivo, a mesma funcao serve para os dois)
// Imprime os cabecalhos e os dados do circuito, caso o circuito seja valido
// Deve utilizar os metodos de impressao da classe Port
std::ostream& Circuito::imprimir(std::ostream& O) const{

    if(!valid()){
        std::cout << "ERROR!! Circuito invalido!!" << std::endl;
        exit(0);
    }

    O << "CIRCUITO " << getNumInputs() << " " << id_out.size() << " " <<  ports.size() << std::endl;
    O << "PORTAS" << std::endl;

    for(unsigned i=0; i<ports.size(); i++){
        O << i+1 <<") " << *ports.at(i) ;
        O << std::endl;
    }

    O << "SAIDAS" << std::endl;
    for(unsigned i=0; i<id_out.size(); i++){
        O << i+1 <<") " << id_out.at(i) << std::endl;
    }

    return O;
}

// Salvar circuito em arquivo, caso o circuito seja valido
// Abre a stream, chama o metodo imprimir e depois fecha a stream
// Retorna true se deu tudo OK; false se deu erro
bool Circuito::salvar(const std::string& arq) const{

    if(!this->valid()) return false;

    std::ofstream arquivo(arq.c_str());

    if (!arquivo.is_open()) return false;

    arquivo << *this;
    arquivo.close();

    return true;
}

/// ***********************
/// SIMULACAO (funcao principal do circuito)
/// ***********************

bool Circuito::simular(const std::vector<bool3S>& in_circ) {
    /*
        // TIPO DE DADO Port
        Port:
        vector<int> id_in
        // ids das entradas da porta:
        // id == índice+1 (se porta)
        // == -índice-1 (se entrada)
        bool3S out_port // Saída da porta
        // DADOS GLOBAIS
        vector<Port> ports
        // portas do circuito
        vector<int> id_out
        // ids das saídas do circuito
        // id == índice+1 (se porta)
        // == -índice-1 (se entrada)
        // ENTRADA:
        vector<bool3S> in_circ
        // Entradas do circuito
        // SAÍDA
        vector<bool3S> out_circ
        // Saídas do circuito

        // VARIÁVEIS LOCAIS:
        bool tudo_def, alguma_def
        vector<bool3S> in_port
        // Entradas de uma porta
        // SIMULAÇÃO DAS PORTAS
        Para i de 0 a Num_ports-1
        | ports[i].out_port ← UNDEF
        Fim Para
    */
    bool tudo_def, alguma_def;
    int id;
    std::vector<bool3S> in_port;

    for(int i=0; i<getNumPorts(); i++){
        ports.at(i)->setOutput(bool3S::UNDEF);
    }

    do{
        tudo_def = true;
        alguma_def = false;

        for(int i = 0; i<getNumPorts(); i++){
            if(ports.at(i)->getOutput() == bool3S::UNDEF){
                for(int j = 0; j < ports.at(i)->getNumInputs(); j++){
                    id = ports.at(i)->getId_in(j);
                    if(id>0) in_port.push_back(ports.at(id-1)->getOutput());
                    else in_port.push_back(in_circ.at(-id-1));
                }
                ports.at(i)->simular(in_port);
                if(ports.at(i)->getOutput() == bool3S::UNDEF) tudo_def = false;
                else alguma_def = true;
            }
        }

    }while(!tudo_def && alguma_def);

    for(unsigned j = 0; j<unsigned(getNumOutputs()); j++){
        id = id_out.at(j);
        if(id > 0) out_circ.at(j) = ports.at(id-1)->getOutput();
        else out_circ.at(j) = in_circ.at(-id-1);
    }
    return tudo_def;
};

std::ostream& operator<<(std::ostream& O, const Circuito& C){
    if(!C.valid()){
        std::cout << "ERROR!! Circuito inválido!!";
    }
    C.imprimir(O);

    return std::cout;
}
