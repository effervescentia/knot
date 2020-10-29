let _buffer_size = 1000;

let temporary_file = (~buffer_size=_buffer_size, file) =>
  Filename.open_temp_file(
    ~mode=[Open_wronly, Open_binary],
    "knot-cached-module-",
    "-maybe-put-a-build-hash-here",
  )
  |> (
    ((tmp_file, tmp_channel)) => {
      let in_channel = open_in_bin(file);

      Util.pipe(buffer_size, in_channel, tmp_channel);

      close_out(tmp_channel);
      close_in(in_channel);

      tmp_file;
    }
  );
