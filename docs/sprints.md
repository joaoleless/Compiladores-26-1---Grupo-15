# Sprints

| Ciclo | Período | Objetivo | Entregas (commits) |
| --- | --- | --- | --- |
| 1 | 13/08 – 23/08 | Preparar o repositório e iniciar o analisador léxico | Repositório e pastas (`3c87a35`, `e97d18e`); primeira versão do `lexer.l` (`83d22db`, `139ddaf`) |
| 2 | 26/08 – 02/09 | Integrar o lexer ao Bison e escrever o primeiro parser | Objetivo e integrantes no README (`142af22`); correção das regras do Flex e da regex de strings (`c012d8c`); lexer devolvendo tokens ao Bison, com `INDENT`/`DEDENT` (`488a9ee`); primeiro `parser.y` (`5e00bd9`); testes e relatório do lexer (`f73e2f9`, `3df5843`) |
| 3 | 14/09 – 26/09 | Documentar o projeto e automatizar os testes para o P1 | Documentação do projeto (`e4f4720`); testes automatizados de lexer e parser (`aa99ad9`); README (`d5b9ca7`) |
| — | 30/09 | **P1** | — |
| 4 | 06/11 | Replanejar: reduzir o escopo da linguagem | Redução do escopo (`3d3929e`), justificada em [decisoes-tecnicas.md](decisoes-tecnicas.md); testes do lexer e do parser (`13b5e59`) |
| — | 11/11 | **P2** | — |
| 5 | 25/11 | Estabilizar o build e corrigir o interpretador | Interpretador no epílogo do Bison e correções de build e warnings (`0bddc17` a `5179551`); 12 correções de comportamento em relação ao Python, cada uma com teste (`b735d99` a `18795db`), registradas em [problemas-e-solucoes.md](problemas-e-solucoes.md); documentação (`7b1a83b` em diante) |

---

## Contribuições por Integrante

Divisão das atividades realizadas por cada membro ao longo de todo o desenvolvimento do projeto, divididas entre **Implementação**, **Testes** e **Documentação**:

| Integrante | Matrícula | Implementação | Testes | Documentação |
| --- | --- | --- | --- | --- |
| **João Eduardo de Souza Leles** | 241025274 | Criação inicial do `lexer.l`; refatoração e redução do escopo da linguagem para o núcleo mínimo executável | Atualização dos testes unitários para o escopo reduzido | Definição inicial do escopo e justificativas de redução no repositório |
| **Guilherme Lorenzi Ventura** | 241011116 | 12 correções de semântica e paridade com Python, análise semântica prévia (`break`/`continue` fora de laço) e modularização do código C (`src/ast.c`, `src/valores.c`, `src/tabela.c`) | Expansão massiva das suítes de testes (`test_parser.py` e `test_lexer.py`) cobrindo todas as correções e casos negativos | Redação técnica completa de `docs/decisoes-tecnicas.md`, `docs/problemas-e-solucoes.md`, `docs/sprints.md` e reestruturação final do `README.md` |
| **Luiz Henrique Pallavicini** | 241012329 | Ajustes de regras do Flex (regex de strings, escapes e rastreamento de linhas), estabilização do build no epílogo do Bison, tratamento em `main.c` e remoção de warnings do GCC | Criação da suíte inicial de testes do lexer `lexer/suite_testes_lexer.py` | Relatório de cobertura dos testes do analisador léxico `lexer/Relatorio_Testes_Lexer.md` |
| **Gabriel Pereira da Silva** | 221008641 | — | Elaboração dos casos de teste funcionais do parser (`tests/cases/*`) e script de execução `run_tests.sh` | Relatório de testes do parser `parser/Relatorio_Testes_Parser.md` e `tests/ACHADOS.md` |
| **Rafael Silva Wasconcelos** | 232030364 | Criação do primeiro `parser.y`, integração do Lexer ao Bison com suporte a indentação (`INDENT`/`DEDENT`), criação de `src/main.c` e `Makefile` | Automação de testes em Python via `unittest` (`test_lexer.py`, `test_parser.py`) e criação do driver isolado em C `tests/lexer_driver.c` | Inclusão do objetivo e integrantes no `README.md` e instruções de compilação e testes |



