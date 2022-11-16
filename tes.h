#pragma once

#include <sys/types.h> // Requerido por pid_t.
#include <stdio.h>

// Tamanho máximo do nome de arquivo.
#define TAMANHO_NOME_ARQUIVO 80

// Tamanho máximo de uma instrução LPAS.
#define TAMANHO_INSTRUCAO 10

// Número máximo de instruções do programa LPAS.
#define NUMERO_MAXIMO_DE_INSTRUCOES 255

// Número máximo de variáveis do programa.
#define NUMERO_MAXIMO_DE_VARIAVEIS 50

// Número máximo de programas da máquina de execução.
#define NUMERO_MAXIMO_DE_PROGRAMAS 4

// Códigos das instruções LPAS.
#define READ 10
#define WRITE 11
#define LOAD 20
#define STORE 21
#define ADD 30
#define SUB 31
#define MUL 32
#define DIV 33
#define HALT 40

// Representa uma instrução LPAS.
typedef char Instrucao[TAMANHO_INSTRUCAO];

// Representa o nome do programa.
typedef char NomePrograma[TAMANHO_NOME_ARQUIVO];

// Representa o nome da variável.
typedef char NomeVariavel[TAMANHO_INSTRUCAO];

// Representa a estrutura de um programa LPAS.
typedef struct  {
   	// Número de instruções do programa.
	unsigned short numeroDeInstrucoes;

	// Nome do programa.
	NomePrograma nome;

	/* Memória de código para armazenar as instruções LPAS que compõem o programa.
	   Cada instrução é armazenada em uma posição do vetor. */
	Instrucao instrucoes[NUMERO_MAXIMO_DE_INSTRUCOES];
} Programa;


/* Códigos resultantes da excução do programa LPAS. Os códigos válidos são:
 
	 0 = execução bem sucedida; 
	 1 = instrução LPAS inválida;
	 2 = argumento de instrução LPAS inválido;
	 3 = argumento de instrução LPAS ausente;
	 4 = duas ou mais instruções LPAS na mesma linha de código;
	 5 = símbolo inválido.
*/
typedef enum { EXECUCAO_BEM_SUCEDIDA, 
			   INSTRUCAO_LPAS_INVALIDA, 
			   ARGUMENTO_INSTRUCAO_LPAS_INVALIDO, 
               ARGUMENTO_INSTRUCAO_LPAS_AUSENTE, 
			   MUITAS_INSTRUCOES, 
			   SIMBOLO_INVALIDO 
} Erro;

// Representa os dados sobre um erro de execução do programa.
typedef struct {
	unsigned short numeroLinha;
	Instrucao instrucao;
	NomePrograma nome;
	Erro erro;
} ErroExecucao;			   

//  Indica o estado atual da tarefa. 
typedef enum { NOVA, PRONTA, EXECUTANDO, SUSPENSA, TERMINADA 
} EstadoTarefa;

// Como a tarefa (processo) é um programa em execução, esta estrutura relaciona a identificação de cada tarefa com o programa a ser executado por ela.
typedef struct {
	// Identificador da tarefa.
	unsigned identificador;

	// Programa a ser executado pela tarefa.
	Programa programa;
} Tarefa;

// Representa o descritor da tarefa, conhecido como bloco de controle da tarefa (TCB - Task Control Block).
typedef struct {
	// Identificação da tarefa deste bloco de controle da tarefa.
	Tarefa tarefa;

	// Indica o estado atual da tarefa. O estado da tarefa deve ser atualizado segundo o seu ciclo de vida durante sua execução. 
	EstadoTarefa estado;

	// O contador de programa (PC - Program Counter) indica a próxima instrução do programa a ser executada.
	unsigned char pc;

	// Indica o tempo total de uso do processador que a tarefa precisa para executar suas instruções. 
	unsigned char tempoCPU;

	// Indica o tempo total das operações de E/S executadas pela tarefa, nesta implementação, o tempo total de E/S apenas das instruções READ. 
	unsigned char tempoES;

	// Salva o valor armazenado no registrador da máquina execução..
	int registrador;

	// Quantidade de variáveis.
	int quantidadeVariaveis;

	// Identificador (nome) das variáveis
	NomeVariavel nomesVariaveis[NUMERO_MAXIMO_DE_VARIAVEIS];

	// Memória de dados que armazena as variáveis do programa LPAS que está em execução na ME.
	int variaveis[NUMERO_MAXIMO_DE_VARIAVEIS];
} DescritorTarefa;

