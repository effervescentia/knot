open Kore;

module IO = File.IO;

let __content = "hello world\n";

let suite =
  "File.IO"
  >::: [
    "read_steam()"
    >: (
      () => {
        let (stream, close) = IO.read_stream(fixture_path);

        Assert.string(__content, Util.read_lazy_char_stream(stream));

        close();
      }
    ),
    "clone()"
    >: (
      () => {
        let target = Filename.concat(Util.get_temp_dir(), "other/test.txt");

        IO.clone(fixture_path, target);

        Assert.true_(Sys.file_exists(target));
        Assert.string(__content, Util.read_file_to_string(target));
      }
    ),
  ];
