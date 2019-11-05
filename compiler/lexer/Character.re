open Core;

let (==>) = Matcher.((c, t) => Matcher(Exactly(c), _ => result(t), None));

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
