open Core;

module UnicodeFileStream = Knot.UnicodeFileStream;
module TokenStream = KnotLex.TokenStream;
module Parser = KnotParse.Parser;

let buffer_size = 1000;

let load = (prog, file) => {
  let tmp_file = Util.cache_as_tmp(buffer_size, file);
  let in_channel = open_in(tmp_file);

  let result =
    UnicodeFileStream.of_channel(in_channel)
    |> TokenStream.of_file_stream(~filter=TokenStream.filter_comments)
    |> Parser.parse(prog)
    |> (x => x |!> CompilationError(ParsingFailed));

  close_in(in_channel);

  result;
};
