type keyword =
  | Main
  | Import
  | From
  | Const
  | Let
  | State
  | View
  | Func
  | If
  | Else
  | Get
  | Mut;

type token =
  | Space
  | Tab
  | Newline
  | Assign
  | Period
  | Comma
  | Colon
  | Semicolon
  | Tilde
  | DollarSign
  | Plus
  | Minus
  | Asterisk
  | VerticalBar
  | Ampersand
  | ForwardSlash
  | LeftParenthese
  | RightParenthese
  | LeftBrace
  | RightBrace
  | LeftBracket
  | RightBracket
  | LeftChevron
  | RightChevron
  | Lambda
  | Equals
  | GreaterThanOrEqual
  | LessThanOrEqual
  | JSXSelfClose /* /> */
  | JSXOpenEnd /* </ */
  | LogicalOr
  | LogicalAnd
  | Keyword(keyword)
  | Identifier(string)
  | Number(int)
  | String(string)
  | JSXTextNode(string)
  | LineComment(string)
  | BlockComment(string)
  | Unexpected(char);