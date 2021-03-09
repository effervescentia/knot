open Kore;

module CharStream = File.CharStream;

let __content = "hello world\n";

let _read_stream = stream => {
  let chars = ref([]);

  stream |> Stream.iter(Char.value % (char => chars := [char, ...chars^]));

  List.rev(chars^) |> String.of_uchars;
};

let suite =
  "File.CharStream"
  >::: [
    "of_string()"
    >: (
      () =>
        [(__content, CharStream.of_string(__content) |> _read_stream)]
        |> Assert.(test_many(string))
    ),
    "of_channel()"
    >: (
      () => {
        let in_ = open_in(fixture_path);

        [(__content, CharStream.of_channel(in_) |> _read_stream)]
        |> Assert.(test_many(string));

        close_in(in_);
      }
    ),
  ];
