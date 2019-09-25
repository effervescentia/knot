open Core;

let _quote = Char('"');

let rec lex_string = _ =>
  Lexers([
    FailingLexer(
      UnclosedString,
      _quote,
      Any,
      s => Result(String(String.sub(s, 1, String.length(s) - 2))),
    ),
    FailingLexer(UnclosedString, Except([_quote]), Any, lex_string),
  ]);

let lexer = Lexer(_quote, Any, lex_string);
