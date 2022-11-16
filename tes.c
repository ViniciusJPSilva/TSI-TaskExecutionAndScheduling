#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Requerido por fork, getpid e getppid.
#include <sys/wait.h> // Requerido por wait.
#include "tes.h"

int main(){
    return tes();
}

int tes(){
    char *linhaDeComando;

    Tarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS];
    unsigned short quantidadeProgramas;

    setlocale(LC_ALL, "pt-BR");

    printf("%s", MSG_BOASVINDAS);

    while(TRUE){
        exibirPrompt(PROMPT);
        linhaDeComando = lerComando();

        if(!linhaDeComando) continue; // Verifica se a linha de comando está vazia, igual à NULL
        if(isExit(linhaDeComando)) break; // Verifica se o comando digitado é EXIT

        quantidadeProgramas = obterNomesProgramas(linhaDeComando, tarefas);
        quantidadeProgramas = obterInstrucoesProgramas(tarefas, quantidadeProgramas);

        executarProgramas(tarefas, quantidadeProgramas);

        free(linhaDeComando);
    }

    free(linhaDeComando);

    return EXIT_SUCCESS;
} // tes()

void exibirPrompt(const char *prompt){
    printf("%s", prompt);
}

char * lerComando(){
    char *comando = (char *) calloc(1, TAMANHO_LINHA_COMANDO);
    
    if(comando){ // Verifica se foi possível alocar memória.
        fgets(comando, TAMANHO_LINHA_COMANDO, stdin);
        retiraEnter(comando);
        if(stringVazia(comando)) comando = desalocarString(comando);// Libera a memória alocada, caso a string lida for vazia.
    }

    return comando;
}

unsigned short obterNomesProgramas(const char *linhaDeComando, Tarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS]){
    unsigned short cont;
    char copiaLinhaComando[TAMANHO_LINHA_COMANDO], *tok;

    strcpy(copiaLinhaComando, linhaDeComando);
    strcpy(tarefas[0].programa.nome, strtok(copiaLinhaComando, DELIMITADOR));

    for(cont = 1, tok = strtok(NULL, DELIMITADOR); cont < NUMERO_MAXIMO_DE_PROGRAMAS; cont++, tok = strtok(NULL, DELIMITADOR)){
        if(!tok) break;
        strcpy(tarefas[cont].programa.nome, tok);
    }

    return cont;
}

unsigned short obterInstrucoesProgramas(Tarefa tarefas[], unsigned short quantidadeProgramas){
    FILE *arquivo;
    char nomeArquivo[TAMANHO_NOME_ARQUIVO];
    for(int i = 0; i < quantidadeProgramas; i++){
        strcpy(nomeArquivo, tarefas[i].programa.nome);

        arquivo = fopen(strcat(nomeArquivo, EXTENSAO_LPAS), APENAS_LEITURA);
        if(!arquivo){
            printf(ERRO_PROGRAMA_NAO_EXISTE, nomeArquivo);
            tarefas[i--] = tarefas[--quantidadeProgramas]; // Substitui o arquivo não encontrado pelo último do vetor.
        }else{
            tarefas[i].programa.numeroDeInstrucoes = lerInstrucoesDoArquivo(&tarefas[i].programa, arquivo);
            fclose(arquivo);
        }
    }

    return quantidadeProgramas;
}

void executarProgramas(Tarefa tarefas[], unsigned short quantidadeProgramas){
    unsigned short controle = 0, contadorProgramas = 0, quantidade, tes = 1;
    Tarefa tarefasParaExecutar[NUMERO_MAXIMO_DE_PROGRAMAS];
    int status;

    while(controle < quantidadeProgramas){
        for(quantidade = 0; contadorProgramas < quantidadeProgramas && quantidade < MAX_PROGRAMAS_PROCESSO; contadorProgramas++, quantidade++)
            tarefasParaExecutar[quantidade] = tarefas[contadorProgramas];

        if(criarProcesso() != 0)
            wait(&status);
        else
            executarEscalonarTarefas(tarefasParaExecutar, quantidade, tes);

        tes++;
        controle += MAX_PROGRAMAS_PROCESSO;
    }
}

