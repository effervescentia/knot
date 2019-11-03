open Core;

/** character lexer matcher */
let (==>) = (c, t) => Matcher(Char(c), _ => result(t));

let matchers = [
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
  '#' ==> NumberSign,
  '?' ==> QuestionMark,
  '!' ==> ExclamationMark,
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
];
