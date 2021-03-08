open Kore;

module Cache = File.Cache;

let __temp_dir = Filename.get_temp_dir_name();

let suite =
  "File.Cache"
  >::: [
    "create()"
    >: (
      () =>
        Cache.create("myProject")
        |> String.starts_with(__temp_dir)
        |> Assert.true_
    ),
    "resolve_path()"
    >: (
      () => {
        let cache = "foo/bar/cache";

        [
          (
            Print.fmt("%s/my/path", cache),
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
        let path = Util.temp_file_name("test", "txt");
        let content = "hello world";

        Util.write_to_file(Filename.concat(__temp_dir, path), content);

        let open_file = Cache.open_file(path, __temp_dir);

        [(content, Util.read_channel_to_string(open_file))]
        |> Assert.(test_many(string));

        close_in(open_file);
      }
    ),
    "destroy()"
    >: (
      () => {
        let content = "hello world";
        let parent_dir = Filename.concat(__temp_dir, "foo");
        let path =
          Filename.concat(parent_dir, Util.temp_file_name("test", "txt"));

        FileUtil.mkdir(parent_dir);
        Util.write_to_file(path, content);
        Cache.destroy(parent_dir);

        [
          (false, Sys.file_exists(path)),
          (false, Sys.file_exists(parent_dir)),
          (true, Sys.file_exists(__temp_dir)),
        ]
        |> Assert.(test_many(bool));
      }
    ),
  ];
