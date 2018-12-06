type keyword =
  | Main
  | Import
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

type context =
  | Normal
  | JSXStartTag
  | JSXEndTag
  | JSXContent;

let print_tkn =
  fun
  | Space => "space( )"
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

exception UnclosedString;
exception UnclosedBlockComment;

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
let lex_jsx_self_close = _lex_pair('>', JSXSelfClose, Unexpected('/'));

let lex_left_chevron = (stream, cursor, ctx) =>
  switch (_peek_next_non_space(stream, cursor), ctx) {
  | (Some('='), Normal) =>
    _junk_non_space(stream);
    LessThanOrEqual;
  | (Some('/'), Normal | JSXContent) =>
    _junk_non_space(stream);
    JSXOpenEnd;
  | _ => LeftChevron
  };

let rec lex_identifier = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(('_' | 'a'..'z' | 'A'..'Z' | '0'..'9') as ch) =>
    FileStream.junk(stream);
    lex_identifier([ch, ...chs], stream);
  | Some(_)
  | None => Util.chs_to_string(chs)
  };

let rec lex_keyword = (matches, chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    let next_matches =
      List.mapi((i, (s, _)) => (s.[0], i), matches)
      |> List.filter(((char, _)) => char == ch)
      |> List.map(((_, i)) => List.nth(matches, i));

    if (List.length(next_matches) == 1
        && fst(List.hd(next_matches))
        |> String.length == 0) {
      FileStream.junk(stream);
      switch (FileStream.peek(stream)) {
      | Some('_' | 'a'..'z' | 'A'..'Z' | '0'..'9') =>
        FileStream.junk(stream);
        Identifier(lex_identifier([ch, ...chs], stream));
      | _ => Keyword(snd(List.hd(next_matches)))
      };
    } else if (List.length(next_matches) == 0) {
      Identifier(lex_identifier(chs, stream));
    } else {
      FileStream.junk(stream);
      lex_keyword(next_matches, [ch, ...chs], stream);
    };
  | None => Identifier(Util.chs_to_string(chs))
  };

let rec lex_jsx_text_node = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | None
  | Some('<' | '{') => Util.chs_to_string(chs)
  | Some(ch) =>
    FileStream.junk(stream);
    lex_jsx_text_node([ch, ...chs], stream);
  };

let rec lex_number = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '0'..'9' =>
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
  | ('|', Normal) => lex_vertical_bar(stream, cursor)
  | ('&', Normal) => lex_ampersand(stream, cursor)
  | ('-', Normal) => lex_minus(stream, cursor)
  | ('>', Normal) => lex_right_chevron(stream, cursor)
  | ('=', Normal) => lex_assign(stream, cursor)
  | ('0'..'9', Normal) => Number(lex_number([ch], stream))

  | ('m', Normal) =>
    lex_keyword([("ain", Main), ("ut", Mut)], [ch], stream)
  | ('i', Normal) =>
    lex_keyword([("mport", Import), ("f", If)], [ch], stream)
  | ('c', Normal) => lex_keyword([("onst", Const)], [ch], stream)
  | ('l', Normal) => lex_keyword([("et", Let)], [ch], stream)
  | ('s', Normal) => lex_keyword([("tate", State)], [ch], stream)
  | ('v', Normal) => lex_keyword([("iew", View)], [ch], stream)
  | ('f', Normal) => lex_keyword([("unc", Func)], [ch], stream)
  | ('e', Normal) => lex_keyword([("lse", Else)], [ch], stream)
  | ('g', Normal) => lex_keyword([("et", Get)], [ch], stream)

  | ('_' | 'a'..'z' | 'A'..'Z', _) =>
    Identifier(lex_identifier([ch], stream))

  | ('=', JSXStartTag) => Equals
  | ('>', JSXStartTag | JSXEndTag) => RightChevron
  | ('"', Normal | JSXStartTag) => String(lex_string([], stream))

  | ('<', _) as res => lex_left_chevron(stream, cursor, snd(res))
  | (_, JSXContent) => JSXTextNode(lex_jsx_text_node([], stream))

  | ('{', _) => LeftBrace
  | ('}', _) => RightBrace
  | ('/', _) => lex_forward_slash(stream, cursor)
  | _ => Unexpected(ch)
  };
/* |> (
     tkn => {
       print_tkn(tkn) |> print_endline;
       tkn;
     }
   ); */