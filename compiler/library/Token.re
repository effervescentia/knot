type keyword =
  | Main
  | Module
  | Import
  | From
  | Const
  | Let
  | Func
  | View
  | State
  | Style
  | If
  | As
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
  | QuestionMark
  | ExclamationMark
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
  | NotEquals
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
  | SidecarIdentifier(string)
  | Number(int)
  | String(string)
  | Boolean(bool)
  | JSXTextNode(string)
  | LineComment(string)
  | BlockComment(string)
  | Unexpected(char);