void executarEscalonarTarefas(Tarefa tarefas[MAX_PROGRAMAS_PROCESSO], unsigned short quantidadeProgramas, unsigned short tes){
    MaquinaExecucao maquina;
    RoundRobin roudRobin;
    unsigned short indexFila = 0;
    int index, resultado;

    inicializarMaquinaExecucao(tarefas, quantidadeProgramas, &maquina);
    inicializarContextos(quantidadeProgramas, &maquina);
    roudRobin.contadorCPU = 0;
    roudRobin.contadorPreempcao = 1;

    // Loop que representa o ciclo do processador: 1 volta no loop equivale à 1 UT.
    while(TRUE){
        // Carregando novas tarefas para o estado PRONTA.
        index = verificarEstadoTarefas(maquina.df, maquina.numeroDeProgramas, NOVA);
        if(index != NAO_ENCONTRADO) {
            maquina.df[index].estado = PRONTA; // Altera o estado da tarefa.
            roudRobin.tempoEntradaESaidaFila[index][0] = roudRobin.contadorCPU; // Salva a UT do momento que a tarefa entrou na fila da CPU.
        }

        // Verifica se há tarefas aptas à serem escalonadas (estado diferente de TERMINADA);
        if(!haTarefasAptas(maquina.df, maquina.numeroDeProgramas))
            break;

        // Escalona a próxima tarefa da fila, caso a tarefa executada anteriormente tenha terminado ou sofrido preempção.
        if(maquina.df[indexFila].estado != EXECUTANDO)
            escalonarProximaTarefaDaFila(&maquina, &indexFila);

        // Interpreta e executa a instrução.
        resultado = interpretador(indexFila, &maquina);

        // Verifica se ocorreu algum erro.
        if(resultado == RETURN_ERRO)
            imprimirDadosErro(maquina.erroExecucao);
        
        // Incrementa o tempo de CPU da tarefa
        maquina.df[indexFila].tempoCPU++;

        // Termina a tarefa, caso a instrução HALT seja executada ou ocorra algum erro.
        if(resultado == RETURN_FIM || resultado == RETURN_ERRO)
            terminarTarefa(&maquina, &roudRobin, indexFila);
        else {
            // Verifica se a preempção por tempo deve ocorrer e se há outra tarefa pronta (caso não tenha, a preempção não ocorre, pois existe somente uma tarefa sendo executada)
            if(roudRobin.contadorPreempcao >= QUANTUM && verificarEstadoTarefas(maquina.df, maquina.numeroDeProgramas, PRONTA) != NAO_ENCONTRADO) // 
                gerarPreempcaoPorTempo(&maquina, &roudRobin, &indexFila);
            else
                roudRobin.contadorPreempcao++;
        }

        // Incrementa a contagem de ut.
        roudRobin.contadorCPU++;
    }

    // Imprime o relatório
    exibirRelatorioProcesso(maquina, roudRobin, quantidadeProgramas, tes);

    exit(EXIT_SUCCESS);
} // executarEscalonarTarefas()

int interpretador(unsigned short indiceTarefa, MaquinaExecucao *maquina) {
    Instrucao instrucao, dado;
    char* argumento;
    int tipoInstrucao;
    unsigned char *pc = &maquina->df[indiceTarefa].pc; // Salva o valor do PC.
    
    strcpy(instrucao, maquina->df[indiceTarefa].tarefa.programa.instrucoes[*pc]); // Obtém somente o comando LPAS, ignorando possíveis argumentos.
    tipoInstrucao = identificarInstrucao(instrucao); // Identifica o tipo de instrução.

    if(*pc == maquina->df[indiceTarefa].tarefa.programa.numeroDeInstrucoes && tipoInstrucao != HALT) // Verifica se a última instrução é HALT.
        tipoInstrucao = RETURN_ERRO;
    
    argumento = obterArgumento(instrucao); // Obtém o argumento da instrução.

    if(argumento)
        strcpy(dado, argumento);

    (*pc)++; // Incrementa o PC.

    return executarInstrucao(tipoInstrucao, dado, indiceTarefa, maquina);
}

