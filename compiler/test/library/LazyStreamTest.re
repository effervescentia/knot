open Kore;

let __chars = ['f', 'o', 'o', 'b', 'a', 'r', '\n'];
let __char_stream = __chars |> Stream.of_list |> LazyStream.of_stream;

let suite =
  "Library.LazyStream"
  >::: [
    "of_stream()"
    >: (
      () =>
        [(__char_stream, __chars |> Stream.of_list |> LazyStream.of_stream)]
        |> Assert.(test_many(lazy_stream(Fmt.char)))
    ),
    "of_function()"
    >: (
      () =>
        [
          (__char_stream, __chars |> Fun.generator |> LazyStream.of_function),
          (LazyStream.Nil, [] |> Fun.generator |> LazyStream.of_function),
        ]
        |> Assert.(test_many(lazy_stream(Fmt.char)))
    ),
    "of_string()"
    >: (
      () =>
        [(__char_stream, LazyStream.of_string("foobar\n"))]
        |> Assert.(test_many(lazy_stream(Fmt.char)))
    ),
    "of_channel()"
    >: (
      () => {
        let in_channel = open_in(input_fixture_path);

        Assert.lazy_stream(
          Fmt.char,
          __char_stream,
          LazyStream.of_channel(in_channel),
        );

        close_in(in_channel);
      }
    ),
    "to_list()"
    >: (
      () =>
        [(__chars, "foobar\n" |> LazyStream.of_string |> LazyStream.to_list)]
        |> Assert.(test_many(char_list))
    ),
  ];
