# TUI para acesso a informações do transceptor SFP

Esse projeto é feito com base nos packages 'sfp-interface' e 'api', onde ele usa exatamente o mesmo client para acesso ao UNIX socket, alterando apenas o model pydantic que ele se baseia, para entregar as informações na TUI (Terminal User Interface).

A principal biblioteca que está sendo usada para construir essa interface, é a prompt_toolkit, é importante seguir os padrões de desenvolvimento e estilização dela, para manter toda a interface em conformidade com os dados que precisam ser apresentados (página a0h e a2h), dessa forma, é necessário apresentar os dados dinâmicos e estáticos de uma forma coerente, que facilite o uso.
