# Configurações para Desenvolvimento Contínuo da interface SFP

## Declaração de Funções
Todas as funções precisam ser declaradas em seu devido header, seguindo o padrão da biblioteca para raspagem de bits, onde temos duas funções:
- Parse: Função responsável para fazer a captura do byte desejado
- Get: Função getter que apenas retorna o valor do byte desejado

*Sempre são necessárias essas duas funções para cada uma das sessões de bytes, no entanto, podem haver algumas funções auxiliares, que devem ser descritas também.*

### Exemplo de declaração de funções:
``` C
void sfp_parse_a0_base_identifier(const uint8_t *a0_base_data, sfp_a0h_base_t *a0);
sfp_identifier_t sfp_a0_get_identifier(const sfp_a0h_base_t *a0);
```
*Sempre seguir esse padrão de nomenclatura das funções.*

---

## Lógica de Comentários
Todas as funções precisam de um comentário que explique o que ela está relacionada, bem como o que ela faz. Não é necessário ser um comentário longo e descritivo, apenas algum referencial.

### Exemplo de comentário de função:
``` C
/* ============================================
 * Descrição da Função
 * ============================================ */
```
*Sempre seguir esse padrão de comentaŕios das funções*
