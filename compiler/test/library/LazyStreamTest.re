open Kore;

let __string = "foobar\n";
let __chars = ['f', 'o', 'o', 'b', 'a', 'r', '\n'];
let __char_stream = __chars |> Stream.of_list |> LazyStream.of_stream;

let suite =
  "Library.LazyStream"
  >::: [
    "of_stream()"
    >: (
      () =>
        Assert.lazy_stream(
          Fmt.char,
          __char_stream,
          __chars |> Stream.of_list |> LazyStream.of_stream,
        )
    ),
    "of_function() - empty"
    >: (
      () =>
        Assert.lazy_stream(
          Fmt.char,
          LazyStream.Nil,
          [] |> Fun.generator |> LazyStream.of_function,
        )
    ),
    "of_function() - not empty"
    >: (
      () =>
        Assert.lazy_stream(
          Fmt.char,
          __char_stream,
          __chars |> Fun.generator |> LazyStream.of_function,
        )
    ),
    "of_string()"
    >: (
      () =>
        Assert.lazy_stream(
          Fmt.char,
          __char_stream,
          LazyStream.of_string(__string),
        )
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
        Assert.char_list(
          __chars,
          __string |> LazyStream.of_string |> LazyStream.to_list,
        )
    ),
    "pp()"
    >: (
      () =>
        Assert.string(
          __string,
          __string |> LazyStream.of_string |> ~@LazyStream.pp(Fmt.char),
        )
    ),
  ];
