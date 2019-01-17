type keyword =
  | Main
  | Import
  | From
  | Const
  | Let
  | Func
  | View
  | State
  | Style
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
  | NumberSign
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
  | JSXStartFragment /* <> */
  | JSXEndFragment /* </> */
  | LogicalOr
  | LogicalAnd
  | Keyword(keyword)
  | Identifier(string)
  | Number(int)
  | String(string)
  | Boolean(bool)
  | JSXTextNode(string)
  | LineComment(string)
  | BlockComment(string)
  | Unexpected(char);