int executarInstrucao(int tipoInstrucao, Instrucao dado, unsigned short indiceTarefa, MaquinaExecucao *maquina) {
    unsigned char pc = maquina->df[indiceTarefa].pc;
    int erro;

    // Chega o tipo da instrução e chama a função correspondente.
    switch (tipoInstrucao) {
    case READ:
        erro = lpasRead(dado, indiceTarefa, maquina);
        break;
    case WRITE:
        erro = lpasWrite(dado, indiceTarefa, maquina);
        break;
    case LOAD:
        erro = lpasLoad(dado, maquina);
        break;
    case ADD:
        erro = lpasAdd(dado, maquina);
        break;
    case SUB:
        erro = lpasSub(dado, maquina);
        break;
    case MUL:
        erro = lpasMul(dado, maquina);
        break;
    case DIV:
        erro = lpasDiv(dado, maquina);
        break;
    case STORE:
        erro = lpasStore(dado, maquina);
        break;
    case HALT:
        erro = EXECUCAO_BEM_SUCEDIDA;
        break;
    default:
        erro = INSTRUCAO_LPAS_INVALIDA;
        break;
    }

    maquina->erroExecucao = geradorDeErrosLpas(pc, maquina->df[indiceTarefa].tarefa.programa.instrucoes[pc - 1], maquina->df[indiceTarefa].tarefa.programa.nome, erro);

    if(tipoInstrucao == HALT)
        return RETURN_FIM;

    if(erro != EXECUCAO_BEM_SUCEDIDA)
        return RETURN_ERRO;
    else
        return RETURN_OK;
}

int lpasRead(Instrucao dado, unsigned short indiceTarefa, MaquinaExecucao *maquina ){
    int qtdeLida;

    if(stringVazia(dado))
        return ARGUMENTO_INSTRUCAO_LPAS_AUSENTE;

    if(atoi(dado))
        return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;

    printf("%s -> %s: ", maquina->df[indiceTarefa].tarefa.programa.nome, INST_READ);
    qtdeLida = scanf("%d", &maquina->variaveis[maquina->quantidadeVariaveis++]);

    if(qtdeLida != 1)
        return SIMBOLO_INVALIDO;

    strcpy(maquina->nomesVariaveis[maquina->quantidadeVariaveis - 1], dado);
    maquina->df[indiceTarefa].tempoES += READ_TIME;

    return EXECUCAO_BEM_SUCEDIDA;
}

int lpasWrite(Instrucao dado, unsigned short indiceTarefa, MaquinaExecucao *maquina){
    int valor, erro;
    erro = obterValorNumericoDoArgumento(dado, maquina, &valor);

    if(erro != EXECUCAO_BEM_SUCEDIDA)
        return erro;

    printf("%s -> %s: %d\n", maquina->df[indiceTarefa].tarefa.programa.nome, INST_WRITE, valor);
    return EXECUCAO_BEM_SUCEDIDA;
}

int lpasLoad(Instrucao dado, MaquinaExecucao *maquina){
    int valor, erro;

    if(stringVazia(dado))
        return ARGUMENTO_INSTRUCAO_LPAS_AUSENTE;

    erro = obterValorNumericoDoArgumento(dado, maquina, &valor);

    if(erro != EXECUCAO_BEM_SUCEDIDA)
        return erro;

    maquina->registrador = valor;

    return EXECUCAO_BEM_SUCEDIDA;
}

int lpasAdd(Instrucao dado, MaquinaExecucao *maquina){
    int valor, erro;
    erro = obterValorNumericoDoArgumento(dado, maquina, &valor);

    if(erro == EXECUCAO_BEM_SUCEDIDA)
        maquina->registrador += valor;

    return erro;
}

