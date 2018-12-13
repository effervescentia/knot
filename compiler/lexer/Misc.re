open Knot.Token;

module FileStream = Knot.FileStream;

exception UnclosedBlockComment;

let lex_minus = Util.lex_pair('>', Lambda, Minus);
let lex_right_chevron = Util.lex_pair('=', GreaterThanOrEqual, RightChevron);
let lex_assign = Util.lex_pair('=', Equals, Assign);
let lex_ampersand = Util.lex_pair('&', LogicalAnd, Ampersand);
let lex_vertical_bar = Util.lex_pair('|', LogicalOr, VerticalBar);
let lex_jsx_self_close = Util.lex_pair('>', JSXSelfClose, Unexpected('/'));

let rec lex_jsx_text_node = (chs, stream) =>
  switch (FileStream.peek(stream)) {
  | None
  | Some('<' | '{') => Util.chs_to_string(chs)
  | Some(ch) =>
    FileStream.junk(stream);
    lex_jsx_text_node([ch, ...chs], stream);
  };

let rec lex_forward_slash = (stream, cursor) =>
  switch (FileStream.peek(stream)) {
  | Some(ch) =>
    switch (ch) {
    | '/' =>
      FileStream.junk(stream);
      lex_block_comment(stream);
    | _ => Util.lex_pair('>', JSXSelfClose, ForwardSlash, stream, cursor)
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
    switch (s |> Util.chs_to_string) {
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