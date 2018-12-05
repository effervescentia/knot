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
  | Number(int)
  | String(string)
  | LineComment(string)
  | BlockComment(string)
  | Unexpected(char);

let (%) = (f, g, x) => f(x) |> g;

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
  | Number(n) => Printf.sprintf("number(%d)", n)
  | String(s) => Printf.sprintf("string(%s)", s)
  | LineComment(s) => Printf.sprintf("line_comment(%s)", s)
  | BlockComment(s) => Printf.sprintf("block_comment(%s)", s)
  | Unexpected(c) => Printf.sprintf("UNEXPECTED(%c)", c);

exception UnclosedString;
exception UnclosedBlockComment;

let _is_number = ch =>
  int_of_char(ch) >= int_of_char('0')
  && int_of_char(ch) <= int_of_char('9');
let rec _next_non_space = stream =>
  switch (FileStream.peek(stream)) {
  | Some(' ' | '\t' | '\n') =>
    FileStream.junk(stream);
    _next_non_space(stream);
  | Some(tkn) => Some(tkn)
  | None => None
  };
let _peek_next_non_space = (stream, cursor) => {
  let char = _next_non_space(stream);
  FileStream.reposition(stream, cursor);
  char;
};
let _junk_non_space = stream => {
  _next_non_space(stream) |> ignore;
  FileStream.junk(stream);
};
let _lex_pair = (ch, match, no_match, stream, cursor) =>
  switch (_peek_next_non_space(stream, cursor)) {
  | Some(c) when c == ch =>
    _junk_non_space(stream);
    match;
  | _ => no_match
  };

let lex_minus = _lex_pair('>', Lambda, Minus);
let lex_right_chevron = _lex_pair('=', GreaterThanOrEqual, RightChevron);
let lex_assign = _lex_pair('=', Equals, Assign);
let lex_ampersand = _lex_pair('&', LogicalAnd, Ampersand);
let lex_vertical_bar = _lex_pair('|', LogicalOr, VerticalBar);

let lex_left_chevron = (stream, cursor) =>
  switch (_peek_next_non_space(stream, cursor)) {
  | Some('=') =>
    _junk_non_space(stream);
    LessThanOrEqual;
  | Some('/') =>
    _junk_non_space(stream);
    JSXOpenEnd;
  | _ => LeftChevron
  };

let rec lex_number = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | _ when _is_number(ch) =>
      FileStream.junk(stream);
      lex_number([ch, ...chs], stream);
    | _ => Util.chs_to_number(chs)
    }
  | None => Util.chs_to_number(chs)
  };

let rec lex_string = (chs, stream) =>
  switch (FileStream.next(stream)) {
  | Some((ch, _)) =>
    switch (ch) {
    | '"' =>
      List.rev(chs)
      |> List.fold_left((acc, c) => acc ++ String.make(1, c), "")
    | _ => lex_string([ch, ...chs], stream)
    }
  | None => raise(UnclosedString)
  };

let rec lex_forward_slash = (stream, cursor) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '/' =>
      FileStream.junk(stream);
      lex_block_comment(stream);
    | _ => _lex_pair('>', JSXSelfClose, ForwardSlash, stream, cursor)
    }
  | None => ForwardSlash
  }
and lex_block_comment = stream =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '/' =>
      FileStream.junk(stream);
      BlockComment(_read_until_end_block([], stream));
    | _ => LineComment(_read_until_newline([], stream))
    }
  | None => LineComment(_read_until_newline([], stream))
  }
and _read_until_newline = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '\n' => Util.chs_to_string(chs)
    | _ =>
      FileStream.junk(stream);
      _read_until_newline([ch, ...chs], stream);
    }
  | None => Util.chs_to_string(chs)
  }
and _read_until_end_block = (chs, stream) =>
  switch (FileStream.npeek(3, stream)) {
  | Some(s) =>
    switch (s) {
    | "///" =>
      FileStream.njunk(3, stream);
      Util.chs_to_string(chs);
    | _ =>
      switch (FileStream.next(stream)) {
      | Some((ch, _)) => _read_until_end_block([ch, ...chs], stream)
      | None => raise(UnclosedBlockComment)
      }
    }
  | None => raise(UnclosedBlockComment)
  };

let lex = ((ch, cursor), stream) =>
  switch (ch) {
  | ' ' => Space
  | '\t' => Tab
  | '\n' => Newline
  | '.' => Period
  | ',' => Comma
  | ':' => Colon
  | ';' => Semicolon
  | '~' => Tilde
  | '+' => Plus
  | '*' => Asterisk
  | '(' => LeftParenthese
  | ')' => RightParenthese
  | '{' => LeftBrace
  | '}' => RightBrace
  | '[' => LeftBracket
  | ']' => RightBracket
  | '|' => lex_vertical_bar(stream, cursor)
  | '&' => lex_ampersand(stream, cursor)
  | '-' => lex_minus(stream, cursor)
  | '<' => lex_left_chevron(stream, cursor)
  | '>' => lex_right_chevron(stream, cursor)
  | '=' => lex_assign(stream, cursor)
  | '/' => lex_forward_slash(stream, cursor)
  | '"' => String(lex_string([], stream))
  | _ when _is_number(ch) => Number(lex_number([ch], stream))
  | _ => Unexpected(ch)
  };
/* |> (
     tkn => {
       print_tkn(tkn) |> print_endline;
       tkn;
     }
   ); */