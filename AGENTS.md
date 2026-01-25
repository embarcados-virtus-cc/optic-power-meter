# Arquitetura
O projeto está separado em packages mantendo uma arquitetura monorepo, onde cada package representa uma parte importante da aplicação que roda de maneira independente, tal qual um serviço.

Cada um deles terá um arquivo (AGENTS.md), tal qual este, que serve de configuração e documentação das patterns utilizadas em cada package, cada uma possui um âmbito completamente diferente, então naturalmente também terão descrições diferentes.
Leia sempre o AGENTS.md referente ao package que você irá dar suporte, ficando atento a toda a configuração atual do projeto.

# Packages
Cada diretório dentro da pasta packages, representa um package, cada parte da aplicação deverá ser dividida em um do mesmo, até o momento essa é a estrutura atual:

── packages
   ├── sfp-interface
   └── view

Onde eles representam:

- sfp-interface: Interface de captura dos dados dos bytes presentes nas páginas a0h e a2h de um módulo transceptor SFP (Small Form-Factor Pluggable).
- view: Interface de visualização da aplicação.

*Se necessário, crie outro package, como por exemplo, um package para a API, outro para os circuitos, e etc.*
