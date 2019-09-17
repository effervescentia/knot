open Core;

module UnicodeFileStream = Knot.UnicodeFileStream;
module TokenStream = KnotLex.TokenStream;
module Parser = KnotParse.Parser;

let buffer_size = 1000;

let load = (prog, file) => {
  let in_channel = Util.cache_as_tmp(buffer_size, file);

  try (
    UnicodeFileStream.of_channel(in_channel)
    |> TokenStream.of_file_stream(~filter=TokenStream.filter_comments)
    |> Parser.parse(prog)
    |> (
      fun
      | Some(_) as res => {
          close_in(in_channel);
          res;
        }
      | None => raise(ParsingFailed)
    )
  ) {
  | InvalidCharacter(ch, cursor) =>
    Printf.sprintf(
      "failed to parse file '%s'\nencountered unexpected character '%s' at [%d, %d]",
      file,
      print_uchar(ch),
      fst(cursor),
      snd(cursor),
    )
    |> print_endline;

    raise(LexingFailed);
  };
};
