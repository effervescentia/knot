open Core;

exception ParsingFailed;

let buffer_size = 1000;

let cache_as_tmp = file =>
  Filename.open_temp_file(
    ~mode=[Open_wronly, Open_binary],
    "knot-cached-module-",
    "-maybe-put-a-build-hash-here",
  )
  |> (
    ((tmp_file, tmp_channel)) => {
      let in_channel = open_in_bin(file);
      let channel_length = in_channel_length(in_channel);

      let rec write = () => {
        let remaining = channel_length - pos_in(in_channel);
        let buf_length = min(buffer_size, remaining);
        let buf = Bytes.create(buf_length);

        let read = input(in_channel, buf, 0, buf_length);
        output(tmp_channel, buf, 0, read);

        if (remaining !== 0) {
          write();
        };
      };

      write();

      flush(tmp_channel);

      close_out(tmp_channel);
      close_in(in_channel);

      open_in(tmp_file);
    }
  );

let load = file => {
  Printf.printf("loading %s\n", file);

  let in_channel = cache_as_tmp(file);

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
