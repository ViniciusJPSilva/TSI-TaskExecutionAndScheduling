# TSI-TaskExecutionAndScheduling
> Trabalho desenvolvido para a disciplina de Sistemas Operacionais do segundo período do curso Tecnólogo em Sistemas para Internet. 
O algoritmo foi desenvolvido utilizando apenas a Linguagem C padrão ISO2 e a GNU C Library3, que inclui a API POSIX. 

O objetivo deste trabalho é desenvolver dois serviços de sistema operacional: Execução e
Escalonamento de Tarefas (Task Execution and Scheduling). Esses serviços devem ser implementados
em um programa chamado tes, que quando for executado deve exibir o prompt abaixo.

* <strong>tes ></strong>

<hr/>
<h3>1. Execução de Tarefas</h3>

Este serviço permite executar simultaneamente até quatro programas da linguagem LPAS,
definida no Apêndice I. Para iniciar a execução de um programa considere duas opções, o usuário
digita:

i. O nome de um programa que deseja executar.
* <strong>tes > soma </strong> 

ii. Os nomes de dois a quatro programas separados por espaço para serem executados simultaneamente.
* <strong>tes > add sub mul div</strong>

Quando os programas LPAS encerrarem sua execução, o prompt tes deve ser exibido
novamente. Para encerrar o programa tes e retornar ao sistema operacional deve-se digitar o
comando exit.

* <strong>tes > exit</strong>

O programa tes deve ler as tarefas de um arquivo texto de extensão lpa,. Veja os exemplos de
programas LPAS no anexo deste texto e no Apêndice I.
Para facilitar para o usuário a identificação de qual programa que está em execução, por
exemplo, lendo ou escrevendo, sempre que um instrução LPAS de leitura (READ) ou escrita
(WRITE) for executada, ela deve ser precedida do nome da tarefa, assim:

* add -> READ: 10
* sub -> READ: 8
* sub -> READ: 20
* add -> READ: 3
* add -> WRITE: 13
* sub -> WRITE: -12

Para cada grupo de duas tarefas a serem executadas, o programa deve criar um processo
usando a chamada de sistema fork. Considerando o exemplo acima, em que é solicitado a
execução de quatro programas LPAS (add, sub, mul e div), um processo tes1 executará as tarefas add
e sub, e um outro processo tes2 executará as tarefas mul e div. Nesse exemplo, o programa terá
então três processos, o processo pai tes e os processos filhos tes1 e tes2. Se for executado um ou
dois programas LPAS, deve-se criar apenas um processo filho. Sendo assim, o serviço de executar
as instruções do programa LPAS e de revezar essas tarefas no processador deve ser implementado
nos processos filhos. A função do processo pai será:

1. Exibir o prompt tes;
2. lê e carregar os programas LPAS do disco para a memória;
3. criar o(s) processo(s) filho(s);
4. aguardar a conclusão do(s) processo(s) filho(s);
5. finalizar o programa quando o usuário digitar exit.

Portanto, dentre as funções de um sistema operacional, pode-se dizer que o processo pai
implementa a carga do programa do disco para memória e a criação de processos, enquanto que
os processos filhos são executores e escalonadores de tarefas.

<hr/>
<h3>2. Escalonamento de Tarefas</h3>

Este serviço deve implementar um escalonador preemptivo que utiliza o algoritmo de
escalonamento Round-Robin para realizar a execução simultânea das tarefas.
Considere a fatia de tempo (quantum) igual a 2 ut (unidades de tempo) e que cada ut
corresponde a 1 instrução LPAS. Sendo assim, a tarefa que possui oito instruções LPAS necessita
de 8 ut para executar essas instruções, logo o seu tempo de CPU (tempo de processamento) é
igual a 8 ut.
O algoritmo Round-Robin deve considerar o instante de tempo em que cada tarefa entra na fila
do processador (fila de tarefas prontas) para execução. Esse instante é obtido de acordo com
ordem das tarefas ao executá-las, por exemplo, de acordo com o prompt abaixo, a tarefa add entra
na fila do processador no instante de tempo zero, a tarefa sub ingressa no instante 1 ut, mul em 2
ut e div em 3 ut.

