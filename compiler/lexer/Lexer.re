open Knot.Token;

module FileStream = Knot.FileStream;

type context =
  | Normal
  | JSXStartTag
  | JSXEndTag
  | JSXContent;

let print_tkn =
  fun
  | Space => " "
  | Tab => "tab(\\t)"
  | Newline => "newline(\\n)"
  | Assign => "="
  | Period => "."
  | Comma => ","
  | Colon => ":"
  | Semicolon => ";"
  | Tilde => "~"
  | DollarSign => "$"
  | Plus => "+"
  | Minus => "-"
  | Asterisk => "*"
  | VerticalBar => "|"
  | Ampersand => "&"
  | ForwardSlash => "/"
  | LeftParenthese => "("
  | RightParenthese => ")"
  | LeftBrace => "{"
  | RightBrace => "}"
  | LeftBracket => "["
  | RightBracket => "]"
  | LeftChevron => "<"
  | RightChevron => ">"
  | Lambda => "->"
  | Equals => "=="
  | GreaterThanOrEqual => ">="
  | LessThanOrEqual => "<="
  | JSXSelfClose => "/>"
  | JSXOpenEnd => "</"
  | LogicalOr => "||"
  | LogicalAnd => "&&"
  | Keyword(kwd) =>
    switch (kwd) {
    | Main => "keyword(main)"
    | Import => "keyword(import)"
    | Const => "keyword(const)"
    | Let => "keyword(let)"
    | State => "keyword(state)"
    | View => "keyword(view)"
    | Func => "keyword(func)"
    | If => "keyword(if)"
    | Else => "keyword(else)"
    | Get => "keyword(get)"
    | Mut => "keyword(mut)"
    }
  | Identifier(s) => Printf.sprintf("identifier(%s)", s)
  | Number(n) => Printf.sprintf("number(%d)", n)
  | JSXTextNode(s) => Printf.sprintf("jsx_text_node(%s)", s)
  | String(s) => Printf.sprintf("string(%s)", s)
  | LineComment(s) => Printf.sprintf("line_comment(%s)", s)
  | BlockComment(s) => Printf.sprintf("block_comment(%s)", s)
  | Unexpected(c) => Printf.sprintf("UNEXPECTED(%c)", c);

let lex_left_chevron = (stream, cursor, ctx) =>
  switch (Util.peek_next_non_space(stream, cursor), ctx) {
  | (Some('='), Normal) =>
    Util.junk_non_space(stream);
    LessThanOrEqual;
  | (Some('/'), Normal | JSXContent) =>
    Util.junk_non_space(stream);
    JSXOpenEnd;
  | _ => LeftChevron
  };

let lex = ((ch, cursor), ctx, stream) =>
  switch (ch, List.length(ctx^) != 0 ? List.nth(ctx^, 0) : Normal) {
  | (' ', Normal) => Space
  | ('\t', Normal) => Tab
  | ('\n', Normal) => Newline
  | ('.', Normal) => Period
  | (',', Normal) => Comma
  | (':', Normal) => Colon
  | (';', Normal) => Semicolon
  | ('~', Normal) => Tilde
  | ('$', Normal) => DollarSign
  | ('+', Normal) => Plus
  | ('*', Normal) => Asterisk
  | ('(', Normal) => LeftParenthese
  | (')', Normal) => RightParenthese
  | ('[', Normal) => LeftBracket
  | (']', Normal) => RightBracket
  | ('|', Normal) => Misc.lex_vertical_bar(stream, cursor)
  | ('&', Normal) => Misc.lex_ampersand(stream, cursor)
  | ('-', Normal) => Misc.lex_minus(stream, cursor)
  | ('>', Normal) => Misc.lex_right_chevron(stream, cursor)
  | ('=', Normal) => Misc.lex_assign(stream, cursor)
  | ('0'..'9', Normal) => Number(Number.lex([ch], stream))

  | ('m', Normal) =>
    Keyword.lex([("ain", Main), ("ut", Mut)], [ch], stream)
  | ('i', Normal) =>
    Keyword.lex([("mport", Import), ("f", If)], [ch], stream)
  | ('c', Normal) => Keyword.lex([("onst", Const)], [ch], stream)
  | ('l', Normal) => Keyword.lex([("et", Let)], [ch], stream)
  | ('s', Normal) => Keyword.lex([("tate", State)], [ch], stream)
  | ('v', Normal) => Keyword.lex([("iew", View)], [ch], stream)
  | ('f', Normal) => Keyword.lex([("unc", Func)], [ch], stream)
  | ('e', Normal) => Keyword.lex([("lse", Else)], [ch], stream)
  | ('g', Normal) => Keyword.lex([("et", Get)], [ch], stream)

  | ('_' | 'a'..'z' | 'A'..'Z', _) =>
    Identifier(Identifier.lex([ch], stream))

  | ('=', JSXStartTag) => Equals
  | ('>', JSXStartTag | JSXEndTag) => RightChevron
  | ('"', Normal | JSXStartTag) => String(Text.lex([], stream))

  | ('<', _) as res => lex_left_chevron(stream, cursor, snd(res))
  | (_, JSXContent) => JSXTextNode(Misc.lex_jsx_text_node([], stream))

  | ('{', _) => LeftBrace
  | ('}', _) => RightBrace
  | ('/', _) => Misc.lex_forward_slash(stream, cursor)
  | _ => Unexpected(ch)
  };
/* |> (
     tkn => {
       print_tkn(tkn) |> print_endline;
       tkn;
     }
   ); */