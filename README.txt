Daniel Sá Barreto Prado Garcia 
Laura Genari Alves de Jesus
Tiago Marino Silva

versão Linux: Ubuntu 19.04
compilador utilizado: g++

Instruções de uso:
Abra uma janela do terminal:
    -make server (compila o servidor)
    -make client (compila o cliente)
    -make start (inicia o servidor)

Em uma nova janela do terminal
    -make mary (adiciona o cliente Mary)

Em uma nova janela do terminal
    -make john (adiciona o cliente John)

Opcionalmente
    - voce pode iniciar o server em outra PORT com ./server PORT, sendo PORT o numero da porta.
    - voce pode iniciar a aplicacao cliente com outro nome com ./client nome port, sendo port a porta que o servidor esta aberto(por padrao 2000) e nome o nome com o qual voce se conectara.

Orientacoes para a aplicacao cliente:
    -Apos iniciar o cliente, digite sua mensagem e aperte enter para enviar.
    -Todos que estão conectados ao server irao receber sua mensagem.
    -Envie uma mensagem com o comando '/quit' para se desconectar do server e encerrar a aplicacao.

Orientacoes para a aplicacao servidor:
    -Use ctrl+c para encerrar o servidor.