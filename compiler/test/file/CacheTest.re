open Kore;

module Cache = File.Cache;

let __content = "hello world\n";

let suite =
  "File.Cache"
  >::: [
    "create()"
    >: (
      () =>
        Cache.create("myProject")
        |> String.starts_with(Filename.get_temp_dir_name())
        |> Assert.true_
    ),
    "resolve_path()"
    >: (
      () => {
        let cache = "foo/bar/cache";

        [
          (
            Fmt.str("%s/my/path", cache),
            cache |> Cache.resolve_path("my/path"),
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "file_exists()"
    >: (
      () =>
        [
          (true, "." |> Cache.file_exists(fixture_path)),
          (false, "." |> Cache.file_exists("path/to/nonexistent/file.oops")),
        ]
        |> Assert.(test_many(bool))
    ),
    "open_file()"
    >: (
      () => {
        let open_file = Cache.open_file("read_me.txt", fixture_dir);

        [(__content, Util.read_channel_to_string(open_file))]
        |> Assert.(test_many(string));

        close_in(open_file);
      }
    ),
    "destroy()"
    >: (
      () => {
        let temp_dir = Util.get_temp_dir();
        let parent_dir =
          Filename.concat(temp_dir, Fmt.str("%f", Sys.time()));
        let path =
          Filename.concat(parent_dir, Util.temp_file_name("test", "txt"));

        FileUtil.mkdir(~parent=true, parent_dir);
        Util.write_to_file(path, __content);
        Cache.destroy(parent_dir);

        [
          (false, Sys.file_exists(path)),
          (false, Sys.file_exists(parent_dir)),
          (true, Sys.file_exists(temp_dir)),
        ]
        |> Assert.(test_many(bool));
      }
    ),
  ];