// Representa a estrutura da máquina de execução LPAS.
typedef struct  {
	// Número de programas LPAS carregados na memória da Máquina de Execução.
	int numeroDeProgramas;

	/* Memória de código que armazena os programas LPAS a serem executados na máquina de execução.
	   Como para cada programa a ser executado na máquina de execução é necessário criar uma tarefa para representá-lo, essas tarefas possuem um 
	   descritor de tarefa para representar o seu contexto de execução e os dados do programa a ser executado, como nome e suas instruções LPAS.
	*/
	DescritorTarefa df[NUMERO_MAXIMO_DE_PROGRAMAS];

	// Registrador da máquina de execução LPAS.
	int registrador;

	// Quantidade de variáveis.
	int quantidadeVariaveis;

	// Identificador (nome) das variáveis
	NomeVariavel nomesVariaveis[NUMERO_MAXIMO_DE_VARIAVEIS];

	// Memória de dados que armazena as variáveis do programa LPAS que está em execução na ME.
	int variaveis[NUMERO_MAXIMO_DE_VARIAVEIS];

	// Identifica a instrução, o número da linha e o nome do programa onde o erro de execução ocorreu.
	ErroExecucao erroExecucao;
} MaquinaExecucao;

// Representa uma unidade de tempo (ut), renomeada para dar mais sentido às variáveis da Struct RoudRobin.
typedef unsigned short int UnidadeTempo;

// Representa os dados coletados pelo escalonador Round-Robin, para calcular os tempos médios após a execução dos programas.
typedef struct {
	// Representa os 'clocks' totais do CPU (UT)
	UnidadeTempo contadorCPU;

	// Representa o contator de Preempção por tempo (Quantum)
	UnidadeTempo contadorPreempcao;

	// Matriz que representa o 'clock' (UT) em que cada programa entrou (NOVA -> PRONTA) e saiu (TERMINDADA) na fila do processador.
	UnidadeTempo tempoEntradaESaidaFila[NUMERO_MAXIMO_DE_PROGRAMAS][2];
} RoundRobin;

// Número de programas que cada processo (fork) é capaz de executar
#define MAX_PROGRAMAS_PROCESSO 2

// Representa a string do prompt à ser exibido.
#define PROMPT "tes > "

// Representa o comando "exit".
#define EXIT "exit"

// Representa a extensão dos arquivos LPAS.
#define EXTENSAO_LPAS ".lpas"

// Instruções LPAS
#define INST_READ "READ"
#define INST_WRITE "WRITE"
#define INST_LOAD "LOAD"
#define INST_STORE "STORE"
#define INST_ADD "ADD"
#define INST_SUB "SUB"
#define INST_MUL "MUL"
#define INST_DIV "DIV"
#define INST_HALT "HALT"

// Número de instruções LPAS existentes.
#define QUANTIDADE_INST_LPAS 9

// Representa o modo de abertura apenas leitura para arquivos FILE.
#define APENAS_LEITURA "r"

// Delimitador entre os comandos inseridos no prompt.
#define DELIMITADOR " "

// Representa o caractere de final de linha '\n'.
#define EOL '\n'

// Representa o caracter de retono ao inicio da linha '\r'.
#define CARRIAGE_RETURN '\r'

#define CHAR_VAZIO ' '
#define CHAR_NULO '\0'

// Representa o caracter de comanetário da linguagem LPAS.
#define COMENTARIO_LPAS ';'

// Unidade de tempo: equivale à 1 instrução LPAS.
#define UT 1

// Fatia de tempo (quantum) = 2ut (unidades de tempo).
#define QUANTUM (2 * UT)

// Tempo de E/S da instrução READ 
#define READ_TIME (3 * UT)

// Tempo de E/S da instrução WRITE 
#define WRITE_TIME (1 * UT)

// Retorno execução sem erros
#define RETURN_OK 0

// Retorno execução com erro
#define RETURN_ERRO -1

// Retorno execução do comando HALT
#define RETURN_FIM 1

// Mensagens
#define ERRO_PROGRAMA_NAO_EXISTE "O programa %s não existe!\n"
#define MSG_BOASVINDAS "\n\tTask Execution and Scheduling [versão 3.0.1]\n\tVinicius J P Silva. Todos os direitos reservados.\n\n"

// Valores boleanos.
#define TRUE 1
#define FALSE 0

// Valor para funções de busca retornarem caso não encontrem determinado elemento.
#define NAO_ENCONTRADO -1

// A linha de comando pode ler o nome de até 4 arquivos LPAS, mais os espaçoes em branco entre os mesmos.
#define TAMANHO_LINHA_COMANDO (TAMANHO_NOME_ARQUIVO * NUMERO_MAXIMO_DE_PROGRAMAS + 3)

/*
	Função responsável por exibir o prompt, ler e executar os programas LPAS e finalizar o prompt quando o comando "exit" for inserido.
	Retorna: EXIT_SUCCESS.
*/
int tes();

/*
	Exibe o prompt.
*/ 
void exibirPrompt(const char *prompt);

