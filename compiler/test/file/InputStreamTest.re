open Kore;

module InputStream = File.InputStream;

let __content = "hello world\n";

let _read_stream = stream => {
  let chars = ref([]);

  stream
  |> Stream.iter(Input.get_value % (char => chars := [char, ...chars^]));

  List.rev(chars^) |> String.of_uchars;
};

let suite =
  "File.InputStream"
  >::: [
    "of_string()"
    >: (
      () =>
        Assert.string(
          __content,
          __content |> InputStream.of_string |> _read_stream,
        )
    ),
    "of_channel()"
    >: (
      () => {
        let in_ = open_in(fixture_path);

        Assert.string(
          __content,
          in_ |> InputStream.of_channel |> _read_stream,
        );

        close_in(in_);
      }
    ),
    "pp()"
    >: (
      () => {
        let in_ = open_in(fixture_path);

        Assert.string(
          __content,
          in_ |> InputStream.of_channel |> ~@InputStream.pp,
        );

        close_in(in_);
      }
    ),
  ];
