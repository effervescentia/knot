open Kore;

exception ParsingFailed;

let buffer_size = 1000;

let load = file => {
  let in_channel = Util.cache_as_tmp(buffer_size, file);

  FileStream.of_channel(in_channel)
  |> TokenStream.of_file_stream(~filter=TokenStream.filter_comments)
  |> Parser.parse(Parser.prog)
  |> (
    fun
    | Some(_) as res => {
        close_in(in_channel);
        res;
      }
    | None => raise(ParsingFailed)
  );
};