* tes > add sub mul div

Sempre que o programa LPAS executa uma instrução de entrada dados (READ) a tarefa deve ter
a sua execução interrompida e permanecer bloqueada (suspensa) até que ocorra um evento que
permita desbloqueá-la, ou seja, até que o usuário digite o número e tecle ENTER. Essa instrução é
a simulação de uma chamada de sistema que é usada para informar ao programa tes que a tarefa
deve ser suspensa e só retornar a ficar pronta após 3 ut (unidades de tempo). Logo, o tempo de
E/S dessa instrução é sempre igual a 3 ut. Se uma tarefa possui 4 instruções READ, o seu tempo
total de E/S será de 12 ut.

Qualquer instrução inválida no programa LPAS deve ter sua execução abortada pelo processo
filho. Um relatório com os erros de execução deve ser exibido para informar quais são as tarefas e
qual o erro ocorrido. Use o modelo de mensagem abaixo.

* Erros de execução LPAS:
* 1- add: instrução LPAD inválida
* 2- div: argumento de instrução LPAS inválido

Após concluir a execução de todas as tarefas LPAS, o processo filho deve exibir um relatório com o
resultado da execução das tarefas gerenciadas por ele com as seguintes informações para cada
tarefa.

1. Tempo de CPU (ut).
2. Tempo de E/S (ut).
3. A taxa percentual de ocupação do processador (CPU) em relação ao tempo total de sua
utilização por todas as tarefas.

E os tempos abaixo (em segundos) para todos os processos que executaram

4. Tempo médio de execução (turnaround time).
5. Tempo médio de espera (waiting time)

Deve-se usar o leiaute abaixo para exibir esse relatório.

<ul>
- Processo tes1 <br>
 
<ul>
-- Tarefa: add.lpas <br>
Tempo de CPU = 7ut <br>
Tempo de E/S = 6 ut <br>
Taxa de ocupação da CPU = 47% <br>
<br>
-- Tarefa: expression.lpas <br>
Tempo de CPU = 8 ut <br>
Tempo de E/S = 0 ut <br>
Taxa de ocupação da CPU = 53% <br>
<br>
-- Round-Robin <br>
Tempo médio de execução = 7,5 s <br>
Tempo médio de espera = 2,5 s <br>
</ul>
</ul>

<br><hr><hr>

<h5> Apendice I </h5>

<h3>LPAS - Linguagem de Programação para Aritmética Simples</h3>

Uma linguagem de máquina denominada LPAS - Linguagem de Programação para Aritmética
Simples - é usada para escrever programas que executam as operações aritméticas somente com
números inteiros. As instruções LPAS são apresentadas na Tabela 1.

LPAS possui a seguinte organização:

1. Cada linha de código deve ter apenas uma única instrução.
2. As instruções e os seus argumentos variáveis sejam todos escritos em maiúsculo.
3. Os comentários devem iniciar com ponto-e-vírgula.
4. Os programas LPAS são executados por uma Máquina de Execução (ME) ou processador.
5. A ME é responsável por carregar o programa para a memória e executá-lo.
6. A ME possui duas memórias (código e dados) e um único registrador que permite armazenar
um número inteiro.
7. A memória de código armazena os programas LPAS a serem executados pela ME.
8. A memória de dados armazena as variáveis do programa que está em execução.

Exemplos de programas LPAS:

![image](https://user-images.githubusercontent.com/81810017/202273701-cb097b85-bee4-48f5-9eed-b87709b4e27b.png)

<hr>

![image](https://user-images.githubusercontent.com/81810017/202273867-c56f0adf-abe3-455c-86ff-7b310106a0dd.png)