/*
	Lê o programa LPAS ou o comando "exit" a ser executado.
	Retorna: a linha de comando lida.
*/
char * lerComando();

/*
	Obtém os nomes dos programas informados na linha de comando e os armazena em um vetor Tarefa.
	Quantidade máxima de nomes lidos = NUMERO_MAXIMO_DE_PROGRAMAS
	Retona: O número de nomes de programas obtidos.
*/
unsigned short obterNomesProgramas(const char *linhaDeComando, Tarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS]);

/*
	Percorre o vetor de tarefas, tenta abrir o arquivo correspondente, armazena as instruções LPAS seus respectivos Programas.
	Retorna: O número de programas que efetivamente podem ser executados (existe um arquivo de extensão EXTENSAO_LPAS)
*/
unsigned short obterInstrucoesProgramas(Tarefa tarefas[], unsigned short quantidadeProgramas);

/*
	Cria um ou dois processos para executar os programas LPAS.
	- 1 ou 2 programas LPAS = 1 processo.
	- 2 ou 3 programas LPAS = 2 processos.
*/
void executarProgramas(Tarefa tarefas[], unsigned short quantidadeProgramas);

/*
	Função responsável por executar as tarefas (progamas LPAS) e escalonar as mesmas.
*/
void executarEscalonarTarefas(Tarefa tarefas[MAX_PROGRAMAS_PROCESSO], unsigned short quantidadeProgramas, unsigned short tes);

/*
	Busca, decodifica e executa a proxima instrução do programa desejado.
	Retorna: EXECUCAO_BEM_SUCEDIDA caso a interpretação seja concluída com sucesso,
		ou o devido código de ERRO, caso não.
*/
int interpretador(unsigned short indiceTarefa, MaquinaExecucao *maquina);

/*
	Executa ums instrução LPAS correspondente ao código da instrução informado (tipoInstrucao).
	Retorna:
		RETURN_ERRO: Caso ocorra algum erro durante a execução.
		RETURN_OK: Caso a execução seja bem sucedida.
		RETURN_FIM: Caso a instrução informada seja HALT.
*/
int executarInstrucao(int tipoInstrucao, Instrucao dado, unsigned short indiceTarefa, MaquinaExecucao *maquina);

/*
	Exibe o prompt “READ: ”, lê um valor inteiro do teclado e armazena na variável correspondente.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int lpasRead(Instrucao dado, unsigned short indiceTarefa, MaquinaExecucao *maquina);

/*
	Escreve na tela a mensagem “WRITE: ”, seguida do valor armazenado na variável informada ou o dígito numérico que acompanha a instrução.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int lpasWrite(Instrucao dado, unsigned short indiceTarefa, MaquinaExecucao *maquina);

/*
	Carrega para o registrador da máquina o valor de uma variável ou o dígito numérico que acompanha a instrução.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int lpasLoad(Instrucao dado, MaquinaExecucao *maquina);

/*
	Adiciona o valor do registrador ao valor de uma variável ou número e armazena o resultado no registrador.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int lpasAdd(Instrucao dado, MaquinaExecucao *maquina);

/*
	Subtrai o valor do registrador do valor de uma variável ou número e armazena o resultado no registrador.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int lpasSub(Instrucao dado, MaquinaExecucao *maquina);

/*
	Multiplica o valor do registrador com valor de uma variável ou número e armazena o resultado no registrador.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int lpasMul(Instrucao dado, MaquinaExecucao *maquina);

/*
	Divide o valor do registrador pelo valor de uma variável ou número e armazena o resultado no registrador.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int lpasDiv(Instrucao dado, MaquinaExecucao *maquina);

/*
	Armazena o valor do registrador em uma variável.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int lpasStore(Instrucao dado, MaquinaExecucao *maquina);

/*
	Busca o endereco (indice) de uma variável, utilizando o seu nome.
	Retorna o endereco (indice) da variável ou NAO_ENCONTRADO, caso não encontre a mesma.
*/
int buscarEnderecoVariavelUsandoNome(Instrucao dado, MaquinaExecucao *maquina);

/*
	Obtém o valor numérico do argumento de uma instrução LPAS e armazena no local referenciado pelo ponteiro *valor.
	> Se for uma variável: Busca no vetor de variáveis utilizando o nome.
	> Se for um número: Converte de string para int.
	Retorna:
		EXECUCAO_BEM_SUCEDIDA: Caso execute sem erros.
		Código de erro: Caso ocorra algum erro.
*/
int obterValorNumericoDoArgumento(Instrucao dado, MaquinaExecucao *maquina, int *valor);

/*
	Verifica a instrução e obtém o seu argumento (dado).
	Retorna: String (char *) contendo o argumento ou NULL caso não exista. 
*/
char * obterArgumento();

