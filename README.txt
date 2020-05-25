Daniel Sá Barreto Prado Garcia 
Laura Genari Alves de Jesus
Tiago Marino Silva

versão Linux: Ubuntu 19.04
compilador utilizado: g++

Instruções de uso:
Abra uma janela do terminal:
    -make server (compila o servidor)
    -make client (compila o cliente)
    -make startServer (inicia o servidor)

Em uma nova janela do terminal
    -make startClient (adiciona um cliente)

Opcionalmente
    - voce pode iniciar o server em outra PORT com ./server PORT, sendo PORT o numero da porta.
    -  Para conectar o cliente em outra porta, use ./client PORT, sendo PORT a porta que o servidor esta aberto (por padrão 2000).

Orientacoes para a aplicacao cliente:
    -Apos iniciar o cliente, digite /connect e seu username e aperte enter para se conectar.
    -Todos que estão conectados ao server irao receber as mensagens que você enviar.
    -Envie uma mensagem com o comando '/quit' para se desconectar do server e encerrar a aplicacao, ou use ctrl+d.

Orientacoes para a aplicacao servidor:
    -Use ctrl+c para encerrar o servidor.