int lpasSub(Instrucao dado, MaquinaExecucao *maquina){
    int valor, erro;
    erro = obterValorNumericoDoArgumento(dado, maquina, &valor);

    if(erro == EXECUCAO_BEM_SUCEDIDA)
        maquina->registrador -= valor;

    return erro;
}

int lpasMul(Instrucao dado, MaquinaExecucao *maquina){
    int valor, erro;
    erro = obterValorNumericoDoArgumento(dado, maquina, &valor);

    if(erro == EXECUCAO_BEM_SUCEDIDA)
        maquina->registrador *= valor;

    return erro;
}

int lpasDiv(Instrucao dado, MaquinaExecucao *maquina){
    int valor, erro;
    erro = obterValorNumericoDoArgumento(dado, maquina, &valor);

    if(erro == EXECUCAO_BEM_SUCEDIDA){
        if(valor != 0) // Verifica se houve divisão por 0.
            maquina->registrador /= valor;
        else
            erro = ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
    }

    return erro;
}

int lpasStore(Instrucao dado, MaquinaExecucao *maquina){
    int index;

    // Verifica se a variável já existe e armazena o valor nela.
    for(index = 0; index < maquina->quantidadeVariaveis; index++)
        if(!strcmp(dado, maquina->nomesVariaveis[index])){
            maquina->variaveis[index] = maquina->registrador;
            return EXECUCAO_BEM_SUCEDIDA;
        }
    
    // Caso não exista, cria a variável e associa o nome e o valor.
    if(index < NUMERO_MAXIMO_DE_VARIAVEIS){
        strcpy(maquina->nomesVariaveis[maquina->quantidadeVariaveis++], dado);
        maquina->variaveis[index] = maquina->registrador;
    } else
        return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;

    return EXECUCAO_BEM_SUCEDIDA;
}

int buscarEnderecoVariavelUsandoNome(Instrucao dado, MaquinaExecucao *maquina){
    for(int index = 0; index < maquina->quantidadeVariaveis; index++)
        if(!strcmp(dado, maquina->nomesVariaveis[index]))
            return index;

    return NAO_ENCONTRADO;
}

int obterValorNumericoDoArgumento(Instrucao dado, MaquinaExecucao *maquina, int *valor){
    int endereco;

    *valor = atoi(dado);
    if(!(*valor)){
        endereco = buscarEnderecoVariavelUsandoNome(dado, maquina);
        if(endereco != NAO_ENCONTRADO)
            *valor = maquina->variaveis[endereco];
        else
            return ARGUMENTO_INSTRUCAO_LPAS_INVALIDO;
    }
    
    return EXECUCAO_BEM_SUCEDIDA;
}

char * obterArgumento(){
    return strtok(NULL, DELIMITADOR);
}

int identificarInstrucao(Instrucao instrucao){
    char *comando;
    Instrucao comandosLPAS[] = {INST_READ, INST_WRITE, INST_LOAD, INST_STORE, INST_ADD, INST_SUB, INST_MUL, INST_DIV, INST_HALT};
    int codigoInstrucao[] = {READ, WRITE, LOAD, STORE, ADD, SUB, MUL, DIV, HALT};

    comando = strtok(instrucao, DELIMITADOR);
    for(int i = 0; i < QUANTIDADE_INST_LPAS; i++)
        if(!strcmp(comando, comandosLPAS[i]))
            return codigoInstrucao[i];

    return NAO_ENCONTRADO;
}

ErroExecucao geradorDeErrosLpas(unsigned short numeroLinha, Instrucao instrucao, NomePrograma nome,	Erro erro){
    ErroExecucao erroExecuxao;

    erroExecuxao.numeroLinha = numeroLinha;
    erroExecuxao.erro = erro;
    strcpy(erroExecuxao.instrucao, instrucao);
    strcpy(erroExecuxao.nome, nome);

    return erroExecuxao;
}

