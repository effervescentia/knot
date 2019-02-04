open Core;

let whitespace = [Char(' '), Char('\t'), newline];

let rec permiss = (lexer, chs) =>
  Lexers([lexer, Lexer(Either(chs), Any, _ => permiss(lexer, chs))]);

let rec (==>) = (p, t) => {
  let next = _ =>
    if (String.length(p) == 1) {
      Result(t);
    } else {
      permiss(String.sub(p, 1, String.length(p) - 1) ==> t, whitespace);
    };

  Lexer(Char(p.[0]), Any, next);
};

let lex_jsx_self_close = "/>" ==> JSXSelfClose;
let lex_jsx_open_end = "</" ==> JSXOpenEnd;
let lex_jsx_start_fragment = "<>" ==> JSXStartFragment;
let lex_jsx_start_end = "</>" ==> JSXEndFragment;

let lexer =
  Lexers([
    "&&" ==> LogicalAnd,
    "||" ==> LogicalOr,
    "->" ==> Lambda,
    "==" ==> Equals,
    "<=" ==> LessThanOrEqual,
    ">=" ==> GreaterThanOrEqual,
    lex_jsx_self_close,
    lex_jsx_open_end,
    lex_jsx_start_fragment,
    lex_jsx_start_end,
  ]);

let jsx_start_tag_lexer = lex_jsx_self_close;

let jsx_content_lexer =
  Lexers([lex_jsx_open_end, lex_jsx_start_fragment, lex_jsx_start_end]);
