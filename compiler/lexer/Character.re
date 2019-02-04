open Core;

let (==>) = (c, t) => Lexer(Char(c), Any, _ => Result(t));

let lex_space = ' ' ==> Space;
let lex_tab = '\t' ==> Tab;
let lex_newline = '\n' ==> Newline;
let lex_assign = '=' ==> Assign;
let lex_left_brace = '{' ==> LeftBrace;
let lex_right_brace = '}' ==> RightBrace;
let lex_left_chevron = '<' ==> LeftChevron;
let lex_right_chevron = '>' ==> RightChevron;

let whitespace_lexers = [lex_space, lex_tab, lex_newline];

let lexer =
  Lexers([
    lex_assign,
    '.' ==> Period,
    ',' ==> Comma,
    ':' ==> Colon,
    ';' ==> Semicolon,
    '~' ==> Tilde,
    '$' ==> DollarSign,
    '#' ==> NumberSign,
    '-' ==> Minus,
    '+' ==> Plus,
    '*' ==> Asterisk,
    '/' ==> ForwardSlash,
    '|' ==> VerticalBar,
    '&' ==> Ampersand,
    '(' ==> LeftParenthese,
    ')' ==> RightParenthese,
    '[' ==> LeftBracket,
    ']' ==> RightBracket,
    lex_left_brace,
    lex_right_brace,
    lex_left_chevron,
    lex_right_chevron,
    ...whitespace_lexers,
  ]);

let jsx_start_tag_lexer =
  Lexers([
    lex_assign,
    lex_left_brace,
    lex_right_brace,
    lex_right_chevron,
    ...whitespace_lexers,
  ]);

let jsx_end_tag_lexer = Lexers([lex_right_chevron, ...whitespace_lexers]);

let jsx_content_lexer = Lexers([lex_left_brace, lex_left_chevron]);
