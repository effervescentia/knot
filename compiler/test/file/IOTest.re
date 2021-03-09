open Kore;

module IO = File.IO;

let __content = "hello world";

let suite =
  "File.IO"
  >::: [
    "read_steam()"
    >: (
      () => {
        let (stream, close) = IO.read_stream(fixture_path);

        [(__content, Util.read_lazy_char_stream(stream))]
        |> Assert.(test_many(string));

        close();
      }
    ),
    "clone()"
    >: (
      () => {
        let target = Filename.concat(Util.get_temp_dir(), "other/test.txt");

        IO.clone(fixture_path, target);

        Sys.file_exists(target) |> Assert.true_;
        Util.read_file_to_string(target) |> Assert.string(__content);
      }
    ),
  ];
