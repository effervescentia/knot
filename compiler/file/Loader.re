open Globals;

module Lexer = KnotLex.Lexer;
module Cache = KnotCache.Cache;
module FileStream = KnotUnicode.FileStream;

let _buffer_size = 1000;

let load = file => {
  let tmp_file = Cache.temporary_file(file);
  let in_channel = open_in(tmp_file);

  let result =
    FileStream.of_channel(in_channel)
    |> TokenStream.of_file_stream(Lexer.next_token);

  (result, () => close_in(in_channel));
};
