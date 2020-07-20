Daniel Sá Barreto Prado Garcia 
Laura Genari Alves de Jesus
Tiago Marino Silva

versão Linux: Ubuntu 19.04
compilador utilizado: g++

Instruções de uso:
Abra uma janela do terminal:
    - make server (compila o servidor)
    - make client (compila o cliente)
    - make startServerLocal (inicia o servidor no ip local (127.0.0.1) e na porta 2000)

Em uma nova janela do terminal
    - make startClient (adiciona um cliente)

Opcionalmente
    - Voce pode iniciar o server em outro ip e porta com ./server IP PORT, sendo IP o numero do ip e PORT o numero da porta.
    
Orientacoes para a aplicacao cliente:
    - Apos iniciar o cliente, siga esses passos:
        - digite '/connect IP PORT' sendo IP o numero do servidor que quer se conectar e PORT a porta na qual ele está aberto 
        - digite '/nickname APELIDO', sendo APELIDO o nickname que será exibido para você.
        - digite '/join CANAL', sendo CANAL o nome do canal que quer se conectar, deve começar com # ou & e ter até 50 caracteres
    - Após conectado ao canal, você pode enviar mensagens para todos que estao no canal digitando elas e dando enter.
    - Também poderá usar alguns comandos:
        - '/nickname nome' para trocar de apelido
        - '/join canal' para trocar de canal (não pode estar conectado em um canal)
        - '/quit' para desconectar do servidor, pode usar ctrl+d também.
        - '/ping' recebe 'pong' do servidor
    - Os canais possuem admins, que são a primeira pessoa a se conectar nele. 
    - Quando o admin sai do canal, outra pessoa que está conectada no canal vira admin.
    - O admin tem alguns comandos a mais que pode usar:
        - '/mute nome' muta o usuario de nick 'nome' 
        - '/unmute nome' desmuta o usuario de nick 'nome'
        - '/whois nome' recebe o ip do usuario de nick 'nome'
        - '/kick nome' expulsa o usuario de nick 'nome' do canal

Orientacoes para a aplicacao servidor:
    - Use ctrl+c para encerrar o servidor.
    - Para ter um log com mais informações na aplicacao do servidor, mude o DEBUG_MODE de irc.h para true


