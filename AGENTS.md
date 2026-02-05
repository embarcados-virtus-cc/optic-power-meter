# Arquitetura
O projeto está separado em packages mantendo uma arquitetura monorepo, onde cada diretório representa uma parte importante da aplicação que roda de maneira independente, tal qual um serviço.

Cada um deles terá um arquivo (AGENTS.md), tal qual este, que serve de configuração e documentação das patterns utilizadas em cada package, cada uma possui um âmbito completamente diferente, então naturalmente também terão descrições diferentes.
Leia sempre o AGENTS.md referente ao package que você irá dar suporte, ficando atento a toda a configuração atual do projeto.

# Packages
Cada diretório, representa um package, cada parte da aplicação deverá ser dividida em um do mesmo, até o momento essa é a estrutura atual:

- display: Aplicação e drivers para controlar o display LCD GMT130-V1.0 na Raspberry Pi.
- gui: Interface web (Dashboard) construída com React, Shadcn/ui e TanStack Router.
- sfp-interface: Interface de baixo nível e daemon C para comunicação I²C com o módulo SFP.
- tui: Interface gráfica via terminal (Text User Interface) para visualização dos dados do SFP.

*Se necessário, crie outro package, como por exemplo, um package para a API, outro para os circuitos, e etc.*
