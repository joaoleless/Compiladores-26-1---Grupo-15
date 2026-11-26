# Sprints



| Ciclo | Período | Objetivo | Entregas (commits) |
| --- | --- | --- | --- |
| 1 | 13/08 – 23/08 | Preparar o repositório e iniciar o analisador léxico | Repositório e pastas (`3c87a35`, `e97d18e`); primeira versão do `lexer.l` (`83d22db`, `139ddaf`) |
| 2 | 26/08 – 02/09 | Integrar o lexer ao Bison e escrever o primeiro parser | Objetivo e integrantes no README (`142af22`); correção das regras do Flex e da regex de strings (`c012d8c`); lexer devolvendo tokens ao Bison, com `INDENT`/`DEDENT` (`488a9ee`); primeiro `parser.y` (`5e00bd9`); testes e relatório do lexer (`f73e2f9`, `3df5843`) |
| 3 | 14/09 – 26/09 | Documentar o projeto e automatizar os testes para o P1 | Documentação do projeto (`e4f4720`); testes automatizados de lexer e parser (`aa99ad9`); README (`d5b9ca7`) |
| — | 30/09 | **P1** | — |
| 4 | 06/11 | Replanejar: reduzir o escopo da linguagem | Redução do escopo (`3d3929e`), justificada em [decisoes-tecnicas.md](decisoes-tecnicas.md); testes do lexer e do parser (`13b5e59`) |
| 5 | 25/11 | Estabilizar o build e corrigir o interpretador | Interpretador no epílogo do Bison e correções de build e warnings (`0bddc17` a `5179551`); 12 correções de comportamento em relação ao Python, cada uma com teste (`b735d99` a `18795db`), registradas em [problemas-e-solucoes.md](problemas-e-solucoes.md); documentação (`7b1a83b` em diante) |
