# DFA Converter

Convert DFA to vectorized (using SSE4.2) Finite Automaton (VFA)

#### Quick Start
```console
make
make run
```

#### Examples
Put header file that defines DFA table in `~/example`

### Implementation Todos

**Regex Runtime**

- [x] Lexer & parser for regular expression (yacc and lex)
- [x] Converter from regular expression into NFA
- [x] Converter from NFA into DFA
- [x] Converter from DFA into VFA
- [x] Probabilistic Finite Automata 

**Query Runtime**

- [x] Lexer & parser for SQL query (yacc and lex)
- [x] Vectorized execution
- [ ] Query optimizer
- [ ] Comparison of vectorized model vs iterator model

**Others**

- [ ] Link between query parser and regex parser & NFA generator
- [ ] Link between query optimizer and DFA generator
- [ ] file & input reader
