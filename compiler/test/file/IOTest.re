open Kore;

module IO = File.IO;

let __temp_dir = Filename.get_temp_dir_name();

let unit_tests =
  "File.IO"
  >::: [
    "read_steam()"
    >: (
      () => {
        let content = "hello world";
        let path = Filename.concat(__temp_dir, "test_read_steam.txt");

        Util.write_to_file(path, content);

        let (stream, close) = IO.read_stream(path);

        [(content, Util.read_lazy_char_stream(stream))]
        |> Assert.(test_many(string));

        close();
      }
    ),
    "clone()"
    >: (
      () => {
        let content = "hello world";
        let source = Filename.concat(__temp_dir, "test_clone.txt");
        let target = Filename.concat(__temp_dir, "other/test_clone.txt");

        Util.write_to_file(source, content);

        IO.clone(source, target);

        Sys.file_exists(target) |> Assert.true_;
        Util.read_file_to_string(target) |> Assert.string(content);
      }
    ),
  ];
