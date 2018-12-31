open Knot.Token;
open Core;

let (==>) = (c, t) => Lexer(Char(c), Any, _ => Result(t));

let lexer =
  Lexers([
    ' ' ==> Space,
    '\t' ==> Tab,
    '\n' ==> Newline,
    '=' ==> Assign,
    '.' ==> Period,
    ',' ==> Comma,
    ':' ==> Colon,
    ';' ==> Semicolon,
    '~' ==> Tilde,
    '$' ==> DollarSign,
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
    '{' ==> LeftBrace,
    '}' ==> RightBrace,
    '<' ==> LeftChevron,
    '>' ==> RightChevron,
  ]);