/*
	Compara o comando da instrução com os comandos da linguagem LPAS.
	Retorna: o código da instrução LPAS ou NAO_ENCONTRADO, caso não exista.
*/
int identificarInstrucao(Instrucao instrucao);

/*
	Preenche uma struct ErroExecucao com os dados informados nos parâmetros.
	Retorna: a struct ErroExecucao preenchida. 
*/
ErroExecucao geradorDeErrosLpas(unsigned short numeroLinha, Instrucao instrucao, NomePrograma nome,	Erro erro);


/*
	Prepara a MaquinaExecução para ser executada.
	- Inicializa os identificadores das tarefas.
	- Preenche os campos do DescritorTarefa.
*/
void inicializarMaquinaExecucao(Tarefa tarefas[], unsigned short quantidadeProgramas, MaquinaExecucao *maquina);

/*
	Inicia os campos das Contextos com os devidos dados.
*/
void inicializarContextos(unsigned short quantidadeProgramas, MaquinaExecucao *maquina);

/*
	Carrega os dados armaenzados no Contexto para a MaquinaExecucao.
*/
void carregarContextoParaMaquina(unsigned short indexTarefa, MaquinaExecucao *maquina);

/*
	Carrega os dados armaenzados na MaquinaExecucao para o Contexto.
*/
void salvarContextoDaMaquina(unsigned short indexTarefa, MaquinaExecucao *maquina);

/*
	Verifica se há tarefas com estado o informado no parâmetro.
	Retorna: indice (index) da primeira tarefa (com estado correspondente) encontrada ou NAO_ENCONTRADO caso não encontre.
*/
int verificarEstadoTarefas(DescritorTarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS], unsigned short quantidade, EstadoTarefa estado);

/*
	Verifica se há tarefas com estado diferente de TERMINADO na fila do processador ou na fila das tarefas NOVAs.
	Retorna: TRUE caso tenha, ou FALSE caso não.
*/
int haTarefasAptas(DescritorTarefa tarefas[NUMERO_MAXIMO_DE_PROGRAMAS], unsigned short quantidade);

/*
	Escalona a próxima tarefa PRONTA para o processador. A função não verifica se há tarefas prontas!
	> Carrega o contexto para a MaquinaExecucao.
	> Incrementa o indexFila.
*/
void escalonarProximaTarefaDaFila(MaquinaExecucao *maquina, unsigned short *indexFila);

/*
	Função responsável por simuar a preemção por tempo.
	> Retira a tarefa em execução da CPU e a coloca na fila de tarefas PRONTAS.
	> Salva o contexto da tarefa em questão.
	> Reinicia a contagem da preempção.
*/
void gerarPreempcaoPorTempo(MaquinaExecucao *maquina, RoundRobin *roudRobin, unsigned short *indexFila);

/*
	Finaliza a tarefa indicada pelo indesFila.
	> Determina seu estado como TERMINADA.
	> Calcula e armazena o momento (ut) que a tareda saiu da fila da CPU.
	> Reinicia a contagem da preempção.
*/
void terminarTarefa(MaquinaExecucao *maquina, RoundRobin *roudRobin, unsigned short indexFila);

/*
	Imprime na saída padrão (stdout) os dados do erro passado por parâmetro.
*/
void imprimirDadosErro(ErroExecucao erro);

/*
	Imprime na saída padrão (stdout) os dados do processo executado.
*/
void exibirRelatorioProcesso(MaquinaExecucao maquina, RoundRobin roudRobin, unsigned short quantidadeProgramas, unsigned short tes);

/*
	Efetivamente cria um processo utilizando a função fork().
	Caso ocorra um erro no processo de criação do processo, a função finaliza o programa.
	Retorna: 
		- 0 para o processo Filho.
		- PID do processo filho ou -1, caso ocorra algum erro, para o processo Pai.
*/
pid_t criarProcesso();

/*
	Percorre o arquivo, lê as instruções e as armazena no Programa.
	Retorna: O número de instruções lidas.
*/
unsigned short lerInstrucoesDoArquivo(Programa *programa, FILE *arquivo);

/*
    Remove o caracter de nova linha ('\n') de uma string, caso exista.
    Retorna: TRUE caso consiga remover ou FALSE caso não remova.
*/
int retiraEnter(char *str);

/*
    Verifica se a string (vetor de caracteres) está vazia, ou seja, seu primeiro caracter é um '\n' ou '\0'.
    Retorna TRUE caso esteja vazia ou FALSE caso não.
*/
int stringVazia(const char *str);

/*
	Verifica se a string informada equivale ao comando EXIT.
	Retorna: TRUE caso seja ou FALSE caso não seja.
*/
int isExit(const char *str);

/*
	Libera a memória alocada cujo um ponteiro tipo char referencia.
	Retorna: NULL
*/
void * desalocarString(char *str);