void inicializarMaquinaExecucao(Tarefa tarefas[], unsigned short quantidadeProgramas, MaquinaExecucao *maquina) {
    for(int index = 0; index < quantidadeProgramas; index++){
        tarefas[index].identificador = index + 1;           // Identificador da tarefa.
        maquina->numeroDeProgramas = quantidadeProgramas;   // Quantidade de programas.
        maquina->df[index].estado = NOVA;                   // Estado da tarefa.
        maquina->df[index].pc = 0;                          // Contador de Programa.
        maquina->df[index].tarefa = tarefas[index];         // Tarefa.
        maquina->df[index].tempoCPU = 0;                    // Tempo de CPU.
        maquina->df[index].tempoES = 0;                     // Tempo de entrada e saída.
    }
}

void inicializarContextos(unsigned short quantidadeProgramas, MaquinaExecucao *maquina) {
    for(int index = 0; index < quantidadeProgramas; index++) {
        maquina->df[index].registrador = 0;
        maquina->df[index].quantidadeVariaveis = 0;
    }
}

void carregarContextoParaMaquina(unsigned short indexTarefa, MaquinaExecucao *maquina){
    int i = 0;
    maquina->registrador = maquina->df[indexTarefa].registrador;
    maquina->quantidadeVariaveis = maquina->df[indexTarefa].quantidadeVariaveis;
    while(i < maquina->df[indexTarefa].quantidadeVariaveis){
        maquina->variaveis[i] = maquina->df[indexTarefa].variaveis[i];
        strcpy(maquina->nomesVariaveis[i], maquina->df[indexTarefa].nomesVariaveis[i]);
        i++;
    }
}

void salvarContextoDaMaquina(unsigned short indexTarefa, MaquinaExecucao *maquina){
    int i = 0;
    maquina->df[indexTarefa].registrador = maquina->registrador;
    maquina->df[indexTarefa].quantidadeVariaveis = maquina->quantidadeVariaveis;
    while(i < maquina->quantidadeVariaveis){
        maquina->df[indexTarefa].variaveis[i] = maquina->variaveis[i];
        strcpy(maquina->df[indexTarefa].nomesVariaveis[i], maquina->nomesVariaveis[i]);
        i++;
    }
}

int verificarEstadoTarefas(DescritorTarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS], unsigned short quantidade, EstadoTarefa estado){
    for(int index = 0; index < quantidade; index++)
        if(tarefas[index].estado == estado)
            return index;

    return NAO_ENCONTRADO;
}

int haTarefasAptas(DescritorTarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS], unsigned short quantidade){
    int cont = 0;
    for(int index = 0; index < quantidade; index++)
        if(tarefas[index].estado != TERMINADA)
            cont++;

    return (cont > 0) ? TRUE : FALSE;
}

void escalonarProximaTarefaDaFila(MaquinaExecucao *maquina, unsigned short *indexFila){
    for(int i = 0; i < maquina->numeroDeProgramas; i++){
        if(maquina->df[*indexFila].estado == PRONTA){
            maquina->df[*indexFila].estado = EXECUTANDO;
            carregarContextoParaMaquina(*indexFila, maquina);
            break;
        } else
            if(++(*indexFila) == maquina->numeroDeProgramas) *indexFila = 0;
    }
}

void gerarPreempcaoPorTempo(MaquinaExecucao *maquina, RoundRobin *roudRobin, unsigned short *indexFila){
    maquina->df[*indexFila].estado = PRONTA;
    salvarContextoDaMaquina(*indexFila, maquina);
    if(++(*indexFila) == maquina->numeroDeProgramas) *indexFila = 0;
    roudRobin->contadorPreempcao = 1;
}

void terminarTarefa(MaquinaExecucao *maquina, RoundRobin *roudRobin, unsigned short indexFila){
    maquina->df[indexFila].estado = TERMINADA;
    roudRobin->contadorPreempcao = 1;
    roudRobin->tempoEntradaESaidaFila[indexFila][1] = roudRobin->contadorCPU + 1;
}

