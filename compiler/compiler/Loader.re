open Core;

module UnicodeFileStream = Knot.UnicodeFileStream;
module TokenStream = KnotLex.TokenStream;
module Parser = KnotParse.Parser;

let buffer_size = 1000;

let handle_error = (file, pretty_path) =>
  fun
  | CompilationError(InvalidCharacter(ch, cursor)) => {
      print_err(
        pretty_path,
        Printf.sprintf(
          "encountered unexpected character '%s' at [%d, %d]",
          Knot.Util.print_uchar(ch),
          fst(cursor),
          snd(cursor),
        ),
      );
      Knot.CodeFrame.print(file, cursor) |> print_endline;
    }
  /* unexpected exception was caught */
  | _ => ();

let load = (prog, file, pretty_path) => {
  let tmp_file = Util.cache_as_tmp(buffer_size, file);
  let in_channel = open_in(tmp_file);

  let result =
    try (
      UnicodeFileStream.of_channel(in_channel)
      |> TokenStream.of_file_stream(~filter=TokenStream.filter_comments)
      |> Parser.parse(prog)
      |> (
        fun
        | Some(_) as res => res
        | None => throw(ParsingFailed)
      )
    ) {
    | err =>
      handle_error(tmp_file, pretty_path, err);

      throw(LexingFailed);
    };

  close_in(in_channel);

  result;
};