void imprimirDadosErro(ErroExecucao erro){
    printf("\n\nERRO - %d", erro.erro);
    printf("\n\tTarefa   : %s", erro.nome);
    printf("\n\tInstrução: %s", erro.instrucao);
    printf("\n\tLinha    : %d\n", erro.numeroLinha);
}

void exibirRelatorioProcesso(MaquinaExecucao maquina, RoundRobin roudRobin, unsigned short quantidadeProgramas, unsigned short tes){
    int tempoMedioExecucao = 0, tempoMedioEspera = 0;
    printf("\n\n- Processo tes%d", tes);

    for(int index = 0; index < quantidadeProgramas; index++) {
        printf("\n\n\t- Tarefa: %s%s", maquina.df[index].tarefa.programa.nome, EXTENSAO_LPAS);
        printf("\n\tTempo de CPU = %d ut", maquina.df[index].tempoCPU);
        printf("\n\tTempo de E/S = %d ut", maquina.df[index].tempoES);
        printf("\n\tTaxa de ocupação da CPU = %.2f%%", (float) maquina.df[index].tempoCPU / roudRobin.contadorCPU * 100);

        tempoMedioEspera += roudRobin.tempoEntradaESaidaFila[index][1] - roudRobin.tempoEntradaESaidaFila[index][0] - maquina.df[index].tempoCPU;
        tempoMedioExecucao += roudRobin.tempoEntradaESaidaFila[index][1] - roudRobin.tempoEntradaESaidaFila[index][0];
    }

    printf("\n\n\t- Round-Robin");
    printf("\n\tTempo médio de execução = %.2f ut", (float) tempoMedioExecucao / quantidadeProgramas);
    printf("\n\tTempo médio de espera = %.2f ut", (float) tempoMedioEspera / quantidadeProgramas);

    printf("\n\n");
}

pid_t criarProcesso(){
    pid_t pid = fork(); // Cria o processo filho.
    if(pid < 0) exit(EXIT_FAILURE); // Verifica se ocorreu algum erro.
    return pid;
}

unsigned short lerInstrucoesDoArquivo(Programa *programa, FILE *arquivo){
    char c;
    int contInstrucoes = 0, contChar = 0;

    while((c = getc(arquivo)) != EOF){
        // Verifica se o caracter é o inicio de um comentário e ignora todo o resto da linha
        if(c == COMENTARIO_LPAS){ 
            while ((c = getc(arquivo)) != EOF && c != EOL); 
            if(contChar > 0){
                programa->instrucoes[contInstrucoes][contChar] = '\0';
                contChar = 0;
                contInstrucoes++;
            }
            continue;
        }
        // Encerra o loop caso encontre o final do arquivo.
        if(c == EOF) break;
        // Checa se o caracter indica o final da linha
        if(c != EOL && c != CARRIAGE_RETURN && contChar < TAMANHO_INSTRUCAO){
            programa->instrucoes[contInstrucoes][contChar] = c;
            contChar++;
        } else if(c == EOL && contChar > 0){
            programa->instrucoes[contInstrucoes][contChar] = '\0';
            contChar = 0;
            contInstrucoes++;
        }

        if(contInstrucoes > NUMERO_MAXIMO_DE_INSTRUCOES - 1) break; 
    }

    return contInstrucoes++;
} // lerInstrucoesDoArquivo()

int retiraEnter(char *str){
    int tam = strlen(str);

    if(str[tam - 1] == '\n'){
        str[tam - 1] = '\0';
        return 1;
    }

    return 0;
}

int stringVazia(const char *str){
    return (str[0] == '\n' || str[0] == '\0') ? TRUE : FALSE;
}

int isExit(const char *str){
    return (strcmp(str, EXIT)) ? FALSE : TRUE;
}

void * desalocarString(char *str){
    free(str);
    return NULL;
}